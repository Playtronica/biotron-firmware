# Changelog
All notable changes to this project will be documented in this file.

## Unreleased — compatible 1.8.3 stabilization candidate

This section describes candidate `cf264aa`; it is not a published release.

### Post-F1 diagnostic lab branch

- RAM-only counters now record MIDI RX/parser/TX/persistence/service timing and
  USB mount/unmount/suspend/resume lifecycle without changing settings;
- provisional read-only vendor SysEx query `124` returns four versioned,
  paged, 7-bit-clean health payloads on the existing service cable;
- this append-only query is for owner review and internal hardware diagnosis.
  It is not part of candidate `cf264aa` and is not approved for release.

### Fixed

- bounded, cable-local USB MIDI parsing with malformed/overflow recovery;
- MIDI Clock Start/Continue/Stop and 24 PPQN behavior;
- startup and runtime arithmetic undefined behavior;
- USB serial descriptor lifetime and settings flash-page packing;
- exact Note On/Off channel/note identity, panic and saturated-TX recovery;
- IRQ/main-loop ownership: callbacks publish work, the main loop owns settings,
  random music decisions and TinyUSB writes;
- changed-only debounced persistence for CC and mutating SysEx; read-only query
  no longer writes flash;
- BOOT saves pending settings instead of performing factory reset;
- explicit, reproducible firmware version and settings compatibility ID.

### Compatibility kept

- released USB identity/topology and two MIDI cables;
- cable 0 Music, cable 1 service SysEx;
- all legacy CC/SysEx IDs and stored `Settings_t` layout;
- released 1.8.2 defaults: stored channels 1/2, human MIDI channels 2/3;
- settings ID `1765723554`.

### Evidence and open gates

- full host suite passes in ASan/UBSan and optimized lanes;
- pinned ARM UF2/ELF build and Mac USB/version/bounded-liveness/software-BOOT
  evidence exist for the exact candidate;
- plant/light/button functional observation, flash-count/power-cycle,
  Windows/REAPER, PCB revisions and shipping-reference rollback remain open;
- see `DEVELOPING.md` before review or testing.

## 1.6.3 - 01-24-2025
### Changed
- Lights logic in active mode.
- Note off percent was changed to note off fraction.

## 1.6.0
### Added
- Same note mode for light (second) channel
- Test Mode

## v1.5.10
### Changed
- Jingle in stabilization mode

## v1.5.9
### Changed
- Led Indication in Stabilization mode
- Functionality of mute button

## v1.5.8
### Added
- Threshold for buttons

## v1.5.7
### Added
- Mute mode SysEx and CC command

## v1.5.6
### Changed
- Stuck mode was renamed to Performance Mode
- Performance Mode invert logic (compare Stuck Mode)

## v1.5.5
### Added
- Stuck Mode Control

## v1.5.4
### Changed
- CC commands save setting after boot

## v1.5.3
### Not Firmware Added
- Changelog file
- New github action workflow file for public releases
