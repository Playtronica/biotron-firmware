#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/time.h"
#include "raw_plant.h"
#include "global.h"
#include "params.h"

extern uint32_t filter_freq(double val, double k);

Settings_t settings;
bool isMutedByButton = false;
bool TestMode = false;
bool isTestModeGreen = false;
bool LOGGER_FLAG = false;
uint32_t button_states[3] = {0};

static alarm_callback_t scheduled_callback = NULL;
static void *scheduled_user_data = NULL;
static alarm_id_t scheduled_id = 11;
static int64_t scheduled_delay = 0;
static size_t schedule_count = 0;
static size_t cancel_count = 0;
static int64_t played_intervals[8];
static size_t play_count = 0;
static size_t critical_entries = 0;
static size_t critical_exits = 0;
static size_t stop_midi_count = 0;
static bool plant_ready = false;
static uint32_t plant_frequency = 0;
static uint8_t calibration_messages[12][6];
static size_t calibration_message_count = 0;
static uint8_t calibration_notes[20][4];
static size_t calibration_note_count = 0;

extern void start_music_alarm(void);
extern void stop_music_alarm(void);

alarm_id_t add_alarm_in_us(int64_t delay_us, alarm_callback_t callback,
                           void *user_data, bool fire_if_past) {
    (void)fire_if_past;
    scheduled_delay = delay_us;
    scheduled_callback = callback;
    scheduled_user_data = user_data;
    ++schedule_count;
    return scheduled_id;
}

bool cancel_alarm(alarm_id_t alarm_id) {
    assert(alarm_id == scheduled_id);
    ++cancel_count;
    return true;
}

void play_music(int64_t interval_us) {
    assert(play_count < sizeof played_intervals / sizeof played_intervals[0]);
    played_intervals[play_count++] = interval_us;
}

void reset_plant_note_off(void) {}
void stop_midi(void) { ++stop_midi_count; }
bool plant_is_ready(void) { return plant_ready; }
uint32_t get_real_freq(void) { return plant_frequency; }
uint16_t adc_read(void) { return 0; }
void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    assert(calibration_note_count < 20);
    calibration_notes[calibration_note_count][0] = 1;
    calibration_notes[calibration_note_count][1] = channel;
    calibration_notes[calibration_note_count][2] = note;
    calibration_notes[calibration_note_count][3] = velocity;
    ++calibration_note_count;
}
void note_off(uint8_t channel, uint8_t note) {
    assert(calibration_note_count < 20);
    calibration_notes[calibration_note_count][0] = 0;
    calibration_notes[calibration_note_count][1] = channel;
    calibration_notes[calibration_note_count][2] = note;
    calibration_notes[calibration_note_count][3] = 0;
    ++calibration_note_count;
}
void plsdk_printf(const char *format, ...) { (void)format; }
bool print_pure(uint8_t cable, const uint8_t data[], uint8_t len) {
    assert(cable <= 1);
    assert(len == 6);
    assert(calibration_message_count < 12);
    for (size_t i = 0; i < len; ++i) {
        calibration_messages[calibration_message_count][i] = data[i];
    }
    ++calibration_message_count;
    return true;
}
uint32_t save_and_disable_interrupts(void) {
    ++critical_entries;
    return 99;
}
void restore_interrupts(uint32_t state) {
    assert(state == 99);
    ++critical_exits;
}

uint64_t time_us_64(void) { return 0; }
uint32_t time_us_32(void) { return 0; }

static int64_t fire_alarm(void) {
    assert(scheduled_callback != NULL);
    return scheduled_callback(scheduled_id, scheduled_user_data);
}

int main(void) {
    settings = (Settings_t){0};
    settings.BPM = 1000;
    settings.swing_first_note_percent = 60;
    status = Active;
    active_status = Active;

    start_music_alarm();
    assert(schedule_count == 1);
    assert(scheduled_delay == 1000);
    assert(play_count == 0);

    assert(fire_alarm() == 1400);
    assert(play_count == 0);
    service_music_alarm();
    assert(play_count == 1);
    assert(played_intervals[0] == 1400);
    assert(schedule_count == 1);

    assert(fire_alarm() == 600);
    service_music_alarm();
    assert(play_count == 2);
    assert(played_intervals[1] == 600);
    assert(schedule_count == 1);

    assert(fire_alarm() == 1400);
    status = Sleep;
    service_music_alarm();
    assert(play_count == 2);
    assert(schedule_count == 1);

    status = Active;
    start_music_alarm();
    assert(schedule_count == 2);
    stop_music_alarm();
    assert(cancel_count == 2);
    assert(fire_alarm() == 0);
    service_music_alarm();
    assert(play_count == 2);
    assert(critical_entries == critical_exits);
    assert(critical_entries >= 2);

    // A partially completed Sleep window must not leak into a requested
    // recalibration. The command also stops notes/alarm work and clears only
    // transient signal state, while preserving the selected post-calibration
    // mode and all Settings_t values.
    plant_ready = true;
    plant_frequency = MIN_FREQ + 1;
    status = Sleep;
    active_status = BPMClockActive;
    for (size_t i = 0; i < STABILIZATION_COUNTER / 2; ++i) status_loop();
    assert(status == Sleep);
    last_freq = 111;
    average_freq = 222;
    average_delta_freq = 333;
    (void)filter_freq(444, 0);
    const Settings_t settings_before_calibration = settings;

    start_plant_calibration(42);
    assert(status == Sleep);
    assert(active_status == BPMClockActive);
    assert(last_freq == 0);
    assert(average_freq == 0);
    assert(average_delta_freq == 0);
    assert(memcmp(&settings, &settings_before_calibration, sizeof settings) == 0);
    assert(stop_midi_count == 1);
    assert(filter_freq(555, 0.3) == 555);
    assert(calibration_message_count == 2);
    for (size_t cable = 0; cable < 2; ++cable) {
        assert(calibration_messages[cable][0] == SYS_EX_START);
        assert(calibration_messages[cable][1] == PLAYTRONICA_SYS_KEY);
        assert(calibration_messages[cable][2] == BIOTRON_RECALIBRATE_COMMAND);
        assert(calibration_messages[cable][3] == 42);
        assert(calibration_messages[cable][4] == BIOTRON_RECALIBRATE_WAITING);
        assert(calibration_messages[cable][5] == SYS_EX_END);
    }

    for (size_t i = 0; i < STABILIZATION_COUNTER - 1; ++i) status_loop();
    assert(status == Sleep);
    status_loop();
    assert(status == Stabilization);
    assert(calibration_message_count == 4);
    assert(calibration_messages[2][4] == BIOTRON_RECALIBRATE_MEASURING);
    assert(calibration_messages[3][4] == BIOTRON_RECALIBRATE_MEASURING);

    for (size_t i = 0; i <= AVERAGE_COUNTER; ++i) status_loop();
    assert(status == BPMClockActive);
    assert(calibration_message_count == 6);
    assert(calibration_messages[4][4] == BIOTRON_RECALIBRATE_READY);
    assert(calibration_messages[5][4] == BIOTRON_RECALIBRATE_READY);

    const uint8_t expected_notes[] = {64, 65, 67, 72, 71, 67, 62, 60};
    const uint8_t expected_velocities[] = {22, 24, 26, 28, 26, 24, 22, 18};
    assert(calibration_note_count == sizeof expected_notes * 2);
    for (size_t i = 0; i < sizeof expected_notes; ++i) {
        const size_t on = i * 2;
        const size_t off = on + 1;
        assert(calibration_notes[on][0] == 1);
        assert(calibration_notes[on][1] == settings.plant_channel);
        assert(calibration_notes[on][2] == expected_notes[i]);
        assert(calibration_notes[on][3] == expected_velocities[i]);
        assert(calibration_notes[off][0] == 0);
        assert(calibration_notes[off][1] == settings.plant_channel);
        assert(calibration_notes[off][2] == expected_notes[i]);
    }

    /* Losing the sensor mid-phrase must emit the matching Note Off. */
    start_plant_calibration(43);
    for (size_t i = 0; i < STABILIZATION_COUNTER; ++i) status_loop();
    assert(status == Stabilization);
    for (size_t i = 0; i < 3; ++i) status_loop();
    assert(calibration_notes[calibration_note_count - 1][0] == 1);
    assert(calibration_notes[calibration_note_count - 1][2] == 64);
    plant_frequency = 0;
    status_loop();
    assert(status == Sleep);
    assert(calibration_notes[calibration_note_count - 1][0] == 0);
    assert(calibration_notes[calibration_note_count - 1][2] == 64);

    puts("music_scheduler: IRQ deferral, swing and calibration cue passed");
    return 0;
}
