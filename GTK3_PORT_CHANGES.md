# GTK3 Port Change History

**Branch:** `crossfire-client-gtkv3`  
**Base:** Crossfire Client 1.75.5 (GTK2 client in `gtk-v2/`)  
**Target:** New `gtk-v3/` directory — parallel GTK3 client  
**License:** GNU General Public License v2  
**Build system:** CMake (see `CMakeLists.txt` at repo root)

---

## Overview

This document describes every deliberate code change made during the GTK2 → GTK3 port of the Crossfire client. It is written for a developer who wants to understand what changed, why it changed, and what GTK3 API difference drove each decision — without having to read the raw git log or diff every file.

Changes are grouped by subsystem. Each section states the GTK2 behaviour, the GTK3 incompatibility that broke it, and the fix applied.

---

## Table of Contents

1. [Build System — CMakeLists.txt](#1-build-system--cmakeliststxt)
2. [GtkBuilder UI Files — Common GTK3 Breakage](#2-gtkbuilder-ui-files--common-gtk3-breakage)
3. [UI Files — Per-Layout Fixes](#3-ui-files--per-layout-fixes)
4. [Window Title Normalization](#4-window-title-normalization)
5. [Config Section Key Rename](#5-config-section-key-rename)
6. [Theme Loading — config.c](#6-theme-loading--configc)
7. [Message Color System — info.c](#7-message-color-system--infoc)
8. [Inventory Rendering — inventory.c](#8-inventory-rendering--inventoryc)
9. [Map Rendering — map.c](#9-map-rendering--mapc)
10. [Spells Panel — spells.c](#10-spells-panel--spellsc)
11. [Skills Panel — skills.c](#11-skills-panel--skillsc)
12. [Account / Character Creation — account.c, create_char.c](#12-account--character-creation--accountc-create_charc)
13. [Label and String Updates](#13-label-and-string-updates)

---

## 1. Build System — CMakeLists.txt

**File:** `CMakeLists.txt` (repo root)

### What changed

A `GTK_VERSION` cmake option (`"2"`, `"3"`, `"both"`) was added to allow building the GTK2 client, the GTK3 client, or both from the same source tree.

GTK2 and GTK3 use different pkg-config namespaces (`gtk+-2.0` vs `gtk+-3.0`). The original CMakeLists used a single set of variables for GTK. These were split into `GTK2_*` and `GTK3_*` namespaces so both can be found and linked independently without one overwriting the other.

`find_package(X11)` is conditionally included only on `UNIX AND NOT APPLE`. X11 is not available on macOS (which uses Quartz) or Windows (which uses Win32), so unconditionally requiring it broke cross-platform builds.

### Why

The existing `gtk-v2/CMakeLists.txt` hardcoded GTK2 pkg-config variables. Adding GTK3 support required a clean separation to avoid cmake variable collisions and to preserve the ability to build the GTK2 client unchanged.

---

## 2. GtkBuilder UI Files — Common GTK3 Breakage

**Files:** All 12 `.ui` files in `gtk-v3/ui/`

GTK3's `GtkBuilder` is stricter than GTK2's. Several properties and signal names that existed in GTK2 were removed entirely in GTK3; GtkBuilder will **abort with a fatal error** when it encounters them. The following were removed from every `.ui` file.

### 2.1 `expose-event` signal connections

**GTK2:** The `expose-event` signal on `GtkDrawingArea` widgets was connected in the UI file to C callbacks that drew content using `GdkEventExpose`.

**GTK3:** The `expose-event` signal was replaced by the `draw` signal, which passes a `cairo_t*` directly. GtkBuilder in GTK3 treats any unknown signal name as a hard error. Because `expose-event` no longer exists in GTK3, every connection to it had to be removed from the UI files.

The draw callbacks are instead connected in C code during widget setup (e.g. `g_signal_connect(widget, "draw", G_CALLBACK(map_draw_callback), NULL)`).

### 2.2 `resize_mode` property on GtkContainer

**GTK2:** `GtkContainer` had a `resize_mode` property that controlled how resize requests propagated up the container hierarchy.

**GTK3:** This property was removed entirely. GtkBuilder emits a fatal parse error when it encounters it.

### 2.3 `extension_events` property on GtkWidget

**GTK2:** `GtkWidget` had an `extension_events` property for enabling extended input device events (e.g. stylus pressure).

**GTK3:** Removed. GtkBuilder emits a fatal parse error. This property appeared in `dialogs.ui` on a `GtkComboBox`, triggering the first fatal startup crash.

### 2.4 `<requires>` version declaration

All UI files had `<requires lib="gtk+" version="2.16"/>`. Updated to `version="3.0"` to reflect the actual minimum runtime requirement.

### 2.5 `use_stock` property — intentionally preserved

**GTK3 status:** Deprecated but **not removed**. GTK3 still honours `use_stock=True` to map stock IDs like `gtk-quit` to human-readable labels and icons.

An earlier attempt to remove `use_stock` was reverted when it caused stock ID strings (e.g. `"gtk-quit"`) to appear literally as button text. The property must remain in the UI files until a proper migration to named icons and `GtkModelButton` is done.

---

## 3. UI Files — Per-Layout Fixes

### 3.1 `gtk-v1.ui` — missing GtkAccelGroup declaration

**Problem:** The layout referenced `accelgroup1` in widget accelerator bindings but never declared the `GtkAccelGroup` object. GTK2's builder was lenient about forward/missing references; GTK3's builder is not and returned a `GtkBuilder: object with ID "accelgroup1" not found` error, preventing the layout from loading.

**Fix:** Added `<object class="GtkAccelGroup" id="accelgroup1"/>` near the top of the file, before the first widget that references it.

### 3.2 `sixforty.ui` — GtkProgressBar `text_xalign` removed

**Problem:** Five `GtkProgressBar` instances used `<property name="text_xalign">0</property>`. This property was removed in GTK3. GtkBuilder raised a fatal error on each occurrence.

**Fix:** All five `text_xalign` property lines were removed.

---

## 4. Window Title Normalization

**Files:** All layout `.ui` files, `dialogs.ui`

### Phase 1 — Consistent base title

The layouts had inconsistent title formats:
- Most layouts: `"Crossfire Client"` (no toolkit label)
- `meflin.ui`, `sixforty.ui`: `"Crossfire GTK V3 Client - <Name>"`
- `dialogs.ui` connect window: `"Crossfire Client"`

All were normalized to `"Crossfire Client - GTK v3"`.

### Phase 2 — Per-layout name suffix

Users switching between layouts had no way to tell from the title bar which layout was active. The `meflin.ui` and `sixforty.ui` originals had included the layout name; this was restored across all layouts in a consistent format:

| UI File | Window Title |
|---|---|
| `gtk-v1.ui` | `Crossfire Client - GTK v3 - GTK v1` |
| `gtk-v2.ui` | `Crossfire Client - GTK v3 - GTK v2` |
| `meflin.ui` | `Crossfire Client - GTK v3 - Meflin` |
| `oroboros.ui` | `Crossfire Client - GTK v3 - Oroboros` |
| `sixforty.ui` | `Crossfire Client - GTK v3 - SixForty` |
| `eureka.ui` | `Crossfire Client - GTK v3 - Eureka` |
| `caelestis.ui` | `Crossfire Client - GTK v3 - Caelestis` |
| `chthonic.ui` | `Crossfire Client - GTK v3 - Chthonic` |
| `lobotomy.ui` | `Crossfire Client - GTK v3 - Lobotomy` |
| `v1-redux.ui` | `Crossfire Client - GTK v3 - V1 Redux` |
| `un-deux.ui` | `Crossfire Client - GTK v3 - Un-Deux` |
| `dialogs.ui` | `Crossfire Client - GTK v3` (not a layout) |

The title is hardcoded in each UI file's top-level `GtkWindow` `title` property. There is no runtime mechanism that sets the title from the loaded filename — each file carries its own identity.

---

## 5. Config Section Key Rename

**File:** `gtk-v3/src/config.c`

All `g_key_file_get_*` and `g_key_file_set_*` calls that used the section name `"GTKv2"` were changed to `"GTKv3"`. This means a fresh GTK3 install starts with a clean config section and does not inherit GTK2 config values that may have been layout- or widget-size-specific. Users who run both clients simultaneously maintain independent configuration.

---

## 6. Theme Loading — config.c

**File:** `gtk-v3/src/config.c`  
**Function:** `load_theme()`

### GTK2 approach

The GTK2 client loaded theme files using `gtk_rc_parse()`. The bundled themes (`themes/Standard`, `themes/Black`) are GTK RC format files — a GTK2-specific plain-text style description language.

### GTK3 approach

GTK3 removed the RC system entirely. Theming is done via CSS loaded with `GtkCssProvider`. The function `init_theme()` was rewritten to use `GtkCssProvider` and looks for `<theme>.gtk3.css` files alongside the layout's `.ui` file.

### The startup error

When `load_theme()` encountered the legacy RC files, it passed them directly to `GtkCssProvider`, which emitted a flood of CSS parse errors to stderr on every startup — one per line of RC syntax it could not understand.

**Fix:** Added a suffix check: if the theme path does not end in `.css`, log a debug message and skip the CSS load. The client falls back to the system GTK3 theme.

### Subsystem notification restructuring

`load_theme()` previously called `info_get_styles()`, `inventory_get_styles()`, `stats_get_styles()`, `spell_get_styles()`, and `update_spell_information()` only after successfully loading a CSS file. Because the early-return on non-CSS files came before those calls, subsystems were never notified of theme state on startup.

**Fix:** Moved the subsystem notification calls outside the CSS-loading conditional block so they always run regardless of whether a CSS file was loaded. This was critical for making the color tag system work (see section 7).

---

## 7. Message Color System — info.c

**File:** `gtk-v3/src/info.c`  
**Functions:** `add_tags_to_textbuffer()`, `add_style_to_textbuffer()`, `info_init()`, `info_get_styles()`

This was the most subtle bug in the port. All server messages appeared in black regardless of their NDI color code. The root cause was two separate bugs that compounded each other.

### Background: NDI colors

The Crossfire server annotates messages with NDI color codes (`NDI_RED`, `NDI_WHITE`, etc.). The client maps these to `GdkRGBA` values in `root_color[]`, which is populated via `gdk_rgba_parse()` in `main.c` before any UI is initialized. The info pane applies colors by applying `GtkTextTag` objects that carry a `foreground-rgba` attribute.

### Bug 1 — Tags were allocated as NULL and never created

**GTK2 version of `add_tags_to_textbuffer()`:** Created `GtkTextTag` objects with colors sourced from a `GtkStyle*` (RC-derived style).

**GTK3 version (broken):** The function body set `pane->color_tags[i] = NULL` in a loop and returned. No tags were ever created. Any call to `gtk_text_buffer_apply_tag()` with a NULL tag pointer silently did nothing.

**Fix:** Rewrote the loop to create one `GtkTextTag` per NDI color slot, setting `foreground-rgba` from `root_color[i]`:

```c
for (i = 0; i < NUM_COLORS; i++) {
    pane->color_tags[i] = gtk_text_buffer_create_tag(
        pane->textbuffer, NULL,
        "foreground-rgba", &root_color[i],
        NULL);
}
```

### Bug 2 — `add_style_to_textbuffer()` destroyed the tags

**GTK2 version:** Called `add_tags_to_textbuffer()` to replace the old RC-derived tags with new ones from the updated style.

**GTK3 version (broken):** Did the same — which destroyed the freshly created tags and replaced them with NULL entries, erasing any color information that had just been set.

**Fix:** Changed the function to update tag properties in-place using `g_object_set()` rather than recreating them. This preserves tag identity (existing text runs keep their tag references) while still allowing theme reloads to update the displayed colors:

```c
for (i = 0; i < NUM_COLORS; i++) {
    if (pane->color_tags[i]) {
        g_object_set(pane->color_tags[i],
                     "foreground-rgba", &root_color[i], NULL);
    }
}
```

### Bug 3 — `info_get_styles()` was never called at startup

Even after fixing the tag creation, colors still did not appear. `info_get_styles()` (which calls `add_tags_to_textbuffer()`) was commented out inside `info_init()`:

```c
/* info_get_styles(); */   /* GTK2 code, was left commented out */
```

Additionally, `load_theme()` returned early for non-CSS files before it reached the `info_get_styles()` call (see section 6).

**Fix:** Uncommented `info_get_styles()` in `info_init()` and restructured `load_theme()` so subsystem calls always run. With both fixes in place, color tags are created immediately when the info panes are initialized and are available before the first server message arrives.

---

## 8. Inventory Rendering — inventory.c

**File:** `gtk-v3/src/inventory.c`

### 8.1 GtkTreeStore column types

**GTK2:** Tree store color columns used `GDK_TYPE_COLOR` (`GdkColor` struct — channels as `guint16` in range 0–65535).

**GTK3:** `GdkColor` was removed. All color data must use `GDK_TYPE_RGBA` (`GdkRGBA` struct — channels as `gdouble` in range 0.0–1.0). Both `gtk_tree_store_new()` calls in inventory setup were updated accordingly.

### 8.2 Cell renderer color properties

**GTK2:** `GtkCellRenderer` used `"background-gdk"` and `"foreground-gdk"` properties accepting `GdkColor*`.

**GTK3:** These properties were removed. Replaced with `"background-rgba"` and `"foreground-rgba"` accepting `GdkRGBA*`.

### 8.3 Row color helper function

**GTK2:** `get_row_style()` returned `GtkStyle*` from which colors were extracted.

**GTK3:** Replaced with `get_row_color()` returning `const GdkRGBA*` directly, since `GtkStyle` no longer exists in GTK3.

### 8.4 Drawing area — expose-event → draw signal

**GTK2:** `drawingarea_inventory_table_expose()` received `GdkEventExpose*` and called `gdk_cairo_create(event->window)` to obtain a `cairo_t`.

**GTK3:** The callback signature changed to `(GtkWidget*, cairo_t*, gpointer)`. The `cairo_t` is provided directly by the framework. Renamed to `drawingarea_inventory_table_draw()` and updated accordingly.

### 8.5 Widget background color

**GTK2:** `gtk_widget_modify_bg()` set the drawing area's background color.

**GTK3:** This function was removed. Replaced with `gtk_widget_override_background_color()`.

### 8.6 Animation redraw

**GTK2:** Animation updates called `draw_inv_table_icon()` directly, writing to the window surface immediately.

**GTK3:** Direct drawing outside a `draw` callback is not supported. Changed animation updates to call `gtk_widget_queue_draw()`, which schedules the draw callback for the next frame.

---

## 9. Map Rendering — map.c

**File:** `gtk-v3/src/map.c`

### The problem with `gdk_cairo_create()`

**GTK2:** `gtk_map_redraw()` called `gdk_cairo_create(gtk_widget_get_window(widget))` to get a `cairo_t` for immediate drawing into the window surface.

**GTK3:** `gdk_cairo_create()` was deprecated in GTK 3.22 and later removed. Drawing must happen inside a `draw` signal callback; calling it from outside that callback produces a runtime warning and may draw to an invalid surface.

### Fix — persistent offscreen surface

A static `cairo_surface_t *map_surface` was added. `gtk_map_redraw()` now renders the complete map tile grid into this surface instead of directly to the window. After rendering, it calls `gtk_widget_queue_draw()` to request a repaint.

A new `map_draw_callback()` function connected to the widget's `draw` signal composites `map_surface` onto the widget's `cairo_t`:

```c
static gboolean map_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    if (map_surface) {
        cairo_set_source_surface(cr, map_surface, 0, 0);
        cairo_paint(cr);
    } else {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_paint(cr);
    }
    return FALSE;
}
```

When the map scale is an exact integer multiple, `CAIRO_FILTER_NEAREST` is used to avoid blurry pixel scaling on the compositing step.

---

## 10. Spells Panel — spells.c

**File:** `gtk-v3/src/spells.c`

### Color column types

Same as inventory (section 8.1–8.2): `GDK_TYPE_COLOR` → `GDK_TYPE_RGBA`, `"background-gdk"/"foreground-gdk"` → `"background-rgba"/"foreground-rgba"`.

### `gtk_tree_view_set_rules_hint()` removed

**GTK2:** `gtk_tree_view_set_rules_hint(TRUE)` enabled alternating row background colours (handled by the theme).

**GTK3:** This function was removed; alternating row colours are now controlled entirely by the CSS theme using the `:nth-child(odd/even)` selectors. The call was removed from `spell_get_styles()`.

### Widget allocation

**GTK2:** `spell_treeview->allocation.width` accessed the widget's allocated width directly from the struct.

**GTK3:** Direct struct access is not part of the public API. Replaced with the accessor `gtk_widget_get_allocated_width(spell_treeview)`.

### `spell_get_styles()` stub

The GTK2 version derived colors from a `GtkStyle*`. In GTK3, `GtkStyle` does not exist. The function was stubbed to set all entries to NULL (colors fall back to system defaults). A full CSS-based implementation is a future task.

---

## 11. Skills Panel — skills.c

**File:** `gtk-v3/src/skills.c`

`gtk_tree_view_set_rules_hint()` was removed for the same reason as in `spells.c` (section 10). No other changes were required in this file.

---

## 12. Account / Character Creation — account.c, create_char.c

**Files:** `gtk-v3/src/account.c`, `gtk-v3/src/create_char.c`

### Function signature mismatch

**GTK2 `add_style_to_textbuffer()`:** Accepted `(info_pane *pane, GtkStyle *style)`. The `GtkStyle*` was used to extract RC-derived colors.

**GTK3 `add_style_to_textbuffer()`:** The `GtkStyle*` parameter was removed since `GtkStyle` does not exist in GTK3. The signature became `(info_pane *pane)`.

Five call sites in `account.c` and `create_char.c` still passed `NULL` as the second argument, causing a compiler error. The `, NULL` was removed from each call site.

Affected call sites:
- `account.c`: 4 calls
- `create_char.c`: 1 call

---

## 13. Label and String Updates

**Files:** `gtk-v3/src/config.c`, `gtk-v3/src/create_char.c`, `gtk-v3/src/info.c`, `gtk-v3/src/info.h`, `gtk-v3/src/inventory.c`, `gtk-v3/src/keys.c`, `gtk-v3/src/menubar.c`, `gtk-v3/src/metaserver.c`, `gtk-v3/src/png.c`, `gtk-v3/src/sound.c`, and all layout `.ui` files

All user-visible and internal identifiers that still referred to GTK v2 were updated:

| What | GTK2 value | GTK3 value |
|---|---|---|
| Window titles in UI files | `"Crossfire Client - GTK v2"` | `"Crossfire Client - GTK v3 - <Layout>"` |
| Config file section key | `"GTKv2"` | `"GTKv3"` |
| Log/error message prefixes | `"gtk-v2::"` | `"gtk-v3::"` |
| Doxygen `@file` paths | `gtk-v2/src/` | `gtk-v3/src/` |
| Doxygen group names | `"GTK V2"` | `"GTK V3"` |
| Version string in connection log | `"GTKv2 Client 1.75.5"` | `"GTKv3 Client 1.75.5"` |

> **Note on scope:** The string replacement was deliberately scoped to avoid touching `GtkWidget` property names, signal names, or any string that contains `"gtk"` as part of a GTK API identifier. An over-broad replacement in an earlier pass caused stock ID strings like `"gtk-quit"` to appear literally as button text; that was corrected by restoring the `use_stock` property (see section 2.5).

---

## Build and Install Reference

```sh
# Configure (GTK3 only, user-local install)
cmake -B build \
  -DGTK_VERSION=3 \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local/crossfire-gtk3

# Build
cmake --build build

# Install
cmake --install build

# Run
$HOME/.local/crossfire-gtk3/bin/crossfire-client-gtk3
```

Data files (UI layouts, themes, sounds) are installed to:  
`$HOME/.local/crossfire-gtk3/share/crossfire-client/`

The `CF_DATADIR` macro is baked into `config.h` at configure time and controls where the client looks for all data files at runtime.

---

## Known Limitations and Future Work

- **CSS theming:** The bundled `themes/Standard` and `themes/Black` are GTK2 RC files and are not loaded. The client uses the system GTK3 theme. A `.gtk3.css` equivalent for each bundled theme has not been written.
- **`spell_get_styles()` / `inventory_get_styles()`:** These functions return null colors, leaving spell and inventory row colours at system defaults. A CSS-driven implementation would be the GTK3-idiomatic approach.
- **`use_stock` deprecation:** Stock IDs (`gtk-quit`, `gtk-ok`, etc.) are deprecated in GTK3 and will be removed in GTK4. Each should be replaced with a named icon (`GtkImage` + `icon-name`) and explicit label text.
- **`gtk_widget_override_background_color()` deprecation:** This function is deprecated in GTK 3.16+. The correct GTK3 replacement is a CSS provider attached to the specific widget.
- **Win32 / macOS cross-compilation:** The cmake structure supports it (X11 excluded on non-Linux), but no CI or packaging has been tested for those platforms in this branch.
