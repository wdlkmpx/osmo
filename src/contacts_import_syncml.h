
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
 *           (C) 2008 Markus Dahms <mad@automagically.de>
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

#ifndef _CONTACTS_IMPORT_SYNCML_H
#define _CONTACTS_IMPORT_SYNCML_H

#include "gui.h"

#ifdef HAVE_LIBSYNCML

#include <libsyncml/syncml.h>
#include <libsyncml/sml_auth.h>
#include <libsyncml/sml_devinf_obj.h>
#include <libsyncml/sml_ds_server.h>
#include <libsyncml/obex_client.h>

typedef struct {
	/* configuration stuff */
	SmlTransportObexClientType oc_type;
	gboolean wbxml;
	gchar *url;
	guint32 port;
	/* session stuff */
	GSList *sml_sessions;
	SmlManager *sml_manager;
	SmlTransport *sml_client;
	/* OSMO stuff */
	GUI *gui;
} OsmoSyncMLData;


#define HANDLE_ERROR do { \
	g_warning("SyncML error: %s", smlErrorPrint(&smlerror)); \
	smlErrorDeref(&smlerror); \
	return FALSE; \
	} while(0);

#endif /* HAVE_LIBSYNCML */

gboolean        import_contacts_from_syncml     (gint type, GUI *appGUI);

#endif /* _CONTACTS_IMPORT_SYNCML_H */

