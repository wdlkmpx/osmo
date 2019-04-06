/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Maka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Maka <silloz@users.sourceforge.net>
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

#ifndef _ABOUT_H
#define _ABOUT_H

#include "gui.h"

#define     OSMO_WEBSITE        "http://clayo.org/osmo"
#define     MAILING_LIST        "http://sourceforge.net/mail/?group_id=206587"
#define     BUG_TRACKER         "http://sourceforge.net/tracker/?group_id=206587&atid=998196"
#define     FEATURE_REQUESTS    "http://sourceforge.net/tracker/?group_id=206587&atid=998199"

typedef struct {
	gchar *tag;
	gchar *name;
	gchar *email;
} OsmoCreditEntry;

GtkWidget * opt_create_about_window (GUI *appGUI);

#endif /* _ABOUT_H */

