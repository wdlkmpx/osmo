
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

#ifndef _CHECK_EVENTS_H
#define _CHECK_EVENTS_H

#include "gui.h"
#include "tasks.h"

enum {
    CE_COLUMN_DATE = 0,
    CE_COLUMN_DATE_JULIAN,
    CE_COLUMN_EVENT_TYPE,
    CE_COLUMN_EVENT_LINE,
    CHECK_EVENTS_NUM_COLUMNS
};

enum {
	NOTIFY_WARNING = 0,
	NOTIFY_ALARM
};

#ifdef HAVE_LIBNOTIFY
#ifdef TASKS_ENABLED
typedef struct {
	guint id;
	gint type;
	NotifyNotification *notify;
	guint32 date;
	gint time;
} TASK_NTF;
#endif  /* TASKS_ENABLED */
#endif  /* HAVE_LIBNOTIFY */

void        free_notifications_list         (GUI *appGUI);
gboolean    time_handler                    (GUI *appGUI);
gboolean    check_contacts                  (guint32 julian_day, GUI *appGUI);
gboolean    check_tasks_contacts            (guint32 julian_day, GUI *appGUI);
gboolean    create_event_checker_window     (GUI *appGUI);

#endif /* _CHECK_EVENTS_H */

