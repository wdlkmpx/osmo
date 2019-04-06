
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

#include "tasks_export.h"
#include "i18n.h"
#include "utils.h"
#include "utils_gui.h"
#include "tasks.h"
#include "tasks_utils.h"
#include "calendar_utils.h"
#include "options_prefs.h"

#if defined(TASKS_ENABLED) && defined(HAVE_LIBICAL)

#include <libical/ical.h>
#include <libical/icalss.h>
#include <libical/icalset.h>
#include <libical/icalclassify.h>

/*-------------------------------------------------------------------------------------*/

gboolean    
export_tasks_to_file (GUI *appGUI) 
{
gchar tmpbuf[BUFFER_SIZE];
gint n, ret;
gint day, month, year, due_time;
guint32 julian_day;
gchar *filename, *summary, *date, *priority, *description, *category;
GtkWidget *dialog;
GtkTreePath *sort_path, *filter_path, *path;
GtkTreeIter iter;
icalset *ics_file;
struct icaltimetype atime;
icalcomponent *event, *calendar;
icalproperty *prop;

	if (get_number_of_visible_tasks_with_date (appGUI) == 0) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, 
							   _("No tasks with defined date found."), GTK_WINDOW(appGUI->main_window));
		return FALSE;
	}

	dialog = gtk_file_chooser_dialog_new (_("Save tasks"),
                                          GTK_WINDOW(appGUI->main_window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(dialog), FALSE);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(dialog), 
									   utl_add_timestamp_to_filename ("tasks", "ics"));
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(dialog), TRUE);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if (ret == GTK_RESPONSE_CANCEL || ret == GTK_RESPONSE_DELETE_EVENT) {
        gtk_widget_destroy(dialog);
		return FALSE;
	}

	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);

	if (utl_gui_check_overwrite_file (filename, appGUI->main_window, appGUI) != 0) {
        return FALSE;
    } else {
        g_unlink (filename);
    }

	ics_file = icalset_new_file (filename); 

    if (ics_file == NULL) {
        g_free (filename);
        return FALSE;
    }

	calendar = icalcomponent_new (ICAL_VCALENDAR_COMPONENT);

    prop = icalproperty_new_prodid ("//Clay//NONSGML Osmo PIM//EN");
    icalcomponent_add_property (calendar, prop);
    prop = icalproperty_new_version (VERSION);
    icalcomponent_add_property (calendar, prop);

	sort_path = gtk_tree_path_new_first ();
	n = 0;

	while (gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter, sort_path) == TRUE) {

		if (sort_path != NULL) {
			filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->tsk->tasks_sort), sort_path);

			if (filter_path != NULL) {
				path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter), filter_path);

				if (path != NULL) {

					gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter, path);
					gtk_tree_model_get (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter,
										TA_COLUMN_SUMMARY, &summary,
										TA_COLUMN_DESCRIPTION, &description,
										TA_COLUMN_CATEGORY, &category,
										TA_COLUMN_DUE_DATE, &date,
										TA_COLUMN_DUE_TIME, &due_time,
										TA_COLUMN_DUE_DATE_JULIAN, &julian_day,
										TA_COLUMN_PRIORITY, &priority,
										-1);

					if (julian_day != 0) {

						event = icalcomponent_new (ICAL_VTODO_COMPONENT);

						utl_date_julian_to_dmy (julian_day, &day, &month, &year);
						atime.day = day;
						atime.month = month;
						atime.year = year;
						if (due_time == -1) {
							atime.is_date = TRUE;
						} else {
							atime.hour = due_time / 3600;
							atime.minute = (due_time % 3600) / 60;
							atime.second = 0;
							atime.is_date = FALSE;
						}

						g_snprintf (tmpbuf, BUFFER_SIZE, "id%d%d%d%d@clayo.org", year, month, day, n);
						prop = icalproperty_new_uid (tmpbuf);
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);
						prop = icalproperty_new_priority (tsk_get_priority_index(priority));
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);

                        if (g_utf8_collate(category, _("None"))) {
							prop = icalproperty_new_categories (category);
							icalcomponent_add_property (event, prop);
							icalproperty_free (prop);
						}

						prop = icalproperty_new_summary (summary);
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);
						prop = icalproperty_new_description (description);
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);
						prop = icalproperty_new_dtstamp (atime);
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);
						prop = icalproperty_new_dtstart (atime);
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);
						prop = icalproperty_new_dtend (atime);
						icalcomponent_add_property (event, prop);
						icalproperty_free (prop);

						icalcomponent_add_component (calendar, event);

						icalcomponent_free (event);
						g_free (summary);
						g_free (description);
						g_free (category);

						n++;
					}
					gtk_tree_path_free (path);
				}

				gtk_tree_path_free (filter_path);
			}

		}

		gtk_tree_path_next (sort_path);
	}

    icalset_add_component (ics_file, calendar);
    icalfileset_commit (ics_file);

    icalcomponent_free (calendar);
	g_free(filename);

	sprintf (tmpbuf, "%d %s", n, _("tasks exported"));
	utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW(appGUI->main_window));

	return TRUE;
}

/*-------------------------------------------------------------------------------------*/

#endif  /* TASKS_ENABLED */

