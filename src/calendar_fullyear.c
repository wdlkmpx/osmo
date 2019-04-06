
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

#include "calendar_fullyear.h"
#include "i18n.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"
#include "options_prefs.h"
#include "calendar.h"
#include "calendar_notes.h"
#include "calendar_ical.h"
#include "calendar_widget.h"
#include "calendar_utils.h"
#include "stock_icons.h"

/*------------------------------------------------------------------------*/

void
fullyear_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	appGUI->cal->last_selected_year = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton));

	gtk_window_get_size (GTK_WINDOW (appGUI->cal->fullyear_window),
	                     &config.fy_window_size_x, &config.fy_window_size_y);
	gtk_widget_destroy (appGUI->cal->fullyear_window);
}

/*------------------------------------------------------------------------------*/

void
button_fullyear_window_close_cb (GtkButton *button, gpointer user_data)
{
	fullyear_window_close_cb (GTK_WIDGET (button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

void
select_date_day_cb (GtkWidget *widget, gpointer user_data)
{
GDate *date;

	MESSAGE *msg = (MESSAGE *) user_data;
	date = g_date_new_julian ((guint32) msg->data);
	g_return_if_fail (date != NULL);

	cal_jump_to_date (date, msg->appGUI);
	g_date_free (date);

	update_aux_calendars (msg->appGUI);
	fullyear_window_close_cb (GTK_WIDGET (widget), NULL, msg->appGUI);
}

/*------------------------------------------------------------------------*/

void
display_calendar (guint year, GUI *appGUI)
{
static MESSAGE selected_date[MAX_MONTHS * FULL_YEAR_COLS];

GDate *cdate;
gint calendar_table[MAX_MONTHS * FULL_YEAR_COLS];
guint current_day, current_month, current_year;
guint month;
gint i, idx, day, first_day, days;
gchar tmpbuf[BUFFER_SIZE], tmpbuf2[BUFFER_SIZE];

	for (i = 0; i < MAX_MONTHS * FULL_YEAR_COLS; i++) {
		calendar_table[i] = -1;
	}

	cdate = g_date_new ();
	g_return_if_fail (cdate != NULL);

	for (month = G_DATE_JANUARY; month <= G_DATE_DECEMBER; month++) {
		g_date_set_dmy (cdate, 1, month, year);
		first_day = g_date_get_weekday (cdate);
		days = g_date_get_days_in_month (month, year);

		for (i = 1; i <= days; i++) {
			calendar_table[(month - 1) * FULL_YEAR_COLS + first_day + i - 2] = i;
		}
	}

	g_date_set_time_t (cdate, time (NULL));
	current_day = g_date_get_day (cdate);
	current_month = g_date_get_month (cdate);
	current_year = g_date_get_year (cdate);

	for (month = G_DATE_JANUARY; month <= G_DATE_DECEMBER; month++) {

		for (i = 0; i < FULL_YEAR_COLS; i++) {

			idx = (month - 1) * FULL_YEAR_COLS + i;
			g_signal_handlers_disconnect_by_func (G_OBJECT (appGUI->cal->calendar_buttons[idx]),
			                                      G_CALLBACK (select_date_day_cb), &selected_date[idx]);
			day = calendar_table[idx];

			if (day > 0) {

				if (day == current_day && month == current_month && year == current_year) {
					g_snprintf (tmpbuf2, BUFFER_SIZE, "<b><u>%2d</u></b>", day);
				} else {
					g_snprintf (tmpbuf2, BUFFER_SIZE, "%2d", day);
				}

				if (i % 7 + 1 == G_DATE_SATURDAY || i % 7 + 1 == G_DATE_SUNDAY) {
					g_snprintf (tmpbuf, BUFFER_SIZE, "<span foreground='firebrick'>%s</span>", tmpbuf2);
				} else if (month % 2 == 0) {
					g_snprintf (tmpbuf, BUFFER_SIZE, "<span foreground='medium blue'>%s</span>", tmpbuf2);
				} else {
					g_strlcpy (tmpbuf, tmpbuf2, BUFFER_SIZE);
				}

				g_date_set_dmy (cdate, (GDateDay) day, month,
				                (GDateYear) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton)));
				selected_date[idx].data = (gpointer) g_date_get_julian (cdate);
				selected_date[idx].appGUI = appGUI;
				g_signal_connect (G_OBJECT (appGUI->cal->calendar_buttons[idx]), "clicked",
				                  G_CALLBACK (select_date_day_cb), &selected_date[idx]);
				gtk_button_set_label (GTK_BUTTON (appGUI->cal->calendar_buttons[idx]), "");
				gtk_label_set_markup (GTK_LABEL (GTK_BIN (appGUI->cal->calendar_buttons[idx])->child), tmpbuf);
				gtk_widget_show (appGUI->cal->calendar_buttons[idx]);

			} else {

				gtk_button_set_label (GTK_BUTTON (appGUI->cal->calendar_buttons[idx]), "");
				gtk_widget_hide (GTK_WIDGET (appGUI->cal->calendar_buttons[idx]));

			}
		}
	}

	g_date_free (cdate);
}

/*------------------------------------------------------------------------*/

void
change_to_current_year_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), utl_date_get_current_year ());
}

/*------------------------------------------------------------------------*/

void
change_to_next_year_cb (GtkWidget *widget, gpointer user_data)
{
guint year;

	GUI *appGUI = (GUI *) user_data;

	year = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton));
	year++;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), year);
}

/*------------------------------------------------------------------------*/

void
change_to_previous_year_cb (GtkWidget *widget, gpointer user_data)
{
guint year;

	GUI *appGUI = (GUI *) user_data;

	year = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton));
	if (year > JULIAN_GREGORIAN_YEAR) {
		year--;
	}
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), year);
}

/*------------------------------------------------------------------------*/

void
change_year_spin_button_cb (GtkSpinButton *spinbutton, gpointer user_data)
{
guint i, month, year;

	GUI *appGUI = (GUI *) user_data;

	year = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

	if (year == utl_date_get_current_year ()) {
		gtk_widget_set_sensitive (appGUI->cal->cyear_button, FALSE);
	} else {
		gtk_widget_set_sensitive (appGUI->cal->cyear_button, TRUE);
	}

	display_calendar (year, appGUI);

	for (month = G_DATE_JANUARY; month <= G_DATE_DECEMBER; month++) {
		i = month - 1;
		gui_calendar_select_month (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), month - 1, year);
		mark_events (appGUI->cal->fy_calendars[i], month - 1, year, appGUI);
	}
}

/*------------------------------------------------------------------------*/

void
year_info_cb (GtkWidget *widget, gpointer user_data)
{
guint month, year, w_days;
gchar tmpbuf[BUFFER_SIZE];

	GUI *appGUI = (GUI *) user_data;

	year = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton));
	w_days = 0;

	for (month = G_DATE_JANUARY; month <= G_DATE_DECEMBER; month++) {
		w_days += utl_get_weekend_days_in_month_my (month, year);
	}

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>: %d\n<b>%s</b>: %s\n\n<b>%s</b>: %s\n\n<b>%s</b>: %d\n<b>%s</b>: %d\n<b>%s</b>: %d (%.1f%%)",
	            _("Year"), year, _("Leap year"), g_date_is_leap_year (year) ? _("Yes"):_("No"),
	            _("Chinese year animal"), get_chinese_year_name (year),
	            _("Number of days"), utl_get_days_per_year (year),
	            _("Number of weeks"), utl_weeks_in_year (year),
	            _("Number of weekend days"), w_days, (double) w_days / utl_get_days_per_year (year) * 100.0);

	utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW (appGUI->cal->fullyear_window));
}

/*------------------------------------------------------------------------------*/

gint
fullyear_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	switch (event->keyval) {
		case GDK_Escape:
			gtk_widget_destroy (appGUI->cal->fullyear_window);
			return TRUE;
		case GDK_F1:
			g_signal_emit_by_name (G_OBJECT(appGUI->cal->fy_alternative_view_checkbutton), "clicked");
			return TRUE;
		case GDK_F2:
			year_info_cb (NULL, appGUI);
			return TRUE;
		case GDK_F3:
			change_to_current_year_cb (NULL, appGUI);
			return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

void
alternative_view_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	config.fy_alternative_view = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (togglebutton));

	if (config.fy_alternative_view == TRUE) {
		gtk_widget_hide (appGUI->cal->fycal_table_1);
		gtk_widget_show (appGUI->cal->fycal_table_2);
	} else {
		gtk_widget_hide (appGUI->cal->fycal_table_2);
		gtk_widget_show (appGUI->cal->fycal_table_1);
	}
}

/*------------------------------------------------------------------------------*/

void
calendar_dbclick_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	GDate *date = g_date_new ();

	gui_calendar_get_gdate (GUI_CALENDAR (widget), date);
	cal_jump_to_date (date, appGUI);
	update_aux_calendars (appGUI);
	g_date_free (date);

	fullyear_window_close_cb (GTK_WIDGET (widget), NULL, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_create_fullyear_window (GUI *appGUI)
{
GtkWidget   *vbox1, *vbox2;
GtkWidget   *hseparator;
GtkWidget   *hbuttonbox;
GtkWidget   *close_button;
GtkWidget   *hbox1;
GtkWidget   *prev_button;
GtkWidget   *info_button;
GtkObject   *fy_spinbutton_adj;
GtkWidget   *next_button;
GtkWidget   *fycal_scrolledwindow;
GtkWidget   *fycal_viewport;
GtkWidget   *vseparator;
GtkWidget   *label;
GtkWidget   *fycal_label, *fycal_vbox;
gchar       tmpbuf[BUFFER_SIZE], buffer[BUFFER_SIZE];
gint        i, j, x, y, idx;
GDate       *cdate;
guint       month, year;
gint rotate = FALSE;

	cdate = g_date_new ();
	g_return_if_fail (cdate != NULL);
	g_date_set_time_t (cdate, time (NULL));

	appGUI->cal->fullyear_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (appGUI->cal->fullyear_window), _("Full-year calendar"));
	gtk_window_set_position (GTK_WINDOW (appGUI->cal->fullyear_window), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_default_size (GTK_WINDOW (appGUI->cal->fullyear_window), config.fy_window_size_x, config.fy_window_size_y);
	gtk_window_set_modal (GTK_WINDOW (appGUI->cal->fullyear_window), TRUE);
	g_signal_connect (G_OBJECT (appGUI->cal->fullyear_window), "delete_event",
	                  G_CALLBACK (fullyear_window_close_cb), appGUI);
	gtk_window_set_transient_for (GTK_WINDOW (appGUI->cal->fullyear_window), GTK_WINDOW (appGUI->main_window));
	gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->fullyear_window), 8);
	g_signal_connect (G_OBJECT (appGUI->cal->fullyear_window), "key_press_event",
	                  G_CALLBACK (fullyear_key_press_cb), appGUI);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (appGUI->cal->fullyear_window), vbox1);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 4);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Year"));
	label = gtk_label_new (tmpbuf);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, TRUE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_misc_set_padding (GTK_MISC (label), 0, 4);

	fy_spinbutton_adj = gtk_adjustment_new (utl_date_get_current_year (), JULIAN_GREGORIAN_YEAR, 9999, 1, 10, 0);
	appGUI->cal->fy_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (fy_spinbutton_adj), 1, 0);
	gtk_widget_show (appGUI->cal->fy_spinbutton);
	g_signal_connect(appGUI->cal->fy_spinbutton, "value-changed", G_CALLBACK(change_year_spin_button_cb), appGUI);
	gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->fy_spinbutton, FALSE, FALSE, 8);
	gtk_widget_set_size_request (appGUI->cal->fy_spinbutton, 80, -1);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), TRUE);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), GTK_UPDATE_IF_VALID);

	if (config.default_stock_icons) {
		info_button = utl_gui_stock_button (GTK_STOCK_INFO, FALSE);
	} else {
		info_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_INFO, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS(info_button, GTK_CAN_FOCUS);
	gtk_widget_show (info_button);
	g_signal_connect (info_button, "clicked", G_CALLBACK (year_info_cb), appGUI);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (info_button, _("Year info"));
	}
	gtk_button_set_relief (GTK_BUTTON (info_button), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox1), info_button, FALSE, FALSE, 2);

	vseparator = gtk_vseparator_new ();
	gtk_widget_show (vseparator);
	gtk_box_pack_start (GTK_BOX (hbox1), vseparator, FALSE, FALSE, 8);

	if (config.default_stock_icons) {
		prev_button = utl_gui_stock_button (GTK_STOCK_GO_BACK, FALSE);
	} else {
		prev_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_PREV_YEAR, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS (prev_button, GTK_CAN_FOCUS);
	gtk_widget_show (prev_button);
	g_signal_connect (prev_button, "clicked", G_CALLBACK (change_to_previous_year_cb), appGUI);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (prev_button, _("Previous year"));
	}
	gtk_button_set_relief (GTK_BUTTON (prev_button), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox1), prev_button, FALSE, FALSE, 2);

	if (config.default_stock_icons) {
		appGUI->cal->cyear_button = utl_gui_stock_button (GTK_STOCK_HOME, FALSE);
	} else {
		appGUI->cal->cyear_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_TODAY, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->cyear_button, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->cyear_button);
	g_signal_connect (appGUI->cal->cyear_button, "clicked", G_CALLBACK (change_to_current_year_cb), appGUI);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (appGUI->cal->cyear_button, _("Current year"));
	}
	gtk_button_set_relief (GTK_BUTTON (appGUI->cal->cyear_button), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->cyear_button, FALSE, FALSE, 2);

	if (config.default_stock_icons) {
		next_button = utl_gui_stock_button (GTK_STOCK_GO_FORWARD, FALSE);
	} else {
		next_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_NEXT_YEAR, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS(next_button, GTK_CAN_FOCUS);
	gtk_widget_show (next_button);
	g_signal_connect (next_button, "clicked", G_CALLBACK (change_to_next_year_cb), appGUI);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (next_button, _("Next year"));
	}
	gtk_button_set_relief (GTK_BUTTON (next_button), GTK_RELIEF_NONE);
	gtk_box_pack_start (GTK_BOX (hbox1), next_button, FALSE, FALSE, 2);

	vseparator = gtk_vseparator_new ();
	gtk_widget_show (vseparator);
	gtk_box_pack_start (GTK_BOX (hbox1), vseparator, FALSE, FALSE, 8);

	appGUI->cal->fy_alternative_view_checkbutton = gtk_check_button_new_with_mnemonic (_("Alternative view"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->fy_alternative_view_checkbutton, GTK_CAN_FOCUS);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->fy_alternative_view_checkbutton), config.fy_alternative_view);
	g_signal_connect (G_OBJECT (appGUI->cal->fy_alternative_view_checkbutton), "toggled",
	                  G_CALLBACK (alternative_view_cb), appGUI);
	gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->fy_alternative_view_checkbutton, FALSE, FALSE, 2);
	gtk_widget_show (appGUI->cal->fy_alternative_view_checkbutton);

	fycal_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (fycal_scrolledwindow);
	gtk_box_pack_start (GTK_BOX (vbox1), fycal_scrolledwindow, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (fycal_scrolledwindow), 4);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (fycal_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	fycal_viewport = gtk_viewport_new (NULL, NULL);
	gtk_widget_show (fycal_viewport);
	gtk_container_add (GTK_CONTAINER (fycal_scrolledwindow), fycal_viewport);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (fycal_viewport), vbox2);

	/*-------------------------------------------------------------------------------------*/
	/* First view */

	if (config.fy_simple_view == TRUE) {
		appGUI->cal->fycal_table_1 = gtk_table_new (MAX_MONTHS + 2, FULL_YEAR_COLS + 1, FALSE);
	} else {
		appGUI->cal->fycal_table_1 = gtk_table_new (MAX_MONTHS + 4, FULL_YEAR_COLS + 3, FALSE);
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->cal->fy_alternative_view_checkbutton)) == FALSE) {
		gtk_widget_show (appGUI->cal->fycal_table_1);
	}
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->fycal_table_1, TRUE, TRUE, 0);

	vseparator = gtk_vseparator_new ();
	gtk_widget_show (vseparator);
	if (config.fy_simple_view == TRUE) {
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), vseparator, 1, 2, 0, 14,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	} else {
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), vseparator, 1, 2, 0, 16,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	}

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), hseparator, 2, 39, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), hseparator, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);

	if (config.fy_simple_view == FALSE) {

		vseparator = gtk_vseparator_new ();
		gtk_widget_show (vseparator);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), vseparator, FULL_YEAR_COLS + 2, FULL_YEAR_COLS + 3, 0, 16,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
		hseparator = gtk_hseparator_new ();
		gtk_widget_show (hseparator);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), hseparator, 2, 39, MAX_MONTHS + 2, MAX_MONTHS + 3,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_FILL), 0, 0);
		hseparator = gtk_hseparator_new ();
		gtk_widget_show (hseparator);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), hseparator, 0, 1, MAX_MONTHS + 2, MAX_MONTHS + 3,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_FILL), 0, 0);
		hseparator = gtk_hseparator_new ();
		gtk_widget_show (hseparator);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), hseparator, FULL_YEAR_COLS + 3, FULL_YEAR_COLS + 4, 1, 2,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_FILL), 0, 0);
		hseparator = gtk_hseparator_new ();
		gtk_widget_show (hseparator);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), hseparator, FULL_YEAR_COLS + 3, FULL_YEAR_COLS + 4,
		                  MAX_MONTHS + 2, MAX_MONTHS + 3,
		                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		                  (GtkAttachOptions) (GTK_FILL), 0, 0);
	}

	g_date_set_day (cdate, 1);

	for (month = G_DATE_JANUARY; month <= G_DATE_DECEMBER; month++) {

		g_date_set_month (cdate, month);
		g_date_strftime (buffer, BUFFER_SIZE, config.fy_simple_view ? "%b" : "%B", cdate);
		g_snprintf (tmpbuf, BUFFER_SIZE, (month % 2) ? "%s" : "<span foreground='medium blue'>%s</span>", buffer);

		label = gtk_label_new (NULL);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), label, 0, 1, month + 1, month + 2,
		                  (GtkAttachOptions) (GTK_FILL),
		                  (GtkAttachOptions) (0), 0, 0);

		gtk_misc_set_padding (GTK_MISC (label), 8, 0);
		gtk_label_set_markup (GTK_LABEL (label), tmpbuf);

		if (config.fy_simple_view == FALSE) {
			label = gtk_label_new (NULL);
			gtk_widget_show (label);
			gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), label,
			                  FULL_YEAR_COLS + 3, FULL_YEAR_COLS + 4, month + 1, month + 2,
			                  (GtkAttachOptions) (GTK_FILL),
			                  (GtkAttachOptions) (0), 0, 0);

			gtk_misc_set_padding (GTK_MISC (label), 8, 0);
			gtk_label_set_markup (GTK_LABEL (label), tmpbuf);
		}
	}

	/* start with monday */
	g_date_set_dmy (cdate, 1, 1, 2007);

	for (i = 0; i < FULL_YEAR_COLS; i++) {

		g_date_set_day (cdate, (i % DAYS_PER_WEEK) + 1);
		g_date_strftime (buffer, BUFFER_SIZE, "%a", cdate);
		if (g_utf8_strlen (buffer, -1) > 2) rotate = TRUE;

		if (i % 7 + 1 == G_DATE_SATURDAY || i % 7 + 1 == G_DATE_SUNDAY) {
			g_snprintf (tmpbuf, BUFFER_SIZE, "<span foreground='firebrick'>%s</span>", buffer);
		} else {
			g_snprintf (tmpbuf, BUFFER_SIZE, "%s", buffer);
		}

		label = gtk_label_new (NULL);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), label, i + 2, i + 3, 0, 1,
		                  (GtkAttachOptions) (GTK_FILL),
		                  (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_padding (GTK_MISC (label), 4, 0);
		if (rotate)
			gtk_label_set_angle (GTK_LABEL (label), 90);
		gtk_label_set_markup (GTK_LABEL (label), tmpbuf);

		if (config.fy_simple_view == FALSE) {
			label = gtk_label_new (NULL);
			gtk_widget_show (label);
			gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), label, i + 2, i + 3, MAX_MONTHS + 3, MAX_MONTHS + 4,
			                  (GtkAttachOptions) (GTK_FILL),
			                  (GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_padding (GTK_MISC (label), 4, 0);
			if (rotate)
				gtk_label_set_angle (GTK_LABEL (label), 90);
			gtk_label_set_markup (GTK_LABEL (label), tmpbuf);
		}
	}

	for (month = G_DATE_JANUARY; month <= G_DATE_DECEMBER; month++) {

		for (j = 0; j < FULL_YEAR_COLS; j++) {

			i = month - 1;
			idx = i * FULL_YEAR_COLS + j;

			appGUI->cal->calendar_buttons[idx] = gtk_button_new ();
			GTK_WIDGET_UNSET_FLAGS (appGUI->cal->calendar_buttons[idx], GTK_CAN_FOCUS);
			gtk_button_set_relief (GTK_BUTTON (appGUI->cal->calendar_buttons[idx]), GTK_RELIEF_NONE);
			gtk_widget_show (appGUI->cal->calendar_buttons[idx]);
			gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_1), appGUI->cal->calendar_buttons[idx], j+2, j+3, i+2, i+3,
			                  (GtkAttachOptions) (GTK_FILL),
			                  (GtkAttachOptions) (0), 0, 0);
		}
	}

	display_calendar (utl_date_get_current_year (), appGUI);
	gtk_widget_set_sensitive (appGUI->cal->cyear_button, FALSE);

	/*-------------------------------------------------------------------------------------*/
	/* Second view */

	appGUI->cal->fycal_table_2 = gtk_table_new (3, 4, FALSE);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->cal->fy_alternative_view_checkbutton)) == TRUE) {
		gtk_widget_show (appGUI->cal->fycal_table_2);
	}
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->fycal_table_2, TRUE, TRUE, 0);

	for (i = y = 0; y < 3; y++) {

		for (x = 0; x < 4; x++, i++) {

			fycal_vbox = gtk_vbox_new (FALSE, 0);
			gtk_widget_show (fycal_vbox);
			gtk_table_attach (GTK_TABLE (appGUI->cal->fycal_table_2), fycal_vbox, x, x + 1, y, y + 1,
			                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
			                  (GtkAttachOptions) (0), 0, 0);
			fycal_label = gtk_label_new (NULL);
			gtk_widget_show (fycal_label);
			gtk_box_pack_start (GTK_BOX (fycal_vbox), fycal_label, FALSE, FALSE, 0);
			appGUI->cal->fy_calendars[i] = gui_calendar_new ();
			g_signal_connect (appGUI->cal->fy_calendars[i], 
							  "day_selected_double_click", G_CALLBACK (calendar_dbclick_cb), appGUI);
			gtk_widget_show (appGUI->cal->fy_calendars[i]);
			GTK_WIDGET_UNSET_FLAGS (appGUI->cal->fy_calendars[i], GTK_CAN_FOCUS);
			gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->fy_calendars[i]),
			                                  (config.display_options & (GUI_CALENDAR_SHOW_DAY_NAMES |
			                                  GUI_CALENDAR_WEEK_START_MONDAY)) | GUI_CALENDAR_NO_MONTH_CHANGE);
			gui_calendar_enable_cursor (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), FALSE);
			gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.mark_color, 0, EVENT_MARKER_COLOR);
			gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.selection_color, 0, SELECTOR_COLOR);
			gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.header_color, 0, HEADER_COLOR);
			gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.weekend_color, 0, WEEKEND_COLOR);
			gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
            gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
			gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.day_note_marker);
			gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.today_marker_type, TODAY_MARKER);
			gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), config.event_marker_type, EVENT_MARKER);

			year = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton));
			month = i + 1;

			gui_calendar_select_month (GUI_CALENDAR (appGUI->cal->fy_calendars[i]), month - 1, year);
			mark_events (appGUI->cal->fy_calendars[i], month - 1, year, appGUI);
			g_date_set_dmy (cdate, 1, month, year);
			g_date_strftime (buffer, BUFFER_SIZE, "%B", cdate);
			gtk_label_set_text (GTK_LABEL (fycal_label), buffer);
			gtk_box_pack_start (GTK_BOX (fycal_vbox), appGUI->cal->fy_calendars[i], FALSE, FALSE, 0);
		}
	}

	if (appGUI->cal->last_selected_year == -1) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), utl_date_get_current_year ());
	} else {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (appGUI->cal->fy_spinbutton), appGUI->cal->last_selected_year);
	}

	/*-------------------------------------------------------------------------------------*/

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	close_button = utl_gui_create_button (GTK_STOCK_CLOSE, OSMO_STOCK_BUTTON_CLOSE, _("Close"));
	gtk_widget_show (close_button);
	g_signal_connect (close_button, "clicked", G_CALLBACK (button_fullyear_window_close_cb), appGUI);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), close_button);

	gtk_widget_show (appGUI->cal->fullyear_window);
}

/*------------------------------------------------------------------------*/

