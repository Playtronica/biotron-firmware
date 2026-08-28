#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "pico/time.h"
#include "global.h"
#include "midi_note_lifecycle.h"
#include "music.h"
#include "params.h"
#include "PLSDK/music.h"

typedef struct {
    uint8_t kind;
    uint8_t channel;
    uint8_t note;
} midi_log_entry_t;

enum {
    LOG_NOTE_ON = 1,
    LOG_NOTE_OFF = 2,
    LOG_ALL_NOTES_OFF = 3,
    LOG_PITCH = 4,
};

Settings_t settings;
bool isMutedByButton = false;
bool TestMode = false;
bool isTestModeGreen = false;
bool LOGGER_FLAG = false;
enum Status status = Active;
enum Status active_status = Active;
uint32_t last_freq = 100;
uint32_t average_freq = 100;
uint32_t average_delta_freq = 1;

static midi_log_entry_t midi_log[64];
static size_t midi_log_len = 0;
static uint8_t calculated_note = 64;
static alarm_callback_t scheduled_callback = NULL;
static void *scheduled_user_data = NULL;
static alarm_id_t scheduled_id = 7;
static bool fail_next_alarm = false;

static void log_midi(uint8_t kind, uint8_t channel, uint8_t note) {
    assert(midi_log_len < sizeof midi_log / sizeof midi_log[0]);
    midi_log[midi_log_len++] = (midi_log_entry_t){kind, channel, note};
}

static size_t count_event(uint8_t kind, uint8_t channel, uint8_t note) {
    size_t count = 0;
    for (size_t i = 0; i < midi_log_len; ++i) {
        if (midi_log[i].kind == kind && midi_log[i].channel == channel &&
            midi_log[i].note == note) {
            ++count;
        }
    }
    return count;
}

uint64_t time_us_64(void) { return 1000; }
uint32_t time_us_32(void) { return 1000; }
uint16_t adc_read(void) { return 1600; }
void light_note_observer(void) {}
void plsdk_printf(const char *format, ...) { (void)format; }
void print_pure(uint8_t cable, const uint8_t data[], uint8_t len) {
    (void)cable;
    (void)data;
    (void)len;
}

alarm_id_t add_alarm_in_us(int64_t delay_us, alarm_callback_t callback,
                           void *user_data, bool fire_if_past) {
    (void)delay_us;
    (void)fire_if_past;
    scheduled_callback = callback;
    scheduled_user_data = user_data;
    if (fail_next_alarm) {
        fail_next_alarm = false;
        return -1;
    }
    return scheduled_id;
}

bool cancel_alarm(alarm_id_t alarm_id) {
    return alarm_id == scheduled_id;
}

int calculate_note_by_scale(uint8_t start_note, int counter,
                            ScaleNums_t scale) {
    (void)start_note;
    (void)counter;
    (void)scale;
    return calculated_note;
}

void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    (void)velocity;
    log_midi(LOG_NOTE_ON, channel, note);
}

void note_off(uint8_t channel, uint8_t note) {
    log_midi(LOG_NOTE_OFF, channel, note);
}

void change_pitch(uint8_t channel, uint8_t lsb, uint8_t msb) {
    (void)lsb;
    log_midi(LOG_PITCH, channel, msb);
}

void stop_all_notes(uint8_t channel) {
    log_midi(LOG_ALL_NOTES_OFF, channel, 0);
}

static void reset_fixture(void) {
    settings = (Settings_t){0};
    settings.fibPower = 0.5;
    settings.firstValue = 0.1;
    settings.scale = SCALE_MAJOR;
    settings.middle_plant_note = 60;
    settings.plant_channel = 5;
    settings.light_channel = 6;
    settings.maxPlantVelocity = 100;
    settings.maxLightVelocity = 100;
    settings.fraction_note_off = 4;
    settings.same_note_plant = 1;
    settings.same_note_light = 1;
    settings.light_note_range = 12;
    settings.lightBPM = 1;
    status = Active;
    active_status = Active;
    isMutedByButton = false;
    midi_log_len = 0;
    scheduled_callback = NULL;
    scheduled_user_data = NULL;
    scheduled_id = 7;
    fail_next_alarm = false;
    reset_plant_note_off();
    midi_log_len = 0;
}

static void test_identity_round_trip(void) {
    for (uint8_t channel = 0; channel < 16; ++channel) {
        for (uint8_t note = 0; note < 128; ++note) {
            const uintptr_t identity = midi_note_identity_pack(channel, note);
            assert(identity != 0);
            assert(midi_note_identity_channel(identity) == channel);
            assert(midi_note_identity_note(identity) == note);
        }
    }
}

static void test_alarm_keeps_exact_note_identity(void) {
    reset_fixture();
    calculated_note = 64;
    midi_plant(4000);
    assert(count_event(LOG_NOTE_ON, 5, 64) == 1);
    assert(scheduled_callback != NULL);

    scheduled_callback(scheduled_id, scheduled_user_data);
    settings.plant_channel = 9;
    last_note_plant = 70;
    assert(count_event(LOG_NOTE_OFF, 5, 64) == 0);
    service_midi_note_lifecycle();
    assert(count_event(LOG_NOTE_OFF, 5, 64) == 1);
    assert(count_event(LOG_NOTE_OFF, 9, 70) == 0);
}

static void test_replacement_and_clock_same_note_do_not_stick(void) {
    reset_fixture();
    calculated_note = 64;
    midi_plant(4000);

    calculated_note = 65;
    midi_plant(4000);
    assert(count_event(LOG_NOTE_OFF, 5, 64) == 1);
    assert(count_event(LOG_NOTE_ON, 5, 65) == 1);

    active_status = BPMClockActive;
    midi_plant(0);
    assert(count_event(LOG_NOTE_OFF, 5, 65) == 1);
    assert(count_event(LOG_NOTE_ON, 5, 65) == 1);
}

static void test_alarm_failure_fails_closed(void) {
    reset_fixture();
    calculated_note = 67;
    fail_next_alarm = true;
    midi_plant(4000);
    assert(count_event(LOG_NOTE_ON, 5, 67) == 1);
    assert(count_event(LOG_NOTE_OFF, 5, 67) == 1);
}

static void test_cancelled_alarm_is_ignored(void) {
    reset_fixture();
    calculated_note = 68;
    midi_plant(4000);
    alarm_callback_t old_callback = scheduled_callback;
    void *old_identity = scheduled_user_data;
    reset_plant_note_off();
    const size_t off_count = count_event(LOG_NOTE_OFF, 5, 68);
    old_callback(scheduled_id, old_identity);
    service_midi_note_lifecycle();
    assert(count_event(LOG_NOTE_OFF, 5, 68) == off_count);
}

int main(void) {
    test_identity_round_trip();
    test_alarm_keeps_exact_note_identity();
    test_replacement_and_clock_same_note_do_not_stick();
    test_alarm_failure_fails_closed();
    test_cancelled_alarm_is_ignored();
    puts("note_lifecycle: identity, replacement, Clock and failure passed");
    return 0;
}
