/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, see the
 * 'LICENSE' and 'COPYING' files.
 *
 * The authors can be reached via e-mail to crossfire-devel@real-time.com
 */

/**
 * @file
 * Covers drawing the magic map.
 *
 * GTK3 port notes:
 *   - Replaced "expose_event" with GTK3 "draw" signal; the draw callback
 *     receives a cairo_t* directly so gdk_cairo_create() is not needed.
 *   - Removed deprecated gdk_window_clear(); background is painted black
 *     in the draw callback with cairo_paint().
 *   - Replaced deprecated gdk_cairo_set_source_color(GdkColor*) with
 *     gdk_cairo_set_source_rgba(GdkRGBA*) to match the root_color[] type.
 *   - draw_magic_map() and magic_map_flash_pos() now queue a redraw via
 *     gtk_widget_queue_draw() so all rendering occurs in on_drawingarea_magic_map_draw().
 */

#include <gtk/gtk.h>

#include "client.h"
#include "main.h"

/** Whether a position flash is pending on the next draw. */
static gboolean magic_map_pending_flash = FALSE;

/** Flash color: TRUE → white (color[1]), FALSE → black (color[0]). */
static gboolean magic_map_flash_on = FALSE;

/**
 * Render the full magic map into a cairo context.
 * Paints a black background then fills each cell with the appropriate color.
 *
 * @param cr            Cairo context provided by the draw callback.
 * @param window_width  Width of the drawing area in pixels.
 * @param window_height Height of the drawing area in pixels.
 */
static void render_magic_map(cairo_t *cr, int window_width, int window_height) {
    if (!cpl.magicmap) {
        return;
    }

    cpl.mapxres = window_width / cpl.mmapx;
    cpl.mapyres = window_height / cpl.mmapy;
    if (cpl.mapxres < 1 || cpl.mapyres < 1) {
        LOG(LOG_WARNING, "render_magic_map",
            "magic map resolution less than 1, map is %dx%d", cpl.mmapx,
            cpl.mmapy);
        return;
    }

    /* Keep cells square by using the smaller dimension for both axes. */
    if (cpl.mapxres > cpl.mapyres) {
        cpl.mapxres = cpl.mapyres;
    } else {
        cpl.mapyres = cpl.mapxres;
    }

    /* Clear background to black (replaces the GTK2 gdk_window_clear() call). */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    for (int y = 0; y < cpl.mmapy; y++) {
        for (int x = 0; x < cpl.mmapx; x++) {
            guint8 val = cpl.magicmap[y * cpl.mmapx + x];
            /* GTK3: gdk_cairo_set_source_rgba() with GdkRGBA. */
            gdk_cairo_set_source_rgba(cr, &root_color[val & FACE_COLOR_MASK]);
            cairo_rectangle(cr, cpl.mapxres * x, cpl.mapyres * y,
                            cpl.mapxres, cpl.mapyres);
            cairo_fill(cr);
        }
    }
}

/**
 * Render the player-position flash rectangle over the already-drawn magic map.
 *
 * @param cr Cairo context provided by the draw callback.
 */
static void render_magic_map_flash(cairo_t *cr) {
    int color_index = magic_map_flash_on ? 1 : 0;
    gdk_cairo_set_source_rgba(cr, &root_color[color_index]);
    cairo_rectangle(cr, cpl.mapxres * cpl.pmapx, cpl.mapyres * cpl.pmapy,
                    cpl.mapxres, cpl.mapyres);
    cairo_fill(cr);
}

/**
 * Request a full redraw of the magic map drawing area.
 * Switches the notebook to the magic map page and queues a redraw; the
 * actual rendering happens inside on_drawingarea_magic_map_draw().
 */
void draw_magic_map() {
    if (!cpl.magicmap) {
        return;
    }
    cpl.showmagic = 1;

    gtk_notebook_set_current_page(GTK_NOTEBOOK(map_notebook), MAGIC_MAP_PAGE);
    gtk_widget_queue_draw(magic_map);
}

/**
 * Flash the player's position on the magic map.
 * Sets a pending-flash flag and alternates the flash color, then queues a
 * redraw so on_drawingarea_magic_map_draw() handles the paint.
 */
void magic_map_flash_pos() {
    magic_map_flash_on = (cpl.showmagic & 2) ? FALSE : TRUE;
    magic_map_pending_flash = TRUE;
    gtk_widget_queue_draw(magic_map);
}

/**
 * GTK3 "draw" signal callback for the magic map drawing area.
 * Replaces the GTK2 "expose_event" callback.  @p cr is provided by GTK and
 * is already set up and clipped; no gdk_cairo_create() call is needed.
 *
 * @param widget    The magic map GtkDrawingArea.
 * @param cr        Cairo context valid for this draw cycle.
 * @param user_data Unused.
 * @return FALSE to allow default GTK processing.
 */
gboolean on_drawingarea_magic_map_draw(GtkWidget *widget, cairo_t *cr,
                                       gpointer user_data) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    render_magic_map(cr, alloc.width, alloc.height);

    if (magic_map_pending_flash) {
        render_magic_map_flash(cr);
        magic_map_pending_flash = FALSE;
    }

    return FALSE;
}
