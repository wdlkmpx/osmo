/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Mąka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Mąka <silloz@users.sourceforge.net>
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

#ifndef _TASKS_UTILS_H
#define _TASKS_UTILS_H

#include "tasks.h"
#include "utils_date_time.h"

enum {
	STATE_NONE = 0,
	STATE_CALENDAR,
	STATE_TASKS,
	STATE_EITHER
};

gboolean tsk_check_tasks           (guint32 julian_start, guint32 julian_end, gint type, GUI *appGUI);
void     tsk_tasks_foreach         (guint32 julian_start, guint32 julian_end, gboolean (*ttfunc)(), GUI *appGUI);
gint     tsk_get_tasks_num         (guint32 julian, gboolean check_only, gboolean show_done, gint hidden_category, GUI *appGUI);
char *   tsk_get_tasks_str         (guint32 julian, gboolean show_done, gint hidden_category, GUI *appGUI);

gboolean      tsk_get_category_state    (gchar *category_name, gint type, GUI *appGUI);
gchar *       tsk_get_priority_text     (gint index);
gint          tsk_get_priority_index    (gchar *text);
GtkTreeIter * tsk_get_iter              (gint id, GUI *appGUI);
TASK_ITEM *   tsk_get_item              (GtkTreeIter *iter, GUI *appGUI);
TASK_ITEM *   tsk_get_item_id           (guint id, GUI *appGUI);
void          tsk_item_free             (TASK_ITEM *item);

#endif /* _TASKS_UTILS_H */

