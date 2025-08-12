#!/bin/sh
# compile script para solo preview_depth.cpp

workpath=$(cd "$(dirname "$0")" && pwd)
builddir="$workpath/build"

echo "workpath: $workpath"

# Crear carpeta build si no existe
mkdir -p "$builddir"

# Generar archivos de build con cmake, indicando el source dir
if ! cmake -B "$builddir" -S "$workpath"; then
    echo "== CMake failed"
    exit 1
fi

# Construir solo el target preview_depth
if cmake --build "$builddir" --config Release --target preview_depth -j4; then
    echo "== Build success"
    echo "== Run $builddir/preview_depth"
else
    echo "== Build failed"
    exit 1
fi

# Ejecutar pruebas si existen
if command -v ctest >/dev/null 2>&1; then
    ctest --output-on-failure -V
else
    echo "== ctest not found, skipping tests"
fi
