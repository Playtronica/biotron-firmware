#include "tusb.h"
#include <stdarg.h>
#include "PLSDK.h"
#include "PLSDK/midi_diagnostics.h"

bool LOGGER_FLAG = false;


void init_midi() {
    tusb_init();
}


void remind_midi() {
    tud_task();
}

void tud_mount_cb(void) {
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_MOUNT, false);
}

void tud_umount_cb(void) {
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_UNMOUNT, false);
}

void tud_suspend_cb(bool remote_wakeup_en) {
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_SUSPEND,
                               remote_wakeup_en);
}

void tud_resume_cb(void) {
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_RESUME, false);
}

void plsdk_printf(const char *__restrict format, ...) {
    if (!LOGGER_FLAG) return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
