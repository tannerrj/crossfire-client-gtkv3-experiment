# Crossfire Client — GTK v3 Build & Installation Guide

**Branch:** `crossfire-client-gtkv3`  
**Version:** 1.75.5  
**License:** GNU General Public License v2

This guide walks a developer or advanced user through cloning the source code,
installing the required build dependencies, compiling the GTK v3 client, and
running it. Instructions are provided for nine Linux distributions.

---

## Table of Contents

1. [Prerequisites Overview](#1-prerequisites-overview)
2. [Cloning the Repository](#2-cloning-the-repository)
3. [Installing Dependencies by Distribution](#3-installing-dependencies-by-distribution)
   - [Debian](#31-debian)
   - [Ubuntu](#32-ubuntu)
   - [Devuan](#33-devuan)
   - [Linux Mint](#34-linux-mint)
   - [Zorin OS](#35-zorin-os)
   - [Pop!\_OS](#36-popos)
   - [openSUSE](#37-opensuse)
   - [Fedora](#38-fedora)
   - [Manjaro](#39-manjaro)
4. [Building with CMake](#4-building-with-cmake)
5. [Install Options](#5-install-options)
6. [Running the Client](#6-running-the-client)
7. [Build Options Reference](#7-build-options-reference)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Prerequisites Overview

The following tools and libraries are required to build the GTK v3 client.
Distribution-specific package names are listed in section 3.

| Component | Purpose |
|---|---|
| **git** | Clone the source repository |
| **cmake** ≥ 3.5 | Build system generator |
| **pkg-config** | Locate installed libraries at configure time |
| **perl** | Code generation scripts used during the build |
| **valac** (Vala compiler) | Compiles the sound server helper (`snd.vala`) |
| **glib-compile-resources** | Bundles UI resource files into the binary |
| **libgtk-3** (dev) | GTK 3 widget toolkit — the core UI framework |
| **libglib-2.0** (dev) | GLib — required by GTK |
| **libpng** (dev) | PNG image loading for sprites and UI graphics |
| **libcurl** (dev) | Metaserver 2 support (server browser) |
| **libSDL2** (dev) | Audio subsystem base |
| **libSDL2\_mixer** (dev) | Sound effect and music playback |
| **libX11** (dev) | X Window System integration *(Linux only)* |

> **Sound and metaserver support are enabled by default.** They can be
> disabled at configure time with `-DSOUND=OFF` and `-DMETASERVER2=OFF`
> respectively, which removes the libcurl and SDL2 requirements.

---

## 2. Cloning the Repository

The GTK v3 port lives on the `crossfire-client-gtkv3` branch of the
experiment repository. Clone it with SSH:

```bash
git clone git@172.25.100.35:leaf/crossfire-client-gtkv3-experiment.git \
    crossfire-client
cd crossfire-client
git checkout crossfire-client-gtkv3
```

> **SSH access required.** You must have your public key added to the server
> before cloning. Contact the repository administrator if you need access.

The entire GTK v3 client source lives under `gtk-v3/`. The original GTK v2
client source is preserved under `gtk-v2/` and is not affected by this branch.

---

## 3. Installing Dependencies by Distribution

### 3.1 Debian

Tested on **Debian 12 (Bookworm)** and **Debian 13 (Trixie)**.

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    pkg-config \
    perl \
    valac \
    libglib2.0-dev \
    libglib2.0-dev-bin \
    libgtk-3-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libx11-dev
```

---

### 3.2 Ubuntu

Tested on **Ubuntu 22.04 LTS (Jammy)** and **Ubuntu 24.04 LTS (Noble)**.

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    pkg-config \
    perl \
    valac \
    libglib2.0-dev \
    libglib2.0-dev-bin \
    libgtk-3-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libx11-dev
```

> The package list is identical to Debian. Ubuntu 24.04 ships GTK 3.24 and
> Vala 0.56, both of which are compatible with this client.

---

### 3.3 Devuan

Tested on **Devuan 5 (Daedalus)** — Debian without systemd.

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    pkg-config \
    perl \
    valac \
    libglib2.0-dev \
    libglib2.0-dev-bin \
    libgtk-3-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libx11-dev
```

> Devuan tracks Debian package names directly. The build procedure is
> identical to Debian.

---

### 3.4 Linux Mint

Tested on **Linux Mint 21.x (Vera / Virginia)** and **Linux Mint 22 (Wilma)**,
which are based on Ubuntu LTS.

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    pkg-config \
    perl \
    valac \
    libglib2.0-dev \
    libglib2.0-dev-bin \
    libgtk-3-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libx11-dev
```

> Linux Mint uses Ubuntu's repositories. Package names are the same as Ubuntu.

---

### 3.5 Zorin OS

Tested on **Zorin OS 16** (Ubuntu 20.04 base) and **Zorin OS 17**
(Ubuntu 22.04 base).

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    pkg-config \
    perl \
    valac \
    libglib2.0-dev \
    libglib2.0-dev-bin \
    libgtk-3-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libx11-dev
```

> Zorin OS is Ubuntu-based; package names are the same as Ubuntu. If you are
> on Zorin OS 16 (Ubuntu 20.04 base) and `valac` is not found, install
> `valac-0.48` instead.

---

### 3.6 Pop!\_OS

Tested on **Pop!\_OS 22.04 LTS** and **Pop!\_OS 24.04 LTS** (System76).

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    pkg-config \
    perl \
    valac \
    libglib2.0-dev \
    libglib2.0-dev-bin \
    libgtk-3-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libx11-dev
```

> Pop!\_OS is Ubuntu-based. Package names are the same as Ubuntu. The
> System76 PPAs do not affect any of the above packages.

---

### 3.7 openSUSE

Tested on **openSUSE Leap 15.5** and **openSUSE Tumbleweed**.

```bash
sudo zypper refresh
sudo zypper install -y \
    git \
    cmake \
    pkg-config \
    perl \
    vala \
    glib2-devel \
    gtk3-devel \
    libpng16-devel \
    libcurl-devel \
    libSDL2-devel \
    libSDL2_mixer-devel \
    libX11-devel
```

> On Tumbleweed, `glib-compile-resources` is provided by `glib2-devel`. On
> Leap 15.4 or earlier, the `vala` package may be at an older version;
> Tumbleweed is preferred for the most up-to-date Vala release.

---

### 3.8 Fedora

Tested on **Fedora 39** and **Fedora 40**.

```bash
sudo dnf install -y \
    git \
    cmake \
    pkgconf-pkg-config \
    perl \
    vala \
    glib2-devel \
    gtk3-devel \
    libpng-devel \
    libcurl-devel \
    SDL2-devel \
    SDL2_mixer-devel \
    libX11-devel
```

> On Fedora, `glib-compile-resources` is part of `glib2-devel`. The
> `pkgconf-pkg-config` package provides the `pkg-config` command.

---

### 3.9 Manjaro

Tested on **Manjaro 23.x** (Arch-based, rolling release).

```bash
sudo pacman -Syu
sudo pacman -S --needed \
    git \
    cmake \
    pkgconf \
    perl \
    vala \
    glib2 \
    gtk3 \
    libpng \
    curl \
    sdl2 \
    sdl2_mixer \
    libx11
```

> On Manjaro / Arch, `glib-compile-resources` is included in the `glib2`
> package and `pkg-config` is provided by `pkgconf`. All packages listed
> above are in the official `extra` repository; no AUR packages are required.

---

## 4. Building with CMake

Once dependencies are installed, configure and compile the client from the
repository root.

### Step 1 — Create a build directory

```bash
cd crossfire-client
mkdir build
```

### Step 2 — Configure with CMake

To build **only the GTK v3 client** and install it to your home directory
(no `sudo` required):

```bash
cmake -B build \
    -DGTK_VERSION=3 \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local/crossfire-gtk3"
```

To build **both** the GTK v2 and GTK v3 clients:

```bash
cmake -B build \
    -DGTK_VERSION=both \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local/crossfire-gtk3"
```

### Step 3 — Compile

```bash
cmake --build build
```

Use `-j` to parallelise across CPU cores and speed up the build:

```bash
cmake --build build -- -j$(nproc)
```

### Step 4 — Install

```bash
cmake --install build
```

This installs:
- **Binary:** `$HOME/.local/crossfire-gtk3/bin/crossfire-client-gtk3`
- **UI layouts:** `$HOME/.local/crossfire-gtk3/share/crossfire-client/ui/`
- **Themes:** `$HOME/.local/crossfire-gtk3/share/crossfire-client/themes/`
- **Sounds:** `$HOME/.local/crossfire-gtk3/share/crossfire-client/sounds/`
  *(if sound support is enabled)*

---

## 5. Install Options

### System-wide install (requires sudo)

```bash
cmake -B build \
    -DGTK_VERSION=3 \
    -DCMAKE_INSTALL_PREFIX=/usr/local

cmake --build build -- -j$(nproc)
sudo cmake --install build
```

The binary will be installed to `/usr/local/bin/crossfire-client-gtk3`.

### Custom prefix

Any writable path works as the install prefix:

```bash
cmake -B build -DGTK_VERSION=3 -DCMAKE_INSTALL_PREFIX=/opt/crossfire
```

> **Important:** The `CF_DATADIR` macro is baked into `config.h` at configure
> time from `CMAKE_INSTALL_PREFIX`. If you move the install directory after
> building, the client will not find its UI layouts and will fail to start.
> Always reconfigure and rebuild when changing the install location.

---

## 6. Running the Client

```bash
$HOME/.local/crossfire-gtk3/bin/crossfire-client-gtk3
```

To make the binary available system-wide from your terminal without specifying
the full path, add the install `bin/` directory to your `PATH`. Add the
following line to your shell's startup file (`~/.bashrc`, `~/.zshrc`, etc.):

```bash
export PATH="$HOME/.local/crossfire-gtk3/bin:$PATH"
```

Then reload your shell:

```bash
source ~/.bashrc   # or ~/.zshrc if using Zsh
```

You can then launch the client simply with:

```bash
crossfire-client-gtk3
```

### Choosing a layout

The client ships eleven UI layouts. The active layout is selected from the
**Settings** menu at runtime. Each layout displays its name in the window
title bar (e.g. `Crossfire Client - GTK v3 - Meflin`).

| Layout file | Title bar name |
|---|---|
| `gtk-v2.ui` | GTK v2 *(default)* |
| `gtk-v1.ui` | GTK v1 |
| `meflin.ui` | Meflin |
| `oroboros.ui` | Oroboros |
| `sixforty.ui` | SixForty |
| `eureka.ui` | Eureka |
| `caelestis.ui` | Caelestis |
| `chthonic.ui` | Chthonic |
| `lobotomy.ui` | Lobotomy |
| `v1-redux.ui` | V1 Redux |
| `un-deux.ui` | Un-Deux |

---

## 7. Build Options Reference

Pass these to `cmake -B build` with `-D<OPTION>=<VALUE>`.

| Option | Default | Description |
|---|---|---|
| `GTK_VERSION` | `both` | Which GTK client to build: `2`, `3`, or `both` |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Root directory for `cmake --install` |
| `SOUND` | `ON` | Build with SDL2\_mixer sound support |
| `METASERVER2` | `ON` | Build with libcurl metaserver 2 support |
| `LUA` | `OFF` | Build with Lua scripting support (requires Lua 5.1) |

**Example — minimal build with no sound and no metaserver:**

```bash
cmake -B build \
    -DGTK_VERSION=3 \
    -DSOUND=OFF \
    -DMETASERVER2=OFF \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local/crossfire-gtk3"
```

---

## 8. Troubleshooting

### CMake cannot find GTK 3

```
CMake Error: gtk+-3.0 not found
```

Verify GTK 3 development headers are installed:

```bash
pkg-config --modversion gtk+-3.0
```

If the command fails, the `-dev` (or `-devel`) package for GTK 3 is not
installed. Re-run the dependency installation step for your distribution.

---

### `valac` not found

```
CMake Error: Could not find Vala
```

On Debian/Ubuntu-based systems try:

```bash
sudo apt install valac
```

On openSUSE/Fedora try `vala` (no `c` suffix). On Manjaro/Arch, the
package is also `vala`.

---

### `glib-compile-resources` not found

```
CMake Error: GLIB_COMPILE_RESOURCES not found
```

This tool ships with the GLib development package:

- Debian/Ubuntu: `libglib2.0-dev-bin`
- openSUSE / Fedora: `glib2-devel`
- Manjaro/Arch: `glib2`

---

### Client crashes on startup: `Could not load default layout`

The binary cannot find the UI layout files. This means `CF_DATADIR` (baked in
at build time) does not match the actual install location. Reconfigure with
the correct prefix and reinstall:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/correct/prefix
cmake --build build -- -j$(nproc)
cmake --install build
```

---

### CSS / theme error spam on startup

The bundled `themes/Standard` and `themes/Black` files are GTK 2 RC format
and are intentionally skipped by the GTK 3 client. The message:

```
Skipping non-CSS theme file: .../themes/Standard
```

is a debug-level log entry, not an error. The client falls back to the system
GTK 3 theme automatically. This is expected behaviour.

---

### Sound does not work

Verify SDL2 and SDL2\_mixer are installed (see section 3 for your
distribution). If you built with `-DSOUND=OFF`, rebuild with `SOUND=ON` and
reinstall. The client also requires a `sounds/` directory under
`CF_DATADIR`; if it is missing, a warning is printed at build time.
