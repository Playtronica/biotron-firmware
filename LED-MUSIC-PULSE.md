# Music + Pulse LED experiment

This is the maintainer contract for the optional note-and-beat light engine.
It is experimental, disabled by default and limited to PCB revisions A06-A08.

## User-visible language

- Green light shows notes. The firmware-assigned first arc follows the light
  generator; the second arc follows the plant generator. Confirm those physical
  roles on every supported PCB before publishing user-facing names.
- Left, center and right mean low, middle and high pitch.
- A stronger isolated note is brighter. Each note has a quick, smooth rise and
  a longer release. Dense or accelerating notes accumulate into a brighter,
  steadier glow instead of restarting the LEDs from darkness.
- The complete blue arc breathes once for each actual musical beat, whether the
  beat comes from the internal clock or 24 incoming MIDI Clock pulses.
- Mute clears green note energy immediately; blue can still show the beat.
- Startup, calibration, sleep, touch buttons and factory LED test keep their
  existing behavior in this first phase.

Only emitted musical Note On messages with velocity above zero create a green
event. Scheduler ticks, Note Off, CC, pitch bend, calibration tones and incoming
notes do not.

## Architecture

`src/led_engine.c` is a small deterministic state machine. Note events add to
an internal target while the visible PWM level follows it through a bounded
attack/release envelope. This makes the result independent of main-loop frame
drops up to the bounded recovery window. It has no Pico SDK,
USB, MIDI, heap allocation, floating point or flash access. `src/leds.c` is the
only hardware adapter; `src/music.c` only reports an emitted note or beat.

The compile flag is intentionally fail-safe:

```text
BIOTRON_LED_MUSIC_PULSE=OFF  old LED firmware, byte-identical UF2
BIOTRON_LED_MUSIC_PULSE=ON   Music + Pulse on A06-A08 test units only
```

The feature does not add a setting, change `Settings_t`, allocate a SysEx ID,
write flash or alter the USB/MIDI descriptors.

## Test and build

Run the full host gate:

```bash
./tests/run_host_tests.sh
```

Then build both variants with the same pinned SDK, toolchain, settings ID and
version. Add only the final flag to the normal command in `DEVELOPING.md`:

```bash
-DBIOTRON_LED_MUSIC_PULSE=OFF
-DBIOTRON_LED_MUSIC_PULSE=ON
```

Latest local code evidence on 2026-08-31:

- the full host suite passed under ASan/UBSan and optimized `-O2`, including
  one million deterministic LED events, slow/medium/fast note-rate comparison,
  breathing timing and frame-drop equivalence;
- both variants built with Arm GNU Toolchain 15.3.1 and Pico SDK 2.3.0;
- OFF remains excluded from the experimental engine;
- the current ON build adds 1,536 bytes of flash text and 36 bytes of RAM over
  OFF. These numbers must be frozen again from the committed candidate.

The deterministic full-velocity trace reaches its smooth peak at about 16 ms,
falls below half around 328 ms and reaches zero by about 1.15 s. The blue beat
peaks around 40 ms and reaches zero by about 0.53 s. These are code contracts,
not claims about perceived brightness on the physical diffuser.

This is code evidence, not permission to release.

## Physical release gate

1. Confirm PCB A06, A07 or A08 and keep a verified rollback UF2.
2. Confirm both green arcs and the blue arc with factory test mode.
3. Trigger isolated low, middle and high notes from both generators; verify the
   physical left/center/right mapping and stronger-note brightness.
4. Play isolated notes, then accelerate from roughly 2 to 8 notes/second. The
   first case must show a smooth rise/release; the second must become a
   brighter continuous glow without hard black gaps or visible strobe.
5. Run internal clock and external MIDI Clock; each real beat must produce one
   smooth full-blue pulse without changing MIDI timing or dropping notes.
6. Check mute, startup, recalibration, sleep and all touch-button feedback.
7. Measure peak and aggregate LED current on the board. Software duty limits do
   not replace this electrical check.
8. Soak for at least one hour while monitoring MIDI health, USB resets and stuck
   notes. Restore the reference firmware after a failed test.

Do not use this binary on A03-A05. Those revisions have different pin/PWM
constraints and require separate board profiles and artifacts.

## Editing rule

Tune pitch, velocity, decay or beat constants only in `src/led_engine.c`. Keep
pin order and polarity only in `src/leds.c`. Add a deterministic engine test for
every changed rule, then repeat the OFF byte-identity check and both ARM builds.
