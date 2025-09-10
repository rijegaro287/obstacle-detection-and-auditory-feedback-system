#!/bin/sh
# compile script para preview_depth.cpp y image_capture_module.cpp

workpath=$(cd "$(dirname "$0")" && pwd)
builddir="$workpath/build"

echo "Cleaning old build..."
rm -rf "$builddir"

mkdir -p "$builddir"

if ! cmake -B "$builddir" -S "$workpath"; then
    echo "== CMake failed"
    exit 1
fi

# Construir ambos targets
if cmake --build "$builddir" --config Release --target test_obstacle_detection_module -j4; then
    echo "== Build success"
    echo "== Run $builddir/obstacle_detection_module"
    echo "== Run $builddir/test_obstacle_detection_module"
else
    echo "== Build failed"
    exit 1
fi

if command -v ctest >/dev/null 2>&1; then
    ctest --output-on-failure -V
else
    echo "== ctest not found, skipping tests"
fi
