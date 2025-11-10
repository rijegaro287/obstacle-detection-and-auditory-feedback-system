#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
COVERAGE_DIR="$ROOT_DIR/test-results/coverage"
PROFDATA_FILE="$COVERAGE_DIR/coverage.profdata"
LCOV_FILE="$COVERAGE_DIR/coverage.info"
PROFILE_PATTERN="$COVERAGE_DIR/coverage-%p.profraw"
IGNORE_REGEX=".*/(_deps|CMakeFiles)/.*|^/usr/.*|^/opt/.*"

for tool in cmake ctest llvm-profdata llvm-cov genhtml; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		echo "Error: $tool not found. Please install it to generate coverage reports." >&2
		exit 1
	fi
done

cmake -B "$BUILD_DIR" -S "$ROOT_DIR" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON
cmake --build "$BUILD_DIR"

mkdir -p "$COVERAGE_DIR"

rm -f "$COVERAGE_DIR"/coverage-*.profraw "$PROFDATA_FILE" "$LCOV_FILE"
rm -rf "$COVERAGE_DIR/html"

cd "$BUILD_DIR"
LLVM_PROFILE_FILE="$PROFILE_PATTERN" ./src/configuration-module/test_configuration_module && \
LLVM_PROFILE_FILE="$PROFILE_PATTERN" ./src/control-module/test_control_module && \
LLVM_PROFILE_FILE="$PROFILE_PATTERN" ./src/image-capture-module/test_image_capture_module && \
LLVM_PROFILE_FILE="$PROFILE_PATTERN" ./src/obstacle-detection-module/test_obstacle_detection_module && \
LLVM_PROFILE_FILE="$PROFILE_PATTERN" ./src/feedback-module/test_feedback_module && \
LLVM_PROFILE_FILE="$PROFILE_PATTERN" ./src/transmission-module/test_transmission_module


cd "$ROOT_DIR"

shopt -s nullglob
profraw_files=($COVERAGE_DIR/coverage-*.profraw)
shopt -u nullglob
if ((${#profraw_files[@]} == 0)); then
	echo "Error: no coverage data (.profraw) files were produced." >&2
	exit 1
fi

llvm-profdata merge -sparse "${profraw_files[@]}" -o "$PROFDATA_FILE"
rm -f "${profraw_files[@]}"

coverage_targets_file="$BUILD_DIR/coverage_targets.txt"
declare -a coverage_targets=()
if [[ -f "$coverage_targets_file" ]]; then
	while IFS= read -r line; do
		[[ -n "$line" ]] && coverage_targets+=("$line")
	done < "$coverage_targets_file"
fi

if ((${#coverage_targets[@]} == 0)); then
	while IFS= read -r -d $'\0' binary; do
		coverage_targets+=("$binary")
	done < <(find "$BUILD_DIR" -type f -perm -111 \( -name "test_*" -o -name "obstacle-detection-and-auditory-feedback-system" \) -print0)
fi

if ((${#coverage_targets[@]} == 0)); then
	echo "Error: no coverage-enabled executables were found." >&2
	exit 1
fi

llvm-cov report --ignore-filename-regex "$IGNORE_REGEX" --instr-profile="$PROFDATA_FILE" "${coverage_targets[@]}"

tmp_lcov_file="$(mktemp)"
trap 'rm -f "$tmp_lcov_file"' EXIT
llvm-cov export --format=lcov --ignore-filename-regex "$IGNORE_REGEX" --instr-profile="$PROFDATA_FILE" "${coverage_targets[@]}" > "$tmp_lcov_file"
mv "$tmp_lcov_file" "$LCOV_FILE"
trap - EXIT

genhtml "$LCOV_FILE" --output-directory "$COVERAGE_DIR/html" --ignore-errors inconsistent,corrupt

echo "Coverage report generated at $COVERAGE_DIR/html/index.html"
