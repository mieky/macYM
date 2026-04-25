#!/bin/bash
set -e

cd "$(dirname "$0")"

# Configure if needed
if [ ! -f build/build.ninja ] && [ ! -f build/Makefile ]; then
    echo "Configuring..."
    cmake -B build -DCMAKE_BUILD_TYPE=Release
fi

echo "Building..."
cmake --build build --target ymvst_VST3 --target ymvst_AU --target ymvst_Standalone --config Release -j$(sysctl -n hw.ncpu)

echo ""
echo "Done. Artifacts:"
echo "  VST3:       build/ymvst_artefacts/Release/VST3/ymVST.vst3"
echo "  AU:         build/ymvst_artefacts/Release/AU/ymVST.component"
echo "  Standalone:  build/ymvst_artefacts/Release/Standalone/ymVST.app"
echo ""
echo "Run standalone:  open build/ymvst_artefacts/Release/Standalone/ymVST.app"
