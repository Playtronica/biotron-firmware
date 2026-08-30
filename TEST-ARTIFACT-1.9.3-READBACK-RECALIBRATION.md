# Biotron 1.9.3 readback + recalibration candidate

Status: **LAB PASS / NO RELEASE**. This exact artifact is for Andrey and
Sergey to review before any customer or production use.

## What was fixed

The previous lab branches accidentally assigned SysEx `123` to two different
features. This candidate keeps the complete settings-readback protocol and
moves runtime recalibration to its own ID:

| ID | Contract | Side effects |
|---:|---|---|
| 123 | Read RAM or persisted settings | none |
| 124 | Read health diagnostics | none |
| 125 | Restart plant calibration | RAM-only; no settings save |
| 126 | Read firmware version | none |

Queries reply only on the logical MIDI cable that requested them. Legacy
responses and the two-cable USB descriptor remain unchanged.

## Exact source and artifact

- Source commit: `2d47e9ee89b24c5701e368dda4b03cdfd7632389`
- Settings compatibility ID: `1765723554` (unchanged)
- Firmware identity: `1.9.3` (laboratory candidate)
- Pico SDK: `2.3.0` at `98a542c1a62fb549ffb5d66a3e5892b06276b670`
- GCC: Arm GNU `13.2.Rel1` / `13.2.1`
- UF2 SHA-256: `a2604083767fb50e7864db8040f18917088a5d55ae5b9a563830a98526128863`
- BIN SHA-256: `ce83a586f063cd086ab825ba5374ef8b94558a593cc2faaaa6514bff447a6a3c`
- Linked size: text `57092`, bss `14368`, total `71460` bytes

Two clean build directories produced byte-identical UF2 and BIN files.
Artifacts and raw logs are outside git at:

`~/ProjectData/playtronica-firmware/biotron/2026-08-30-2d47e9e-readback-recalibration/`

## Automated evidence

- Full host suite: 14 production-linked groups, ASan/UBSan + optimized: PASS.
- Exact 27-field readback vector and all supported Web ranges: PASS.
- Same-requesting-cable routing: PASS.
- Registry collision gate for IDs `123..126`: PASS.
- Pinned RP2040 Release build: PASS.
- Web beta suite including lint, production isolation, build, MIDI lifecycle,
  calibration nonce/state flow, PWA/offline and browser tests: PASS.

## Physical Mac evidence

- Preflight detected exact two-input/two-output Biotron topology on `1.9.2`.
- Software BOOT succeeded without contacts. macOS did not mount `RPI-RP2`, so
  the old harness reported a false failure; pinned picotool independently
  proved BOOTSEL before writing.
- `picotool load -v` verified every programmed block; device rebooted as
  `1.9.3` and replied through both ports.
- Settings query `123` replied on the matching cable for both Port 1 and Port 2.
- Before and after 14 calibration cycles, persisted settings were exactly:
  `[39,4,14,6,58,58,15,5,38,107,0,68,1,2,2,17,1,0,0,1,1,0,61,2,3,54,0]`.
- Dirty/persisted generations stayed `0/0`; readback added zero flash saves.
- Calibration `125`: 2-cycle smoke + 10 normal cycles + 2 Clock-mode cycles,
  alternating both outputs: PASS. Duration range `10.062..10.114 s`.

## Remaining release gates

1. Sergey reviews the ID allocation and code shape.
2. Real Chrome/Edge → real Biotron calibration/readback on beta (automated Web
   tests use fake MIDI).
3. Windows 11 + REAPER handoff and ordinary functional smoke.
4. Fix the BOOT harness to accept verified picotool BOOTSEL when macOS does not
   mount the mass-storage volume.

Rollback remains available as the saved full-device backup and exact earlier
artifacts in `~/ProjectData/playtronica-firmware/biotron/`.
