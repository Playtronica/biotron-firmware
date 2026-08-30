#!/usr/bin/env python3
"""Characterize release-1.8.2 surfaces before safety fixes."""

from __future__ import annotations

import pathlib
import re


ROOT = pathlib.Path(__file__).resolve().parents[1]


def source(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def decimal_arguments(text: str, function: str) -> list[int]:
    pattern = rf"{function}\([^,]+,\s*(\d+)\s*\);"
    return [int(value) for value in re.findall(pattern, text)]


def length_registrations(text: str, function: str) -> list[tuple[int, int]]:
    pattern = rf"{function}\([^,]+,\s*(\d+)\s*,\s*(\d+)\s*\);"
    return [(int(number), int(length)) for number, length in re.findall(pattern, text)]


def simple_function_body(text: str, signature: str) -> str:
    match = re.search(rf"{re.escape(signature)}\s*\{{(.*?)\n\}}", text, re.S)
    assert match is not None, signature
    return match.group(1)


def main() -> None:
    params = source("src/params.c")
    params_h = source("include/params.h")
    descriptors = source("PLSDK/src/usb_descriptors.c")
    tusb_config = source("PLSDK/include/tusb_config.h")
    main_source = source("main.c")
    raw_plant = source("src/raw_plant.c")
    global_source = source("src/global.c")
    developer_guide = source("DEVELOPING.md")
    settings_guide = source("SettingsDescription.md")
    changelog = source("CHANGELOG.md")
    host_runner = source("tests/run_host_tests.sh")

    assert decimal_arguments(params, "add_CC") == [
        14, 22, 23, 3, 24, 9, 25, 26, 31, 15, 20, 21, 28, 27, 30, 85, 86, 87,
    ]
    assert length_registrations(params, "add_sys_ex_com_len") == [
        (0, 1), (9, 1), (1, 1), (2, 1), (3, 1), (4, 1), (5, 1),
        (6, 1), (15, 1), (17, 1), (16, 1), (18, 1), (22, 1), (23, 1),
        (7, 0), (10, 1), (11, 1), (24, 1), (12, 1), (13, 1), (19, 1),
        (21, 1), (25, 1), (26, 1), (27, 1), (127, 2),
    ]
    query_registrations = length_registrations(params, "add_sys_ex_query_len")
    assert query_registrations == [(123, 1), (124, 1), (126, 1)]
    assert "BIOTRON_RECALIBRATE_COMMAND 123" in params_h
    assert "BIOTRON_RECALIBRATE_WAITING 1" in params_h
    assert "BIOTRON_RECALIBRATE_MEASURING 2" in params_h
    assert "BIOTRON_RECALIBRATE_READY 3" in params_h
    assert "add_sys_ex_query_len(start_plant_calibration_sys_ex" in params
    assert "add_sys_ex_query_len(get_health_sys_ex, 124, 1);" in params

    # Shipping 1.8.2 stores zero-based 1/2 and therefore emits human MIDI 2/3.
    assert len(re.findall(r"\.plant_channel\s*=\s*1\s*,", params)) == 4
    assert len(re.findall(r"\.light_channel\s*=\s*2\s*,", params)) == 4

    assert "#define FLASH_TARGET_OFFSET (512 * 1024)" in params_h
    assert re.search(r"typedef struct\s*\{.*?int id;.*?\}\s*Settings_t;", params_h, re.S)
    assert "flash_range_erase(FLASH_TARGET_OFFSET" in params
    assert "flash_range_program(FLASH_TARGET_OFFSET" in params
    assert "clear_flash" not in params
    assert main_source.index("read_settings();") < main_source.index("init_midi();")
    assert main_source.index("read_settings();") < main_source.index("init_plant();")

    dispatcher = simple_function_body(params, "void get_sys_ex_and_behave()")
    health_query = simple_function_body(
        params, "void get_health_sys_ex(const uint8_t data[], uint8_t len)"
    )
    assert "midi_diagnostics_snapshot" in health_query
    assert "midi_health_encode_page" in health_query
    assert "print_sys_ex" in health_query
    for forbidden in ("save_settings", "schedule_settings_save", "clear_flash", "reset_usb_boot"):
        assert forbidden not in health_query
    recalibration_action = simple_function_body(
        params, "void start_plant_calibration_sys_ex(const uint8_t data[], uint8_t len)"
    )
    assert "start_plant_calibration(data[0]);" in recalibration_action
    for forbidden in ("save_settings", "schedule_settings_save", "clear_flash", "reset_usb_boot"):
        assert forbidden not in recalibration_action
    recalibration_runtime = simple_function_body(
        global_source, "void start_plant_calibration(uint8_t request_nonce)"
    )
    for forbidden in ("settings =", "save_settings", "clear_flash", "reset_usb_boot"):
        assert forbidden not in recalibration_runtime
    query_case = re.search(
        r"case CUSTOM_QUERY_COMMAND:\s*(.*?)\s*break;", dispatcher, re.S
    )
    assert query_case is not None
    assert "schedule_settings_save" not in query_case.group(1)
    reset_case = re.search(
        r"case RESET_DEVICE:\s*(.*?)\s*return;", dispatcher, re.S
    )
    assert reset_case is not None
    assert "save_pending_settings_now();" in reset_case.group(1)
    assert "clear_flash" not in reset_case.group(1)
    assert reset_case.group(1).index("save_pending_settings_now();") < (
        reset_case.group(1).index("reset_usb_boot(0, 0);")
    )

    # IRQ callbacks may publish bounded work only. Settings, randomness, music
    # calculation and USB MIDI writes belong to the main loop.
    raw_timer = simple_function_body(
        raw_plant,
        "static bool _repeating_timer_callback_t(repeating_timer_t *rt)",
    )
    raw_timer = re.sub(r"//.*", "", raw_timer)
    assert "settings" not in raw_timer
    assert "rand(" not in raw_timer
    assert "print_" not in raw_timer
    music_timer = simple_function_body(
        global_source,
        "int64_t play_music_alarm(alarm_id_t id, void *user_data)",
    )
    music_timer = re.sub(r"//.*", "", music_timer)
    assert "settings" not in music_timer
    assert "play_music(" not in music_timer
    assert "print_" not in music_timer

    assert "#define USB_VID   0xCafe" in descriptors
    assert "#define USB_BCD   0x0200" in descriptors
    assert "#define MIDI_NUM_CABLES 2" in descriptors
    class_counts = {
        name: int(re.search(rf"#define CFG_TUD_{name}\s+(\d+)", tusb_config).group(1))
        for name in ("CDC", "MSC", "HID", "MIDI", "VENDOR")
    }
    pid = 0x3000 | class_counts["CDC"] | (class_counts["MSC"] << 1) | (
        class_counts["HID"] << 2
    ) | (class_counts["MIDI"] << 3) | (class_counts["VENDOR"] << 4)
    assert pid == 0x3011
    assert '"Playtronica"' in descriptors
    assert '"Biotron"' in descriptors
    assert re.search(
        r"TUD_CONFIG_DESCRIPTOR\(1,.*?TUD_CDC_DESCRIPTOR\(.*?TUD_MIDI_DESC_HEAD\(",
        descriptors,
        re.S,
    )
    assert "TUD_MIDI_JACKID_IN_EMB(1)" in descriptors
    assert "TUD_MIDI_JACKID_IN_EMB(2)" in descriptors
    assert "TUD_MIDI_JACKID_OUT_EMB(1)" in descriptors
    assert "TUD_MIDI_JACKID_OUT_EMB(2)" in descriptors

    # Keep the maintainer entry point synchronized with the production
    # compatibility values and make sure harness simplification does not drop
    # a test group silently.
    for required in (
        "cf264aa", "1765723554", "0x3011", "human MIDI channels\n   `2/3`",
        "F0 0B 14 0D 7F F7", "customer is last", "`lightBPM = 0` behaves as `1`",
        "bottom pad currently changes only its",
    ):
        assert required in developer_guide, required
    for required in (
        "values 0 and 1 both mean", "There is no \"disable light\" value",
    ):
        assert required in settings_guide, required
    assert re.findall(r"^run_pair ([a-z0-9-]+)", host_runner, re.M) == [
        "midi-parser", "commands", "midi-diagnostics", "midi-health", "runtime-safety", "usb-string",
        "settings-storage", "persistence-scheduler", "storage-v1",
        "music-v1", "note-lifecycle", "music-scheduler", "raw-plant",
        "midi-tx",
    ]
    for required in ("cf264aa", "1765723554", "human MIDI channels 2/3"):
        assert required in changelog, required

    print("release_contract: v1 registry, storage, USB and developer map passed")


if __name__ == "__main__":
    main()
