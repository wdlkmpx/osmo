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

#include "check_events.h"
#include "i18n.h"
#include "calendar.h"
#include "gui.h"
#include "utils.h"
#include "utils_gui.h"
#include "calendar_notes.h"
#include "options_prefs.h"
#include "calendar_utils.h"
#include "stock_icons.h"
#include "tasks_items.h"
#include "tasks_utils.h"

/*------------------------------------------------------------------------------*/

#ifdef TASKS_ENABLED

/*------------------------------------------------------------------------------*/

#ifdef HAVE_LIBNOTIFY
static void
tsk_status_icon_set_normal (GUI *appGUI)
{
	gtk_status_icon_set_blinking (appGUI->osmo_trayicon, FALSE);
	gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_NORMAL);
}
#endif /* HAVE_LIBNOTIFY */

/*------------------------------------------------------------------------------*/

static gboolean
tsk_check_notification_id (guint id, gint type, GUI *appGUI)
{
#ifdef HAVE_LIBNOTIFY
	GSList *node;
	TASK_NTF *a;

	for (node = appGUI->tsk->notifications; node != NULL; node = node->next) {
		a = (TASK_NTF *) node->data;
		/* Don't show warning notification when alarm notification is visible */
		if (a->id == id && (a->type == NOTIFY_ALARM || a->type == type))
			return TRUE;
	}
#endif /* HAVE_LIBNOTIFY */
	return FALSE;
}

/*------------------------------------------------------------------------------*/

#ifdef HAVE_LIBNOTIFY

void
free_notifications_list (GUI *appGUI)
{
	GSList *node;
	TASK_NTF *a;

	for (node = appGUI->tsk->notifications; node != NULL; node = node->next) {
		a = (TASK_NTF *) node->data;
		notify_notification_close (a->notify, NULL);
		g_free (a);
	}

	if (appGUI->tsk->notifications != NULL) {
		g_slist_free (appGUI->tsk->notifications);
		appGUI->tsk->notifications = NULL;
	}
}

/*------------------------------------------------------------------------------*/

void
tsk_show_info_dialog (GUI *appGUI)
{
gchar tmpbuf[BUFFER_SIZE];

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s\n%s", _("Cannot perform selected operation."),
	            _("Task has been modified or removed."));
	utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW (appGUI->main_window));
}

/*------------------------------------------------------------------------------*/

void
tsk_done_cb (NotifyNotification *n, const char *action, GUI *appGUI)
{
	GtkTreeIter *iter;
	GSList *node;
	TASK_NTF *a;
	TASK_ITEM *t;

	for (node = appGUI->tsk->notifications; node != NULL; node = node->next) {
		a = (TASK_NTF *) node->data;

		if (a->notify == n) {
			iter = tsk_get_iter (a->id, appGUI);

			if (iter != NULL) {

				t = tsk_get_item (iter, appGUI);

				if (t->repeat == TRUE) {
					tasks_repeat_done (iter, t, appGUI);
				} else if (config.delete_completed) {
					gtk_list_store_remove (appGUI->tsk->tasks_list_store, iter);
				} else {
					gtk_list_store_set (appGUI->tsk->tasks_list_store, iter,
					                    TA_COLUMN_COLOR, get_date_color (t->due_date_julian, t->due_time, TRUE, appGUI),
					                    TA_COLUMN_DONE, TRUE, -1);
				}

				tsk_item_free (t);

			} else {
				tsk_show_info_dialog (appGUI);
			}

			tsk_status_icon_set_normal (appGUI);
			notify_notification_close (a->notify, NULL);
			appGUI->tsk->notifications = g_slist_remove (appGUI->tsk->notifications, a);
			g_free (a);

			break;
		}
	}
}

/*------------------------------------------------------------------------------*/

void
tsk_postpone_notify_cb (NotifyNotification *n, const char *action, GUI *appGUI)
{
	GSList *node;
	TASK_NTF *a;
	TASK_ITEM *t;

	for (node = appGUI->tsk->notifications; node != NULL; node = node->next) {
		a = (TASK_NTF *) node->data;

		if (a->notify == n) {

			t = tsk_get_item_id (a->id, appGUI);
			g_return_if_fail (t != NULL);

			a->time = utl_time_get_current_seconds () + t->postpone_time * 60;
			a->date = utl_date_get_current_julian ();

			if (a->time >= 24 * 3600) {
				a->time -= 24 * 3600;
				a->date++;
			}

			tsk_item_free (t);
			tsk_status_icon_set_normal (appGUI);
			notify_notification_close (a->notify, NULL);
			break;
		}
	}
}

/*------------------------------------------------------------------------------*/

void
tsk_show_task_cb (NotifyNotification *n, const char *action, GUI *appGUI)
{
	GtkTreeIter iter;
	GSList *tnode;
	GtkTreePath *sort_path, *filter_path, *path;
	GtkTreeModel *model;
	TASK_NTF *a;
	TASK_ITEM *t;
	guint id;

	model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);

    if (config.enable_systray == TRUE && appGUI->window_visible == FALSE) {
		gtk_widget_show (appGUI->main_window);
		appGUI->window_visible = TRUE;
	} else {
		gtk_window_deiconify (GTK_WINDOW (appGUI->main_window));
	}

	/* select task tab */
	if (config.hide_tasks == FALSE) {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), PAGE_TASKS);
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), 0);

	/* select task on list */
	for (tnode = appGUI->tsk->notifications; tnode != NULL; tnode = tnode->next) {
		a = (TASK_NTF *) tnode->data;

		if (a->notify == n) {
			tasks_selection_activate (FALSE, appGUI);

			if (gtk_tree_model_get_iter_first (model, &iter) == TRUE) {
				sort_path = gtk_tree_model_get_path (model, &iter);

				while (sort_path != NULL) {
					gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->tsk->tasks_list), sort_path, NULL, FALSE);
					filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->tsk->tasks_sort), sort_path);

					if (filter_path != NULL) {
						path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter), filter_path);

						if (path != NULL) {
							gtk_tree_model_get_iter (model, &iter, path);
							gtk_tree_model_get (model, &iter, TA_COLUMN_ID, &id, -1);

							if (a->id == id) {
								tasks_selection_activate (TRUE, appGUI);
								gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->tsk->tasks_list), sort_path, NULL, FALSE);
								gtk_tree_path_free (path);
								gtk_tree_path_free (filter_path);
								g_signal_emit_by_name (G_OBJECT (appGUI->tsk->tasks_list_selection), "changed");
								break;
							}

							gtk_tree_path_free (path);
						}

						gtk_tree_path_free (filter_path);
					}

					gtk_tree_path_next (sort_path);
					if (gtk_tree_model_get_iter (model, &iter, sort_path) == FALSE) break;
				}

				gtk_tree_path_free (sort_path);
			}

			tsk_status_icon_set_normal (appGUI);

			if (a->type == NOTIFY_WARNING)
				break;

			notify_notification_clear_actions (a->notify);

			t = tsk_get_item_id (a->id, appGUI);
			g_return_if_fail (t != NULL);

			if (t->postpone_time > 0) {
				notify_notification_add_action (a->notify, "postpone", _("Remind me later"),
				                                (NotifyActionCallback) tsk_postpone_notify_cb, appGUI, NULL);
			}
			notify_notification_add_action (a->notify, "done", _("Done"), (NotifyActionCallback) tsk_done_cb, appGUI, NULL);
		    notify_notification_show (a->notify, NULL);
			tsk_item_free (t);
			break;
		}
	}
}

/*------------------------------------------------------------------------------*/

void
show_postponed_notification (GUI *appGUI)
{
	guint32 current_date;
	gint current_time;
	GSList *node;
	TASK_NTF *a;
	TASK_ITEM *t;
	gchar *datestr, *datestri, *title, *text = NULL;
	gboolean sound_flag = TRUE;

	current_date = utl_date_get_current_julian ();
	current_time = utl_time_get_current_seconds ();

	for (node = appGUI->tsk->notifications; node != NULL; node = node->next) {
		a = (TASK_NTF *) node->data;

		if (a->type == NOTIFY_WARNING) continue;

		if ((g_date_valid_julian (a->date) && utl_time_valid_seconds (a->time)) == FALSE)
			continue;

		if (utl_date_time_compare_js (a->date, a->time, current_date, current_time) <= 0) {
			a->date = 0;
			a->time = -1;

			t = tsk_get_item_id (a->id, appGUI);
			g_return_if_fail (t != NULL);

			datestr = utl_date_time_print_default (t->due_date_julian, t->due_time, FALSE);
			datestri = g_strdup_printf ("<i>%s</i>", datestr);
			g_free (datestr);

			if (t->desc != NULL && strlen (t->desc))
				text = g_strdup_printf ("%s\n%.100s", datestri, t->desc);

			title = g_strdup_printf ("%s (%s)", t->summary, _("postponed"));

			if (text != NULL)
				notify_notification_update (a->notify, title, text, GTK_STOCK_DIALOG_WARNING);
			else
				notify_notification_update (a->notify, title, datestri, GTK_STOCK_DIALOG_WARNING);

			g_free (title);
			g_free (text);
			g_free (datestri);

			if (strlen (config.global_notification_command))
				gui_save_data_and_run_command (config.global_notification_command, appGUI);

			if (t->sound_enable && sound_flag) {
				utl_play_alarm_sound (config.sound_alarm_repeat);
				sound_flag = FALSE;
			}

			tsk_item_free (t);

			notify_notification_show (a->notify, NULL);
		}
	}
}

/*------------------------------------------------------------------------------*/

#endif /* HAVE_LIBNOTIFY */

/*------------------------------------------------------------------------------*/

gboolean
tsk_show_warning_notification (TASK_ITEM *item, GtkTreeIter *iter, GUI *appGUI)
{
guint32 current_date, warning_date;
gint current_time, warning_time;

	if (tsk_check_notification_id (item->id, NOTIFY_WARNING, appGUI))
		return FALSE;

	if (item->warning_days > 0 || item->warning_time > 0) {

		current_date = utl_date_get_current_julian ();
		current_time = utl_time_get_current_seconds ();
		utl_subtract_from_date (item->due_date_julian, item->due_time,
		                        item->warning_days, item->warning_time * 60, &warning_date, &warning_time);

		if (warning_date < current_date + (warning_time <= current_time) ? 1 : 0) {

			if (item->repeat == TRUE && item->offline_ignore == TRUE) {
				if (warning_date < appGUI->run_date + (warning_time < appGUI->run_time) ? 1 : 0) {
					return FALSE;
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
tsk_show_task_notification (TASK_ITEM *item, GtkTreeIter *iter, GUI *appGUI)
{
	if (item->due_date_julian == 0 || tsk_check_notification_id (item->id, NOTIFY_ALARM, appGUI))
		return FALSE;

	if (utl_date_time_in_the_past_js (item->due_date_julian, item->due_time)) {

		if (item->repeat == TRUE && item->offline_ignore == TRUE) {
			if (utl_date_time_compare_js (item->due_date_julian, item->due_time, appGUI->run_date, appGUI->run_time) < 0) {
				tasks_repeat_done (iter, item, appGUI);
				return FALSE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
notify_tasks (GUI *appGUI)
{
guint32 current_date;
gint current_time;

GtkTreeIter iter;
TASK_ITEM *item;
gint i;
gboolean sound_flag = TRUE;

#ifdef HAVE_LIBNOTIFY
	TASK_NTF *a;
	gchar *datestr, *text, *textdesc;

	show_postponed_notification (appGUI);
#endif /* HAVE_LIBNOTIFY */

	current_date = utl_date_get_current_julian ();
	current_time = utl_time_get_current_seconds ();

	i = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter, NULL, i++)) {

		item = tsk_get_item (&iter, appGUI);
		if (item != NULL) {

			if (item->active == TRUE && item->done == FALSE) {

				if (tsk_show_task_notification (item, &iter, appGUI) == TRUE) {

					if (item->alarm_command != NULL) {
						if (strlen (item->alarm_command)) {
							gui_save_data_and_run_command (item->alarm_command, appGUI);
						}
					}

#ifdef HAVE_LIBNOTIFY

					datestr = utl_date_time_print_default (item->due_date_julian, item->due_time, FALSE);
					text = g_strdup_printf ("<i>%s</i>", datestr);
					g_free (datestr);

					textdesc = NULL;
					if (item->desc != NULL && strlen (item->desc))
						textdesc = g_strdup_printf ("%s\n%.100s", text, item->desc);

					a = g_malloc (sizeof (TASK_NTF));

					a->id = item->id;
					a->type = NOTIFY_ALARM;
					a->time = -1;
					a->date = 0;

					if (textdesc != NULL)
//BK						a->notify = notify_notification_new (item->summary, textdesc, GTK_STOCK_DIALOG_WARNING, NULL);
						a->notify = notify_notification_new (item->summary, textdesc, GTK_STOCK_DIALOG_WARNING);
					else
//BK						a->notify = notify_notification_new (item->summary, text, GTK_STOCK_DIALOG_WARNING, NULL);
						a->notify = notify_notification_new (item->summary, text, GTK_STOCK_DIALOG_WARNING);

					g_free (textdesc);
					g_free (text);

					notify_notification_set_timeout (a->notify, NOTIFY_EXPIRES_NEVER);
					switch (tsk_get_priority_index (item->priority)) {
						case LOW_PRIORITY: notify_notification_set_urgency (a->notify, NOTIFY_URGENCY_LOW); break;
						case MEDIUM_PRIORITY: notify_notification_set_urgency (a->notify, NOTIFY_URGENCY_NORMAL); break;
						case HIGH_PRIORITY: notify_notification_set_urgency (a->notify, NOTIFY_URGENCY_CRITICAL); break;
					}

					if (item->postpone_time > 0) {
						notify_notification_add_action (a->notify, "postpone", _("Remind me later"),
						                                (NotifyActionCallback) tsk_postpone_notify_cb, appGUI, NULL);
					}

					if (tsk_get_category_state (item->category, STATE_TASKS, appGUI) == TRUE) {
						notify_notification_add_action (a->notify, "show_task", _("Show task"),
						                                (NotifyActionCallback) tsk_show_task_cb, appGUI, NULL);
					}

					notify_notification_add_action (a->notify, "done", _("Done"),
					                                (NotifyActionCallback) tsk_done_cb, appGUI, NULL);
#endif /* HAVE_LIBNOTIFY */

					if (gtk_status_icon_get_visible (appGUI->osmo_trayicon)) {
//BK#ifdef HAVE_LIBNOTIFY
//BK						notify_notification_attach_to_status_icon (a->notify, appGUI->osmo_trayicon);
//BK#endif /* HAVE_LIBNOTIFY */
				        gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_TASK);

						if (config.blink_on_events) {
							gtk_status_icon_set_blinking (appGUI->osmo_trayicon, TRUE);
						}
					}

                    if (strlen (config.global_notification_command)) {
                        gui_save_data_and_run_command (config.global_notification_command, appGUI);
                    }

					if (item->sound_enable && sound_flag) {
						utl_play_alarm_sound (config.sound_alarm_repeat);
						sound_flag = FALSE;
					}

#ifdef HAVE_LIBNOTIFY
					/* Delete alarm warning */
					tsk_delete_notify (item->id, appGUI);

					if (!notify_notification_show (a->notify, NULL)) {
						g_warning ("Failed to send notification");
						return FALSE;
					}

					appGUI->tsk->notifications = g_slist_prepend (appGUI->tsk->notifications, a);
#endif /* HAVE_LIBNOTIFY */

				} else if (tsk_show_warning_notification (item, &iter, appGUI)) {

#ifdef HAVE_LIBNOTIFY
					datestr = utl_date_time_print_default (item->due_date_julian, item->due_time, FALSE);
					text = g_strdup_printf ("<b>%s</b>\n<i>%s</i>", item->summary, datestr);
					g_free (datestr);

					textdesc = NULL;
					if (item->desc != NULL && strlen (item->desc))
						textdesc = g_strdup_printf ("%s\n%.100s", text, item->desc);

					a = g_malloc (sizeof (TASK_NTF));

					a->id = item->id;
					a->type = NOTIFY_WARNING;
					a->time = -1;
					a->date = 0;
					if (textdesc != NULL)
//BK						a->notify = notify_notification_new (_("Alarm warning!"), textdesc, GTK_STOCK_DIALOG_INFO, NULL);
						a->notify = notify_notification_new (_("Alarm warning!"), textdesc, GTK_STOCK_DIALOG_INFO);
					else
//BK						a->notify = notify_notification_new (_("Alarm warning!"), text, GTK_STOCK_DIALOG_INFO, NULL);
						a->notify = notify_notification_new (_("Alarm warning!"), text, GTK_STOCK_DIALOG_INFO);

					notify_notification_set_timeout (a->notify, NOTIFY_EXPIRES_NEVER);
					notify_notification_set_urgency (a->notify, NOTIFY_URGENCY_NORMAL);
					if (tsk_get_category_state (item->category, STATE_TASKS, appGUI) == TRUE)
						notify_notification_add_action (a->notify, "show_task", _("Show task"),
						                                (NotifyActionCallback)tsk_show_task_cb, appGUI, NULL);

#endif /* HAVE_LIBNOTIFY */

					if (gtk_status_icon_get_visible (appGUI->osmo_trayicon)) {
//BK#ifdef HAVE_LIBNOTIFY
//BK						notify_notification_attach_to_status_icon (a->notify, appGUI->osmo_trayicon);
//BK#endif /* HAVE_LIBNOTIFY */
				        gtk_status_icon_set_from_stock (appGUI->osmo_trayicon, OSMO_STOCK_SYSTRAY_TASK);

						if (config.blink_on_events) {
							gtk_status_icon_set_blinking (appGUI->osmo_trayicon, TRUE);
						}
					}

					if (item->sound_enable && sound_flag) {
						utl_play_alarm_sound (config.sound_alarm_repeat);
						sound_flag = FALSE;
					}

#ifdef HAVE_LIBNOTIFY
					if (!notify_notification_show (a->notify, NULL)) {
						g_warning ("Failed to send notification");
						return FALSE;
					}

					appGUI->tsk->notifications = g_slist_prepend (appGUI->tsk->notifications, a);
#endif /* HAVE_LIBNOTIFY */
				}

			}
			tsk_item_free (item);
		}
	}

	return TRUE;
}

#endif  /* TASKS_ENABLED */

/*------------------------------------------------------------------------------*/

gboolean
time_handler (GUI *appGUI)
{
static gint minute = -1;
gint current_minute;

	update_clock (appGUI);

    current_minute = get_current_minute ();

	if (minute != current_minute) {

#ifdef TASKS_ENABLED
		notify_tasks (appGUI);
#endif  /* TASKS_ENABLED */

		minute = current_minute;

		if (minute == 0) 
			if (get_current_hour () == 0) {
#ifdef TASKS_ENABLED
				refresh_tasks (appGUI);
#endif  /* TASKS_ENABLED */

				/* update systray status */
				gui_systray_update_icon (appGUI);

				/* set calendar cursor */
				calendar_set_today (appGUI);
			}

		gui_systray_tooltip_update (appGUI);
	}

	return TRUE;
}

/*------------------------------------------------------------------------------*/

gboolean
check_tasks_contacts (guint32 julian_day, GUI *appGUI) {

    if (appGUI->calendar_only == TRUE || appGUI->all_pages_added == FALSE) {
        return FALSE;
    }

#ifdef CONTACTS_ENABLED
    /* check contacts */
    if (check_contacts (julian_day, appGUI) == TRUE) {
        return TRUE;
    }
#endif  /* CONTACTS_ENABLED */

#ifdef TASKS_ENABLED
    /* check tasks */
    return tsk_check_tasks (julian_day, julian_day, STATE_CALENDAR, appGUI);
#else
    return FALSE;
#endif  /* TASKS_ENABLED */

}

/*------------------------------------------------------------------------------*/

#ifdef CONTACTS_ENABLED

gboolean
check_contacts (guint32 julian_day, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	GDate *date = NULL;
	guint32 julian;
	gint age, year, flag;

	model = GTK_TREE_MODEL (appGUI->cnt->contacts_list_store);
	g_return_val_if_fail (model != NULL, FALSE);

	date = g_date_new ();
	g_return_val_if_fail (date != NULL, FALSE);

	year = julian_to_year (julian_day);
	flag = FALSE;
	path = gtk_tree_path_new_first ();

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {
		gtk_tree_model_get (model, &iter, COLUMN_BIRTH_DAY_DATE, &julian, -1);

		if (g_date_valid_julian (julian) == TRUE) {

			g_date_set_julian (date, julian);
			age = year - g_date_get_year (date);

			if (age >= 0) {
				if (g_date_valid_dmy (g_date_get_day (date), g_date_get_month (date), year) == FALSE) {
					g_date_subtract_days (date, 1);
				}

				g_date_set_year (date, year);
				julian = g_date_get_julian (date);

				if (julian_day == julian) {
					flag = TRUE;
					break;
				}
			}

		}

		gtk_tree_path_next (path);
	}

	gtk_tree_path_free (path);
	g_date_free (date);

	return flag;
}

#endif  /* CONTACTS_ENABLED */

/*------------------------------------------------------------------------------*/

void
button_event_checker_window_delete_cb (GtkButton *button, gpointer user_data) {}

void
button_event_checker_window_close_cb (GtkButton *button, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    gtk_widget_destroy (appGUI->event_checker_window);
    gui_quit_osmo (appGUI);
}

/*------------------------------------------------------------------------------*/

void
button_event_checker_show_osmo_window_cb (GtkButton *button, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    appGUI->check_events = FALSE;
    gtk_widget_destroy (appGUI->event_checker_window);

    gui_systray_initialize (appGUI);
    gtk_widget_show (appGUI->main_window);
    gtk_window_move (GTK_WINDOW (appGUI->main_window), config.window_x, config.window_y);
}

/*------------------------------------------------------------------------------*/

void
event_checker_select_item (GUI *appGUI)
{
GtkTreeIter iter;
GtkTreeModel *model;
guint32 julian_day;
GDate *date;

	date = g_date_new ();
	g_return_if_fail (date != NULL);

	if (gtk_tree_selection_get_selected (appGUI->event_checker_list_selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, CE_COLUMN_DATE_JULIAN, &julian_day, -1);

		if (g_date_valid_julian (julian_day) == TRUE) {
			g_date_set_julian (date, julian_day);
			cal_jump_to_date (date, appGUI);
			update_aux_calendars (appGUI);
			gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->notebook), PAGE_CALENDAR);
			button_event_checker_show_osmo_window_cb (NULL, appGUI);
		}
	}

	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

gint
event_checker_list_dbclick_cb(GtkWidget * widget, GdkEventButton * event, gpointer func_data) {

    GUI *appGUI = (GUI *)func_data;

    if ((event->type==GDK_2BUTTON_PRESS) && (event->button == 1)) {
        event_checker_select_item (appGUI);
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

gint 
event_checker_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    switch(event->keyval) {

        case GDK_Return:
            event_checker_select_item (appGUI);
            return TRUE;
        case GDK_Escape:
            button_event_checker_window_close_cb (NULL, appGUI);
            return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

static gboolean
add_note_to_event_window (struct note *n, GUI *appGUI)
{
	GtkTreeIter iter;
	gchar *note_str, *date_str, *stripped;

	note_str = cal_note_remove_empty_lines (n->note);
	stripped = utl_gui_text_strip_tags (note_str);
	g_free (note_str);
	
	date_str = g_strdup_printf ("%s\n(%s)",
	                            julian_to_str (n->date, config.date_format, config.override_locale_settings),
	                            utl_get_julian_day_name (n->date));

	gtk_list_store_append (appGUI->event_checker_list_store, &iter);
	gtk_list_store_set (appGUI->event_checker_list_store, &iter,
	                    CE_COLUMN_DATE, date_str,
	                    CE_COLUMN_DATE_JULIAN, n->date,
	                    CE_COLUMN_EVENT_TYPE, _("Day note"),
	                    CE_COLUMN_EVENT_LINE, stripped, -1);

	g_free (stripped);
	g_free (date_str);

	return TRUE;
}

/*------------------------------------------------------------------------------*/

gboolean
create_event_checker_window (GUI *appGUI) {

GtkWidget           *scrolledwindow;
GtkWidget           *vbox1;
GtkWidget           *hseparator;
GtkWidget           *hbuttonbox;
GtkWidget           *close_button;
GtkWidget           *show_osmo_button;
GtkTreeViewColumn   *column, *julian_day_column;
GtkCellRenderer     *renderer;

#if defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED)
GtkTreePath         *path;
GtkTreeIter         iter;
gchar               date_str[BUFFER_SIZE];
guint32             j;
#endif

guint32             start_date, current_date;
gboolean            day_notes_flag, tasks_flag = FALSE;

#ifdef TASKS_ENABLED
gchar               *summary, *category;
guint32             julian_day;
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
gboolean            birthdays_flag;
guint32             date, age;
GDate               *new_cdate;
gchar               tmpbuf[BUFFER_SIZE], template[BUFFER_SIZE];
gchar               *first_name, *last_name;
#endif  /* CONTACTS_ENABLED */


    start_date = config.lastrun_date;           /* - */
    current_date = utl_date_get_current_julian () + appGUI->check_ndays_events;     /* + */

    /* check day notes */
	day_notes_flag = cal_check_notes (start_date, current_date, appGUI);


#ifdef TASKS_ENABLED
    /* check tasks */

    tasks_flag = FALSE;

    if (appGUI->tsk->tasks_list_store != NULL) {
        path = gtk_tree_path_new_first();

        while (gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, path) == TRUE) {
            gtk_tree_model_get (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, 
                                TA_COLUMN_DUE_DATE_JULIAN, &julian_day, TA_COLUMN_CATEGORY, &category, -1);

            j = start_date;

            while (j <= current_date) {
                if (j == julian_day && (tsk_get_category_state (category, STATE_EITHER, appGUI) == TRUE)) {
                    tasks_flag = TRUE;
                    break;
                }
                j++;
            };

            if (tasks_flag == TRUE) {
                g_free (category);
                break;
            }

            g_free (category);
            gtk_tree_path_next(path);
        }
        gtk_tree_path_free(path);

    }
#endif  /* TASKS_ENABLED */


#ifdef CONTACTS_ENABLED

    /* check birthdays */

    birthdays_flag = FALSE;

    if (appGUI->cnt->contacts_list_store != NULL) {

        path = gtk_tree_path_new_first();

        while (gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, path) == TRUE) {
            gtk_tree_model_get (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, 
                                COLUMN_BIRTH_DAY_DATE, &date, -1);
            if (date != 0) {

                new_cdate = g_date_new_julian (date);

                if (new_cdate != NULL) {

                    age = g_date_get_year (appGUI->cal->date) - g_date_get_year (new_cdate);

                    if (age >= 0) {

                        j = start_date;

                        while (j <= current_date) {
                            g_date_set_year (new_cdate, julian_to_year(j));
                            date = g_date_get_julian (new_cdate);
                            if (j == date) {
                                birthdays_flag = TRUE;
                                break;
                            }
                            j++;
                        };
                    }

                    g_date_free(new_cdate);
                }
            }

            if (birthdays_flag == TRUE) break;

            gtk_tree_path_next(path);
        }
        gtk_tree_path_free(path);
    }
#endif  /* CONTACTS_ENABLED */

    /* any events available ? */

#ifdef CONTACTS_ENABLED
    if (day_notes_flag == FALSE && tasks_flag == FALSE && birthdays_flag == FALSE) {
#else
    if (day_notes_flag == FALSE && tasks_flag == FALSE) {
#endif  /* CONTACTS_ENABLED */
        return FALSE;           /* if not then quit */
    }
        
    appGUI->event_checker_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (appGUI->event_checker_window), _("Events"));
    gtk_window_set_position (GTK_WINDOW (appGUI->event_checker_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW(appGUI->event_checker_window), 750, 650);
    gtk_window_set_modal (GTK_WINDOW (appGUI->event_checker_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->event_checker_window), "delete_event",
                      G_CALLBACK(button_event_checker_window_delete_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->event_checker_window), "key_press_event",
                      G_CALLBACK (event_checker_key_press_cb), appGUI);
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->event_checker_window), 8);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->event_checker_window), vbox1);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow, TRUE, TRUE, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    appGUI->event_checker_list_store = gtk_list_store_new (CHECK_EVENTS_NUM_COLUMNS, 
                                                           G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);

    appGUI->event_checker_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(appGUI->event_checker_list_store));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(appGUI->event_checker_list), TRUE);
    gtk_widget_show (appGUI->event_checker_list);
    GTK_WIDGET_SET_FLAGS (appGUI->event_checker_list, GTK_CAN_DEFAULT);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), appGUI->event_checker_list);
    g_signal_connect(G_OBJECT(appGUI->event_checker_list), "button_press_event",
                     G_CALLBACK(event_checker_list_dbclick_cb), appGUI);

    appGUI->event_checker_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (appGUI->event_checker_list));

    /* create columns */

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (_("Date"), renderer, 
                                                       "text", CE_COLUMN_DATE, 
                                                       NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->event_checker_list), column);

    renderer = gtk_cell_renderer_text_new();
	g_object_set (G_OBJECT(renderer), "xpad", 8, NULL);
    julian_day_column = gtk_tree_view_column_new_with_attributes ("Julian", renderer, 
                                                                  "text", CE_COLUMN_DATE_JULIAN, 
                                                                  NULL);
    gtk_tree_view_column_set_visible (julian_day_column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->event_checker_list), julian_day_column);
    gtk_tree_view_column_set_sort_column_id (julian_day_column, CE_COLUMN_DATE_JULIAN);

    renderer = gtk_cell_renderer_text_new();
	g_object_set (G_OBJECT(renderer), "xpad", 8, NULL);
    column = gtk_tree_view_column_new_with_attributes (_("Type"), renderer, 
                                                       "text", CE_COLUMN_EVENT_TYPE, 
                                                       NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->event_checker_list), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set (G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    column = gtk_tree_view_column_new_with_attributes (_("Event"), renderer, 
                                                       "text", CE_COLUMN_EVENT_LINE, 
                                                       NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->event_checker_list), column);

    /* insert day notes */

    if (day_notes_flag == TRUE) {
		cal_notes_foreach (start_date, current_date, add_note_to_event_window, appGUI);
    }

#ifdef TASKS_ENABLED

    /* insert tasks */

    if (tasks_flag == TRUE) {

        path = gtk_tree_path_new_first();

        while (gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, path) == TRUE) {
            gtk_tree_model_get (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, 
                                TA_COLUMN_DUE_DATE_JULIAN, &julian_day, 
                                TA_COLUMN_SUMMARY, &summary, 
                                TA_COLUMN_CATEGORY, &category, -1);

            j = start_date;

            while (j <= current_date) {
                if (j == julian_day && (tsk_get_category_state (category, STATE_EITHER, appGUI) == TRUE)) {
                    gtk_list_store_append(appGUI->event_checker_list_store, &iter);
                    g_snprintf (date_str, BUFFER_SIZE, "%s\n(%s)", 
								julian_to_str (julian_day, config.date_format, config.override_locale_settings), 
								utl_get_julian_day_name (julian_day));
                    gtk_list_store_set(appGUI->event_checker_list_store, &iter, 
                                       CE_COLUMN_DATE, date_str,
                                       CE_COLUMN_DATE_JULIAN, julian_day, 
                                       CE_COLUMN_EVENT_TYPE, _("Task"),
                                       CE_COLUMN_EVENT_LINE, summary, -1);
                }
                j++;
            };

            g_free (summary);
            g_free (category);

            gtk_tree_path_next(path);
        }
        gtk_tree_path_free(path);
    }
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    /* insert birthdays */

    if (birthdays_flag == TRUE) {

        sprintf(template, "(%s)", _("None"));

        path = gtk_tree_path_new_first();

        while (gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, path) == TRUE) {
            gtk_tree_model_get (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, 
                                COLUMN_BIRTH_DAY_DATE, &date, 
                                COLUMN_FIRST_NAME, &first_name,
                                COLUMN_LAST_NAME, &last_name, -1);

            if (date != 0) {

                new_cdate = g_date_new_julian (date);

                if (new_cdate != NULL) {

                    age = g_date_get_year (appGUI->cal->date) - g_date_get_year(new_cdate);

                    if (age >= 0) {

                        j = start_date;

                        while (j <= current_date) {
                            g_date_set_year (new_cdate, julian_to_year(j));
                            date = g_date_get_julian (new_cdate);
                            if (j == date) {
                                if (g_utf8_collate(first_name, template) == 0) {
                                    sprintf(tmpbuf, "%s", last_name);
                                } else if (g_utf8_collate(last_name, template) == 0) {
                                    sprintf(tmpbuf, "%s", first_name);
                                } else {
                                    sprintf(tmpbuf, "%s %s", first_name, last_name);
                                }
                                gtk_list_store_append(appGUI->event_checker_list_store, &iter);
                                sprintf (date_str, "%s\n(%s)", 
                                         julian_to_str (j, config.date_format, config.override_locale_settings), 
										 utl_get_julian_day_name (j));
                                gtk_list_store_set(appGUI->event_checker_list_store, &iter, 
                                                   CE_COLUMN_DATE, date_str,
                                                   CE_COLUMN_DATE_JULIAN, j, 
                                                   CE_COLUMN_EVENT_TYPE, _("Birthday"),
                                                   CE_COLUMN_EVENT_LINE, tmpbuf, -1);
                            }
                            j++;
                        };
                    }

                    g_date_free(new_cdate);
                }
            }

            g_free(first_name);
            g_free(last_name);

            gtk_tree_path_next(path);
        }
        gtk_tree_path_free(path);
    }

#endif  /* CONTACTS_ENABLED */

    g_signal_emit_by_name(julian_day_column, "clicked");

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 8);

    show_osmo_button = gtk_button_new_with_label (_("Show Osmo"));
    gtk_widget_show (show_osmo_button);
    GTK_WIDGET_UNSET_FLAGS(show_osmo_button, GTK_CAN_FOCUS);
    g_signal_connect(show_osmo_button, "clicked", G_CALLBACK(button_event_checker_show_osmo_window_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), show_osmo_button);

    if (config.default_stock_icons) {
        close_button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    } else {
        close_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_CLOSE);
    }
    gtk_widget_show (close_button);
    GTK_WIDGET_UNSET_FLAGS(close_button, GTK_CAN_FOCUS);
    g_signal_connect(close_button, "clicked", G_CALLBACK(button_event_checker_window_close_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), close_button);

    gtk_widget_show(appGUI->event_checker_window);

    return TRUE;
}

/*------------------------------------------------------------------------------*/

