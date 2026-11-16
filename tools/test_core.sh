#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/core_test.cpp -o .build/core_test
.build/core_test
