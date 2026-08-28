#!/bin/sh
set -eu

test_dir="$(mktemp -d "${TMPDIR:-/tmp}/biotron-safety.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT INT TERM
compiler="${CC:-cc}"

python3 tests/test_release_contract.py

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I PLSDK/include PLSDK/src/midi_parser.c tests/test_midi_parser.c \
  -o "$test_dir/midi-parser-sanitized"
"$test_dir/midi-parser-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I PLSDK/include PLSDK/src/midi_parser.c tests/test_midi_parser.c \
  -o "$test_dir/midi-parser-optimized"
"$test_dir/midi-parser-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -Wno-strict-prototypes -fno-omit-frame-pointer \
  -fsanitize=address,undefined -I tests/stubs -I PLSDK/include \
  PLSDK/src/midi_parser.c PLSDK/src/commands.c \
  tests/test_commands_integration.c -o "$test_dir/commands-sanitized"
"$test_dir/commands-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -Wno-strict-prototypes -I tests/stubs -I PLSDK/include \
  PLSDK/src/midi_parser.c PLSDK/src/commands.c \
  tests/test_commands_integration.c -o "$test_dir/commands-optimized"
"$test_dir/commands-optimized"

"$compiler" -std=c11 -Wall -Wextra -Werror -Wno-strict-prototypes \
  -Wno-unused-parameter -pedantic -fsyntax-only \
  -I tests/stubs -I include -I PLSDK/include src/music.c

"$compiler" -std=c11 -Wall -Wextra -Werror -Wno-strict-prototypes \
  -Wno-unused-parameter -pedantic -fsyntax-only \
  -DFLASH_ID_STARTUP=1720000000 -I tests/stubs -I include -I PLSDK/include \
  src/params.c

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_runtime_safety.c \
  -o "$test_dir/runtime-safety-sanitized"
"$test_dir/runtime-safety-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_runtime_safety.c \
  -o "$test_dir/runtime-safety-optimized"
"$test_dir/runtime-safety-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I tests/stubs -I PLSDK/include \
  PLSDK/src/usb_descriptors.c tests/test_usb_string_descriptor.c \
  -o "$test_dir/usb-string-sanitized"
"$test_dir/usb-string-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I tests/stubs -I PLSDK/include \
  PLSDK/src/usb_descriptors.c tests/test_usb_string_descriptor.c \
  -o "$test_dir/usb-string-optimized"
"$test_dir/usb-string-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_settings_storage.c \
  -o "$test_dir/settings-storage-sanitized"
"$test_dir/settings-storage-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_settings_storage.c \
  -o "$test_dir/settings-storage-optimized"
"$test_dir/settings-storage-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_persistence_scheduler.c \
  -o "$test_dir/persistence-scheduler-sanitized"
"$test_dir/persistence-scheduler-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_persistence_scheduler.c \
  -o "$test_dir/persistence-scheduler-optimized"
"$test_dir/persistence-scheduler-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -Wno-strict-prototypes -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I tests/stubs -I include -I PLSDK/include \
  tests/test_storage_v1_contract.c -o "$test_dir/storage-v1-sanitized"
"$test_dir/storage-v1-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -Wno-strict-prototypes -pedantic \
  -I tests/stubs -I include -I PLSDK/include \
  tests/test_storage_v1_contract.c -o "$test_dir/storage-v1-optimized"
"$test_dir/storage-v1-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -Wno-strict-prototypes -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I tests/stubs -I PLSDK/include \
  PLSDK/src/music.c tests/test_music_v1_contract.c \
  -o "$test_dir/music-v1-sanitized"
"$test_dir/music-v1-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -Wno-strict-prototypes -pedantic \
  -I tests/stubs -I PLSDK/include \
  PLSDK/src/music.c tests/test_music_v1_contract.c \
  -o "$test_dir/music-v1-optimized"
"$test_dir/music-v1-optimized"
