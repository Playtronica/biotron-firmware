#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/time.h"
#include "global.h"
#include "params.h"

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
void stop_midi(void) {}
bool plant_is_ready(void) { return false; }
uint32_t get_real_freq(void) { return 0; }
uint16_t adc_read(void) { return 0; }
void note_off(uint8_t channel, uint8_t note) {
    (void)channel;
    (void)note;
}
void plsdk_printf(const char *format, ...) { (void)format; }
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

    puts("music_scheduler: IRQ deferral and swing cadence passed");
    return 0;
}
