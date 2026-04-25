#!/bin/bash
set -e

cd "$(dirname "$0")"

# Configure if needed
if [ ! -f build/build.ninja ] && [ ! -f build/Makefile ]; then
    echo "Configuring..."
    cmake -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev
fi

echo "Building..."
cmake --build build --target macym_VST3 --target macym_AU --target macym_Standalone --config Release -j$(sysctl -n hw.ncpu)

echo ""
echo "Done. Artifacts:"
echo "  VST3:       build/macym_artefacts/Release/VST3/macYM.vst3"
echo "  AU:         build/macym_artefacts/Release/AU/macYM.component"
echo "  Standalone:  build/macym_artefacts/Release/Standalone/macYM.app"
echo ""
echo "Run standalone:  open build/macym_artefacts/Release/Standalone/macYM.app"
