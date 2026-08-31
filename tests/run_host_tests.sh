#!/bin/sh
set -eu

test_dir="$(mktemp -d "${TMPDIR:-/tmp}/biotron-safety.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT INT TERM
compiler="${CC:-cc}"

# Every runtime test is compiled twice. The sanitizer lane catches undefined
# behaviour and memory errors; the optimized lane catches optimizer-sensitive
# mistakes and mirrors release semantics more closely.
run_pair() {
  name="$1"
  shift

  "$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
    -fno-omit-frame-pointer -fsanitize=address,undefined \
    "$@" -o "$test_dir/$name-sanitized"
  "$test_dir/$name-sanitized"

  "$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
    "$@" -o "$test_dir/$name-optimized"
  "$test_dir/$name-optimized"
}

python3 tests/test_release_contract.py

run_pair midi-parser \
  -I PLSDK/include \
  PLSDK/src/midi_parser.c tests/test_midi_parser.c

run_pair commands \
  -Wno-strict-prototypes -I tests/stubs -I PLSDK/include \
  PLSDK/src/midi_parser.c PLSDK/src/midi_diagnostics.c \
  PLSDK/src/midi_tx.c PLSDK/src/commands.c \
  tests/test_commands_integration.c

run_pair midi-diagnostics \
  -I PLSDK/include \
  PLSDK/src/midi_diagnostics.c tests/test_midi_diagnostics.c

run_pair midi-health \
  -I PLSDK/include \
  PLSDK/src/midi_health.c tests/test_midi_health.c

run_pair settings-readback \
  -Wno-strict-prototypes -I include -I PLSDK/include \
  src/settings_readback.c tests/test_settings_readback.c

run_pair led-engine \
  -I include src/led_engine.c tests/test_led_engine.c

run_pair led-adapter \
  -Wno-strict-prototypes -DBIOTRON_LED_MUSIC_PULSE=1 \
  -I tests/stubs -I include -I PLSDK/include \
  src/led_engine.c src/leds.c tests/test_led_adapter.c

# These syntax checks make sure the production translation units compile with
# the host stubs even when their focused runtime test links only selected code.
"$compiler" -std=c11 -Wall -Wextra -Werror -Wno-strict-prototypes \
  -Wno-unused-parameter -pedantic -fsyntax-only \
  -I tests/stubs -I include -I PLSDK/include src/music.c

"$compiler" -std=c11 -Wall -Wextra -Werror -Wno-strict-prototypes \
  -Wno-unused-parameter -pedantic -fsyntax-only \
  -DBIOTRON_LED_MUSIC_PULSE=1 \
  -I tests/stubs -I include -I PLSDK/include src/leds.c

"$compiler" -std=c11 -Wall -Wextra -Werror -Wno-strict-prototypes \
  -Wno-unused-parameter -pedantic -fsyntax-only \
  -DFLASH_ID_STARTUP=1720000000 -I tests/stubs -I include -I PLSDK/include \
  src/params.c

run_pair runtime-safety \
  -I include tests/test_runtime_safety.c

run_pair usb-string \
  -I tests/stubs -I PLSDK/include \
  PLSDK/src/usb_descriptors.c tests/test_usb_string_descriptor.c

run_pair settings-storage \
  -I include tests/test_settings_storage.c

run_pair persistence-scheduler \
  -I include tests/test_persistence_scheduler.c

run_pair storage-v1 \
  -Wno-strict-prototypes -I tests/stubs -I include -I PLSDK/include \
  tests/test_storage_v1_contract.c

run_pair music-v1 \
  -Wno-strict-prototypes -I tests/stubs -I PLSDK/include \
  PLSDK/src/music.c tests/test_music_v1_contract.c

run_pair note-lifecycle \
  -Wno-strict-prototypes -DBIOTRON_LED_MUSIC_PULSE=1 \
  -I tests/stubs -I include -I PLSDK/include \
  src/music.c tests/test_note_lifecycle.c

run_pair music-scheduler \
  -Wno-strict-prototypes -I tests/stubs -I include -I PLSDK/include \
  src/global.c tests/test_music_scheduler.c

run_pair raw-plant \
  -Wno-strict-prototypes -I tests/stubs -I include -I PLSDK/include \
  src/raw_plant.c tests/test_raw_plant_runtime.c

run_pair midi-tx \
  -I tests/stubs -I PLSDK/include \
  PLSDK/src/midi_diagnostics.c PLSDK/src/midi_tx.c tests/test_midi_tx.c
