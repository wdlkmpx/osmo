/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Mąka <pasp@users.sourceforge.net>
 *           (C) 2007-2009 Piotr Mąka <silloz@users.sourceforge.net>
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

#include "about.h"
#include "calendar.h"
#include "calendar_utils.h"
#include "calendar_widget.h"
#include "i18n.h"
#include "tasks_preferences_gui.h"
#include "options_prefs.h"
#include "preferences_gui.h"
#include "stock_icons.h"
#include "tasks.h"
#include "tasks_items.h"
#include "tasks_export.h"
#include "tasks_print.h"
#include "tasks_utils.h"
#include "utils.h"
#include "utils_gui.h"

#ifdef TASKS_ENABLED

/*============================================================================*/

static void
show_about_window_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkWidget *window = opt_create_about_window (appGUI);
	gtk_widget_show (window);
}

/*============================================================================*/

static void
show_preferences_window_cb (GtkWidget *widget, GUI *appGUI)
{
	appGUI->opt->window = opt_create_preferences_window (appGUI);
	gtk_widget_show (appGUI->opt->window);

	gint page = gtk_notebook_page_num (GTK_NOTEBOOK (appGUI->opt->notebook), appGUI->opt->tasks);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->opt->notebook), page);
}

/*============================================================================*/

gint
task_calculate_new_date (TASK_ITEM *item, GUI *appGUI, guint32 *new_date, gint *new_time)
{
GDate *cdate;
guint32 current_julian;
gint current_time;
gint i, cycles;
gboolean repeat_time, repeat_date, set_start_time;

	gboolean month_mode_flag = item->repeat_month_interval > 0 && item->repeat_day_interval == 0;

	repeat_time = repeat_date = FALSE;
	set_start_time = TRUE;

	cycles = 0;
	current_julian = utl_date_get_current_julian ();
	current_time = utl_time_get_current_seconds ();
	*new_date = item->due_date_julian;
	*new_time = item->due_time;

	if (item->repeat_time_start < item->repeat_time_end && item->repeat_time_interval > 0 && item->due_time >= 0)
		repeat_time = TRUE;
	
	if (item->repeat_month_interval > 0 || item->repeat_day_interval > 0)
		repeat_date = TRUE;

	if (repeat_time) {
		for (i = item->repeat_time_start; i <= item->repeat_time_end; i += item->repeat_time_interval) {
			if (i * 60 > item->due_time && i * 60 > current_time) {
				*new_time = i * 60;
				set_start_time = FALSE;
				break;
			}
		}
		if (set_start_time && repeat_date)
			*new_time = item->repeat_time_start * 60;
	}

	if (*new_time < current_time || item->due_time < 0) current_julian++;

	if (item->due_date_julian < current_julian && repeat_date) {
		cdate = g_date_new_julian (item->due_date_julian);

		while (*new_date < current_julian) {
			g_date_add_months (cdate, item->repeat_month_interval);
			g_date_add_days (cdate, item->repeat_day_interval);

			if (month_mode_flag) {
				utl_date_set_valid_day (cdate, item->repeat_start_day);
				utl_date_set_nearest_weekday (cdate, item->repeat_day, TRUE);
			} else
				utl_date_set_nearest_weekday (cdate, item->repeat_day, FALSE);

			*new_date = g_date_get_julian (cdate);
			cycles++;

			if (item->repeat_counter != 0 && cycles >= item->repeat_counter) {
				g_date_free (cdate);
				return cycles;
			}
		}

		if (repeat_time) *new_time = item->repeat_time_start * 60;
		g_date_free (cdate);
	}

	return cycles;
}

/*------------------------------------------------------------------------------*/

void
tasks_repeat_done (GtkTreeIter *iter, TASK_ITEM *item, GUI *appGUI)
{
guint32 new_date;
gint new_time;
gint cycles;

	g_return_if_fail (item->repeat == TRUE);

	cycles = task_calculate_new_date (item, appGUI, &new_date, &new_time);

	if (item->repeat_counter == 0 || item->repeat_counter > cycles) {

		if (item->repeat_counter > cycles) {
			gtk_list_store_set (appGUI->tsk->tasks_list_store, iter,
			                    TA_COLUMN_REPEAT_COUNTER, item->repeat_counter - cycles, -1);
		}

		if (new_date == item->due_date_julian && new_time == item->due_time) {

			if (config.delete_completed) {
				gtk_list_store_remove (appGUI->tsk->tasks_list_store, iter);
			} else {
				gtk_list_store_set (appGUI->tsk->tasks_list_store, iter,
				                    TA_COLUMN_COLOR, get_date_color (item->due_date_julian, item->due_time, TRUE, appGUI),
				                    TA_COLUMN_DONE, TRUE, -1);
			}

		} else {
			gtk_list_store_set (appGUI->tsk->tasks_list_store, iter,
			                    TA_COLUMN_COLOR, get_date_color (new_date, new_time, FALSE, appGUI),
			                    TA_COLUMN_DUE_DATE_JULIAN, new_date,
			                    TA_COLUMN_DUE_DATE, get_date_time_full_str (new_date, new_time),
			                    TA_COLUMN_DUE_TIME, new_time,
			                    TA_COLUMN_DONE, FALSE, -1);
		}

	} else {

		if (config.delete_completed) {
			gtk_list_store_remove (appGUI->tsk->tasks_list_store, iter);
		} else {
			gtk_list_store_set (appGUI->tsk->tasks_list_store, iter,
			                    TA_COLUMN_REPEAT_COUNTER, 0,
			                    TA_COLUMN_COLOR, get_date_color (new_date, new_time, TRUE, appGUI),
			                    TA_COLUMN_DUE_DATE_JULIAN, new_date,
			                    TA_COLUMN_DUE_DATE, get_date_time_full_str (new_date, new_time),
			                    TA_COLUMN_DUE_TIME, new_time,
			                    TA_COLUMN_DONE, TRUE, -1);
		}

	}

}

/*------------------------------------------------------------------------------*/

void
show_tasks_desc_panel (gboolean enable, GUI *appGUI) {

GtkTreeIter  iter;
GdkRectangle rect, visible_rect;
GtkTreePath  *visible_path;
GtkTreeModel *model;

    if(enable == TRUE) {

        if (gtk_tree_selection_get_selected (appGUI->tsk->tasks_list_selection, &model, &iter)) {

            gtk_paned_set_position(GTK_PANED(appGUI->tsk->tasks_paned), config.tasks_pane_pos);

            while (g_main_context_iteration(NULL, FALSE));

            visible_path = gtk_tree_model_get_path (model, &iter);

            if (visible_path) {

                gtk_tree_view_get_cell_area (GTK_TREE_VIEW (appGUI->tsk->tasks_list), visible_path, NULL, &rect);
                gtk_tree_view_get_visible_rect (GTK_TREE_VIEW (appGUI->tsk->tasks_list), &visible_rect);

                if (rect.y < visible_rect.y || rect.y > visible_rect.y + visible_rect.height) {
                        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (appGUI->tsk->tasks_list), visible_path, NULL, TRUE, 0.5, 0.0);
                }

                gtk_tree_path_free(visible_path);
            }

        } else {
            enable = FALSE;
        }

    } else {

        config.tasks_pane_pos = gtk_paned_get_position(GTK_PANED(appGUI->tsk->tasks_paned));
		if (!config.gui_layout) {
	        gtk_paned_set_position(GTK_PANED(appGUI->tsk->tasks_paned), 99999);
		}

    }

    appGUI->tsk->tasks_panel_status = enable;
}

/*------------------------------------------------------------------------------*/

void
panel_close_desc_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    show_tasks_desc_panel(FALSE, appGUI);
}

/*------------------------------------------------------------------------------*/

void
tasks_item_selected (GtkTreeSelection *selection, gpointer data) {

GtkTreeIter iter;
GtkTreeModel *model;
gchar *text;
GtkTextIter titer;
GtkTextBuffer *text_buffer;
GtkTextChildAnchor *anchor;
GtkWidget *hseparator;
guint32 start_date_julian, done_date_julian, due_date_julian;
gint due_time;
gchar tmpbuf[BUFFER_SIZE];
gboolean repeat, prev_state, next_state;

    GUI *appGUI = (GUI *)data;

    text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview));
    utl_gui_clear_text_buffer (text_buffer, &titer);

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

        gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/edit"), TRUE);
        gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/delete"), TRUE);

        gtk_tree_model_get (model, &iter, TA_COLUMN_DESCRIPTION, &text,
                            TA_COLUMN_DUE_DATE_JULIAN, &due_date_julian,
                            TA_COLUMN_START_DATE_JULIAN, &start_date_julian,
                            TA_COLUMN_DONE_DATE_JULIAN, &done_date_julian,
                            TA_COLUMN_DUE_TIME, &due_time,
                            TA_COLUMN_REPEAT, &repeat, -1);

		if (repeat == TRUE) {
			prev_state = FALSE;
			next_state = utl_date_time_in_the_past_js (due_date_julian, due_time);
		} else {
			prev_state = next_state = (due_date_julian != 0) ? TRUE : FALSE;
		}

        gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/prev_date"), prev_state);
        gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/next_date"), next_state);

        g_snprintf (tmpbuf, BUFFER_SIZE, "\n%s: %s\n", 
                    _("Started"), julian_to_str (start_date_julian, DATE_FULL, config.override_locale_settings));

        if (text != NULL) {
            gtk_text_buffer_insert_with_tags_by_name (text_buffer, &titer, text, -1,
                                                      "info_font", NULL);
            gtk_text_buffer_insert(text_buffer, &titer, "\n", -1);
        }

        if (start_date_julian) {
            gtk_text_buffer_insert(text_buffer, &titer, "\n", -1);
            anchor = gtk_text_buffer_create_child_anchor (text_buffer, &titer);
            gtk_text_buffer_insert_with_tags_by_name (text_buffer, &titer, tmpbuf, -1, 
                                                      "italic", NULL);
            if (done_date_julian != 0) {
                g_snprintf (tmpbuf, BUFFER_SIZE, "%s: %s\n", 
                            _("Finished"), julian_to_str (done_date_julian, DATE_FULL, config.override_locale_settings));
                gtk_text_buffer_insert_with_tags_by_name (text_buffer, &titer, tmpbuf, -1, 
                                                          "italic", NULL);
            }

            gtk_text_view_set_buffer (GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), text_buffer);

            hseparator = gtk_hseparator_new ();
            gtk_widget_show (hseparator);
			if (!config.gui_layout) {
                gtk_widget_set_size_request (hseparator, 320, -1);
			} else {
                gtk_widget_set_size_request (hseparator, 200, -1);
			}
            gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), hseparator, anchor);
        }

        g_free(text);
    } else {
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/edit"), FALSE);
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/delete"), FALSE);
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/prev_date"), FALSE);
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/next_date"), FALSE);
    }
}

/*------------------------------------------------------------------------------*/

void
tasks_add_item_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	tasks_add_edit_dialog_show (FALSE, 0, utl_time_get_current_seconds (), appGUI);
}

/*------------------------------------------------------------------------------*/

void
tasks_edit_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (gtk_tree_selection_get_selected (appGUI->tsk->tasks_list_selection, NULL, NULL)) {
        tasks_add_edit_dialog_show (TRUE, appGUI->tsk->tasks_due_julian_day, appGUI->tsk->tasks_due_time, appGUI);
    }
}

/*------------------------------------------------------------------------------*/

void
tasks_remove_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    tasks_remove_dialog_show(appGUI->tsk->tasks_list, appGUI->tsk->tasks_list_store, appGUI);
}

/*------------------------------------------------------------------------------*/

void
task_modify_due_date (gint value, GUI *appGUI)
{
GtkTreeIter iter, *e_iter;
GtkTreeModel *model;
guint32 due_date;
gint due_time, id;
TASK_ITEM *item;

	if (gtk_tree_selection_get_selected (appGUI->tsk->tasks_list_selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_DATE_JULIAN, &due_date,
		                    TA_COLUMN_DUE_TIME, &due_time, TA_COLUMN_ID, &id, -1);

		e_iter = tsk_get_iter (id, appGUI);
		g_return_if_fail (e_iter != NULL);

		item = tsk_get_item (e_iter, appGUI);
		g_return_if_fail (item != NULL);

		if (item->repeat == TRUE) {
			tasks_repeat_done (e_iter, item, appGUI);
		} else {
			due_date += value;
			gtk_list_store_set (appGUI->tsk->tasks_list_store, e_iter,
			                    TA_COLUMN_COLOR, get_date_color (due_date, due_time, FALSE, appGUI),
			                    TA_COLUMN_DUE_DATE_JULIAN, due_date,
			                    TA_COLUMN_DUE_DATE, get_date_time_full_str (due_date, due_time), -1);
		}

		tsk_item_free (item);
	}

}

/*------------------------------------------------------------------------------*/

void
tasks_change_due_date_to_prev_date_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    task_modify_due_date (-1, appGUI);
}

/*------------------------------------------------------------------------------*/

void
tasks_change_due_date_to_next_date_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    task_modify_due_date (1, appGUI);
}

/*------------------------------------------------------------------------------*/

#ifdef HAVE_LIBICAL
void
tasks_export_visible_items_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
	export_tasks_to_file (appGUI);
}
#endif /* HAVE_LIBICAL */

/*------------------------------------------------------------------------------*/

#ifdef PRINTING_SUPPORT
void
tasks_print_visible_items_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    tasks_print (appGUI);
}
#endif /* PRINTING_SUPPORT */

/*------------------------------------------------------------------------------*/

void
done_toggled (GtkCellRendererToggle *cell, gchar *path_str, gpointer user_data)
{
GtkTreePath *sort_path, *filter_path, *path;
GtkTreeIter  iter;
gboolean done_status;
guint32 done_date, category;
GtkTreeModel *model;

    GUI *appGUI = (GUI *) user_data;
    model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);

	category = gtk_combo_box_get_active (GTK_COMBO_BOX(appGUI->tsk->cf_combobox));

    sort_path = gtk_tree_path_new_from_string (path_str);

    if (sort_path != NULL) {
        filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->tsk->tasks_sort), sort_path);

        if (filter_path != NULL) {
            path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter), filter_path);

            if (path != NULL) {
                gtk_tree_model_get_iter (model, &iter, path);   /* get toggled iter */
                gtk_tree_model_get (model, &iter, TA_COLUMN_DONE, &done_status, -1);

                if (done_status == FALSE) {
                    done_date = utl_date_get_current_julian ();
                } else {
                    done_date = 0;
                }

                gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                                    TA_COLUMN_DONE, !done_status, 
                                    TA_COLUMN_DONE_DATE_JULIAN, done_date, -1);

                if (done_status == FALSE && config.delete_completed) {
                    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
                }

                gtk_tree_path_free (path);
            }

            gtk_tree_path_free (filter_path);
        }

        gtk_tree_path_free (sort_path);
    }

    apply_task_attributes (appGUI);
    cal_set_day_info (appGUI);
	refresh_tasks (appGUI);

	gtk_combo_box_set_active (GTK_COMBO_BOX(appGUI->tsk->cf_combobox), category);

    if (config.save_data_after_modification) {
        write_tasks_entries (appGUI);
    }
}

/*------------------------------------------------------------------------------*/

gchar *
get_date_color (guint32 julian_day, gint time, gboolean done, GUI *appGUI)
{
	static gchar due_date_color[MAXCOLORNAME];
	GdkColor *color;
	gint current_time;
	gint32 r;

	current_time = utl_time_get_current_seconds ();

	color = &appGUI->main_window->style->text[GTK_STATE_NORMAL];
	g_snprintf (due_date_color, MAXCOLORNAME, "#%02X%02X%02X", color->red * 256 / 65536,
	            color->green * 256 / 65536, color->blue * 256 / 65536);

	if (julian_day != 0 && done == FALSE) {

		r = julian_day - utl_date_get_current_julian ();

		if (r == 0) {
			if (time >= 0 && current_time > time) {
				g_strlcpy (due_date_color, config.past_due_color, MAXCOLORNAME);
			}
			else {
				g_strlcpy (due_date_color, config.due_today_color, MAXCOLORNAME);
			}
		} else if (r > 0 && r < 7) {
			g_strlcpy (due_date_color, config.due_7days_color, MAXCOLORNAME);
		} else if (r < 0) {
			g_strlcpy (due_date_color, config.past_due_color, MAXCOLORNAME);
		}
	}

	return due_date_color;
}

/*------------------------------------------------------------------------------*/

void
apply_task_attributes (GUI *appGUI) {

GtkTreeIter iter;
gint i;
gboolean done;
gchar *priority;
guint32 julian_day;
gint time;

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, NULL, i++)) {

        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, 
                           TA_COLUMN_DONE, &done, TA_COLUMN_DUE_DATE_JULIAN, &julian_day, 
                           TA_COLUMN_DUE_TIME, &time, TA_COLUMN_PRIORITY, &priority, -1);

        if (tsk_get_priority_index (priority) == HIGH_PRIORITY && config.tasks_high_in_bold == TRUE) {    /* high priority ? */
            gtk_list_store_set(appGUI->tsk->tasks_list_store, &iter, 
                               TA_COLUMN_COLOR, get_date_color (julian_day, time, done, appGUI), 
                               TA_COLUMN_BOLD, PANGO_WEIGHT_BOLD,-1);
        } else {
            gtk_list_store_set(appGUI->tsk->tasks_list_store, &iter, 
                               TA_COLUMN_COLOR, get_date_color (julian_day, time, done, appGUI), 
                               TA_COLUMN_BOLD, PANGO_WEIGHT_NORMAL,-1);
        }

        g_free (priority);
    }
}

/*------------------------------------------------------------------------------*/

void
refresh_tasks (GUI *appGUI) {

GtkTreeIter iter;
gint i, n, due_time;
guint32 julian_day;
gchar *category;
gboolean done, tasks_state;

    n = utl_gui_get_combobox_items(GTK_COMBO_BOX (appGUI->tsk->cf_combobox));

    for (i = n-1; i >= 0; i--) {
        gtk_combo_box_remove_text (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), i);
    }
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), _("All items"));

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->tasks_category_store), &iter, NULL, i++)) {
        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->tasks_category_store), &iter, 
                           TC_COLUMN_NAME, &category, TC_COLUMN_TASKS, &tasks_state, -1);
        if (tasks_state == TRUE) {
            gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), category);
        }
        g_free(category);
    }

    if (config.remember_category_in_tasks == TRUE) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(appGUI->tsk->cf_combobox), config.current_category_in_tasks);
    } else {
        gtk_combo_box_set_active (GTK_COMBO_BOX(appGUI->tsk->cf_combobox), 0);
    }

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, NULL, i++)) {

        gtk_tree_model_get (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, 
                            TA_COLUMN_DONE, &done,
                            TA_COLUMN_DUE_DATE_JULIAN, &julian_day,
                            TA_COLUMN_DUE_TIME, &due_time,
                            -1);

        gtk_list_store_set (appGUI->tsk->tasks_list_store, &iter, 
                            TA_COLUMN_DUE_DATE, get_date_time_full_str (julian_day, due_time), 
                            TA_COLUMN_COLOR, get_date_color (julian_day, due_time, done, appGUI),
                            -1);
    }

    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter));
}

/*------------------------------------------------------------------------------*/

void
update_tasks_number (GUI *appGUI)
{
GtkTreeIter iter;
gint i;
gchar tmpbuf[BUFFER_SIZE];

	i = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->tsk->tasks_filter), &iter, NULL, i++));

	i--;

	if (i > 0) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<i>%4d %s</i>", i, ngettext ("entry", "entries", i));
#ifdef PRINTING_SUPPORT
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/print"), TRUE);
#endif /* PRINTING_SUPPORT */
#ifdef HAVE_LIBICAL
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/export"), TRUE);
#endif /* HAVE_LIBICAL */
	} else {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<i>%s</i>", _("no entries"));
#ifdef PRINTING_SUPPORT
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/print"), FALSE);
#endif /* PRINTING_SUPPORT */
#ifdef HAVE_LIBICAL
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/export"), FALSE);
#endif /* HAVE_LIBICAL */
	}

	gtk_label_set_markup (GTK_LABEL (appGUI->tsk->n_items_label), tmpbuf);

}

/*------------------------------------------------------------------------------*/

guint
get_number_of_visible_tasks_with_date (GUI *appGUI)
{
guint32 date, n;
GtkTreePath *sort_path, *filter_path, *path;
GtkTreeIter iter;

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
										TA_COLUMN_DUE_DATE_JULIAN, &date, -1);
					if (date != 0) n++;
					gtk_tree_path_free (path);
				}
				gtk_tree_path_free (filter_path);
			}
		}
		gtk_tree_path_next (sort_path);
	}

	return n;
}

/*------------------------------------------------------------------------------*/

void
add_item_to_list (TASK_ITEM *item, GUI *appGUI)
{
	GtkTreeIter iter;
	GdkPixbuf *image;
	GtkWidget *helper;
	const gchar *stock_id;
	gchar *date_time_str;

	gtk_list_store_append (appGUI->tsk->tasks_list_store, &iter);

	gtk_list_store_set (appGUI->tsk->tasks_list_store, &iter,
	                    TA_COLUMN_DONE, item->done,
	                    TA_COLUMN_DUE_DATE_JULIAN, item->due_date_julian,
	                    TA_COLUMN_START_DATE_JULIAN, item->start_date_julian,
	                    TA_COLUMN_DONE_DATE_JULIAN, item->done_date_julian,
	                    TA_COLUMN_DUE_TIME, item->due_time,
	                    TA_COLUMN_PRIORITY, item->priority,
	                    TA_COLUMN_CATEGORY, item->category,
	                    TA_COLUMN_SUMMARY, item->summary,
	                    TA_COLUMN_DESCRIPTION, item->desc,
	                    TA_COLUMN_SOUND_ENABLE, item->sound_enable,
	                    TA_COLUMN_COLOR, get_date_color (item->due_date_julian, item->due_time, item->done, appGUI),
	                    TA_COLUMN_ACTIVE, item->active,
	                    TA_COLUMN_OFFLINE_IGNORE, item->offline_ignore,
	                    TA_COLUMN_REPEAT, item->repeat,
	                    TA_COLUMN_REPEAT_DAY, item->repeat_day,
	                    TA_COLUMN_REPEAT_MONTH_INTERVAL, item->repeat_month_interval,
	                    TA_COLUMN_REPEAT_DAY_INTERVAL, item->repeat_day_interval,
	                    TA_COLUMN_REPEAT_START_DAY, item->repeat_start_day,
	                    TA_COLUMN_REPEAT_TIME_START, item->repeat_time_start,
	                    TA_COLUMN_REPEAT_TIME_END, item->repeat_time_end,
	                    TA_COLUMN_REPEAT_TIME_INTERVAL, item->repeat_time_interval,
	                    TA_COLUMN_REPEAT_COUNTER, item->repeat_counter,
	                    TA_COLUMN_ALARM_COMMAND, item->alarm_command,
	                    TA_COLUMN_WARNING_DAYS, item->warning_days,
	                    TA_COLUMN_WARNING_TIME, item->warning_time,
	                    TA_COLUMN_POSTPONE_TIME, item->postpone_time,
	                    TA_COLUMN_ID, item->id,
	                    -1);

    helper = gtk_image_new ();
    stock_id = (item->repeat == TRUE) ? OSMO_STOCK_TYPE_RECURRENT : OSMO_STOCK_TYPE_NORMAL;
    image = gtk_widget_render_icon (helper, stock_id, GTK_ICON_SIZE_MENU, NULL);
    gtk_list_store_set(appGUI->tsk->tasks_list_store, &iter, TA_COLUMN_TYPE, image, -1);
    g_object_unref (image);

	date_time_str = utl_date_time_print_default (item->due_date_julian, item->due_time, FALSE);
	gtk_list_store_set (appGUI->tsk->tasks_list_store, &iter, TA_COLUMN_DUE_DATE, date_time_str, -1);
	g_free (date_time_str);
}

/*------------------------------------------------------------------------------*/

gint
list_dbclick_cb(GtkWidget * widget, GdkEventButton * event, gpointer func_data) {

    GUI *appGUI = (GUI *)func_data;

    if ((event->type==GDK_2BUTTON_PRESS) && (event->button == 1)) {

        if (appGUI->tsk->tasks_panel_status == TRUE) {
            if (config.add_edit == FALSE) {
                tasks_edit_item_cb (NULL, appGUI);
            } else {
				tasks_add_edit_dialog_show (FALSE, 0, utl_time_get_current_seconds (), appGUI);
            }
        } else {
            show_tasks_desc_panel(TRUE, appGUI);
        }
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
tasks_list_filter_cb (GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {

gint done, idx;
gchar *category, *category_cf;
const gchar *text;
gchar *value;
gint text_len, len, j;

    GUI *appGUI = (GUI *)data;

	text = gtk_entry_get_text (GTK_ENTRY(appGUI->tsk->tasks_find_entry));

    if (text == NULL) {
        return TRUE;
    }

    text_len = strlen (text);

    if (appGUI->tsk->tasks_filter_disabled == TRUE)
        return TRUE;

    gtk_tree_model_get (model, iter, TA_COLUMN_DONE, &done, 
					    TA_COLUMN_CATEGORY, &category, TA_COLUMN_SUMMARY, &value, -1);
    
    if (tsk_get_category_state (category, STATE_TASKS, appGUI) == FALSE) {
        g_free (category);
        g_free (value);
        return FALSE;
    }

    category_cf = gtk_combo_box_get_active_text (GTK_COMBO_BOX (appGUI->tsk->cf_combobox));
    idx = utl_gui_list_store_get_text_index (appGUI->opt->tasks_category_store, category_cf);
    g_free (category_cf);

    if (idx) {

        if (utl_gui_list_store_get_text_index (appGUI->opt->tasks_category_store, category) == idx) {

            if(config.hide_completed && done == TRUE) {
                g_free (category);
                g_free (value);
                return FALSE;
            }

			if (value != NULL) {

				len = strlen(value) - text_len;

				if (len >= 0) {
					for(j=0; j <= len; j++) {
						if(!g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value+j, -1), text_len)) {
							g_free (value);
							g_free (category);
							return TRUE;
						}
					}
				}

				g_free(value);
			}

            g_free (category);
            return FALSE;

        } else {
            g_free (category);
            g_free (value);
            return FALSE;
        }

    } else {

		if (config.hide_completed && done == TRUE) {
            g_free (category);
            g_free (value);
            return FALSE;
		}

		if (value != NULL) {

			len = strlen(value) - text_len;

			if (len >= 0) {
				for(j=0; j <= len; j++) {
					if(!g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value+j, -1), text_len)) {
						g_free (value);
						g_free (category);
						return TRUE;
					}
				}
			}

			g_free(value);
		}
    }

    g_free (category);
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
category_filter_cb (GtkComboBox *widget, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    appGUI->tsk->filter_index = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    if (appGUI->tsk->filter_index != -1) {
        gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter));
        update_tasks_number (appGUI);
    }
}

/*------------------------------------------------------------------------------*/

gint 
custom_tasks_sort_function (GtkTreeModel *model, GtkTreeIter *iter_a, GtkTreeIter *iter_b, gpointer user_data) {

gint done_a, done_b, done_s;
gchar *priority_a, *priority_b;
gint priority_s;
guint32 due_date_a, due_date_b, due_date_s;
gint due_time_a, due_time_b, due_time_s;


    if(iter_a == NULL || iter_b == NULL) {
        return 0;
    }

    gtk_tree_model_get (model, iter_a, TA_COLUMN_DONE, &done_a, TA_COLUMN_DUE_TIME, &due_time_a, 
                        TA_COLUMN_DUE_DATE_JULIAN, &due_date_a, TA_COLUMN_PRIORITY, &priority_a, -1);
    gtk_tree_model_get (model, iter_b, TA_COLUMN_DONE, &done_b, TA_COLUMN_DUE_TIME, &due_time_b,
                        TA_COLUMN_DUE_DATE_JULIAN, &due_date_b, TA_COLUMN_PRIORITY, &priority_b, -1);

    done_s = done_b - done_a;
    if (!due_date_a) due_date_a = 1 << 31;
    if (!due_date_b) due_date_b = 1 << 31;
    due_date_s = due_date_b - due_date_a;
    due_time_s = due_time_b - due_time_a;
    priority_s = tsk_get_priority_index (priority_a) - tsk_get_priority_index (priority_b);
    g_free (priority_a);
    g_free (priority_b);

    switch(config.tasks_sorting_mode) {

        /* Done, Due Date, Priority */
        case 0:
            if (done_s !=0)
                return done_s;
            if (due_date_s == 0 && due_time_s != 0)
                return due_time_s;
            if (due_date_s != 0)
                return due_date_s;
            if (priority_s != 0)
                return priority_s;
            break;

        /* Done, Priority, Due Date */
        case 1:
            if (done_s !=0)
                return done_s;
            if (priority_s != 0)
                return priority_s;
            if (due_date_s == 0 && due_time_s != 0)
                return due_time_s;
            if (due_date_s != 0)
                return due_date_s;
            break;

        /* Priority, Due Date, Done */
        case 2:
            if (priority_s != 0)
                return priority_s;
            if (due_date_s == 0 && due_time_s != 0)
                return due_time_s;
            if (due_date_s != 0)
                return due_date_s;
            if (done_s !=0)
                return done_s;
            break;

        /* Priority, Done, Due Date */
        case 3:
            if (priority_s != 0)
                return priority_s;
            if (done_s !=0)
                return done_s;
            if (due_date_s == 0 && due_time_s != 0)
                return due_time_s;
            if (due_date_s != 0)
                return due_date_s;
            break;

        /* Due Date, Priority, Done */
        case 4:
            if (due_date_s == 0 && due_time_s != 0)
                return due_time_s;
            if (due_date_s != 0)
                return due_date_s;
            if (priority_s != 0)
                return priority_s;
            if (done_s !=0)
                return done_s;
            break;

        /* Due Date, Done, Priority */
        case 5:
            if (due_date_s == 0 && due_time_s != 0)
                return due_time_s;
            if (due_date_s != 0)
                return due_date_s;
            if (done_s !=0)
                return done_s;
            if (priority_s != 0)
                return priority_s;
            break;

        default:
            break;
    }

    return 0;
}

/*------------------------------------------------------------------------------*/

void
tasks_select_first_position_in_list(GUI *appGUI) {

GtkTreeIter     iter;
GtkTreePath     *path;

    /* set cursor at first position */
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter) == TRUE) {
        path = gtk_tree_model_get_path (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter);
        if (path != NULL) {
            gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->tsk->tasks_list), path, NULL, TRUE);
            gtk_tree_path_free(path);
        }
    }
}

/*------------------------------------------------------------------------------*/

gboolean
category_combo_box_focus_cb (GtkWidget *widget, GtkDirectionType *arg1, gpointer user_data) {
    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
add_tasks_toolbar_widget (GtkUIManager *tasks_uim_widget, GtkWidget *widget, gpointer user_data) {

GtkWidget *handle_box;

    GUI *appGUI = (GUI *)user_data;

    if (GTK_IS_TOOLBAR (widget)) {

        appGUI->tsk->tasks_toolbar = GTK_TOOLBAR (widget);

        handle_box = gtk_handle_box_new ();
        gtk_widget_show (handle_box);
        gtk_container_add (GTK_CONTAINER (handle_box), widget);
        gtk_box_pack_start (appGUI->tsk->vbox, handle_box, FALSE, FALSE, 0);
        g_signal_connect_swapped (widget, "destroy", 
                                  G_CALLBACK (gtk_widget_destroy), handle_box);

    } else {
        gtk_box_pack_start (GTK_BOX(appGUI->tsk->vbox), widget, FALSE, FALSE, 0);
    }

    gtk_widget_show (widget);
}

/*------------------------------------------------------------------------------*/

void
tasks_selection_activate (gboolean active, GUI *appGUI) {
    if (active == TRUE) {
        g_signal_connect (G_OBJECT(appGUI->tsk->tasks_list_selection), "changed",
                          G_CALLBACK(tasks_item_selected), appGUI);
    } else {
        g_signal_handlers_disconnect_by_func (G_OBJECT (appGUI->tsk->tasks_list_selection),
                                              G_CALLBACK (tasks_item_selected), appGUI);
    }
}

/*------------------------------------------------------------------------------*/

void
store_task_columns_info (GUI *appGUI) {

	gint n;

	config.tasks_column_idx_0 = utl_gui_get_column_position (appGUI->tsk->tasks_columns[TA_COLUMN_DONE], 
															 GTK_TREE_VIEW(appGUI->tsk->tasks_list), MAX_VISIBLE_TASK_COLUMNS, appGUI);
	config.tasks_column_idx_1 = utl_gui_get_column_position (appGUI->tsk->tasks_columns[TA_COLUMN_TYPE], 
															 GTK_TREE_VIEW(appGUI->tsk->tasks_list), MAX_VISIBLE_TASK_COLUMNS, appGUI);
	config.tasks_column_idx_2 = utl_gui_get_column_position (appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE], 
															 GTK_TREE_VIEW(appGUI->tsk->tasks_list), MAX_VISIBLE_TASK_COLUMNS, appGUI);
	config.tasks_column_idx_3 = utl_gui_get_column_position (appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY], 
															 GTK_TREE_VIEW(appGUI->tsk->tasks_list), MAX_VISIBLE_TASK_COLUMNS, appGUI);
	config.tasks_column_idx_4 = utl_gui_get_column_position (appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY], 
															 GTK_TREE_VIEW(appGUI->tsk->tasks_list), MAX_VISIBLE_TASK_COLUMNS, appGUI);
	config.tasks_column_idx_5 = utl_gui_get_column_position (appGUI->tsk->tasks_columns[TA_COLUMN_SUMMARY], 
															 GTK_TREE_VIEW(appGUI->tsk->tasks_list), MAX_VISIBLE_TASK_COLUMNS, appGUI);

	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 0));
	if (n > 1) {
		config.tasks_column_idx_0_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 1));
	if (n > 1) {
		config.tasks_column_idx_1_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 2));
	if (n > 1) {
		config.tasks_column_idx_2_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 3));
	if (n > 1) {
		config.tasks_column_idx_3_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 4));
	if (n > 1) {
		config.tasks_column_idx_4_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 5));
	if (n > 1) {
		config.tasks_column_idx_5_width = n;
	}
}

/*------------------------------------------------------------------------------*/

void
set_tasks_columns_width (GUI *appGUI) {

GtkTreeViewColumn   *col;
gint w;

	w = 2 * utl_gui_get_sw_vscrollbar_width (appGUI->tsk->scrolled_win);

	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 0);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.tasks_column_idx_0_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.tasks_column_idx_0_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 1);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.tasks_column_idx_1_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.tasks_column_idx_1_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 2);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.tasks_column_idx_2_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.tasks_column_idx_2_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 3);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.tasks_column_idx_3_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.tasks_column_idx_3_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 4);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.tasks_column_idx_4_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.tasks_column_idx_4_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), 5);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.tasks_column_idx_5_width > w) {
		gtk_tree_view_column_set_fixed_width (col, config.tasks_column_idx_5_width - w);
	}
}

/*------------------------------------------------------------------------------*/

gboolean
tasks_search_entry_changed_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

GtkTreePath *path;
GtkTreeIter iter;
gint i;

    GUI *appGUI = (GUI *)user_data;

    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter));

    if(strlen(gtk_entry_get_text (GTK_ENTRY(appGUI->tsk->tasks_find_entry)))) {

		i = 0;
		while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->tsk->tasks_filter), &iter, NULL, i++));

		if (i-1 != 0) {

			path = gtk_tree_path_new_first();
			if (path != NULL) {
				gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->tsk->tasks_list), path, NULL, FALSE);
				gtk_tree_path_free (path);
			}
		} 
    }
		
	update_tasks_number (appGUI);

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
gui_clear_find_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
    if (strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->tsk->tasks_find_entry)))) {
        gtk_entry_set_text(GTK_ENTRY(appGUI->tsk->tasks_find_entry), "");
		refresh_tasks (appGUI);
	}
}

/*------------------------------------------------------------------------------*/

void 
gui_create_tasks(GUI *appGUI) {

GtkWidget           *vbox1;
GtkWidget           *vbox2;
GtkWidget           *vbox3;
GtkWidget           *table;
GtkWidget           *label;
GtkWidget           *hseparator;
GtkWidget           *close_button;
GtkCellRenderer     *renderer;
GtkWidget           *top_viewport;
GtkWidget           *bottom_viewport;
GtkTextBuffer       *text_buffer;
GError              *error = NULL;
GtkActionGroup      *action_group = NULL;
gchar tmpbuf[BUFFER_SIZE];
gint i, n;

 const gchar *ui_info =
"  <toolbar name=\"toolbar\">\n"
"    <toolitem name=\"add\" action=\"add\" />\n"
"    <toolitem name=\"edit\" action=\"edit\" />\n"
"    <separator/>\n"
"    <toolitem name=\"delete\" action=\"delete\" />\n"
"    <separator/>\n"
"    <toolitem name=\"prev_date\" action=\"prev_date\" />\n"
"    <toolitem name=\"next_date\" action=\"next_date\" />\n"
"    <separator/>\n"
#if 0
"    <toolitem name=\"import\" action=\"import\" />\n"
#endif
#ifdef HAVE_LIBICAL
"    <toolitem name=\"export\" action=\"export\" />\n"
#endif
#ifdef PRINTING_SUPPORT
"    <separator/>\n"
"    <toolitem name=\"print\" action=\"print\" />\n"
"    <separator expand=\"true\" />\n"
"    <toolitem name=\"preferences\" action=\"preferences\" />\n"
"    <toolitem name=\"about\" action=\"about\" />\n"
#endif /* PRINTING_SUPPORT */
"  </toolbar>\n";

GtkActionEntry entries[] = {
  { "add", OSMO_STOCK_TASKS_ADD, _("New task"), NULL, _("New task"), G_CALLBACK(tasks_add_item_cb)},
  { "edit", OSMO_STOCK_TASKS_EDIT, _("Edit task"), NULL, _("Edit task"), G_CALLBACK(tasks_edit_item_cb)},
  { "delete", OSMO_STOCK_TASKS_REMOVE, _("Remove task"), NULL, _("Remove task"), G_CALLBACK(tasks_remove_item_cb)},
  { "prev_date", OSMO_STOCK_TASKS_PREV_DATE, _("Change due date to previous date"), NULL, _("Change due date to previous date"), G_CALLBACK(tasks_change_due_date_to_prev_date_cb)},
  { "next_date", OSMO_STOCK_TASKS_NEXT_DATE, _("Change due date to next date"), NULL, _("Change due date to next date"), G_CALLBACK(tasks_change_due_date_to_next_date_cb)},
#if 0
  { "import", OSMO_STOCK_TASKS_IMPORT, _("Import tasks"), NULL, _("Import tasks"), NULL },
#endif
#ifdef HAVE_LIBICAL
  { "export", OSMO_STOCK_TASKS_EXPORT, _("Export tasks"), NULL, _("Export tasks"), G_CALLBACK(tasks_export_visible_items_cb)},
#endif
#ifdef PRINTING_SUPPORT
  { "print", OSMO_STOCK_PRINT, _("Print visible tasks list"), NULL, _("Print visible tasks list"), G_CALLBACK(tasks_print_visible_items_cb)},
#endif /* PRINTING_SUPPORT */
	{ "preferences", OSMO_STOCK_PREFERENCES, _("Preferences"), NULL, _("Preferences"), G_CALLBACK (show_preferences_window_cb)},
	{ "about", OSMO_STOCK_ABOUT, _("About"), NULL, _("About"), G_CALLBACK (show_about_window_cb)},
};

gint columns_order[MAX_VISIBLE_TASK_COLUMNS];

gint ta_columns[MAX_VISIBLE_TASK_COLUMNS] = { 
		TA_COLUMN_DONE, TA_COLUMN_TYPE, TA_COLUMN_DUE_DATE,
		TA_COLUMN_PRIORITY, TA_COLUMN_CATEGORY, TA_COLUMN_SUMMARY 
};

guint n_entries = G_N_ELEMENTS (entries);

    appGUI->tsk->filter_index = 0;

    vbox1 = gtk_vbox_new (FALSE, 1);
    gtk_widget_show (vbox1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox1), 8);
    sprintf(tmpbuf, "<b>%s</b>", _("Tasks"));
    gui_add_to_notebook (vbox1, tmpbuf, appGUI);

    appGUI->tsk->vbox = GTK_BOX(vbox1);

    if (config.hide_tasks == TRUE) {
        gtk_widget_hide(GTK_WIDGET(appGUI->tsk->vbox));
    }

    /*-------------------------------------------------------------------------------------*/

    action_group = gtk_action_group_new ("_actions");
    gtk_action_group_add_actions (action_group, entries, n_entries, appGUI);
    gtk_action_group_set_sensitive(action_group, TRUE);

    appGUI->tsk->tasks_uim_widget = gtk_ui_manager_new ();

    gtk_ui_manager_insert_action_group (appGUI->tsk->tasks_uim_widget, action_group, 0);
    g_signal_connect (appGUI->tsk->tasks_uim_widget, "add_widget", G_CALLBACK (add_tasks_toolbar_widget), appGUI);

    if (!gtk_ui_manager_add_ui_from_string (appGUI->tsk->tasks_uim_widget, ui_info, -1, &error)) {
        g_message ("building toolbar failed: %s", error->message);
        g_error_free (error);
    }
    gtk_ui_manager_ensure_update (appGUI->tsk->tasks_uim_widget);

    gtk_toolbar_set_style (appGUI->tsk->tasks_toolbar, GTK_TOOLBAR_ICONS);

    /*-------------------------------------------------------------------------------------*/

    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/edit"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/delete"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/prev_date"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/next_date"), FALSE);
#ifdef PRINTING_SUPPORT
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->tsk->tasks_uim_widget, "/toolbar/print"), FALSE);
#endif /* PRINTING_SUPPORT */

    /*-------------------------------------------------------------------------------------*/

	if (!config.gui_layout) {
	    appGUI->tsk->tasks_paned = gtk_vpaned_new();
        gtk_paned_set_position(GTK_PANED(appGUI->tsk->tasks_paned), 99999);
	} else {
	    appGUI->tsk->tasks_paned = gtk_hpaned_new();
	}
    gtk_widget_show (appGUI->tsk->tasks_paned);
    gtk_box_pack_start(GTK_BOX(vbox1), appGUI->tsk->tasks_paned, TRUE, TRUE, 0);

    top_viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (top_viewport);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (top_viewport), GTK_SHADOW_NONE);
    gtk_paned_pack1 (GTK_PANED (appGUI->tsk->tasks_paned), top_viewport, FALSE, TRUE);

    vbox3 = gtk_vbox_new (FALSE, 1);
    gtk_widget_show (vbox3);
    gtk_container_add (GTK_CONTAINER (top_viewport), vbox3);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox3), hseparator, FALSE, TRUE, 6);

    table = gtk_table_new (2, 4, FALSE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox3), table, FALSE, TRUE, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);

    sprintf(tmpbuf, "<b>%s:</b>", _("Category"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    appGUI->tsk->cf_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->tsk->cf_combobox);
    gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), FALSE);
    GTK_WIDGET_UNSET_FLAGS(appGUI->tsk->cf_combobox, GTK_CAN_FOCUS);
    if (appGUI->tiny_gui == TRUE) {
        gtk_table_attach (GTK_TABLE (table), appGUI->tsk->cf_combobox, 1, 2, 0, 1,
                         (GtkAttachOptions) (0),
                         (GtkAttachOptions) (0), 0, 0);
    } else {
        gtk_table_attach (GTK_TABLE (table), appGUI->tsk->cf_combobox, 1, 2, 0, 1,
                         (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                         (GtkAttachOptions) (GTK_FILL), 0, 0);
    }
    g_signal_connect(G_OBJECT(appGUI->tsk->cf_combobox), "changed", 
                     G_CALLBACK(category_filter_cb), appGUI);
    g_signal_connect(G_OBJECT(appGUI->tsk->cf_combobox), "focus", 
                     G_CALLBACK(category_combo_box_focus_cb), NULL);

    appGUI->tsk->n_items_label = gtk_label_new ("");
    gtk_widget_show (appGUI->tsk->n_items_label);
    if (appGUI->tiny_gui == FALSE) {
        gtk_widget_set_size_request (appGUI->tsk->n_items_label, 100, -1);
    }
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->n_items_label, 3, 4, 0, 1,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (appGUI->tsk->n_items_label), TRUE);

	sprintf(tmpbuf, "<b>%s:</b>", _("Search"));
	label = gtk_label_new (tmpbuf);
	gtk_widget_show (label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
					 (GtkAttachOptions) (GTK_FILL),
					 (GtkAttachOptions) (0), 0, 0);

	appGUI->tsk->tasks_find_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(appGUI->tsk->tasks_find_entry), 128);
	gtk_widget_show (appGUI->tsk->tasks_find_entry);
	g_signal_connect (G_OBJECT(appGUI->tsk->tasks_find_entry), "key_release_event",
					  G_CALLBACK(tasks_search_entry_changed_cb), appGUI);
	gtk_table_attach (GTK_TABLE (table), appGUI->tsk->tasks_find_entry, 1, 4, 1, 2,
					 (GtkAttachOptions) (GTK_FILL),
					 (GtkAttachOptions) (0), 0, 0);

	appGUI->tsk->tasks_find_clear_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLEAR, FALSE);
	gtk_widget_show (appGUI->tsk->tasks_find_clear_button);
	GTK_WIDGET_UNSET_FLAGS (appGUI->tsk->tasks_find_clear_button, GTK_CAN_FOCUS);
	gtk_button_set_relief (GTK_BUTTON(appGUI->tsk->tasks_find_clear_button), GTK_RELIEF_NONE);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (appGUI->tsk->tasks_find_clear_button, _("Clear"));
	}
	g_signal_connect (G_OBJECT (appGUI->tsk->tasks_find_clear_button), "clicked",
						G_CALLBACK (gui_clear_find_cb), appGUI);
	gtk_table_attach (GTK_TABLE (table), appGUI->tsk->tasks_find_clear_button, 4, 5, 1, 2,
					 (GtkAttachOptions) (GTK_FILL),
					 (GtkAttachOptions) (0), 0, 0);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox3), hseparator, FALSE, TRUE, 6);


    appGUI->tsk->scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (appGUI->tsk->scrolled_win);
    gtk_box_pack_start (GTK_BOX (vbox3), appGUI->tsk->scrolled_win, TRUE, TRUE, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (appGUI->tsk->scrolled_win), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->tsk->scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	appGUI->tsk->tasks_list_store = gtk_list_store_new (TASKS_NUM_COLUMNS,
	                                    G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT,
	                                    G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
	                                    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN,
	                                    G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_INT,
	                                    G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
	                                    G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

	appGUI->tsk->tasks_filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter),
	                                        (GtkTreeModelFilterVisibleFunc) tasks_list_filter_cb,
	                                        appGUI, NULL);

    appGUI->tsk->tasks_sort = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(appGUI->tsk->tasks_filter));

    appGUI->tsk->tasks_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(appGUI->tsk->tasks_sort));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(appGUI->tsk->tasks_list), config.rules_hint);
    gtk_widget_show (appGUI->tsk->tasks_list);
    gtk_widget_modify_fg(GTK_WIDGET(appGUI->tsk->tasks_list), GTK_STATE_SELECTED,
                         (& GTK_WIDGET(appGUI->tsk->tasks_list)->style->base[GTK_STATE_SELECTED]));
    gtk_widget_modify_fg(GTK_WIDGET(appGUI->tsk->tasks_list), GTK_STATE_NORMAL,
                         (& GTK_WIDGET(appGUI->tsk->tasks_list)->style->bg[GTK_STATE_NORMAL]));

    g_signal_connect(G_OBJECT(appGUI->tsk->tasks_list), "button_press_event",
                     G_CALLBACK(list_dbclick_cb), appGUI);

    appGUI->tsk->tasks_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (appGUI->tsk->tasks_list));
    tasks_selection_activate (TRUE, appGUI);

    /* create columns */

    renderer = gtk_cell_renderer_toggle_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_DONE] = gtk_tree_view_column_new_with_attributes (_("Done"),
                             renderer,
                             "active", TA_COLUMN_DONE,
                             NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_DONE]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_DONE]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_DONE]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_DONE]), GTK_TREE_VIEW_COLUMN_FIXED);

	g_signal_connect (renderer, "toggled", G_CALLBACK (done_toggled), appGUI);

    renderer = gtk_cell_renderer_pixbuf_new();  /* icon */
    appGUI->tsk->tasks_columns[TA_COLUMN_TYPE]  = gtk_tree_view_column_new_with_attributes(_("Type"),
                             renderer,
                             "pixbuf", TA_COLUMN_TYPE,
                             NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_TYPE]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_TYPE]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_TYPE]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_TYPE]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE] = gtk_tree_view_column_new_with_attributes(_("Due date"),
                              renderer,
                              "text", TA_COLUMN_DUE_DATE,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE], config.tsk_visible_due_date_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY] = gtk_tree_view_column_new_with_attributes(_("Priority"),
                              renderer,
                              "text", TA_COLUMN_PRIORITY,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY], config.tsk_visible_priority_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY] = gtk_tree_view_column_new_with_attributes(_("Category"),
                              renderer,
                              "text", TA_COLUMN_CATEGORY,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY], config.tsk_visible_category_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    g_object_set (G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    gtk_cell_renderer_set_fixed_size (renderer, 0, -1);
    appGUI->tsk->tasks_columns[TA_COLUMN_SUMMARY] = gtk_tree_view_column_new_with_attributes(_("Summary"),
                              renderer,
                              "text", TA_COLUMN_SUMMARY,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_SUMMARY]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_SUMMARY]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_SUMMARY]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->tsk->tasks_columns[TA_COLUMN_SUMMARY]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE_JULIAN] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_DUE_DATE_JULIAN,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE_JULIAN], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE_JULIAN]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_DUE_TIME] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_DUE_TIME,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_DUE_TIME], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_DUE_TIME]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_START_DATE_JULIAN] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_START_DATE_JULIAN,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_START_DATE_JULIAN], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_START_DATE_JULIAN]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_DONE_DATE_JULIAN] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_DONE_DATE_JULIAN,
                              "strikethrough", TA_COLUMN_DONE,
                              "foreground", TA_COLUMN_COLOR,
                              "weight", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_DONE_DATE_JULIAN], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_DONE_DATE_JULIAN]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_DESCRIPTION] = gtk_tree_view_column_new_with_attributes(_("Description"),
                              renderer,
                              "text", TA_COLUMN_DESCRIPTION,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_DESCRIPTION], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_DESCRIPTION]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_COLOR] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_COLOR,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_COLOR], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_COLOR]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_BOLD] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_BOLD,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_BOLD], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_BOLD]);

    renderer = gtk_cell_renderer_toggle_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_ACTIVE] = gtk_tree_view_column_new_with_attributes (NULL,
                             renderer,
                             "active", TA_COLUMN_ACTIVE,
                             NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_ACTIVE], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_ACTIVE]);

    renderer = gtk_cell_renderer_toggle_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_OFFLINE_IGNORE] = gtk_tree_view_column_new_with_attributes (NULL,
                             renderer,
                             "active", TA_COLUMN_OFFLINE_IGNORE,
                             NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_OFFLINE_IGNORE], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_OFFLINE_IGNORE]);

    renderer = gtk_cell_renderer_toggle_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_SOUND_ENABLE] = gtk_tree_view_column_new_with_attributes (NULL,
                             renderer,
                             "active", TA_COLUMN_SOUND_ENABLE,
                             NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_SOUND_ENABLE], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_SOUND_ENABLE]);

    renderer = gtk_cell_renderer_toggle_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT] = gtk_tree_view_column_new_with_attributes (NULL,
                             renderer,
                             "active", TA_COLUMN_REPEAT,
                             NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_DAY] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_DAY,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_DAY], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_DAY]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_MONTH_INTERVAL] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_MONTH_INTERVAL,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_MONTH_INTERVAL], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_MONTH_INTERVAL]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_DAY_INTERVAL] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_DAY_INTERVAL,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_DAY_INTERVAL], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_DAY_INTERVAL]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_START_DAY] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_START_DAY,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_START_DAY], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_START_DAY]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_START] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_TIME_START,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_START], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_START]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_END] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_TIME_END,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_END], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_END]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_INTERVAL] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_TIME_INTERVAL,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_INTERVAL], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_TIME_INTERVAL]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_COUNTER] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_REPEAT_COUNTER,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_COUNTER], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_REPEAT_COUNTER]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_ALARM_COMMAND] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_ALARM_COMMAND,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_ALARM_COMMAND], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_ALARM_COMMAND]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_WARNING_DAYS] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_WARNING_DAYS,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_WARNING_DAYS], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_WARNING_DAYS]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_WARNING_TIME] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_WARNING_TIME,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_WARNING_TIME], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_WARNING_TIME]);

	renderer = gtk_cell_renderer_text_new ();
    appGUI->tsk->tasks_columns[TA_COLUMN_POSTPONE_TIME] = gtk_tree_view_column_new_with_attributes (NULL,
                              renderer,
                              "text", TA_COLUMN_POSTPONE_TIME,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_POSTPONE_TIME], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_POSTPONE_TIME]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->tsk->tasks_columns[TA_COLUMN_ID] = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", TA_COLUMN_ID,
                              NULL);
    gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_ID], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->tsk->tasks_list), appGUI->tsk->tasks_columns[TA_COLUMN_ID]);

	/* restore columns order */

	columns_order[0] = config.tasks_column_idx_0;
	columns_order[1] = config.tasks_column_idx_1;
	columns_order[2] = config.tasks_column_idx_2;
	columns_order[3] = config.tasks_column_idx_3;
	columns_order[4] = config.tasks_column_idx_4;
	columns_order[5] = config.tasks_column_idx_5;

	n = MAX_VISIBLE_TASK_COLUMNS-1;

	while (n >= 0) {
		for (i = 0; i < MAX_VISIBLE_TASK_COLUMNS; i++) {
			if (n == columns_order[i]) {
				gtk_tree_view_move_column_after(GTK_TREE_VIEW(appGUI->tsk->tasks_list),
												appGUI->tsk->tasks_columns[ta_columns[i]], NULL);
				n--;
			}
		}
	}

	set_tasks_columns_width (appGUI);

    /* configure list options */

    gtk_container_add (GTK_CONTAINER (appGUI->tsk->scrolled_win), appGUI->tsk->tasks_list);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW(appGUI->tsk->tasks_list), FALSE);

    /* configure sorting */

    gtk_tree_sortable_set_sort_func ((GtkTreeSortable *)appGUI->tsk->tasks_sort, 0, 
                                     (GtkTreeIterCompareFunc)custom_tasks_sort_function, NULL, NULL);

    gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *)appGUI->tsk->tasks_sort, 
                                          TA_COLUMN_DUE_DATE, config.tasks_sorting_order);
    gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *)appGUI->tsk->tasks_sort, 
                                          TA_COLUMN_PRIORITY, config.tasks_sorting_order);
    gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *)appGUI->tsk->tasks_sort, 
                                          TA_COLUMN_DONE, config.tasks_sorting_order);

    /*----------------------------------------------------------------------------*/

    bottom_viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (bottom_viewport);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (bottom_viewport), GTK_SHADOW_NONE);
    gtk_paned_pack2 (GTK_PANED (appGUI->tsk->tasks_paned), bottom_viewport, TRUE, TRUE);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), 0);
    gtk_container_add (GTK_CONTAINER (bottom_viewport), vbox2);

    appGUI->tsk->panel_hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox2), appGUI->tsk->panel_hbox, FALSE, FALSE, 0);
    gtk_widget_show(appGUI->tsk->panel_hbox);

    sprintf(tmpbuf, "%s:", _("Task details"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (appGUI->tsk->panel_hbox), label, FALSE, FALSE, 0);

	if (!config.gui_layout) {
		if (config.default_stock_icons) {
			close_button = utl_gui_stock_button(GTK_STOCK_CLOSE, FALSE);
		} else {
			close_button = utl_gui_stock_button(OSMO_STOCK_BUTTON_CLOSE, FALSE);
		}
		GTK_WIDGET_UNSET_FLAGS(close_button, GTK_CAN_FOCUS);
		gtk_button_set_relief (GTK_BUTTON(close_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (close_button, _("Close description panel"));
		}
		gtk_box_pack_end (GTK_BOX (appGUI->tsk->panel_hbox), close_button, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (close_button), "clicked",
							G_CALLBACK (panel_close_desc_cb), appGUI);
	}

    appGUI->tsk->panel_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (appGUI->tsk->panel_scrolledwindow);
    gtk_box_pack_start (GTK_BOX (vbox2), appGUI->tsk->panel_scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (appGUI->tsk->panel_scrolledwindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->tsk->panel_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    appGUI->tsk->tasks_desc_textview = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->tsk->tasks_desc_textview), GTK_WRAP_WORD);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), 4);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), 4);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview), FALSE);
    gtk_widget_show (appGUI->tsk->tasks_desc_textview);
    gtk_container_add (GTK_CONTAINER (appGUI->tsk->panel_scrolledwindow), appGUI->tsk->tasks_desc_textview);

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appGUI->tsk->tasks_desc_textview));
    gtk_text_buffer_create_tag (text_buffer, "italic",
                  "style", PANGO_STYLE_ITALIC, NULL);
    appGUI->tsk->font_tag_object = gtk_text_buffer_create_tag (text_buffer, "info_font",
                      "font", (gchar *) config.task_info_font, NULL);

	if (config.gui_layout) {
	    gtk_paned_set_position(GTK_PANED(appGUI->tsk->tasks_paned), config.tasks_pane_pos);
	}

	gtk_widget_grab_focus (appGUI->tsk->tasks_find_entry);
}

/*------------------------------------------------------------------------------*/

#endif  /* TASKS_ENABLED */

