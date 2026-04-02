#!/bin/bash
# Genera LGA_VimeoDownloader.icns con fondo oscuro redondeado (#1b1b1d)
# compositeando el PNG original sobre el background.
# Uso: bash make_icns.sh (desde la carpeta resources/icons/)

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_PNG="$SCRIPT_DIR/Alta/LGA_VimeoDownloader.png"
SVG_TMP="/tmp/VimeoDownloader_composed.svg"
PNG_TMP="/tmp/VimeoDownloader_1024.png"
ICONSET="/tmp/VimeoDownloader.iconset"
ICNS="$SCRIPT_DIR/LGA_VimeoDownloader.icns"

echo "→ Componiendo SVG con fondo oscuro..."
python3 - "$SRC_PNG" "$SVG_TMP" <<'PYEOF'
import base64, sys

src_png = sys.argv[1]
svg_out = sys.argv[2]

with open(src_png, 'rb') as f:
    png_b64 = base64.b64encode(f.read()).decode()

svg = f"""<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 1024 1024">
  <!-- Fondo oscuro redondeado — mismo color que PipeSync/FileManager/ThetaExplorer -->
  <rect width="1024" height="1024" rx="224" ry="224" fill="#1b1b1d"/>
  <!-- Ícono original centrado (círculo CMY con flecha de descarga) -->
  <image x="0" y="0" width="1024" height="1024"
         xlink:href="data:image/png;base64,{png_b64}"/>
</svg>"""

with open(svg_out, 'w') as f:
    f.write(svg)
print(f"  SVG escrito en {svg_out}")
PYEOF

echo "→ Renderizando SVG a PNG 1024x1024..."
qlmanage -t -s 1024 -o /tmp/ "$SVG_TMP" > /dev/null 2>&1
mv /tmp/VimeoDownloader_composed.svg.png "$PNG_TMP"

echo "→ Creando iconset..."
rm -rf "$ICONSET"
mkdir "$ICONSET"

sips -z 16   16   "$PNG_TMP" --out "$ICONSET/icon_16x16.png"       > /dev/null
sips -z 32   32   "$PNG_TMP" --out "$ICONSET/icon_16x16@2x.png"    > /dev/null
sips -z 32   32   "$PNG_TMP" --out "$ICONSET/icon_32x32.png"       > /dev/null
sips -z 64   64   "$PNG_TMP" --out "$ICONSET/icon_32x32@2x.png"    > /dev/null
sips -z 128  128  "$PNG_TMP" --out "$ICONSET/icon_128x128.png"     > /dev/null
sips -z 256  256  "$PNG_TMP" --out "$ICONSET/icon_128x128@2x.png"  > /dev/null
sips -z 256  256  "$PNG_TMP" --out "$ICONSET/icon_256x256.png"     > /dev/null
sips -z 512  512  "$PNG_TMP" --out "$ICONSET/icon_256x256@2x.png"  > /dev/null
sips -z 512  512  "$PNG_TMP" --out "$ICONSET/icon_512x512.png"     > /dev/null
cp "$PNG_TMP"                     "$ICONSET/icon_512x512@2x.png"

echo "→ Convirtiendo a .icns..."
iconutil -c icns "$ICONSET" -o "$ICNS"

echo "→ Limpiando temporales..."
rm -rf "$ICONSET" "$PNG_TMP" "$SVG_TMP"

echo "✓ Generado: $ICNS ($(du -h "$ICNS" | cut -f1))"
