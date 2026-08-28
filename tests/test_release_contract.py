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


def main() -> None:
    params = source("src/params.c")
    params_h = source("include/params.h")
    descriptors = source("PLSDK/src/usb_descriptors.c")
    tusb_config = source("PLSDK/include/tusb_config.h")

    assert decimal_arguments(params, "add_CC") == [
        14, 22, 23, 3, 24, 9, 25, 26, 31, 15, 20, 21, 28, 27, 30, 85, 86, 87,
    ]
    assert decimal_arguments(params, "add_sys_ex_com") == [
        0, 9, 1, 2, 3, 4, 5, 6, 15, 17, 16, 18, 22, 23, 7, 10, 11, 24,
        12, 13, 19, 21, 25, 26, 27, 127, 126,
    ]

    # Shipping 1.8.2 stores zero-based 1/2 and therefore emits human MIDI 2/3.
    assert len(re.findall(r"\.plant_channel\s*=\s*1\s*,", params)) == 4
    assert len(re.findall(r"\.light_channel\s*=\s*2\s*,", params)) == 4

    assert "#define FLASH_TARGET_OFFSET (512 * 1024)" in params_h
    assert re.search(r"typedef struct\s*\{.*?int id;.*?\}\s*Settings_t;", params_h, re.S)
    assert "flash_range_erase(FLASH_TARGET_OFFSET" in params
    assert "flash_range_program(FLASH_TARGET_OFFSET" in params

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

    print("release_contract: v1 registry, storage and USB source snapshots passed")


if __name__ == "__main__":
    main()
