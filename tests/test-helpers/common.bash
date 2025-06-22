#!/usr/bin/env bash

_common_setup() {
  load '../../test-helpers/bats-support/load'
  load '../../test-helpers/bats-assert/load'

  PATH="$BATS_TEST_DIRNAME/../../../build:$PATH"
}

_run_file() {
  run asbtl "$BATS_TEST_DIRNAME/$1"
}
