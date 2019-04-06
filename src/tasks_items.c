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

#include "calendar.h"
#include "calendar_notes.h"
#include "calendar_utils.h"
#include "calendar_widget.h"
#include "check_events.h"
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "tasks.h"
#include "tasks_items.h"
#include "tasks_preferences_gui.h"
#include "tasks_utils.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_time.h"

#ifdef TASKS_ENABLED

/*------------------------------------------------------------------------------*/

void
tsk_delete_notify (guint id, GUI *appGUI)
{
#ifdef HAVE_LIBNOTIFY
	GSList *node;
	TASK_NTF *a;

	for (node = appGUI->tsk->notifications; node != NULL; node = node->next) {
		a = (TASK_NTF *) node->data;
		if (a->id == id) {
			notify_notification_close (a->notify, NULL);
			appGUI->tsk->notifications = g_slist_remove (appGUI->tsk->notifications, a);
			g_free (a);
			break;
		}
	}
#endif /* HAVE_LIBNOTIFY */
}

/*------------------------------------------------------------------------------*/

void
tasks_item_entered_cb (GtkWidget *widget, gpointer data) {

GtkTextBuffer *text_buffer;
GtkTextIter iter_a, iter_b;
GtkTreePath *sort_path, *filter_path, *path;
GtkTreeIter iter;
guint32 fstartdate;
guint id;
TASK_ITEM *item;
GDate *cdate;
gchar tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)data;

    item = g_new0 (TASK_ITEM, 1);

    if (appGUI->tsk->tasks_edit_state == TRUE) {

        gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->tsk->tasks_list), &sort_path, NULL);

        if (sort_path != NULL) {

            filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->tsk->tasks_sort), sort_path);

            if (filter_path != NULL) {

                path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter), filter_path);

                if (path != NULL) {
                    gtk_tree_model_get_iter(GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, path);
                    gtk_tree_model_get (GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter,
                               TA_COLUMN_ID, &id,
                               TA_COLUMN_START_DATE_JULIAN, &fstartdate, -1);
                    /* FIXME */
                    gtk_list_store_remove(appGUI->tsk->tasks_list_store, &iter);
                    gtk_tree_path_free(path);
                }

                gtk_tree_path_free(filter_path);
            }

            gtk_tree_path_free(sort_path);
        }
    }

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appGUI->tsk->desc_textview));
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &iter_a, 0);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &iter_b, -1);

    item->done = FALSE;
    item->due_date_julian = appGUI->tsk->tasks_due_julian_day;
    item->due_time = appGUI->tsk->tasks_due_time;

    if(appGUI->tsk->tasks_edit_state == TRUE) {
        item->start_date_julian = fstartdate;
        item->id = id;
    } else {
        item->start_date_julian = utl_date_get_current_julian ();
        item->id = appGUI->tsk->next_id++;
    }

	item->active = TRUE;
	item->sound_enable = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->checkb_sound_enable));

    item->repeat = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->rt_enable_checkbutton));
    item->offline_ignore = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->ignore_alarm_checkbutton));

    item->warning_days = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->aw_days_spinbutton));
    item->warning_time = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->aw_hours_spinbutton)) * 60 +
                         gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->aw_minutes_spinbutton));
    item->postpone_time = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->tsk->postpone_spinbutton));
    item->alarm_command = g_strdup((gchar *) gtk_entry_get_text(GTK_ENTRY(appGUI->tsk->alarm_cmd_entry)));

    item->repeat_time_start = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_start_hour_spinbutton)) * 60 +
                         gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_start_minute_spinbutton));
    item->repeat_time_end = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_end_hour_spinbutton)) * 60 +
                         gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_end_minute_spinbutton));
    item->repeat_time_interval = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_interval_hour_spinbutton)) * 60 +
                         gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_interval_minute_spinbutton));

    item->repeat_day_interval = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_dp_day_spinbutton));
    item->repeat_month_interval = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->rt_dp_month_spinbutton));
    item->repeat_counter = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(appGUI->tsk->repeat_counter_spinbutton));

    item->priority = g_strdup(gtk_combo_box_get_active_text(GTK_COMBO_BOX(appGUI->tsk->priority_combobox)));
    item->category = g_strdup(gtk_combo_box_get_active_text(GTK_COMBO_BOX(appGUI->tsk->category_combobox)));
    item->summary = g_strdup((gchar *) gtk_entry_get_text(GTK_ENTRY(appGUI->tsk->summary_entry)));
    item->desc = g_strdup(gtk_text_buffer_get_text(GTK_TEXT_BUFFER(text_buffer), &iter_a, &iter_b, TRUE));

	if (g_date_valid_julian (item->due_date_julian)) {
		cdate = g_date_new_julian (item->due_date_julian);
		item->repeat_start_day = g_date_get_day (cdate);
		g_date_free (cdate);
	} else {
		item->repeat_start_day = 0;
	}

    item->repeat_day = 0;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_1_checkbutton))) item->repeat_day |= 1;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_2_checkbutton))) item->repeat_day |= 2;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_3_checkbutton))) item->repeat_day |= 4;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_4_checkbutton))) item->repeat_day |= 8;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_5_checkbutton))) item->repeat_day |= 16;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_6_checkbutton))) item->repeat_day |= 32;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_7_checkbutton))) item->repeat_day |= 64;

    if (item->repeat == TRUE && item->repeat_day == 0) {
        tsk_item_free (item);
        g_snprintf (tmpbuf, BUFFER_SIZE, "%s", _("Please select at least one day when recurrency is enabled."));
        utl_gui_create_dialog (GTK_MESSAGE_ERROR, tmpbuf, GTK_WINDOW (appGUI->tsk->tasks_add_window));
        return;
    }

    item->done_date_julian = 0;

#ifdef HAVE_LIBNOTIFY
	if (appGUI->tsk->tasks_edit_state)
		tsk_delete_notify (id, appGUI);
#endif /* HAVE_LIBNOTIFY */
    add_item_to_list (item, appGUI);
    gui_systray_tooltip_update(appGUI);

    gtk_widget_destroy (appGUI->tsk->tasks_add_window);
    update_tasks_number (appGUI);
    tasks_select_first_position_in_list (appGUI);
    g_signal_emit_by_name (G_OBJECT(appGUI->cal->calendar), "day-selected");
    apply_task_attributes (appGUI);

    tsk_item_free (item);
    cal_refresh_marks (appGUI);
    update_aux_calendars (appGUI);

    if (config.save_data_after_modification) {
        write_tasks_entries (appGUI);
    }
}

/*------------------------------------------------------------------------------*/

static void
tasks_date_related_set_state (gboolean state, GUI *appGUI)
{
	gtk_widget_set_sensitive (appGUI->tsk->checkb_sound_enable, state);
	gtk_widget_set_sensitive (appGUI->tsk->aw_days_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->aw_hours_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->aw_minutes_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->postpone_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->alarm_cmd_entry, state);
	gtk_widget_set_sensitive (appGUI->tsk->alarm_cmd_valid_image, state);
	gtk_widget_set_sensitive (appGUI->tsk->rt_enable_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->rt_dp_day_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->repeat_counter_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->rt_dp_month_spinbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_1_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_2_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_3_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_4_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_5_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_6_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->dp_7_checkbutton, state);
	gtk_widget_set_sensitive (appGUI->tsk->ignore_alarm_checkbutton, state);
}

/*------------------------------------------------------------------------------*/

void
tasks_time_related_set_state (gboolean state, GUI *appGUI) {

    gtk_widget_set_sensitive (appGUI->tsk->aw_hours_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->aw_minutes_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->rt_start_hour_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->rt_start_minute_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->rt_end_hour_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->rt_end_minute_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->rt_interval_hour_spinbutton, state);
    gtk_widget_set_sensitive (appGUI->tsk->rt_interval_minute_spinbutton, state);
}

/*------------------------------------------------------------------------------*/

void
day_selected_cb (GuiCalendar *calendar, GUI *appGUI)
{
	TIME *tm;
	GDate *d = g_date_new ();

	gui_calendar_get_gdate (calendar, d);
	appGUI->tsk->tasks_due_julian_day = g_date_get_julian (d);
	g_date_free (d);

	tasks_date_related_set_state (TRUE, appGUI);

	if (gtk_expander_get_expanded (GTK_EXPANDER (appGUI->tsk->time_expander)) == TRUE) {
		tm = utl_time_new_hms (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->spinbutton_start_hour)),
		                       gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->spinbutton_start_minute)),
		                       0);
		appGUI->tsk->tasks_due_time = utl_time_get_seconds (tm);
		utl_time_free (tm);
		tasks_time_related_set_state (TRUE, appGUI);
	} else {
		appGUI->tsk->tasks_due_time = -1;
		tasks_time_related_set_state (FALSE, appGUI);
	}

	gtk_widget_destroy (appGUI->tsk->td_calendar_window);
	gtk_entry_set_text (GTK_ENTRY (appGUI->tsk->due_date_entry),
	                    get_date_time_full_str (appGUI->tsk->tasks_due_julian_day, appGUI->tsk->tasks_due_time));
}

/*------------------------------------------------------------------------------*/

void
close_calendar_cb (GtkWidget *widget, GdkEvent *event, GUI *appGUI)
{
	gtk_widget_destroy (appGUI->tsk->td_calendar_window);
}

/*------------------------------------------------------------------------------*/

void
button_close_calendar_cb (GtkButton *button, gpointer user_data)
{
	close_calendar_cb (GTK_WIDGET (button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

void
button_ok_calendar_cb (GtkButton *button, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	day_selected_cb (GUI_CALENDAR (appGUI->tsk->td_calendar), appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_set_date (GDate *cdate, GUI *appGUI)
{
	gui_calendar_select_month (GUI_CALENDAR (appGUI->tsk->td_calendar),
	                           g_date_get_month (cdate) - 1, g_date_get_year (cdate));
	gui_calendar_select_day (GUI_CALENDAR (appGUI->tsk->td_calendar), g_date_get_day (cdate));
	day_selected_cb (GUI_CALENDAR (appGUI->tsk->td_calendar), appGUI);
}

/*------------------------------------------------------------------------------*/

void
set_today_calendar_cb (GtkWidget *widget, GUI *appGUI)
{
	GDate *date = utl_date_new_current ();
	calendar_set_date (date, appGUI);
	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void
set_tomorrow_calendar_cb (GtkWidget *widget, GUI *appGUI)
{
	GDate *date = utl_date_new_current ();
	g_date_add_days (date, 1);
	calendar_set_date (date, appGUI);
	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void
nodate_calendar_cb (GtkWidget *widget, GUI *appGUI)
{
    gtk_entry_set_text (GTK_ENTRY(appGUI->tsk->due_date_entry), _("No date"));
    appGUI->tsk->tasks_due_julian_day = 0;
    appGUI->tsk->tasks_due_time = -1;
    tasks_date_related_set_state (FALSE, appGUI);
    tasks_time_related_set_state (FALSE, appGUI);
    close_calendar_cb (widget, NULL, appGUI);
}

/*------------------------------------------------------------------------------*/

gint
sd_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
        close_calendar_cb (NULL, NULL, appGUI);
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
select_date_cb (GtkWidget *widget, gpointer data) {

GtkWidget *vbox1;
GtkWidget *hbox1;
GtkWidget *alignment;
GtkWidget *today_button;
GtkWidget *tomorrow_button;
GtkWidget *nodate_button;
GtkWidget *close_button;
GtkWidget *ok_button;
GtkWidget *table;
GtkWidget *label;
GtkObject *spinbutton_adjustment;
GDate *dt;
TIME *tm;

    GUI *appGUI = (GUI *)data;

    appGUI->tsk->td_calendar_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position (GTK_WINDOW (appGUI->tsk->td_calendar_window), GTK_WIN_POS_MOUSE);
    gtk_window_set_modal (GTK_WINDOW (appGUI->tsk->td_calendar_window), TRUE);
    gtk_window_set_transient_for (GTK_WINDOW (appGUI->tsk->td_calendar_window), GTK_WINDOW (appGUI->tsk->tasks_add_window));
    gtk_window_set_decorated (GTK_WINDOW (appGUI->tsk->td_calendar_window), FALSE);
    g_signal_connect (G_OBJECT (appGUI->tsk->td_calendar_window), "key_press_event",
                      G_CALLBACK (sd_key_press_cb), appGUI);
    gtk_window_set_resizable (GTK_WINDOW (appGUI->tsk->td_calendar_window), FALSE);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->tsk->td_calendar_window), vbox1);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_box_pack_start (GTK_BOX (vbox1), alignment, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);

    /* calendar */

    appGUI->tsk->td_calendar = gui_calendar_new();
    gui_calendar_set_cursor_type (GUI_CALENDAR (appGUI->tsk->td_calendar), CURSOR_BLOCK);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->tsk->td_calendar), config.selection_color, config.selector_alpha, SELECTOR_COLOR);
    gui_calendar_set_marker (GUI_CALENDAR (appGUI->tsk->td_calendar), config.today_marker_type, TODAY_MARKER);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->tsk->td_calendar), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->tsk->td_calendar), config.header_color, 0,  HEADER_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->tsk->td_calendar), config.weekend_color, 0, WEEKEND_COLOR);
    gtk_widget_show (appGUI->tsk->td_calendar);
    g_signal_connect (GTK_OBJECT (appGUI->tsk->td_calendar), "day_selected_double_click",
                      G_CALLBACK (day_selected_cb), appGUI);
    gui_calendar_set_display_options (GUI_CALENDAR (appGUI->tsk->td_calendar), 
                                      (config.display_options & !GUI_CALENDAR_NO_MONTH_CHANGE) | 
                                      GUI_CALENDAR_SHOW_HEADING | GUI_CALENDAR_SHOW_DAY_NAMES | 
                                      (config.display_options & GUI_CALENDAR_WEEK_START_MONDAY));
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->tsk->td_calendar);

    /* time selector */

    appGUI->tsk->time_expander = gtk_expander_new (_("Set time"));
    GTK_WIDGET_UNSET_FLAGS(appGUI->tsk->time_expander, GTK_CAN_FOCUS);
    gtk_box_pack_start (GTK_BOX (vbox1), appGUI->tsk->time_expander, FALSE, FALSE, 0);
    gtk_widget_show (appGUI->tsk->time_expander);

    table = gtk_table_new (2, 3, FALSE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (appGUI->tsk->time_expander), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 4);
    gtk_table_set_row_spacings (GTK_TABLE (table), 4);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);

    label = gtk_label_new (_("Hour"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new (_("Minute"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    spinbutton_adjustment = gtk_adjustment_new (get_current_hour(), 0, 23, 1, 10, 0);
    appGUI->cal->spinbutton_start_hour = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adjustment), 1, 0);
    gtk_widget_show (appGUI->cal->spinbutton_start_hour);
    gtk_table_attach (GTK_TABLE (table), appGUI->cal->spinbutton_start_hour, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    spinbutton_adjustment = gtk_adjustment_new (get_current_minute(), 0, 59, 1, 10, 0);
    appGUI->cal->spinbutton_start_minute = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adjustment), 1, 0);
    gtk_widget_show (appGUI->cal->spinbutton_start_minute);
    gtk_table_attach (GTK_TABLE (table), appGUI->cal->spinbutton_start_minute, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    /* update status */
	if (g_date_valid_julian (appGUI->tsk->tasks_due_julian_day)) {
		dt = g_date_new_julian (appGUI->tsk->tasks_due_julian_day);
		gui_calendar_select_month (GUI_CALENDAR (appGUI->tsk->td_calendar), g_date_get_month (dt) - 1, g_date_get_year (dt));
		gui_calendar_select_day (GUI_CALENDAR (appGUI->tsk->td_calendar), g_date_get_day (dt));
		g_date_free (dt);

		if (appGUI->tsk->tasks_due_time >= 0) {
			tm = utl_time_new_seconds (appGUI->tsk->tasks_due_time);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->spinbutton_start_hour), utl_time_get_hour (tm));
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->spinbutton_start_minute), utl_time_get_minute (tm));
			gtk_expander_set_expanded (GTK_EXPANDER (appGUI->tsk->time_expander), TRUE);
			utl_time_free (tm);
		}
	}

    /* auxilary buttons */

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_box_pack_start (GTK_BOX (vbox1), alignment, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);

    hbox1 = gtk_hbox_new (FALSE, 2);
    gtk_widget_show (hbox1);
    gtk_container_add (GTK_CONTAINER (alignment), hbox1);

    today_button = gtk_button_new_with_mnemonic (_("Today"));
    gtk_widget_show (today_button);
    gtk_box_pack_start (GTK_BOX (hbox1), today_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (today_button), "clicked",
                        G_CALLBACK (set_today_calendar_cb), appGUI);

    tomorrow_button = gtk_button_new_with_mnemonic (_("Tomorrow"));
    gtk_widget_show (tomorrow_button);
    gtk_box_pack_start (GTK_BOX (hbox1), tomorrow_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (tomorrow_button), "clicked",
                        G_CALLBACK (set_tomorrow_calendar_cb), appGUI);

    nodate_button = gtk_button_new_with_mnemonic (_("No date"));
    gtk_widget_show (nodate_button);
    gtk_box_pack_start (GTK_BOX (hbox1), nodate_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (nodate_button), "clicked",
                        G_CALLBACK (nodate_calendar_cb), appGUI);

    if (config.default_stock_icons) {
        ok_button = utl_gui_stock_button (GTK_STOCK_OK, FALSE);
    } else {
        ok_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_OK, FALSE);
    }
    gtk_button_set_relief (GTK_BUTTON (ok_button), GTK_RELIEF_NONE);
    gtk_widget_show (ok_button);
    gtk_box_pack_end (GTK_BOX (hbox1), ok_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (ok_button), "clicked",
                        G_CALLBACK (button_ok_calendar_cb), appGUI);

    if (config.default_stock_icons) {
        close_button = utl_gui_stock_button (GTK_STOCK_CANCEL, FALSE);
    } else {
        close_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CANCEL, FALSE);
    }
    GTK_WIDGET_UNSET_FLAGS (close_button, GTK_CAN_FOCUS);
    gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
    gtk_widget_show (close_button);
    gtk_box_pack_end (GTK_BOX (hbox1), close_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (close_button), "clicked",
                        G_CALLBACK (button_close_calendar_cb), appGUI);

    gtk_widget_show (appGUI->tsk->td_calendar_window);
}

/*------------------------------------------------------------------------------*/

void
tasks_add_edit_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	gtk_window_get_size (GTK_WINDOW (appGUI->tsk->tasks_add_window),
	                     &config.tasks_addedit_win_w, &config.tasks_addedit_win_h);
	gdk_window_get_root_origin (GDK_WINDOW (appGUI->tsk->tasks_add_window->window),
	                            &config.tasks_addedit_win_x, &config.tasks_addedit_win_y);
	gtk_widget_destroy (appGUI->tsk->tasks_add_window);
}

/*------------------------------------------------------------------------------*/

void
button_tasks_add_edit_window_close_cb (GtkButton *button, gpointer user_data)
{
	tasks_add_edit_window_close_cb (GTK_WIDGET (button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

gint
tasks_add_edit_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

GUI *appGUI = (GUI *)data;

    switch(event->keyval) {

        case GDK_Escape:
            tasks_add_edit_window_close_cb (appGUI->tsk->tasks_add_window, NULL, appGUI);
            return TRUE;
    }

    if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->tsk->summary_entry)))) {
        appGUI->tsk->tasks_accept_state = TRUE;
    } else {
        appGUI->tsk->tasks_accept_state = FALSE;
    }

    gtk_widget_set_sensitive (appGUI->tsk->tasks_ok_button, appGUI->tsk->tasks_accept_state);

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
tasks_remove_dialog_show (GtkWidget *list, GtkListStore *list_store, GUI *appGUI) {

gint response;
GtkTreePath *sort_path, *filter_path, *path;
GtkTreeIter iter;
gchar tmpbuf[BUFFER_SIZE];
guint id;

    gtk_tree_view_get_cursor (GTK_TREE_VIEW (list), &sort_path, NULL);

    if (sort_path != NULL) {

        filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->tsk->tasks_sort), sort_path);

        if (filter_path != NULL) {

            path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter), filter_path);

            if (path != NULL) {

                sprintf (tmpbuf, "%s\n\n%s", _("Selected task will be removed."), _("Are you sure?"));

                response = utl_gui_create_dialog (GTK_MESSAGE_QUESTION, tmpbuf, GTK_WINDOW(appGUI->main_window));

                if (response == GTK_RESPONSE_YES) {
                    gtk_tree_model_get_iter (GTK_TREE_MODEL(list_store), &iter, path);
                    gtk_tree_model_get (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter,
                                        TA_COLUMN_ID, &id, -1);
#ifdef HAVE_LIBNOTIFY
                    tsk_delete_notify (id, appGUI);
#endif /* HAVE_LIBNOTIFY */
                    gtk_list_store_remove (list_store, &iter);
                    gtk_tree_path_free (path);
                    tasks_select_first_position_in_list (appGUI);
                    update_tasks_number (appGUI);
                    g_signal_emit_by_name (G_OBJECT(appGUI->cal->calendar), "day-selected");
                    gui_systray_tooltip_update (appGUI);
                    if (config.save_data_after_modification) {
                        write_tasks_entries (appGUI);
                    }
                }
            }

            gtk_tree_path_free (filter_path);
        }

        gtk_tree_path_free (sort_path);
    }
}

/*------------------------------------------------------------------------------*/

void
recurrent_task_enable_cb (GtkToggleButton *togglebutton, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    gtk_widget_set_sensitive (appGUI->tsk->recurrent_task_vbox,
                              gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(togglebutton)));
}

/*------------------------------------------------------------------------------*/

gint
alarm_cmd_entry_key_release_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
    utl_gui_update_command_status (appGUI->tsk->alarm_cmd_entry, appGUI->tsk->alarm_cmd_valid_image, appGUI);

	return FALSE;
}

/*------------------------------------------------------------------------------*/

void
tasks_add_edit_dialog_show (gboolean tasks_edit_mode, guint32 julian_date, gint time, GUI *appGUI) {

GtkWidget *hbox1, *hbox2;
GtkWidget *vbox0, *vbox1, *vbox2, *vbox3, *vbox4;
GtkWidget *vbox5, *vbox6;
GtkWidget *label;
GtkWidget *select_date_button;
GtkWidget *scrolledwindow;
GtkWidget *hseparator;
GtkWidget *vseparator;
GtkWidget *hbuttonbox;
GtkWidget *cancel_button;
GtkWidget *frame;
GtkWidget *alignment;
GtkWidget *m_table;
GtkWidget *table;
GtkWidget *notebook;
GtkObject *aw_days_spinbutton_adj;
GtkObject *aw_hours_spinbutton_adj;
GtkObject *aw_minutes_spinbutton_adj;
GtkObject *rt_start_hour_spinbutton_adj;
GtkObject *rt_start_minute_spinbutton_adj;
GtkObject *rt_end_hour_spinbutton_adj;
GtkObject *rt_end_minute_spinbutton_adj;
GtkObject *rt_interval_hour_spinbutton_adj;
GtkObject *rt_interval_minute_spinbutton_adj;
GtkObject *rt_dp_day_spinbutton_adj;
GtkObject *rt_dp_month_spinbutton_adj;
GtkObject *repeat_counter_spinbutton_adj;
GtkObject *spinbutton_adjustment;
GtkTreeIter iter;
GtkTextIter titer;
GtkTreePath *sort_path, *filter_path, *path;
GtkTextBuffer *text_buffer;
TASK_ITEM *item;
gchar tmpbuf[BUFFER_SIZE];


    appGUI->tsk->tasks_add_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    if(tasks_edit_mode == TRUE) {
        gtk_window_set_title (GTK_WINDOW (appGUI->tsk->tasks_add_window), _("Edit task"));
    } else {
        gtk_window_set_title (GTK_WINDOW (appGUI->tsk->tasks_add_window), _("Add task"));
        appGUI->tsk->tasks_due_julian_day = 0;
        appGUI->tsk->tasks_due_time = -1;
    }

    gtk_window_move (GTK_WINDOW (appGUI->tsk->tasks_add_window),
                     config.tasks_addedit_win_x, config.tasks_addedit_win_y);
    gtk_window_set_default_size (GTK_WINDOW(appGUI->tsk->tasks_add_window),
                                 config.tasks_addedit_win_w, config.tasks_addedit_win_h);

    gtk_window_set_transient_for(GTK_WINDOW(appGUI->tsk->tasks_add_window), GTK_WINDOW(appGUI->main_window));
    gtk_window_set_modal(GTK_WINDOW(appGUI->tsk->tasks_add_window), TRUE);

    g_signal_connect (G_OBJECT (appGUI->tsk->tasks_add_window), "key_press_event",
                      G_CALLBACK (tasks_add_edit_key_press_cb), appGUI);

    g_signal_connect (G_OBJECT (appGUI->tsk->tasks_add_window), "delete_event",
                      G_CALLBACK(tasks_add_edit_window_close_cb), appGUI);

    vbox0 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox0);
    gtk_container_add (GTK_CONTAINER (appGUI->tsk->tasks_add_window), vbox0);

	notebook = gtk_notebook_new ();
    GTK_WIDGET_UNSET_FLAGS (notebook, GTK_CAN_FOCUS);
    gtk_widget_show (notebook);
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_container_set_border_width (GTK_CONTAINER(notebook), 6);
    gtk_box_pack_start (GTK_BOX (vbox0), notebook, TRUE, TRUE, 0);

	/* First Tab */

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    label = gtk_label_new (_("Basic"));
    gtk_widget_show (label);
    gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox1, label);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_box_pack_start (GTK_BOX (vbox1), vbox2, TRUE, TRUE, 0);

    m_table = gtk_table_new (4, 4, FALSE);
    gtk_widget_show (m_table);
    gtk_table_set_row_spacings (GTK_TABLE (m_table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (m_table), 4);
    gtk_container_set_border_width (GTK_CONTAINER(m_table), 6);

    gtk_box_pack_start (GTK_BOX (vbox2), m_table, FALSE, FALSE, 0);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Summary"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    gtk_table_attach (GTK_TABLE (m_table), label, 0, 1, 0, 1,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Due date"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    gtk_table_attach (GTK_TABLE (m_table), label, 0, 1, 1, 2,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    appGUI->tsk->summary_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->tsk->summary_entry);

    gtk_table_attach (GTK_TABLE (m_table), appGUI->tsk->summary_entry, 1, 4, 0, 1,
                     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);

    appGUI->tsk->due_date_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->tsk->due_date_entry);
    GTK_WIDGET_UNSET_FLAGS(appGUI->tsk->due_date_entry, GTK_CAN_FOCUS);
    gtk_editable_set_editable (GTK_EDITABLE (appGUI->tsk->due_date_entry), FALSE);

    gtk_table_attach (GTK_TABLE (m_table), appGUI->tsk->due_date_entry, 1, 2, 1, 2,
                     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);

	select_date_button = utl_gui_stock_name_button (OSMO_STOCK_BUTTON_SELECT_DATE, _("Select date and time"));
    gtk_widget_show (select_date_button);
    GTK_WIDGET_UNSET_FLAGS(select_date_button, GTK_CAN_FOCUS);
    g_signal_connect(select_date_button, "clicked",
                     G_CALLBACK(select_date_cb), appGUI);

    gtk_table_attach (GTK_TABLE (m_table), select_date_button, 2, 4, 1, 2,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Category"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    gtk_table_attach (GTK_TABLE (m_table), label, 0, 1, 2, 3,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    appGUI->tsk->category_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->tsk->category_combobox);
    gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (appGUI->tsk->category_combobox), FALSE);

    gtk_table_attach (GTK_TABLE (m_table), appGUI->tsk->category_combobox, 1, 2, 2, 3,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);

	utl_gui_create_category_combobox (GTK_COMBO_BOX (appGUI->tsk->category_combobox),
									  appGUI->opt->tasks_category_store, TRUE);

	gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->tsk->category_combobox),
	                          utl_gui_list_store_get_text_index (appGUI->opt->tasks_category_store,
																 gtk_combo_box_get_active_text (GTK_COMBO_BOX (appGUI->tsk->cf_combobox))));

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Priority"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    gtk_table_attach (GTK_TABLE (m_table), label, 2, 3, 2, 3,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    appGUI->tsk->priority_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->tsk->priority_combobox);
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->tsk->priority_combobox), _("Low"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->tsk->priority_combobox), _("Medium"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->tsk->priority_combobox), _("High"));
    gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (appGUI->tsk->priority_combobox), FALSE);

    gtk_table_attach (GTK_TABLE (m_table), appGUI->tsk->priority_combobox, 3, 4, 2, 3,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);

    gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->tsk->priority_combobox), MEDIUM_PRIORITY);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_box_pack_start (GTK_BOX (vbox2), alignment, FALSE, FALSE, 2);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 6, 0);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Description"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_container_add (GTK_CONTAINER (alignment), label);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 6);
    gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

    appGUI->tsk->desc_textview = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->tsk->desc_textview), GTK_WRAP_WORD);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(appGUI->tsk->desc_textview), 4);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(appGUI->tsk->desc_textview), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(appGUI->tsk->desc_textview), 4);
    gtk_widget_show (appGUI->tsk->desc_textview);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), appGUI->tsk->desc_textview);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, TRUE, 2);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    appGUI->tsk->checkb_sound_enable = gtk_check_button_new_with_mnemonic (_("Enable sound notification"));
    gtk_widget_show (appGUI->tsk->checkb_sound_enable);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->tsk->checkb_sound_enable);
    gtk_widget_set_sensitive (appGUI->tsk->checkb_sound_enable, FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->checkb_sound_enable), TRUE);

    g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Options"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	/* Second Tab */

    vbox5 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox5);
    label = gtk_label_new (_("Advanced"));
    gtk_widget_show (label);
    gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox5, label);

	frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    gtk_box_pack_start (GTK_BOX (vbox5), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	vbox6 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox6);
    gtk_container_add (GTK_CONTAINER (frame), vbox6);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox2);
    gtk_box_pack_start (GTK_BOX (vbox6), hbox2, FALSE, TRUE, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (hbox2), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Alarm warning"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 4, 0, 12, 0);

    table = gtk_table_new (1, 6, FALSE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (alignment), table);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Days"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Hours"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Minutes"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 4, 5, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    aw_days_spinbutton_adj = gtk_adjustment_new (0, 0, 10000, 1, 10, 0);
    appGUI->tsk->aw_days_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (aw_days_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->aw_days_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->aw_days_spinbutton, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    aw_hours_spinbutton_adj = gtk_adjustment_new (0, 0, 23, 1, 10, 0);
    appGUI->tsk->aw_hours_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (aw_hours_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->aw_hours_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->aw_hours_spinbutton, 3, 4, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    aw_minutes_spinbutton_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
    appGUI->tsk->aw_minutes_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (aw_minutes_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->aw_minutes_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->aw_minutes_spinbutton, 5, 6, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

	vseparator = gtk_vseparator_new ();
	gtk_widget_show (vseparator);
	gtk_box_pack_start (GTK_BOX (hbox2), vseparator, FALSE, FALSE, 8);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (hbox2), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Postpone time"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 4, 0, 12, 0);

    table = gtk_table_new (1, 2, FALSE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (alignment), table);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Minutes"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    spinbutton_adjustment = gtk_adjustment_new (config.postpone_time, 0, 1440, 1, 10, 0);
    appGUI->tsk->postpone_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adjustment), 1, 0);
    gtk_widget_show (appGUI->tsk->postpone_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->postpone_spinbutton, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox6), frame, FALSE, FALSE, 4);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    sprintf(tmpbuf, "<b>%s:</b>", _("Alarm command"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 4, 0, 12, 0);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_container_add (GTK_CONTAINER (alignment), hbox1);

    appGUI->tsk->alarm_cmd_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->tsk->alarm_cmd_entry);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->tsk->alarm_cmd_entry, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (appGUI->tsk->alarm_cmd_entry), "key_release_event",
                      G_CALLBACK (alarm_cmd_entry_key_release_cb), appGUI);
    appGUI->tsk->alarm_cmd_valid_image = gtk_image_new();
    gtk_widget_show (appGUI->tsk->alarm_cmd_valid_image);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->tsk->alarm_cmd_valid_image, FALSE, FALSE, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox6), frame, FALSE, FALSE, 4);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    sprintf(tmpbuf, "<b>%s:</b>", _("Recurrent task"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 4, 0, 12, 0);

    vbox4 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox4);
    gtk_box_pack_start (GTK_BOX (vbox6), vbox4, FALSE, FALSE, 0);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_box_pack_start (GTK_BOX (vbox4), alignment, FALSE, FALSE, 2);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 4, 0);

    appGUI->tsk->rt_enable_checkbutton = gtk_check_button_new_with_mnemonic (_("Enable"));
    gtk_widget_show (appGUI->tsk->rt_enable_checkbutton);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->tsk->rt_enable_checkbutton);
    g_signal_connect (G_OBJECT (appGUI->tsk->rt_enable_checkbutton), "toggled",
                      G_CALLBACK (recurrent_task_enable_cb), appGUI);

    appGUI->tsk->recurrent_task_vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (appGUI->tsk->recurrent_task_vbox);
    gtk_box_pack_start (GTK_BOX (vbox4), appGUI->tsk->recurrent_task_vbox, FALSE, FALSE, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (appGUI->tsk->recurrent_task_vbox), frame, TRUE, TRUE, 2);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 2, 0, 12, 0);

    m_table = gtk_table_new (2, 3, FALSE);
    gtk_widget_show (m_table);
    gtk_container_add (GTK_CONTAINER (alignment), m_table);
    gtk_table_set_col_spacings (GTK_TABLE (m_table), 16);

    label = gtk_label_new (_("Start"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (m_table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    label = gtk_label_new (_("End"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (m_table), label, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    label = gtk_label_new (_("Interval"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (m_table), label, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (table);
    gtk_table_attach (GTK_TABLE (m_table), table, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<small>%s</small>", _("Hour"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<small>%s</small>", _("Minute"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    rt_start_hour_spinbutton_adj = gtk_adjustment_new (0, 0, 23, 1, 10, 0);
    appGUI->tsk->rt_start_hour_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_start_hour_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_start_hour_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_start_hour_spinbutton, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    rt_start_minute_spinbutton_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
    appGUI->tsk->rt_start_minute_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_start_minute_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_start_minute_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_start_minute_spinbutton, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (table);
    gtk_table_attach (GTK_TABLE (m_table), table, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<small>%s</small>", _("Hour"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<small>%s</small>", _("Minute"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    rt_end_hour_spinbutton_adj = gtk_adjustment_new (23, 0, 23, 1, 10, 0);
    appGUI->tsk->rt_end_hour_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_end_hour_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_end_hour_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_end_hour_spinbutton, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    rt_end_minute_spinbutton_adj = gtk_adjustment_new (59, 0, 59, 1, 10, 0);
    appGUI->tsk->rt_end_minute_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_end_minute_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_end_minute_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_end_minute_spinbutton, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (table);
    gtk_table_attach (GTK_TABLE (m_table), table, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<small>%s</small>", _("Hour"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<small>%s</small>", _("Minute"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    rt_interval_hour_spinbutton_adj = gtk_adjustment_new (0, 0, 23, 1, 10, 0);
    appGUI->tsk->rt_interval_hour_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_interval_hour_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_interval_hour_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_interval_hour_spinbutton, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    rt_interval_minute_spinbutton_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
    appGUI->tsk->rt_interval_minute_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_interval_minute_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_interval_minute_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_interval_minute_spinbutton, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>", _("Time period"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (appGUI->tsk->recurrent_task_vbox), frame, TRUE, TRUE, 2);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>", _("Date period"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 2, 0, 12, 0);

    vbox3 = gtk_vbox_new (FALSE, 6);
    gtk_widget_show (vbox3);
    gtk_container_add (GTK_CONTAINER (alignment), vbox3);

    table = gtk_table_new (1, 6, FALSE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox3), table, FALSE, TRUE, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

    rt_dp_day_spinbutton_adj = gtk_adjustment_new (0, 0, 100000, 1, 10, 0);
    appGUI->tsk->rt_dp_day_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_dp_day_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_dp_day_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_dp_day_spinbutton, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    rt_dp_month_spinbutton_adj = gtk_adjustment_new (0, 0, 10000, 1, 10, 0);
    appGUI->tsk->rt_dp_month_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (rt_dp_month_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->rt_dp_month_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->rt_dp_month_spinbutton, 3, 4, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    repeat_counter_spinbutton_adj = gtk_adjustment_new (0, 0, 10000, 1, 10, 0);
    appGUI->tsk->repeat_counter_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (repeat_counter_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->tsk->repeat_counter_spinbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->repeat_counter_spinbutton, 5, 6, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Days"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Months"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Repeat"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 4, 5, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Repeat in the following days"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (vbox3), label, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    table = gtk_table_new (2, 4, FALSE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox3), table, FALSE, FALSE, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

    appGUI->tsk->dp_1_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (1, FALSE));
    gtk_widget_show (appGUI->tsk->dp_1_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_1_checkbutton, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_1_checkbutton), TRUE);

    appGUI->tsk->dp_2_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (2, FALSE));
    gtk_widget_show (appGUI->tsk->dp_2_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_2_checkbutton, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_2_checkbutton), TRUE);

    appGUI->tsk->dp_3_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (3, FALSE));
    gtk_widget_show (appGUI->tsk->dp_3_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_3_checkbutton, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_3_checkbutton), TRUE);

    appGUI->tsk->dp_4_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (4, FALSE));
    gtk_widget_show (appGUI->tsk->dp_4_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_4_checkbutton, 3, 4, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_4_checkbutton), TRUE);

    appGUI->tsk->dp_5_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (5, FALSE));
    gtk_widget_show (appGUI->tsk->dp_5_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_5_checkbutton, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_5_checkbutton), TRUE);

    appGUI->tsk->dp_6_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (6, FALSE));
    gtk_widget_show (appGUI->tsk->dp_6_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_6_checkbutton, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_6_checkbutton), TRUE);

    appGUI->tsk->dp_7_checkbutton = gtk_check_button_new_with_mnemonic (utl_get_day_name (7, FALSE));
    gtk_widget_show (appGUI->tsk->dp_7_checkbutton);
    gtk_table_attach (GTK_TABLE (table), appGUI->tsk->dp_7_checkbutton, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_7_checkbutton), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (appGUI->tsk->recurrent_task_vbox), frame, TRUE, TRUE, 2);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    appGUI->tsk->ignore_alarm_checkbutton = gtk_check_button_new_with_mnemonic (_("Ignore alarm when task expired offline"));
    gtk_widget_show (appGUI->tsk->ignore_alarm_checkbutton);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->tsk->ignore_alarm_checkbutton);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>", _("Options"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    gtk_widget_set_sensitive (GTK_WIDGET(appGUI->tsk->recurrent_task_vbox), FALSE);


	/* Common */

    tasks_time_related_set_state (FALSE, appGUI);
	tasks_date_related_set_state (FALSE, appGUI);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_box_pack_start (GTK_BOX (vbox0), alignment, FALSE, FALSE, 0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 6, 6);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_container_add (GTK_CONTAINER (alignment), hseparator);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox0), hbuttonbox, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox), 6);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 16);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
    gtk_widget_show (cancel_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
	g_signal_connect (cancel_button, "clicked",
	                  G_CALLBACK (button_tasks_add_edit_window_close_cb), appGUI);

	appGUI->tsk->tasks_ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
    gtk_widget_show (appGUI->tsk->tasks_ok_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->tsk->tasks_ok_button);
	g_signal_connect (appGUI->tsk->tasks_ok_button, "clicked",
	                  G_CALLBACK (tasks_item_entered_cb), appGUI);

    appGUI->tsk->tasks_edit_state = tasks_edit_mode;
    appGUI->tsk->tasks_accept_state = FALSE;

    gtk_entry_set_text (GTK_ENTRY(appGUI->tsk->due_date_entry), _("No date"));

    if (julian_date != 0 ) {
        appGUI->tsk->tasks_due_julian_day = julian_date;
        appGUI->tsk->tasks_due_time = -1;
		gtk_entry_set_text (GTK_ENTRY (appGUI->tsk->due_date_entry),
		                    get_date_time_full_str (appGUI->tsk->tasks_due_julian_day, appGUI->tsk->tasks_due_time));
    }

    if (tasks_edit_mode == TRUE) {

        appGUI->tsk->tasks_accept_state = TRUE;

        /* fill gui fields */

        gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->tsk->tasks_list), &sort_path, NULL);

        text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(appGUI->tsk->desc_textview));
        utl_gui_clear_text_buffer (GTK_TEXT_BUFFER(text_buffer), &titer);

        if (sort_path != NULL) {

            filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->tsk->tasks_sort), sort_path);

            if (filter_path != NULL) {

                path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->tsk->tasks_filter), filter_path);

                if (path != NULL) {

                    gtk_tree_model_get_iter(GTK_TREE_MODEL(appGUI->tsk->tasks_list_store), &iter, path);

                    item = tsk_get_item (&iter, appGUI);

                    if (item != NULL) {

                        appGUI->tsk->tasks_due_julian_day = item->due_date_julian;
                        appGUI->tsk->tasks_due_time = item->due_time;


                        gtk_entry_set_text (GTK_ENTRY (appGUI->tsk->due_date_entry),
						                               get_date_time_full_str (item->due_date_julian, item->due_time));
                        gtk_entry_set_text(GTK_ENTRY(appGUI->tsk->summary_entry), item->summary);

						gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->tsk->category_combobox),
						                          utl_gui_list_store_get_text_index (appGUI->opt->tasks_category_store, 
																					 item->category));
						gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->tsk->priority_combobox),
						                          tsk_get_priority_index (item->priority));

                        gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &titer, 0);

                        if (item->desc != NULL) {
                            gtk_text_buffer_insert(text_buffer, &titer, item->desc, -1);
                            gtk_text_view_set_buffer(GTK_TEXT_VIEW(appGUI->tsk->desc_textview), text_buffer);
                        }

                        if (item->due_date_julian != 0) {

                            tasks_date_related_set_state (TRUE, appGUI);

                            if (item->due_time != -1) {
                                tasks_time_related_set_state (TRUE, appGUI);
                            } else {
                                tasks_time_related_set_state (FALSE, appGUI);
                            }

							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->aw_days_spinbutton),
							                           item->warning_days);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->aw_hours_spinbutton),
							                           item->warning_time / 60);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->aw_minutes_spinbutton),
							                           item->warning_time % 60);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->postpone_spinbutton),
							                           item->postpone_time);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_start_hour_spinbutton),
							                           item->repeat_time_start / 60);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_start_minute_spinbutton),
							                           item->repeat_time_start % 60);

							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_end_hour_spinbutton),
							                           item->repeat_time_end / 60);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_end_minute_spinbutton),
							                           item->repeat_time_end % 60);

							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_interval_hour_spinbutton),
							                           item->repeat_time_interval / 60);
							gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_interval_minute_spinbutton),
							                           item->repeat_time_interval % 60);

							gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->rt_enable_checkbutton),
							                              item->repeat);
							gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->ignore_alarm_checkbutton),
							                              item->offline_ignore);
							gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->checkb_sound_enable),
							                              item->sound_enable);

                            if (item->alarm_command) {
                                gtk_entry_set_text(GTK_ENTRY(appGUI->tsk->alarm_cmd_entry), item->alarm_command);
                                utl_gui_update_command_status (appGUI->tsk->alarm_cmd_entry, 
															   appGUI->tsk->alarm_cmd_valid_image, appGUI);
                            }

                            gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_dp_day_spinbutton), 
													   item->repeat_day_interval);
                            gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->rt_dp_month_spinbutton), 
													   item->repeat_month_interval);
                            gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->tsk->repeat_counter_spinbutton), 
													   item->repeat_counter);

                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_1_checkbutton), 
														  item->repeat_day & 1);
                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_2_checkbutton), 
														  item->repeat_day & 2);
                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_3_checkbutton), 
														  item->repeat_day & 4);
                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_4_checkbutton), 
														  item->repeat_day & 8);
                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_5_checkbutton), 
														  item->repeat_day & 16);
                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_6_checkbutton), 
														  item->repeat_day & 32);
                            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->tsk->dp_7_checkbutton), 
														  item->repeat_day & 64);

                        }

                        tsk_item_free (item);

                    }

                    gtk_tree_path_free(path);

                }

                gtk_tree_path_free(filter_path);
            }

            gtk_tree_path_free(sort_path);
        }

    }

    gtk_widget_set_sensitive(appGUI->tsk->tasks_ok_button, appGUI->tsk->tasks_accept_state);

    gtk_widget_show(appGUI->tsk->tasks_add_window);
}

/*------------------------------------------------------------------------------*/

void
read_tasks_entries (GUI *appGUI)
{
xmlDocPtr doc;
xmlChar *key;
xmlNodePtr node, cnode, category_node, main_node;
GtkTreeIter iter;
gchar *calendar_prop, *tasks_prop;
gboolean calendar_state, tasks_state;
TASK_ITEM *item;
gint priority_n;
gchar *prop;
gint tasks_version;

	if (g_file_test (prefs_get_config_filename (TASKS_ENTRIES_FILENAME, appGUI), G_FILE_TEST_IS_REGULAR) == FALSE)
		return;

	if ((doc = xmlParseFile (prefs_get_config_filename (TASKS_ENTRIES_FILENAME, appGUI)))) {

		if (!(node = xmlDocGetRootElement (doc))) {
			xmlFreeDoc (doc);
			return;
		}

		if (xmlStrcmp (node->name, (const xmlChar *) TASKS_NAME)) {
			xmlFreeDoc (doc);
			return;
		} else {
			prop = (gchar *) xmlGetProp (node, (const xmlChar *) "version");
			if (prop != NULL) {
				tasks_version = atoi (prop);
				xmlFree (prop);
			} else
				tasks_version = 0;
		}

		main_node = node->xmlChildrenNode;

		while (main_node != NULL) {

			if (!xmlStrcmp (main_node->name, (xmlChar *) TASKS_CATEGORY_ENTRIES_NAME)) {

				/* read note */
				category_node = main_node->xmlChildrenNode;

				while (category_node != NULL) {
					calendar_state = tasks_state = TRUE;

					if ((!xmlStrcmp (category_node->name, (const xmlChar *) "name"))) {
						key = xmlNodeListGetString (doc, category_node->xmlChildrenNode, 1);
						calendar_prop = (gchar *) xmlGetProp (category_node, (const xmlChar *) "calendar");
						if (calendar_prop != NULL) {
							calendar_state = atoi (calendar_prop);
							xmlFree (calendar_prop);
						}
						tasks_prop = (gchar *) xmlGetProp (category_node, (const xmlChar *) "tasks");
						if (tasks_prop != NULL) {
							tasks_state = atoi (tasks_prop);
							xmlFree (tasks_prop);
						}
						if (key != NULL) {
							gtk_list_store_append (appGUI->opt->tasks_category_store, &iter);
							gtk_list_store_set (appGUI->opt->tasks_category_store, &iter,
							                    TC_COLUMN_NAME, (gchar *) key,
							                    TC_COLUMN_CALENDAR, calendar_state,
							                    TC_COLUMN_TASKS, tasks_state, -1);
							xmlFree (key);
						}
					}

					category_node = category_node->next;
				}
			}

			/*---------------------------------------------------------------------------------------*/

			if (!xmlStrcmp (main_node->name, (xmlChar *) TASKS_ENTRIES_NAME)) {
				node = main_node->xmlChildrenNode;

				while (node != NULL) {

					if (!xmlStrcmp (node->name, (xmlChar *) "entry")) {
						cnode = node->xmlChildrenNode;

						item = g_new0 (TASK_ITEM, 1);
						if (item == NULL) continue;

						item->id = 0;
						item->done = FALSE;
						item->active = TRUE;
						item->offline_ignore = FALSE;
						item->repeat = FALSE;
						item->repeat_day = 127;
						item->repeat_day_interval = 0;
						item->repeat_start_day = 0;
						item->repeat_month_interval = 0;
						item->repeat_time_start = 0;
						item->repeat_time_end = 0;
						item->repeat_time_interval = 0;
						item->repeat_counter = 0;
						item->alarm_command = NULL;
						item->warning_days = 0;
						item->warning_time = 0;
						item->postpone_time = 0;
						item->due_date_julian = 0;
						item->due_time = -1;
						item->start_date_julian = 0;
						item->done_date_julian = 0;
						item->priority = NULL;
						item->category = NULL;
						item->summary = NULL;
						item->desc = NULL;
						item->sound_enable = TRUE;
						priority_n = MEDIUM_PRIORITY;

						while (cnode != NULL) {
							utl_xml_get_uint ("id", &(item->id), cnode);
							utl_xml_get_int ("status", &(item->done), cnode);
							utl_xml_get_uint ("due_date", &(item->due_date_julian), cnode);
							utl_xml_get_int ("due_time", &(item->due_time), cnode);
							utl_xml_get_uint ("start_date", &(item->start_date_julian), cnode);
							utl_xml_get_uint ("done_date", &(item->done_date_julian), cnode);
							utl_xml_get_int ("active", &(item->active), cnode);
							utl_xml_get_int ("offline_ignore", &(item->offline_ignore), cnode);
							utl_xml_get_int ("repeat", &(item->repeat), cnode);
							utl_xml_get_int ("repeat_day", &(item->repeat_day), cnode);
							utl_xml_get_int ("repeat_month_interval", &(item->repeat_month_interval), cnode);
							utl_xml_get_int ("repeat_day_interval", &(item->repeat_day_interval), cnode);
							utl_xml_get_int ("repeat_start_day", &(item->repeat_start_day), cnode);
							utl_xml_get_int ("repeat_time_start", &(item->repeat_time_start), cnode);
							utl_xml_get_int ("repeat_time_end", &(item->repeat_time_end), cnode);
							utl_xml_get_int ("repeat_time_interval", &(item->repeat_time_interval), cnode);
							utl_xml_get_int ("repeat_counter", &(item->repeat_counter), cnode);
							utl_xml_get_int ("warning_days", &(item->warning_days), cnode);
							utl_xml_get_int ("warning_time", &(item->warning_time), cnode);
							utl_xml_get_int ("postpone_time", &(item->postpone_time), cnode);
							utl_xml_get_int ("sound_enable", &(item->sound_enable), cnode);
							utl_xml_get_int ("priority", &priority_n, cnode);
							utl_xml_get_str ("alarm_command", &(item->alarm_command), cnode);
							utl_xml_get_str ("category", &(item->category), cnode);
							utl_xml_get_str ("summary", &(item->summary), cnode);
							utl_xml_get_str ("description", &(item->desc), cnode);
							cnode = cnode->next;
						}

						if (item->summary != NULL) {
							item->priority = g_strdup (tsk_get_priority_text (priority_n));

							if (item->due_date_julian == 0) {
								item->due_time = -1;
								item->warning_days = 0;
								item->warning_time = 0;
								item->repeat = FALSE;
								item->active = TRUE;
								item->postpone_time = 0;
							} else if (item->due_time == -1){
								item->warning_time = 0;
							}

							if (tasks_version < 208)
								item->active = TRUE;

							if (item->id == 0) {

								g_return_if_fail (tasks_version < 208);
								item->id = appGUI->tsk->next_id++;

							} else if (item->id >= appGUI->tsk->next_id)
								appGUI->tsk->next_id = item->id + 1;

							add_item_to_list (item, appGUI);
						}

						tsk_item_free (item);
					}

					node = node->next;
				}

			}

			/*---------------------------------------------------------------------------------------*/

			main_node = main_node->next;
		}

		xmlFree (node);
		xmlFreeDoc (doc);
	}
}

/*------------------------------------------------------------------------------*/

void
write_tasks_entries (GUI *appGUI)
{
xmlDocPtr doc;
xmlNodePtr main_node, node, note_node, dc_node;
xmlAttrPtr attr;
GtkTreeIter iter;
gchar *name;
gint tc_calendar, tc_tasks;
TASK_ITEM *item;
gchar temp[BUFFER_SIZE];
gint i;
xmlChar *escaped;

	if ((appGUI->save_status & WRT_TASKS) != 0) return;

	appGUI->save_status |= WRT_TASKS;

	doc = xmlNewDoc ((const xmlChar *) "1.0");
	attr = xmlNewDocProp (doc, (const xmlChar *) "encoding", (const xmlChar *) "utf-8");
	xmlFreeProp (attr);

	main_node = xmlNewNode (NULL, (const xmlChar *) TASKS_NAME);
	attr = xmlNewProp (main_node, (const xmlChar *) "version", (const xmlChar *) appGUI->version);
	xmlDocSetRootElement (doc, main_node);

	node = xmlNewChild (main_node, NULL, (const xmlChar *) TASKS_CATEGORY_ENTRIES_NAME, (xmlChar *) NULL);

	i = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->tasks_category_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->tasks_category_store), &iter,
		                    TC_COLUMN_NAME, &name, TC_COLUMN_CALENDAR, &tc_calendar, TC_COLUMN_TASKS, &tc_tasks, -1);

		escaped = xmlEncodeEntitiesReentrant(doc, (const xmlChar *) name);
		dc_node = xmlNewChild (node, NULL, (const xmlChar *) "name", (xmlChar *) escaped);
        xmlFree (escaped);

		g_snprintf (temp, BUFFER_SIZE, "%d", tc_calendar);
		escaped = xmlEncodeEntitiesReentrant(doc, (const xmlChar *) temp);
		xmlNewProp (dc_node, (const xmlChar *) "calendar", (xmlChar *) escaped);
        xmlFree (escaped);
		g_snprintf (temp, BUFFER_SIZE, "%d", tc_tasks);
		escaped = xmlEncodeEntitiesReentrant(doc, (const xmlChar *) temp);
		xmlNewProp (dc_node, (const xmlChar *) "tasks", (xmlChar *) escaped);
        xmlFree (escaped);

		g_free (name);
	}

	node = xmlNewChild (main_node, NULL, (const xmlChar *) TASKS_ENTRIES_NAME, (xmlChar *) NULL);

	i = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter, NULL, i++)) {
		item = tsk_get_item (&iter, appGUI);

		if (item != NULL) {
			note_node = xmlNewChild (node, NULL, (const xmlChar *) "entry", (xmlChar *) NULL);

			utl_xml_put_int ("id", item->id, note_node);
			utl_xml_put_int ("status", item->done, note_node);
			utl_xml_put_uint ("due_date", item->due_date_julian, note_node);
			utl_xml_put_int ("due_time", item->due_time, note_node);
			utl_xml_put_uint ("start_date", item->start_date_julian, note_node);
			utl_xml_put_uint ("done_date", item->done_date_julian, note_node);
			utl_xml_put_int ("active", item->active, note_node);
			utl_xml_put_int ("offline_ignore", item->offline_ignore, note_node);
			utl_xml_put_int ("repeat", item->repeat, note_node);
			utl_xml_put_int ("repeat_day", item->repeat_day, note_node);
			utl_xml_put_int ("repeat_month_interval", item->repeat_month_interval, note_node);
			utl_xml_put_int ("repeat_day_interval", item->repeat_day_interval, note_node);
			utl_xml_put_int ("repeat_start_day", item->repeat_start_day, note_node);
			utl_xml_put_int ("repeat_time_start", item->repeat_time_start, note_node);
			utl_xml_put_int ("repeat_time_end", item->repeat_time_end, note_node);
			utl_xml_put_int ("repeat_time_interval", item->repeat_time_interval, note_node);
			utl_xml_put_int ("repeat_counter", item->repeat_counter, note_node);
			utl_xml_put_int ("warning_days", item->warning_days, note_node);
			utl_xml_put_int ("warning_time", item->warning_time, note_node);
			utl_xml_put_int ("postpone_time", item->postpone_time, note_node);
			utl_xml_put_int ("sound_enable", item->sound_enable, note_node);
			utl_xml_put_int ("priority", tsk_get_priority_index (gettext (item->priority)), note_node);
			utl_xml_put_str ("alarm_command", item->alarm_command, note_node, doc);
			utl_xml_put_str ("category", item->category, note_node, doc);
			utl_xml_put_str ("summary", item->summary, note_node, doc);
			utl_xml_put_str ("description", item->desc, note_node, doc);

			tsk_item_free (item);
		}
	}

	xmlSaveFormatFileEnc (prefs_get_config_filename (TASKS_ENTRIES_FILENAME, appGUI), doc, "utf-8", 1);
	xmlFreeDoc (doc);

	appGUI->save_status &= ~WRT_TASKS;
}

/*------------------------------------------------------------------------------*/

#endif  /* TASKS_ENABLED */

