# Windows Build Guide — GTK3 Client

This document explains every change made on the `crossfire-client-gtkv3-windows-experiment`
branch to enable building, packaging, and releasing the GTK3 client as a
self-contained Windows `.exe` installer via GitHub Actions CI/CD.

---

## Background

The upstream crossfire-client repository already contained a Windows build
script (`gtk-v3/win32/autobuild.ps1`) and an NSIS installer script
(`gtk-v3/win32/client.nsi`), but both targeted the **GTK2** client
(`crossfire-client-gtk2.exe`).  The GTK3 port introduced here required
updating those artifacts and adding a modern, automated CI/CD pipeline.

GTK3 is fully available on Windows via **MSYS2/MinGW64** — the same toolchain
the original scripts already used — so no new toolchain was introduced.

---

## Changes Made and Why

### 1. `gtk-v3/win32/client.nsi` — Fix binary name (2 lines)

**What changed:**
Lines 90 and 94 referenced `crossfire-client-gtk2.exe`.  Both were changed to
`crossfire-client-gtk3.exe`.

**Why:**
The NSIS installer creates Start Menu and Desktop shortcuts that point directly
to the binary by filename.  With the wrong name the shortcuts would be created
but would fail to launch the client.  Everything else in the installer script
(version strings, install directory, file copying from `${INPUTDIR}`,
uninstaller) is binary-agnostic and required no changes.

---

### 2. `gtk-v3/win32/bundle-dlls.sh` — New: DLL and runtime bundler

**What it does:**

GTK3 on Windows does not statically link its dependencies.  The compiled
`.exe` depends on approximately 40 shared libraries (`*.dll`) that live inside
the MSYS2 MinGW prefix (`C:\msys64\mingw64\bin`).  A bare Windows machine has
none of these.  The script assembles everything needed into one flat directory
so that NSIS can then package it into a single installer.

**Each section and why it is necessary:**

| Section | What is copied | Why it is required |
|---|---|---|
| DLL discovery | All MinGW DLLs listed by `ldd` | GTK3, GLib, Cairo, Pango, libcurl, SDL2, libpng and their transitive deps are not on Windows by default |
| Vorbis/OGG DLLs | `libvorbis-0.dll`, `libvorbisfile-3.dll`, `libogg-0.dll` | SDL2\_mixer loads these lazily at runtime when it first plays an OGG file — `ldd` never reports them because they are not referenced at link time |
| GDK-Pixbuf loaders | `lib/gdk-pixbuf-2.0/.../loaders/*.dll` + `loaders.cache` | GTK3 uses GDK-Pixbuf to decode all images (PNG map tiles, item icons). Without the loaders every image renders as a broken placeholder |
| GLib schemas | `share/glib-2.0/schemas/gschemas.compiled` | GTK3 uses GSettings internally (file-chooser sort order, font hinting, etc.). Without the compiled schema blob GTK3 logs warnings and some dialogs may fail to open |
| Icon themes | `share/icons/hicolor`, `share/icons/Adwaita` | GTK3 renders standard named icons (close button, resize grip, dialog icons). Without at least the `hicolor` fallback theme, window chrome is invisible or shows broken-image placeholders |
| `settings.ini` | `share/gtk-3.0/settings.ini` | Instructs GTK3 to use the native `win32` theme and `hicolor` icons so the client looks like a normal Windows application instead of defaulting to Adwaita |

**Why a shell script rather than inline CI steps:**

Keeping the bundling logic in `bundle-dlls.sh` means it can be run locally by
any developer with MSYS2 to reproduce the same package outside of CI.  It also
keeps the workflow YAML readable.

**GPLv2 note:** `bundle-dlls.sh` is licensed under GPLv2 (same as the
client) and carries the standard Crossfire GPL header.

---

### 3. `.github/workflows/build-windows-gtk3.yml` — New: Windows CI

**Trigger:** Push or pull request to `crossfire-client-gtkv3-windows-experiment`
that touches `gtk-v3/`, `common/`, `cmake/`, or build infrastructure files.

**What it does:**

1. Installs MSYS2/MinGW64 via the official `msys2/setup-msys2@v2` action.
2. Installs all required MinGW64 packages:

   | Package | Purpose |
   |---|---|
   | `mingw-w64-x86_64-toolchain` | GCC, binutils, MinGW runtime |
   | `mingw-w64-x86_64-cmake` | Build system |
   | `mingw-w64-x86_64-pkg-config` | Dependency detection |
   | `mingw-w64-x86_64-perl` | Code generation (`msgtypes.pl`) |
   | `mingw-w64-x86_64-vala` | Vala compiler for `snd.vala` |
   | `mingw-w64-x86_64-gtk3` | GTK3 headers and import libs |
   | `mingw-w64-x86_64-libpng` | PNG image support |
   | `mingw-w64-x86_64-curl` | Metaserver2 HTTP support |
   | `mingw-w64-x86_64-SDL2_mixer` | Audio support |

3. Configures CMake with `-G "MinGW Makefiles"` — required because the
   `windows-latest` runner defaults to MSVC toolchain detection; this flag
   forces CMake to use the MinGW GCC in the MSYS2 PATH.
4. Builds all targets (not just `crossfire-client-gtk3`) so that
   `test-metaserver` is also compiled.
5. Runs `ctest` to execute the metaserver unit test.

**Why `shell: msys2 {0}` on every step:**

The `msys2/setup-msys2@v2` action creates a POSIX shell environment that puts
the MinGW64 `bin` directory first in `PATH`.  Without `shell: msys2 {0}`,
steps would revert to PowerShell and `pkg-config`, `valac`, and the MinGW GCC
would all be missing from PATH.

---

### 4. `.github/workflows/release-windows-gtk3.yml` — New: Windows release pipeline

**Trigger:** Push of a tag matching `win-v*.*.*` (e.g. `win-v1.75.5`).

The `win-v` prefix is intentional: it keeps the Windows release track
independent from the Linux release track (which uses bare `v*.*.*` tags),
allowing either to be published without triggering the other.

**Three-job pipeline:**

```
push win-v*.*.* tag
        │
        ▼
 build-and-test          ← compile + ctest on windows-latest (gate)
        │
        ▼ (only if tests pass)
  package-exe            ← cmake --install → bundle-dlls.sh → makensis
        │
        ▼
    release              ← gh release create (runs on ubuntu-latest)
```

**Job: `build-and-test`**

Identical to the CI workflow but with `CMAKE_BUILD_TYPE=Release`.  Acts as a
gate — `package-exe` is blocked by `needs: build-and-test`.

**Job: `package-exe`**

1. Builds with `CMAKE_INSTALL_PREFIX=dist` so `cmake --install build` puts
   the binary, UI files, themes, and desktop icons under `dist/`.
2. Runs `bundle-dlls.sh dist/bin/crossfire-client-gtk3.exe package/` to
   assemble the complete self-contained directory.
3. Copies the client's own `dist/share/` (themes, UI layouts, icons) into
   `package/share/`.
4. Extracts the numeric version from the tag (`win-v1.75.5` → `1.75.5`) and
   appends `.0` to produce the four-part version string NSIS requires
   (`1.75.5.0`).
5. Runs `makensis` against `gtk-v3/win32/client.nsi` with the assembled
   `package/` as `${INPUTDIR}`.  The output is a single
   `CrossfireClient-<tag>.exe` installer.

**Job: `release`**

Runs on `ubuntu-latest` (cheaper than a Windows runner) and uses the `gh` CLI
to create a GitHub Release with auto-generated notes (`--generate-notes`) and
the installer attached as a release asset.  Uses `secrets.GITHUB_TOKEN` which
is automatically available to every GitHub Actions workflow — no additional
secrets need to be configured.

---

## How to Trigger a Release

```bash
git tag win-v1.75.5
git push origin win-v1.75.5
```

The pipeline runs automatically.  When it completes the release will appear at
`https://github.com/<org>/<repo>/releases` with the installer `.exe` attached.

---

## GPLv2 Compliance

All changes in this branch comply with the GNU General Public License,
version 2 (GPLv2), under which the Crossfire client is distributed:

| File | License status |
|---|---|
| `gtk-v3/win32/client.nsi` | Existing file — minor edit (2 lines), no license change |
| `gtk-v3/win32/bundle-dlls.sh` | New file — carries standard Crossfire GPLv2 header |
| `.github/workflows/*.yml` | CI/CD configuration — not source code subject to copyleft; no license header required |
| `WINDOWS_BUILD.md` | Documentation — not subject to copyleft |

The bundled MSYS2 runtime DLLs (GTK3, GLib, SDL2, etc.) are themselves
distributed under LGPL/MIT/BSD-compatible licenses and can legally be
redistributed alongside a GPLv2 application.  The client installer (`.exe`)
produced by NSIS is an aggregate of independently licensed components; the
GPLv2 source requirement is satisfied by the availability of this repository.

---

## Local Developer Build (without CI)

Requirements: MSYS2 with MinGW64 installed.

```bash
# In an MSYS2 MinGW64 shell:

# Install packages (one time)
pacman -S \
  mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-pkg-config \
  mingw-w64-x86_64-perl \
  mingw-w64-x86_64-vala \
  mingw-w64-x86_64-gtk3 \
  mingw-w64-x86_64-libpng \
  mingw-w64-x86_64-curl \
  mingw-w64-x86_64-SDL2_mixer \
  mingw-w64-x86_64-nsis

# Build
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=dist -DGTK_VERSION=3 \
      -DMETASERVER2=ON -DSOUND=ON -DLUA=OFF
cmake --build build --parallel
cmake --install build

# Bundle DLLs
chmod +x gtk-v3/win32/bundle-dlls.sh
./gtk-v3/win32/bundle-dlls.sh dist/bin/crossfire-client-gtk3.exe package/
cp -r dist/share package/share

# Build NSIS installer
makensis \
  /DVERSION=1.75.5.0 \
  /DGITVERSION=local \
  /DINPUTDIR=$(pwd)/package \
  /DSOURCELOCATION=$(pwd) \
  /DOUTPUTDIR=$(pwd)/installer \
  gtk-v3/win32/client.nsi
```

The installer will be at `installer/CrossfireClient-local.exe`.
