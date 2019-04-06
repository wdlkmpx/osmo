
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

#ifndef _TASKS_ITEMS_H
#define _TASKS_ITEMS_H

#include "gui.h"
#include "tasks.h"

void            read_tasks_entries          (GUI *appGUI);
void            write_tasks_entries         (GUI *appGUI);
void            tasks_remove_dialog_show    (GtkWidget *list, GtkListStore *list_store, GUI *appGUI);
void            tasks_add_edit_dialog_show  (gboolean tasks_edit_mode, guint32 julian_date, gint time, GUI *appGUI);
void            tsk_delete_notify           (guint id, GUI *appGUI);

#endif /* _TASKS_ITEMS_H */

