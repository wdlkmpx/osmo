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

#include "i18n.h"
#include "tasks_preferences_gui.h"
#include "tasks_utils.h"

#ifdef TASKS_ENABLED

/*------------------------------------------------------------------------------*/

gboolean
tsk_check_tasks (guint32 julian_start, guint32 julian_end, gint type, GUI *appGUI)
{
	GtkTreeModel *model = NULL;
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *category;
	guint32 julian;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	if (model == NULL) return FALSE;

	path = gtk_tree_path_new_first ();

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {

		gtk_tree_model_get (model, &iter,
		                    TA_COLUMN_DUE_DATE_JULIAN, &julian,
		                    TA_COLUMN_CATEGORY, &category, -1);

		if (julian >= julian_start && julian <= julian_end)
			if (tsk_get_category_state (category, type, appGUI) == TRUE) {
				g_free (category);
				gtk_tree_path_free (path);
				return TRUE;
			}

		g_free (category);
		gtk_tree_path_next (path);
	}

	gtk_tree_path_free (path);

	return FALSE;
}

/*------------------------------------------------------------------------------*/

void
tsk_tasks_foreach (guint32 julian_start, guint32 julian_end, gboolean (*ttfunc)(), GUI *appGUI)
{
	GtkTreeModel *model = NULL;
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *category;
	guint32 julian;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	if (model == NULL) return;

	path = gtk_tree_path_new_first ();

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {

		gtk_tree_model_get (model, &iter,
		                    TA_COLUMN_DUE_DATE_JULIAN, &julian,
		                    TA_COLUMN_CATEGORY, &category, -1);

		if (julian >= julian_start && julian <= julian_end)
			if (tsk_get_category_state (category, STATE_EITHER, appGUI) == TRUE) {
				if ((*ttfunc)(julian, model, &iter, appGUI) == TRUE) {
					g_free (category);
					break;
				}
			}

		g_free (category);
		gtk_tree_path_next (path);
	}

	gtk_tree_path_free (path);
}

/*------------------------------------------------------------------------------*/

gint
tsk_get_tasks_num (guint32 julian, gboolean check_only, gboolean show_done, gint hidden_category, GUI *appGUI)
{
	GtkTreeModel *model = NULL;
	GtkTreePath *path;
	GtkTreeIter iter;
	guint32 tsk_julian;
	gchar *buf;
	gint tasks = 0;
	gint done;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	g_return_val_if_fail (model != NULL, 0);

	path = gtk_tree_path_new_first ();

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {

		gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_DATE_JULIAN, &tsk_julian,
		                                  TA_COLUMN_CATEGORY, &buf,
		                                  TA_COLUMN_DONE, &done, -1);

		if (tsk_julian == julian && (show_done || !done) && tsk_get_category_state (buf, hidden_category, appGUI)) {
			tasks++;
		}

		g_free (buf);
		if (check_only && tasks) break;
		gtk_tree_path_next (path);

	}

	gtk_tree_path_free (path);

	return tasks;
}

/*------------------------------------------------------------------------------*/

char *
tsk_get_tasks_str (guint32 julian, gboolean show_done, gint hidden_category, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;

	char buffer[BUFFER_SIZE], tmpbuf[BUFFER_SIZE];
	char *summ, *categ;
	guint32 tsk_julian;
	gint time;
	gint done;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	g_return_val_if_fail (model != NULL, NULL);

	path = gtk_tree_path_new_first ();
	buffer[0] = '\0';

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {

		gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_DATE_JULIAN, &tsk_julian,
		                                  TA_COLUMN_CATEGORY, &categ,
		                                  TA_COLUMN_DONE, &done, -1);

		if (tsk_julian == julian && (show_done || !done) && tsk_get_category_state (categ, hidden_category, appGUI)) {
			gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_TIME, &time, TA_COLUMN_SUMMARY, &summ, -1);

			if (time >= 0) {
				g_snprintf (tmpbuf, BUFFER_SIZE, "\n[%02d:%02d] %s", time / 3600, time / 60 % 60, summ);
			} else {
				g_snprintf (tmpbuf, BUFFER_SIZE, "\n%s", summ);
			}

			g_strlcat (buffer, tmpbuf, BUFFER_SIZE);
			g_free (summ);
		}

		g_free (categ);
		gtk_tree_path_next (path);
	}

	gtk_tree_path_free (path);
	g_strstrip (buffer);

	return g_strdup (buffer);
}

/*------------------------------------------------------------------------------*/

gboolean
tsk_get_category_state (gchar *category_name, gint type, GUI *appGUI)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	gboolean state, state_calendar, state_tasks;
	gchar *name;
	gint i;

	if (type == STATE_NONE || category_name == NULL)
		return TRUE;

	model = GTK_TREE_MODEL (appGUI->opt->tasks_category_store);
	g_return_val_if_fail (model != NULL, TRUE);

	i = 0;
	while (gtk_tree_model_iter_nth_child (model, &iter, NULL, i++)) {
		gtk_tree_model_get (model, &iter, TC_COLUMN_NAME, &name, -1);

		if (name != NULL && !strcmp (category_name, name)) {

			gtk_tree_model_get (model, &iter,
			                    TC_COLUMN_CALENDAR, &state_calendar,
			                    TC_COLUMN_TASKS, &state_tasks,
			                    -1);

			state = (type == STATE_CALENDAR && state_calendar) ||
			        (type == STATE_TASKS && state_tasks) ||
			        (type == STATE_EITHER && (state_calendar || state_tasks));

			g_free (name);
			return state;
		}

		g_free (name);
	}

	return TRUE;
}

/*------------------------------------------------------------------------------*/

gchar *
tsk_get_priority_text (gint index)
{
	gchar *priority_table[] = {
		N_("Low"), N_("Medium"), N_("High")
	};

	g_return_val_if_fail (index >= LOW_PRIORITY && index <= HIGH_PRIORITY, NULL);
	return gettext (priority_table[index]);
}

/*------------------------------------------------------------------------------*/

gint
tsk_get_priority_index (gchar *text)
{
	gchar *priority_table[] = {
		N_("Low"), N_("Medium"), N_("High")
	};
	gint i;

/*  FIXME */
/*	g_return_val_if_fail (text != NULL, -1);*/
	if (text == NULL) return -1;

	for (i = 0; i < 3; i++) {
		if (!strcmp (gettext (priority_table[i]), text))
			break;
	}

	return (i == 3 ? -1: i);
}

/*------------------------------------------------------------------------------*/

GtkTreeIter *
tsk_get_iter (gint id, GUI *appGUI)
{
	static GtkTreeIter iter;
	GtkTreeModel *model;
	gint i, n;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	g_return_val_if_fail (model != NULL, NULL);

	i = 0;
	while (gtk_tree_model_iter_nth_child (model, &iter, NULL, i++)) {
		gtk_tree_model_get (model, &iter, TA_COLUMN_ID, &n, -1);
		if (n == id)
			return &iter;
	}

	return NULL;
}

/*============================================================================*/

TASK_ITEM *
tsk_get_item_id (guint id, GUI *appGUI)
{
	GtkTreeIter *iter;

	iter = tsk_get_iter (id, appGUI);
	if (iter == NULL) return NULL;

	return tsk_get_item (iter, appGUI);
}

/*============================================================================*/

TASK_ITEM *
tsk_get_item (GtkTreeIter *iter, GUI *appGUI)
{
	GtkTreeModel *model;
	TASK_ITEM *item;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	g_return_val_if_fail (model != NULL, NULL);

	item = g_new0 (TASK_ITEM, 1);

	gtk_tree_model_get (model, iter,
	                    TA_COLUMN_ID, &(item->id),
	                    TA_COLUMN_DONE, &(item->done),
	                    TA_COLUMN_DUE_DATE_JULIAN, &(item->due_date_julian),
	                    TA_COLUMN_DUE_TIME, &(item->due_time),
	                    TA_COLUMN_START_DATE_JULIAN, &(item->start_date_julian),
	                    TA_COLUMN_DONE_DATE_JULIAN, &(item->done_date_julian),
	                    TA_COLUMN_ACTIVE, &(item->active),
	                    TA_COLUMN_OFFLINE_IGNORE, &(item->offline_ignore),
	                    TA_COLUMN_SOUND_ENABLE, &(item->sound_enable),
	                    TA_COLUMN_REPEAT, &(item->repeat),
	                    TA_COLUMN_REPEAT_DAY, &(item->repeat_day),
	                    TA_COLUMN_REPEAT_MONTH_INTERVAL, &(item->repeat_month_interval),
	                    TA_COLUMN_REPEAT_DAY_INTERVAL, &(item->repeat_day_interval),
	                    TA_COLUMN_REPEAT_START_DAY, &(item->repeat_start_day),
	                    TA_COLUMN_REPEAT_TIME_START, &(item->repeat_time_start),
	                    TA_COLUMN_REPEAT_TIME_END, &(item->repeat_time_end),
	                    TA_COLUMN_REPEAT_TIME_INTERVAL, &(item->repeat_time_interval),
	                    TA_COLUMN_REPEAT_COUNTER, &(item->repeat_counter),
	                    TA_COLUMN_WARNING_DAYS, &(item->warning_days),
	                    TA_COLUMN_WARNING_TIME, &(item->warning_time),
	                    TA_COLUMN_POSTPONE_TIME, &(item->postpone_time),
	                    TA_COLUMN_PRIORITY, &(item->priority),
	                    TA_COLUMN_ALARM_COMMAND, &(item->alarm_command),
	                    TA_COLUMN_CATEGORY, &(item->category),
	                    TA_COLUMN_SUMMARY, &(item->summary),
	                    TA_COLUMN_DESCRIPTION, &(item->desc),
	                    -1);

	return item;
}

/*------------------------------------------------------------------------------*/

void
tsk_item_free (TASK_ITEM *item)
{
	g_return_if_fail (item != NULL);

	g_free (item->alarm_command);
	g_free (item->priority);
	g_free (item->category);
	g_free (item->summary);
	g_free (item->desc);
	g_free (item);

	item = NULL;
}

/*------------------------------------------------------------------------------*/

#endif  /* TASKS_ENABLED */

