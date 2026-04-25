#!/bin/bash
#
# release.sh - Build, package, and publish a macYM release.
#
# Usage:
#   ./release.sh [version]    Cut a release. Version defaults to the
#                             project version in CMakeLists.txt.
#   ./release.sh --dry-run    Build and package only; skip tag + publish.
#
# Produces a universal (arm64 + x86_64) zip in dist/ and creates a GitHub
# release with the artifact attached.

set -euo pipefail

cd "$(dirname "$0")"

DRY_RUN=0
VERSION=""
for arg in "$@"; do
    case "$arg" in
        --dry-run) DRY_RUN=1 ;;
        -h|--help)
            sed -n '3,12p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *) VERSION="$arg" ;;
    esac
done

if [ -z "$VERSION" ]; then
    VERSION=$(sed -nE 's/^project\(macym VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' CMakeLists.txt)
fi
if [ -z "$VERSION" ]; then
    echo "ERROR: could not determine version from CMakeLists.txt" >&2
    exit 1
fi
TAG="v${VERSION}"

echo "==> macYM ${VERSION} (tag: ${TAG})$([ $DRY_RUN -eq 1 ] && echo " [dry run]")"

# ---- Pre-flight ------------------------------------------------------------

if [ $DRY_RUN -eq 0 ]; then
    command -v gh >/dev/null 2>&1 || { echo "ERROR: gh not installed (brew install gh)" >&2; exit 1; }
    gh auth status >/dev/null 2>&1 || { echo "ERROR: gh not authenticated (gh auth login)" >&2; exit 1; }

    if ! git diff --quiet || ! git diff --cached --quiet; then
        echo "ERROR: working tree not clean — commit or stash first:" >&2
        git status --short >&2
        exit 1
    fi

    BRANCH=$(git rev-parse --abbrev-ref HEAD)
    if [ "$BRANCH" != "main" ]; then
        echo "ERROR: not on main (currently: ${BRANCH})" >&2
        exit 1
    fi

    git fetch --quiet origin main
    if [ "$(git rev-parse HEAD)" != "$(git rev-parse origin/main)" ]; then
        echo "ERROR: local main not in sync with origin/main — push or pull first" >&2
        exit 1
    fi

    if git rev-parse "$TAG" >/dev/null 2>&1; then
        echo "ERROR: tag ${TAG} already exists locally" >&2
        exit 1
    fi
    if git ls-remote --tags origin "refs/tags/${TAG}" | grep -q .; then
        echo "ERROR: tag ${TAG} already exists on origin" >&2
        exit 1
    fi
fi

# ---- Build (universal) -----------------------------------------------------

echo "==> Building universal binary (arm64 + x86_64)..."
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" >/dev/null
cmake --build build \
    --target macym_VST3 --target macym_AU --target macym_Standalone \
    --config Release \
    -j"$(sysctl -n hw.ncpu)"

ARCHS=$(lipo -archs build/macym_artefacts/Release/Standalone/macYM.app/Contents/MacOS/macYM)
case "$ARCHS" in
    *arm64*x86_64*|*x86_64*arm64*) : ;;
    *) echo "ERROR: expected universal binary, got: ${ARCHS}" >&2; exit 1 ;;
esac
echo "==> Verified architectures: ${ARCHS}"

# ---- Package ---------------------------------------------------------------

DIST=dist
STAGE_NAME="macYM-${VERSION}"
STAGE="${DIST}/${STAGE_NAME}"
ZIP="${STAGE_NAME}.zip"

rm -rf "$DIST"
mkdir -p "$STAGE"

cp -R build/macym_artefacts/Release/VST3/macYM.vst3 "$STAGE/"
cp -R build/macym_artefacts/Release/AU/macYM.component "$STAGE/"
cp -R build/macym_artefacts/Release/Standalone/macYM.app "$STAGE/"
cp LICENSE README.md "$STAGE/"

echo "==> Zipping..."
( cd "$DIST" && zip -qry "$ZIP" "$STAGE_NAME" )

SHA=$(shasum -a 256 "${DIST}/${ZIP}" | awk '{print $1}')
SIZE=$(du -h "${DIST}/${ZIP}" | awk '{print $1}' | tr -d ' ')
echo "==> ${DIST}/${ZIP} (${SIZE}, sha256: ${SHA})"

if [ $DRY_RUN -eq 1 ]; then
    echo "==> Dry run complete. Skipping tag + GitHub release."
    exit 0
fi

# ---- Publish ---------------------------------------------------------------

NOTES_FILE=$(mktemp -t macym-release-notes)
trap 'rm -f "$NOTES_FILE"' EXIT

cat >"$NOTES_FILE" <<EOF
## Install

1. Download \`${ZIP}\`, unzip.
2. Copy plugins to your user library:

   \`\`\`bash
   cp -r macYM.vst3 ~/Library/Audio/Plug-Ins/VST3/
   cp -r macYM.component ~/Library/Audio/Plug-Ins/Components/
   \`\`\`

3. The binaries are not codesigned, so remove the macOS quarantine flag:

   \`\`\`bash
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/macYM.vst3
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/macYM.component
   \`\`\`

4. Rescan plugins in your DAW.

Universal binary (Apple Silicon + Intel). macOS 11+.

\`\`\`
sha256: ${SHA}
\`\`\`
EOF

echo "==> Creating GitHub release ${TAG}..."
gh release create "$TAG" "${DIST}/${ZIP}" \
    --target main \
    --title "macYM ${VERSION}" \
    --notes-file "$NOTES_FILE"

URL=$(gh release view "$TAG" --json url --jq .url)
echo ""
echo "==> Done: ${URL}"
