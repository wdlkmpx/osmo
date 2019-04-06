
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _NOTES_H
#define _NOTES_H

#include "gui.h"

#define NOTES_NAME                  "osmo_notes"
#define NOTES_ENTRIES_NAME          "notes_entries"
#define NOTES_CATEGORY_ENTRIES_NAME "category_entries"
#define NOTES_DIRNAME               "notes"
#define NOTES_ENTRIES_FILENAME      "notes_entries.xml"

void        notes_show_selector_editor  (gint mode, GUI *appGUI);
void        gui_create_notes            (GUI *appGUI);
void        update_notes_items          (GUI *appGUI);
void        refresh_notes               (GUI *appGUI);
gboolean    check_if_encrypted          (gchar *filename, GUI *appGUI);
void        check_notes_type            (GUI *appGUI);
void        store_note_columns_info     (GUI *appGUI);
void        set_note_columns_width      (GUI *appGUI);

#endif /* _NOTES_H */

