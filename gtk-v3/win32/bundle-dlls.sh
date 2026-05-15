#!/bin/sh
# bundle-dlls.sh -- Collect all MSYS2/MinGW DLLs and GTK3 runtime data
# needed to run crossfire-client-gtk3.exe on a bare Windows system that
# does not have MSYS2 installed.
#
# Usage: bundle-dlls.sh <path-to-exe> <output-dir>
#
# Environment:
#   MINGW_PREFIX  Path to the MSYS2 MinGW prefix (default: /mingw64)
#
# Copyright (C) 2024 The Crossfire Development Team
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

set -e

EXE="$1"
OUTDIR="$2"
MINGW_PREFIX="${MINGW_PREFIX:-/mingw64}"

if [ -z "$EXE" ] || [ -z "$OUTDIR" ]; then
    echo "Usage: $0 <path-to-exe> <output-dir>" >&2
    exit 1
fi

if [ ! -f "$EXE" ]; then
    echo "Error: executable not found: $EXE" >&2
    exit 1
fi

mkdir -p "$OUTDIR"

echo "==> Copying executable"
cp "$EXE" "$OUTDIR/"

# ── DLLs ─────────────────────────────────────────────────────────────────────
# ldd lists every shared library the binary loads at startup.  We filter for
# paths under the MSYS2 MinGW prefix so we don't try to copy Windows system
# DLLs (which are already present on every Windows machine).

echo "==> Bundling DLLs (via ldd)"
ldd "$EXE" \
    | grep -i "$MINGW_PREFIX/bin" \
    | awk '{print $3}' \
    | xargs -I{} cp -n {} "$OUTDIR/" 2>/dev/null || true

# SDL2_mixer defers loading the Vorbis/OGG codec DLLs until it actually
# opens an OGG file, so ldd never reports them.  Copy them manually.
for lib in libvorbis-0.dll libvorbisfile-3.dll libogg-0.dll; do
    src="$MINGW_PREFIX/bin/$lib"
    [ -f "$src" ] && { cp -n "$src" "$OUTDIR/"; echo "  $lib"; } || true
done

# ── GDK-Pixbuf loaders ───────────────────────────────────────────────────────
# GTK3 uses GDK-Pixbuf to decode images (PNG icons, map tiles, etc.).
# The loaders are separate .dll files that GTK3 locates via a cache file.
# Without these, all images render as broken placeholders.

echo "==> Bundling GDK-Pixbuf loaders"
PIXBUF_VER=$(ls "$MINGW_PREFIX/lib/gdk-pixbuf-2.0/" 2>/dev/null | head -1)
if [ -n "$PIXBUF_VER" ]; then
    LOADER_SRC="$MINGW_PREFIX/lib/gdk-pixbuf-2.0/$PIXBUF_VER/loaders"
    LOADER_DST="$OUTDIR/lib/gdk-pixbuf-2.0/$PIXBUF_VER/loaders"
    mkdir -p "$LOADER_DST"
    cp -r "$LOADER_SRC/"* "$LOADER_DST/" 2>/dev/null || true

    # Rewrite the loaders.cache so the paths point into the bundle directory
    # instead of the MSYS2 prefix.  GTK3 reads this file at startup to find
    # the loader DLLs.
    CACHE_SRC="$MINGW_PREFIX/lib/gdk-pixbuf-2.0/$PIXBUF_VER/loaders.cache"
    if [ -f "$CACHE_SRC" ]; then
        sed "s|$MINGW_PREFIX|.|g" "$CACHE_SRC" \
            > "$OUTDIR/lib/gdk-pixbuf-2.0/$PIXBUF_VER/loaders.cache"
    fi
fi

# ── GLib compiled schemas ────────────────────────────────────────────────────
# GTK3 uses GSettings (backed by GLib schemas) for its own internal settings
# (e.g., file-chooser sort order, font rendering).  Without the compiled
# schema file GTK3 prints warnings and some dialogs may not open.

echo "==> Bundling GLib schemas"
mkdir -p "$OUTDIR/share/glib-2.0/schemas"
COMPILED="$MINGW_PREFIX/share/glib-2.0/schemas/gschemas.compiled"
[ -f "$COMPILED" ] && cp "$COMPILED" "$OUTDIR/share/glib-2.0/schemas/" || true

# ── Icon themes ──────────────────────────────────────────────────────────────
# GTK3 looks up named icons (close button, menu arrow, etc.) from the current
# icon theme.  Without at least the hicolor fallback theme the window
# decorations will be missing or use ugly placeholder icons.

echo "==> Bundling icon themes"
mkdir -p "$OUTDIR/share/icons"
for theme in hicolor Adwaita; do
    src="$MINGW_PREFIX/share/icons/$theme"
    [ -d "$src" ] && cp -r "$src" "$OUTDIR/share/icons/" || true
done

# ── GTK3 default settings ────────────────────────────────────────────────────
# Tell GTK3 to use the native Win32 theme and the hicolor icon set so the
# client blends with the host desktop without requiring any GTK theme files.

echo "==> Writing GTK3 settings"
mkdir -p "$OUTDIR/share/gtk-3.0"
cat > "$OUTDIR/share/gtk-3.0/settings.ini" <<'SETTINGS'
[Settings]
gtk-theme-name = win32
gtk-icon-theme-name = hicolor
SETTINGS

echo "==> Bundle complete: $OUTDIR"
