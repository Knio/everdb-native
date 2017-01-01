#!/bin/bash
set -euox pipefail

lcov --capture --base-directory . --directory . --output-file all.info
lcov --remove all.info "/usr*" "lib/*" "test/*" --output-file all.info
genhtml -o coverage all.info

