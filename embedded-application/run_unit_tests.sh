set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
COVERAGE_DIR="$ROOT_DIR/test-results/coverage"

for tool in lcov genhtml; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		echo "Error: $tool not found. Please install it to generate coverage reports." >&2
		exit 1
	fi
done

cmake -B "$BUILD_DIR" -S "$ROOT_DIR" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON
cmake --build "$BUILD_DIR"

mkdir -p "$COVERAGE_DIR"

# Reset counters so we only capture coverage for this test run.
lcov --directory "$BUILD_DIR" --zerocounters
lcov --directory "$BUILD_DIR" --capture --initial --output-file "$COVERAGE_DIR/base.info"

cd "$ROOT_DIR"

LLVM_PROFILE_FILE="coverage-%p.profraw" ./build/src/configuration-module/test_configuration_module

llvm-profdata merge -sparse coverage-*.profraw -o coverage.profdata
llvm-cov report ./test-results -instr-profile=coverage.profdata

genhtml "$COVERAGE_DIR/coverage.filtered.info" --output-directory "$COVERAGE_DIR/html"
