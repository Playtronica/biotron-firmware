# Developing Biotron firmware

This is the maintainer entry point for the compatible F1 stabilization work.
Read it before changing MIDI, USB, settings, timers or BOOT.

## Current branch in one minute

- Shipping source baseline: tag `1.8.2`, commit `67920c2`.
- Exact production candidate: `cf264aa` (`1.8.3`).
- Commits after `cf264aa` are tests/docs/tooling only unless a new candidate is
  explicitly declared and rebuilt.
- F1 is a compatibility maintenance release. Protocol v2, CRC/A-B storage,
  new USB identity and expanded diagnostics are later releases.
- Branch `codex/biotron-p1-midi-diagnostics` is a post-F1 review branch. It
  adds internal counters and a provisional read-only SysEx query `124`; it does
  not make a new release candidate until the firmware owner accepts F1 and the
  wire contract. Event counters are native 32-bit saturating values; uptime is
  sampled as 64-bit microseconds from the main loop and encoded as saturated
  32-bit seconds on the wire.
  `settings_dirty_generation` counts accepted mutation events. A successful
  save, same-value command or revert to the persisted value makes
  `settings_persisted_generation` catch up, so equality means RAM is clean.
  TX completion means a full message was handed to TinyUSB, not delivered to a
  host. The counters are RAM-only and query `124` only snapshots them; it does
  not save, reset, enter BOOT or clear counters.
  Reset reason distinguishes power/RUN, watchdog-enable timeout and
  forced/bootrom reset. No watchdog is enabled by this branch.
- The post-F1 recalibration draft reserves vendor command `123`. Request
  `F0 14 0D 7B <nonce> F7`; progress is reported on both logical cables as
  `F0 0B 7B <nonce> <state> F7`, where state `1` is waiting for a stable plant
  signal, `2` is measuring and `3` is ready. It stops active notes and clears
  only the transient sensor baseline. It does not change settings, write flash,
  reset USB or enter BOOT. This ID remains provisional until firmware-owner
  review and physical testing.
- The exact `cf264aa` UF2 has passed Mac USB/version, bounded CC liveness,
  settings-preserving software BOOT on both MIDI outputs and exact legacy-unit
  rollback. It has not passed the complete Windows/REAPER/hardware matrix.

Do not publish, merge upstream or call it fixed from this branch alone.

## Runtime flow

```text
RP2040 IRQ/timer
  -> publish a small flag/sample only
main loop
  -> service note/alarm work
  -> update plant state and LEDs/buttons
  -> TinyUSB task
  -> drain at most 32 MIDI packets
     -> cable-local parser
     -> command/Clock action in main context
     -> changed-only persistence scheduler
  -> bounded MIDI TX queue with NoteOff/Panic reserve
```

TinyUSB, settings mutation, random music decisions and MIDI writes belong to
the main loop, never to an IRQ callback.

## File map

| Area | Production files | Contract tests |
|---|---|---|
| Startup/main ownership | `main.c`, `src/global.c`, `src/raw_plant.c` | `test_music_scheduler.c`, `test_raw_plant_runtime.c` |
| MIDI RX/Clock/SysEx | `PLSDK/src/midi_parser.c`, `PLSDK/src/commands.c`, `src/params.c` | `test_midi_parser.c`, `test_commands_integration.c` |
| MIDI observability | `PLSDK/src/midi_diagnostics.c`, `PLSDK/src/midi_health.c` | `test_midi_diagnostics.c`, `test_midi_health.c`, integration assertions |
| MIDI TX and note identity | `PLSDK/src/midi_tx.c`, `PLSDK/src/music.c`, `src/music.c` | `test_midi_tx.c`, `test_music_v1_contract.c`, `test_note_lifecycle.c` |
| Settings/flash | `src/params.c`, `include/settings_storage.h`, `include/persistence_scheduler.h` | `test_settings_storage.c`, `test_persistence_scheduler.c`, `test_storage_v1_contract.c` |
| USB identity | `PLSDK/src/usb_descriptors.c` | `test_usb_string_descriptor.c`, `test_release_contract.py` |
| User controls/LED | `src/buttons.c`, `PLSDK/src/cap_buttons.c`, `src/leds.c` | physical functional card; factory logger has the caveat below |
| Wire documentation | `SettingsDescription.md` | registry assertions in `test_release_contract.py` and `test_commands_integration.c` |

## F1 compatibility rules

Keep these byte-for-byte or behaviorally compatible unless a separate migration
release is approved:

1. USB VID/PID, product name, interfaces/endpoints and two virtual MIDI cables.
   F1 enumerates as VID `0xCAFE`, PID `0x3011`, product `Biotron`.
2. Cable 0 is Music output; cable 1 is service SysEx output. Incoming commands
   are accepted on either cable. Never mirror/randomize cable roles.
3. All existing CC/SysEx IDs and value domains.
4. Shipping 1.8.2 stored channels `1/2`, which produce human MIDI channels
   `2/3`. Do not silently “correct” them to `1/2`.
5. Raw `Settings_t` layout, 32-bit settings ID `1765723554`, flash offset and
   four presets.
6. Version/info queries are read-only. Mutating CC and SysEx share the same
   changed-only one-second save scheduler.
7. BOOT is not factory reset. Candidate system frame
   `F0 0B 14 0D 7F F7` flushes pending settings, then enters BOOT.

The physical device exposes two logical MIDI cables, not two interchangeable
devices or two guaranteed Windows clients.

## Build and test

Run the complete host suite first:

```bash
./tests/run_host_tests.sh
```

It compiles 14 production-linked test groups twice: ASan/UBSan and optimized
`-O2`, plus source/ABI/descriptor contracts. A focused test is useful while
editing, but the full script is the pre-commit gate.

Build with a pinned Pico SDK/toolchain and explicit identity. Never use a build
timestamp as the settings ID:

```bash
cmake -S . -B build -G Ninja \
  -DBIOTRON_FLASH_ID=1765723554 \
  -DBIOTRON_VERSION_MAJOR=1 \
  -DBIOTRON_VERSION_MINOR=8 \
  -DBIOTRON_VERSION_PATCH=3 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
shasum -a 256 build/biotron.uf2 build/biotron.elf
```

`PICO_SDK_PATH` must point to the reviewed SDK checkout. The historical
`make release` target uses a timestamp as settings identity and the Dockerfile
clones mutable dependencies; neither is release evidence. Keep them only for
legacy reference until the team replaces the production pipeline.

## How to change something

### Reserved Biotron query/action IDs

- `123` — read saved/RAM settings (read-only)
- `124` — health diagnostics (read-only)
- `125` — restart plant calibration (RAM-only action)
- `126` — firmware version (read-only)

Keep these IDs distinct. `tests/test_release_contract.py` is the gate: update
the protocol and Web client together rather than reusing an assigned number.

1. Name one observable behavior and its compatibility boundary.
2. Add/update a production-linked regression that fails for the old behavior.
3. Make the smallest production change; do not combine protocol, storage, USB
   and musical behavior in one commit.
4. Run the full host suite and pinned ARM build.
5. Compare USB identity, settings ABI, command map and artifact hashes.
6. Test the exact artifact on company hardware with verified rollback.
7. Only then prepare the Windows/REAPER card; the customer is last.

If a physical test fails, preserve the first log, stop, make one hypothesis and
produce a new candidate SHA. Do not debug by repeatedly flashing `latest`.

## Why the stabilization commits exist

| Commit | Intent |
|---|---|
| `a6419e5` | freeze the released 1.8.2 behavior before fixes |
| `99ad9e8` | remove startup/arithmetic/value undefined behavior |
| `7c23b37` | fix USB serial lifetime and safe flash-page packing |
| `4e5951d` | bounded cable-local parser and exact Clock semantics |
| `cb1edf8` | coalesce changed settings instead of flash-per-event |
| `5996bb1` | preserve exact note/channel identity and panic recovery |
| `9124fca` | move music/MIDI/flash ownership out of IRQ context |
| `fd05b81` | preserve swing cadence after IRQ deferral |
| `7ba8394` | atomic scheduler handoff and saturated-TX recovery |
| `cf264aa` | fail-closed build version/settings identity |

Review in that order. Each commit answers one risk and has a nearby test.

## Known compatibility quirks

These are intentional F1 compatibility constraints, not invitations for a
drive-by cleanup:

- `lightBPM = 0` behaves as `1`: the light path runs on every plant beat. An
  older copy of `SettingsDescription.md` called zero "inactive", but that was
  not the shipped 1.8.2 behavior. Changing it needs a product decision and a
  separate migration release.
- Presets store zero-based channels `1/2`, so users see MIDI channels `2/3`.
  Keep the storage values and the wire output unchanged in F1.
- The fingerprint pad advances the preset on release. The top pad toggles mute
  only when Button Mute is enabled. The bottom pad currently changes only its
  pressed/LED state; its old settings action remains commented out. Do not
  document or restore a bottom-pad musical action without a product decision.
- `button_states` in factory JSON contains raw pulse intervals, not debounced
  booleans. Use a human-observed pad/LED/note check for F1 acceptance.

When documentation and executable 1.8.2 behavior disagree, characterize both,
preserve the executable behavior in F1, and open a named migration decision.

## Known diagnostic caveat

The post-F1 branch reserves vendor query `124` only for review and internal
testing. Request `F0 14 0D 7C <page> F7`, where page is `0..3`. The response is
sent on cable 1 as `F0 14 0D 7C 01 <page> 04 <count> ... F7`; every field is an
ID followed by a five-byte little-endian base-128 uint32. The query is
append-only and read-only, but it is not a release contract until the firmware
owner reviews the field map and physical traffic tests pass.

Candidate factory commands are:

```text
F0 0B 14 0D 00 F7  all green LEDs / test mode
F0 0B 14 0D 01 F7  all blue LEDs / test mode
F0 0B 14 0D 02 F7  return to Play
F0 0B 14 0D 03 F7  CDC logger on
F0 0B 14 0D 04 F7  CDC logger off
```

The logger reliably exposes `generator_freq` and `photoresistor_adc`.
`button_states` is the last captured pulse interval, not a stable pressed flag:
when a touch prevents the expected edge it may retain the previous value. Do
not mark touch buttons PASS from that JSON. Fix this in the later diagnostic
protocol by exposing the debounced `isPressed` state and a versioned schema;
do not widen F1 merely to improve the test interface.

System test opcodes changed in older releases, so never send factory commands
to an unknown version. The BOOT opcode `127` is separately classified in the
historical compatibility matrix.

## Product boundary

Firmware can improve live MIDI stability, note lifecycle, persistence and safe
BOOT. It cannot provide an offline GUI, release a Windows port held by Chrome,
fix Help typography or guarantee two applications can own a legacy MIDI
endpoint. Test these as separate web/Windows/Help gates.
