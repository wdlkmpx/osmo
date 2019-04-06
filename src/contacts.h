
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

#ifndef _CONTACTS_H
#define _CONTACTS_H

#include "gui.h"

#define CONTACTS_NAME               "osmo_contacts"
#define CONTACTS_ENTRIES_NAME       "contacts_records"
#define CONTACTS_GROUP_ENTRIES_NAME "contacts_groups"
#define CONTACTS_ENTRIES_FILENAME   "contacts_records.xml"

#define GOOGLE_MAPS_QUERY           "http://maps.google.com/maps?q="

/* photo widths */
#define PHOTO_SMALL     80
#define PHOTO_MEDIUM    120
#define PHOTO_LARGE     160

enum {
    CONTACTS_FF_FIRST_NAME = 0,
    CONTACTS_FF_LAST_NAME,
    CONTACTS_FF_TAGS,
    CONTACTS_FF_ALL_FIELDS
};

enum {
	HOME_ADDRESS = 0,
	WORK_ADDRESS
};

void        gui_create_contacts                     (GUI *appGUI);
void        show_contacts_desc_panel                (gboolean enable, GUI *appGUI);
void        set_export_active                       (GUI *appGUI);
void        store_contact_columns_info              (GUI *appGUI);
void        set_contacts_columns_width              (GUI *appGUI);
void        contacts_select_first_position_in_list  (GUI *appGUI);
void        insert_photo                            (gchar *photo_filename, GtkTextIter *iter, GUI *appGUI);
gboolean    check_address                           (gint address_type, GUI *appGUI);
void        show_contact_location_on_map            (gint address_type, GUI *appGUI);
void        contacts_selection_activate             (gboolean active, GUI *appGUI);

#endif /* _CONTACTS_H */


