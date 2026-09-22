#!/bin/sh
#
# SPDX-License-Identifier: LGPL-3.0-only
#
# Copyright (C) 2024-2026 Corey Pennycuff
#
# This file is part of Ghoti.io Tang.
#
# Ghoti.io Tang is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# Ghoti.io Tang is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Exercises the command line utility and checks what it produces.
#
# The gtest suites link the library and never run the binary, so everything
# the binary does on its own - reading the file, reading stdin, reporting a
# file it cannot read - was previously covered only by running it and
# discarding the result.
#
# Usage: cli-test.sh <path to tang>

set -u

TANG="${1:?usage: cli-test.sh <path to tang>}"
HERE="$(dirname "$0")"
failures=0

# check <name> <expected> <actual>
check() {
  if [ "$2" = "$3" ]; then
    printf '  ok    %s\n' "$1"
  else
    printf '  FAIL  %s\n        expected [%s]\n        got      [%s]\n' "$1" "$2" "$3"
    failures=$((failures + 1))
  fi
}

# A script and a template read from a file.
check "file, script" \
  "The 30th Fibonacci number is: 832040" \
  "$("$TANG" -s "$HERE/fib.tang")"
check "file, template" \
  "The 30th Fibonacci number is: 832040" \
  "$("$TANG" "$HERE/fib.template.tang")"

# The same, read from stdin.
check "stdin, script" \
  "from stdin" \
  "$(printf 'print("from stdin");' | "$TANG" -s)"
check "stdin, template" \
  "Hello 2!" \
  "$(printf 'Hello <%%= 1+1 %%>!' | "$TANG")"

# Empty stdin is an empty program, not a read of whatever followed the
# buffer in memory.
check "stdin, empty" "" "$(printf '' | "$TANG" -s)"

# A byte that is not valid UTF-8, in the middle of the input. Read one
# character at a time into a char, as this once was, 0xFF is -1 on a platform
# where char is signed and so compares equal to EOF: everything after it was
# silently dropped.
check "stdin, 0xFF byte mid-input" \
  "before-after" \
  "$(printf 'print("before");\n/* \377 */\nprint("-after");\n' | "$TANG" -s)"

# Input larger than any single buffer the reader starts with.
expected_big="$(awk 'BEGIN { while (i++ < 5000) printf "x" }')"
check "stdin, large input" \
  "$expected_big" \
  "$(awk 'BEGIN { while (i++ < 5000) printf "print(\"x\");" }' | "$TANG" -s)"

# A file that cannot be read is reported, rather than read as empty. A
# directory opens successfully and fails on the first read, so it is the case
# that a check on fopen() alone lets through.
out="$("$TANG" -s /nonexistent/no-such-file.tang 2>&1)"
case "$out" in
  *"failed to read the file"*) printf '  ok    missing file is reported\n' ;;
  *) printf '  FAIL  missing file is reported\n        got [%s]\n' "$out"
     failures=$((failures + 1)) ;;
esac

out="$("$TANG" -s "$HERE" 2>&1)"
case "$out" in
  *"failed to read the file"*) printf '  ok    directory is reported\n' ;;
  *) printf '  FAIL  directory is reported\n        got [%s]\n' "$out"
     failures=$((failures + 1)) ;;
esac

if [ "$failures" -ne 0 ]; then
  printf '\n%s CLI check(s) failed.\n' "$failures"
  exit 1
fi
printf '\nAll CLI checks passed.\n'
