#!/bin/sh
set -eu
test_bin="${TMPDIR:-/tmp}/biotron-midi-parser-test"
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I PLSDK/include PLSDK/src/midi_parser.c tests/test_midi_parser.c \
  -o "$test_bin"
"$test_bin"
