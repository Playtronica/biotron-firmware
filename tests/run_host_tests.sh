#!/bin/sh
set -eu
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/biotron-midi-parser.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT INT TERM
compiler="${CC:-cc}"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I PLSDK/include PLSDK/src/midi_parser.c tests/test_midi_parser.c \
  -o "$test_dir/parser-sanitized"
"$test_dir/parser-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I PLSDK/include PLSDK/src/midi_parser.c tests/test_midi_parser.c \
  -o "$test_dir/parser-optimized"
"$test_dir/parser-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -Wno-strict-prototypes \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I tests/stubs -I PLSDK/include PLSDK/src/midi_parser.c \
  PLSDK/src/commands.c tests/test_commands_integration.c \
  -o "$test_dir/commands-sanitized"
"$test_dir/commands-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -Wno-strict-prototypes \
  -I tests/stubs -I PLSDK/include PLSDK/src/midi_parser.c \
  PLSDK/src/commands.c tests/test_commands_integration.c \
  -o "$test_dir/commands-optimized"
"$test_dir/commands-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_persistence.c \
  -o "$test_dir/persistence-sanitized"
"$test_dir/persistence-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_persistence.c \
  -o "$test_dir/persistence-optimized"
"$test_dir/persistence-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_note_lifecycle.c \
  -o "$test_dir/note-lifecycle-sanitized"
"$test_dir/note-lifecycle-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_note_lifecycle.c \
  -o "$test_dir/note-lifecycle-optimized"
"$test_dir/note-lifecycle-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I tests/stubs -I include -I PLSDK/include \
  PLSDK/src/music.c tests/test_music_messages.c \
  -o "$test_dir/music-messages-sanitized"
"$test_dir/music-messages-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I tests/stubs -I include -I PLSDK/include \
  PLSDK/src/music.c tests/test_music_messages.c \
  -o "$test_dir/music-messages-optimized"
"$test_dir/music-messages-optimized"

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_midi_value_safety.c \
  -o "$test_dir/midi-value-safety-sanitized"
"$test_dir/midi-value-safety-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_midi_value_safety.c \
  -o "$test_dir/midi-value-safety-optimized"
"$test_dir/midi-value-safety-optimized"
