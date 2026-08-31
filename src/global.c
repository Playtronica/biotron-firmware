#include <stdlib.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/adc.h>
#include <hardware/sync.h>

#include "PLSDK/music.h"
#include "PLSDK/commands.h"
#include "PLSDK.h"
#include "PLSDK/cap_buttons.h"

#include "params.h"
#include "music.h"
#include "raw_plant.h"
#include "leds.h"

#include "global.h"

enum Status status = Sleep;
enum Status active_status = Active;
uint32_t last_freq = 0;
uint32_t average_freq = 0;
uint32_t average_delta_freq = 0;
static uint8_t status_counter = 0;
static bool requested_calibration_active = false;
static uint8_t requested_calibration_nonce = 0;

typedef struct {
    uint8_t tick;
    uint8_t note;
    uint8_t velocity;
    uint8_t duration_ticks;
} calibration_cue_event_t;

/*
 * 100 ms ticks: a quiet cadence with a clear opening, high point and final
 * resting note. These remain normal MIDI notes for DAWs and external
 * instruments; the firmware only replaces the old rapid 91/92 alternation
 * used during Stabilization.
 */
static const calibration_cue_event_t CALIBRATION_CUE[] = {
        {3, 64, 22, 3}, {8, 65, 24, 3}, {13, 67, 26, 4},
        {18, 72, 28, 4}, {24, 71, 26, 3}, {29, 67, 24, 3},
        {34, 62, 22, 4}, {40, 60, 18, 9},
};
static uint8_t calibration_cue_index = 0;
static uint8_t calibration_cue_active_note = 0xff;
static uint8_t calibration_cue_note_off_tick = 0;

static void stop_calibration_cue(void) {
    if (calibration_cue_active_note != 0xff) {
        note_off(settings.plant_channel, calibration_cue_active_note);
        calibration_cue_active_note = 0xff;
    }
}

static void reset_calibration_cue(void) {
    stop_calibration_cue();
    calibration_cue_index = 0;
}

static void service_calibration_cue(uint8_t tick) {
    if (calibration_cue_active_note != 0xff &&
        tick >= calibration_cue_note_off_tick) stop_calibration_cue();
    if (calibration_cue_index >=
        sizeof CALIBRATION_CUE / sizeof CALIBRATION_CUE[0]) return;
    const calibration_cue_event_t *event =
            &CALIBRATION_CUE[calibration_cue_index];
    if (event->tick != tick) return;
    note_on(settings.plant_channel, event->note, event->velocity);
    calibration_cue_active_note = event->note;
    calibration_cue_note_off_tick = tick + event->duration_ticks;
    calibration_cue_index++;
}

static void report_requested_calibration(uint8_t state) {
    if (!requested_calibration_active) return;
    const uint8_t response[] = {
            SYS_EX_START, PLAYTRONICA_SYS_KEY,
            BIOTRON_RECALIBRATE_COMMAND, requested_calibration_nonce,
            state, SYS_EX_END,
    };
    // The browser may have either logical cable selected. Report on both
    // without changing their established music/service roles.
    print_pure(0, response, sizeof response);
    print_pure(1, response, sizeof response);
    if (state == BIOTRON_RECALIBRATE_READY) {
        requested_calibration_active = false;
    }
}

uint32_t filter_freq(double val, double k) {
    static uint32_t filter_val = 0;
    if (filter_val == 0) {
        filter_val = val;
        return filter_val;
    }
    if (val == 0 && k == 0) {
        filter_val = 0;
    }
    filter_val += (int32_t)((val - filter_val) * (1 - k));
    if (!settings.random_note) filter_val = (filter_val << 1) >> 1;
    return filter_val;
}


static volatile bool music_alarm_due = false;
static volatile bool next_is_swing_note = false;
static volatile int64_t due_music_interval_us = 1;
static volatile int64_t short_music_interval_us = 1;
static volatile int64_t long_music_interval_us = 1;
alarm_id_t play_music_alarm_id = -1;


int64_t play_music_alarm(alarm_id_t id, void *user_data) {
    (void)user_data;
    if (play_music_alarm_id != id) return 0;
    // Alarm callbacks run in IRQ context. Only publish work here; all settings
    // reads, music calculations and TinyUSB writes happen in the main loop.
    const int64_t interval_us = next_is_swing_note ?
            short_music_interval_us : long_music_interval_us;
    next_is_swing_note = !next_is_swing_note;
    due_music_interval_us = interval_us;
    music_alarm_due = true;
    return interval_us;
}

void refresh_music_alarm_timing(void) {
    const int64_t bpm_us = settings.BPM > 0 ? settings.BPM : 1;
    const int swing_percent = settings.swing_first_note_percent < 1 ? 1 :
            settings.swing_first_note_percent > 100 ? 100 :
            settings.swing_first_note_percent;
    const int64_t short_interval =
            (bpm_us * swing_percent) / 100;
    const int64_t long_interval =
            (bpm_us * (200 - swing_percent)) / 100;
    const uint32_t irq_state = save_and_disable_interrupts();
    short_music_interval_us = short_interval > 0 ? short_interval : 1;
    long_music_interval_us = long_interval > 0 ? long_interval : 1;
    restore_interrupts(irq_state);
}

void service_music_alarm(void) {
    const uint32_t irq_state = save_and_disable_interrupts();
    const bool due = music_alarm_due;
    const int64_t to_the_next_beat_us = due_music_interval_us;
    music_alarm_due = false;
    restore_interrupts(irq_state);
    if (!due) return;
    if (status != Active) return;
    play_music(to_the_next_beat_us > 0 ? to_the_next_beat_us : 1);
}

void stop_music_alarm(void);

void start_music_alarm() {
    stop_music_alarm();
    refresh_music_alarm_timing();
    play_music_alarm_id = add_alarm_in_us(settings.BPM > 0 ? settings.BPM : 1,
                                          play_music_alarm, NULL, false);
}

void stop_music_alarm() {
    const uint32_t irq_state = save_and_disable_interrupts();
    const alarm_id_t alarm_id = play_music_alarm_id;
    play_music_alarm_id = -1;
    music_alarm_due = false;
    restore_interrupts(irq_state);
    if (alarm_id >= 0) cancel_alarm(alarm_id);
}


void bpm_clock_control(bool enabled) {
    if (enabled && status == Active) {
        stop_music_alarm();
        status = BPMClockActive;

    }
    else if (!enabled && status == BPMClockActive) {
        start_music_alarm();
        status = Active;
    }
    active_status = enabled ? BPMClockActive : Active;
    reset_plant_note_off();
}

void stop_bpm() {
    if (status == Active) {
        reset_plant_note_off();
        stop_music_alarm();
    }
}


void reset_bpm() {
    if (status == Active) {
        stop_bpm();
        start_music_alarm();
    }
}



void load_settings() {
    static bool is_stopped = false;
    if (is_stopped) {
        if (status == Active) {
            start_music_alarm();
        }

    } else {
        if (status == Active) {
            stop_music_alarm();
            reset_plant_note_off();
        }
    }
    is_stopped = !is_stopped;
}

void start_plant_calibration(uint8_t request_nonce) {
    // Calibration is a runtime reset only: preserve user settings and the
    // selected internal/Clock mode, but stop every note and discard the old
    // sensor baseline before returning to the normal Sleep -> Stabilization
    // state machine.
    stop_music_alarm();
    stop_midi();
    status_counter = 0;
    last_freq = 0;
    average_freq = 0;
    average_delta_freq = 0;
    filter_freq(0, 0);
    reset_calibration_cue();
    status = Sleep;
    requested_calibration_nonce = request_nonce & 0x7f;
    requested_calibration_active = true;
    report_requested_calibration(BIOTRON_RECALIBRATE_WAITING);
}


void status_loop() {
    if (!plant_is_ready()) {
        return;
    }

    uint32_t raw_freq = get_real_freq();

    switch (status) {
        case Sleep:
            if (TestMode) {
                plsdk_printf("{\"generator_freq\": %d, \"photoresistor_adc\": %d,"
                       "\"buttons_state\": {"
                       "\"finger_button\": %d, \"button_bottom\": %d, \"button_top\": %d}}\n",
                       raw_freq, adc_read(), button_states[0],  button_states[1],  button_states[2]);

                break;
            }

            if (raw_freq > MIN_FREQ) {
                status_counter++;
            } else {
                status_counter = 0;
            }

            if (status_counter >= STABILIZATION_COUNTER) {
                status = Stabilization;
                status_counter = 0;
                reset_calibration_cue();
                report_requested_calibration(BIOTRON_RECALIBRATE_MEASURING);
                plsdk_printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
        case Stabilization: {
            if (raw_freq > MIN_FREQ && !TestMode) {
                status_counter++;
                service_calibration_cue(status_counter);
                uint32_t b = filter_freq(raw_freq, 0.3);
                if (average_freq == 0) {
                    last_freq = raw_freq;
                }
                else {
                    average_delta_freq += abs((int)last_freq - (int)raw_freq);
                    raw_freq = last_freq;
                }
                average_freq += b;
            } else {
                status_counter = 0;
                average_delta_freq = 0;
                average_freq = 0;
                last_freq = 0;
                reset_calibration_cue();
                status = Sleep;
                report_requested_calibration(BIOTRON_RECALIBRATE_WAITING);
                plsdk_printf("[+] Change status: Stab -> Sleep\n");
                break;
            }

            if (status_counter > AVERAGE_COUNTER) {
                average_freq /= status_counter;
                average_delta_freq /= status_counter;
                status_counter = 0;
                stop_calibration_cue();
                if (active_status == Active) {
                    start_music_alarm();
                }
                status = active_status;
                report_requested_calibration(BIOTRON_RECALIBRATE_READY);
                plsdk_printf("[+] Change status: Stab -> Active\n");
            }

            break;
        }
        case Active:
        case BPMClockActive:
            if (raw_freq < MIN_FREQ) {
                status_counter++;
            } else {
                status_counter = 0;
            }

            if (settings.filterPercent != 0) {
                last_freq = filter_freq(raw_freq,  settings.filterPercent);
            }
            else {
                last_freq = raw_freq;
            }


            if (status_counter > SLEEP_COUNTER || TestMode) {
                status_counter = 0;
                last_freq = 0;
                average_freq = 0;
                average_delta_freq = 0;
                if (status == Active) {
                    stop_music_alarm();
                }
                status = Sleep;
                filter_freq(0, 0);
                stop_midi();
                plsdk_printf("[+] Change status: Active -> Sleep\n");
                return;
            }

            break;
    }
}
