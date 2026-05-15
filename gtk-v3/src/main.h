/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file gtk-v3/src/main.h
 * Global definitions, widget pointers, and XML path defaults for the GTK3 client.
 * Uses GdkRGBA instead of the GTK2 GdkColor type.
 */

#ifndef MAIN_H
#define MAIN_H

/** Number of named colors used for map, magic map, and text coloring. */
#define NUM_COLORS 13

/** Global color table; indexed by FACE_COLOR_MASK and related constants. */
extern GdkRGBA root_color[NUM_COLORS];

extern GtkWidget *window_root, *spinbutton_count;
extern GtkBuilder *dialog_xml, *window_xml;

extern GtkNotebook *main_notebook;

extern GtkWidget *magic_map;
extern GtkWidget *map_notebook;
extern GtkWidget *connect_window;

#define DEFAULT_IMAGE_SIZE      32
extern int map_image_size, image_size;

/** Path to the current UI file. */
extern char window_xml_file[MAX_BUF];

#define MAGIC_MAP_PAGE  1 /**< Notebook page of the magic map */

extern char account_password[256];
/* gtk3proto.h depends on this - so may as well just include it here */
#include "info.h"

extern void hide_main_client(void);

#endif
