
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

#ifndef _CONTACTS_IMPORT_H
#define _CONTACTS_IMPORT_H

#include "gui.h"

enum {
    IMPORT_TYPE_FILE = 0,
};

void        import_contacts_show_dialog     (GUI *appGUI);
gboolean    import_contacts_from_csv_file   (GUI *appGUI);
gboolean    add_csv_records                 (gchar *filename, GUI *appGUI);

#endif /* _CONTACTS_IMPORT_H */

