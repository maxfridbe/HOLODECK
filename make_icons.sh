#!/bin/bash
# Renders every raster icon from the single source web/favicon.svg:
#   - Android launcher PNGs for each screen density (used below Android 8;
#     Android 8+ uses the vector adaptive icon in assets/android-res/)
#   - assets/macos-icon.png (1024x1024), picked up by build_macos_bundle.sh
# Needs resvg:  cargo install resvg --locked
# Re-run after editing the SVG and commit the results.
set -e
cd "$(dirname "$0")"
command -v resvg >/dev/null || { echo "resvg not found - install with: cargo install resvg --locked"; exit 1; }

SRC=web/favicon.svg
RES=assets/android-res
for pair in mdpi:48 hdpi:72 xhdpi:96 xxhdpi:144 xxxhdpi:192; do
    density=${pair%%:*}; size=${pair##*:}
    mkdir -p "$RES/mipmap-$density"
    resvg -w "$size" -h "$size" "$SRC" "$RES/mipmap-$density/ic_launcher.png"
    echo "  $RES/mipmap-$density/ic_launcher.png (${size}px)"
done
resvg -w 1024 -h 1024 "$SRC" assets/macos-icon.png
echo "  assets/macos-icon.png (1024px)"
cp "$SRC" assets/icon.svg
