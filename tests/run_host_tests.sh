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
