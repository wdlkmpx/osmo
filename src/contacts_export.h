
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

#ifndef _CONTACTS_EXPORT_H
#define _CONTACTS_EXPORT_H

#include "gui.h"

enum {
    EXPORT_TO_CSV = 0,
    EXPORT_TO_XHTML
};

enum {
    SELECT_ALL = 0,
    SELECT_NONE,
    SELECT_INVERT
};

gboolean    export_contacts_to_file         (gchar *filename, gboolean header, GUI *appGUI);
void        contacts_create_export_window   (GUI *appGUI);

#endif /* _CONTACTS_EXPORT_H */

