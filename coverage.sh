#!/bin/bash
set -e

BUILD_DIR=$(sugoi-target -b)
EXECUTABLE="${BUILD_DIR}/test-serial"

"${EXECUTABLE}"

xcrun llvm-profdata merge default.profraw -o default.profdata
xcrun llvm-cov show "${EXECUTABLE}" -instr-profile=default.profdata -format=html -o profile
