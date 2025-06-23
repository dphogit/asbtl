#!/usr/bin/env bash

_common_setup() {
  load '../test-helpers/bats-support/load'
  load '../test-helpers/bats-assert/load'

  PATH="$BATS_TEST_DIRNAME/../../build:$PATH"

  TMP_SOURCE_FILE="$(mktemp)"
}

_common_teardown() {
  rm -f "$TMP_SOURCE_FILE"
}

_run_asbtl() {
  local source="$1"
  echo "$source" >"$TMP_SOURCE_FILE"
  run asbtl "$TMP_SOURCE_FILE"
}
