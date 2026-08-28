#!/bin/sh
set -eu

test_dir="$(mktemp -d "${TMPDIR:-/tmp}/biotron-safety.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT INT TERM
compiler="${CC:-cc}"

python3 tests/test_release_contract.py

"$compiler" -std=c11 -Wall -Wextra -Werror -Wno-strict-prototypes \
  -Wno-unused-parameter -pedantic -fsyntax-only \
  -I tests/stubs -I include -I PLSDK/include src/music.c

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fno-omit-frame-pointer -fsanitize=address,undefined \
  -I include tests/test_runtime_safety.c \
  -o "$test_dir/runtime-safety-sanitized"
"$test_dir/runtime-safety-sanitized"

"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I include tests/test_runtime_safety.c \
  -o "$test_dir/runtime-safety-optimized"
"$test_dir/runtime-safety-optimized"

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
