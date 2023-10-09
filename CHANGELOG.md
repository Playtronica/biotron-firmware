# Changelog

## [Unreleased]

## Changed

Implement PLSDK

## [1.2.0] - 2023-09-12

### Added
1) New Commands about light note range

### Change
1) Algorithm of save preferences has been changed
2) Some MIDI commands has been split

## [1.1.5] - 2023-09-12

### Changed
1) Logic of function "Same Note"

## [1.1.4] - 2023-09-04

### Added
1) Bpm clock mode
2) Send CC commands with notes
3) Mute mode

### Changed
1) Tap tempo disabled and replaced by Mute mode

## [1.1.3] - 2023-08-21

### Added
- All function from Sys ex have been copied to CC.

### Changed
- Fixed problem with changing BPM

## [1.1.2] - 2023-08-21

### Added
- Tap Tempo function. When user taps on captive button (gpio 8), device will count new BPM

## [1.1.1] - 2023-08-18

### Added
- New midi function. Turn off notes by percent of plant BPM

### Changed
- Changed midi function - default settings.

## [1.1.0] - 2023-08-10

### Added
- A lot of new midi commands

## [1.0.0] - 2023-02-10

### Added
- MIDI Commands. You can send MIDI commands to change some preferences.
- Second MIDI virtual port. For settings, when the first port is busy.

### Changed
- New way to play notes. 

## [0.0.4] - 2022-01-13

### Added
- Docker

## [0.0.3] - 2022-12-16

### Changed

- Changed sequence of playing note. First step - Note off, and after - Note on.
- Fix bug with led on stage Average.

### Added

- Added note off, when device change status to Sleep.
- Blue Led fade in on power.

## [0.0.2] - 2022-12-14

### Added

- Added Led Fade In, Fade Out

## Changed

- Changed Readme

## [0.0.1] - 2022-12-13

### Added

- Added changelog and readme files



