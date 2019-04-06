
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

#include "about.h"
#include "calendar.h"
#include "i18n.h"
#include "calendar_print.h"
#include "calendar_widget.h"
#include "calendar_jumpto.h"
#include "calendar_fullyear.h"
#include "tasks.h"
#include "tasks_items.h"
#include "contacts.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"
#include "options_prefs.h"
#include "tasks_utils.h"
#include "calendar_notes.h"
#include "calendar_timeline.h"
#include "calendar_calc.h"
#include "calendar_ical.h"
#include "check_events.h"
#include "calendar_moon.h"
#include "stock_icons.h"
#include "preferences_gui.h"

/*------------------------------------------------------------------------------*/

static void
show_about_window_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkWidget *window = opt_create_about_window (appGUI);
	gtk_widget_show (window);
}

/*------------------------------------------------------------------------------*/

static void
show_preferences_window_cb (GtkWidget *widget, GUI *appGUI)
{
	appGUI->opt->window = opt_create_preferences_window (appGUI);
	gtk_widget_show (appGUI->opt->window);

	gint page = gtk_notebook_page_num (GTK_NOTEBOOK (appGUI->opt->notebook), appGUI->opt->calendar);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->opt->notebook), page);
}

/*------------------------------------------------------------------------------*/

static void
cal_set_moon_icon (gint moon_phase, GUI *appGUI)
{
const guint8 *icons_table[] = {
    moon_phase_0, moon_phase_1, moon_phase_2, moon_phase_3,
    moon_phase_4, moon_phase_5, moon_phase_6, moon_phase_7
};

GdkPixbuf *icon;
gchar tmpbuf[BUFFER_SIZE];

    icon = gdk_pixbuf_new_from_inline (-1, icons_table[moon_phase], FALSE, NULL);
    gtk_image_set_from_pixbuf (GTK_IMAGE (appGUI->cal->moon_icon), icon);
    g_object_unref (icon);

    g_snprintf (tmpbuf, BUFFER_SIZE, "(%s)", utl_get_moon_phase_name (moon_phase));
    gtk_label_set_text (GTK_LABEL (appGUI->cal->moon_phase_label), tmpbuf);
}

/*------------------------------------------------------------------------------*/

static void
cal_mark_days_with_notes (GDate *date, GUI *appGUI)
{
GDate *tmpdate;
gint i, days;

	tmpdate = g_date_new_dmy (1, g_date_get_month (date), g_date_get_year (date));
	g_return_if_fail (tmpdate != NULL);

	if (appGUI->calendar_only == TRUE) return;

	gui_calendar_clear_marks (GUI_CALENDAR (appGUI->cal->calendar), DAY_NOTE_MARK);
	gui_calendar_clear_marks (GUI_CALENDAR (appGUI->cal->calendar), EVENT_MARK);
	gui_calendar_clear_marks (GUI_CALENDAR (appGUI->cal->calendar), BIRTHDAY_MARK);

	if (config.enable_day_mark == FALSE) return;

	days = utl_date_get_days_in_month (tmpdate);

	for (i = 1; i <= days; i++) {
		g_date_set_day (tmpdate, i);

		if (cal_check_note (g_date_get_julian (tmpdate), appGUI) == TRUE) {
			gui_calendar_set_day_color (GUI_CALENDAR (appGUI->cal->calendar), i, 
										cal_get_note_color (g_date_get_julian (tmpdate), appGUI));
		}

		calendar_mark_events (appGUI->cal->calendar, g_date_get_julian (tmpdate), i, appGUI);
	}

	g_date_free (tmpdate);
}

/*------------------------------------------------------------------------------*/

void
cal_refresh_marks (GUI *appGUI)
{
	cal_mark_days_with_notes (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

gint
cal_get_marked_days (GDate *date, GUI *appGUI)
{
	guint32 julian;
	gint i, n, days;

	julian = utl_date_dmy_to_julian (1, g_date_get_month (date), g_date_get_year (date));
	days = utl_date_get_days_in_month (date);

	n = 0;

	for (i = 0; i < days; i++)
		if (cal_check_note (julian + i, appGUI) == TRUE)
			n++;

	return n;
}

/*------------------------------------------------------------------------------*/

gint
get_marked_days (guint month, guint year, GUI *appGUI)
{
	guint32 julian;
	gint i, n, days;

    n = 0;
	days = g_date_get_days_in_month (month + 1, year);
	julian = utl_date_dmy_to_julian (1, month + 1, year);

    for (i = 0; i < days; i++)
        if (cal_check_note (julian + i, appGUI) == TRUE)
            n++;

    return n;
}

/*------------------------------------------------------------------------------*/

void
cal_update_note (GDate *date, gchar *color, GUI *appGUI)
{
GtkTextBuffer *textbuffer;
gchar *text;

	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));
	text = utl_gui_text_buffer_get_text_with_tags (GTK_TEXT_BUFFER (textbuffer));

	if (text != NULL) {

		if (strlen (text)) {
			cal_refresh_marks (appGUI);
			update_aux_calendars (appGUI);
			cal_add_note (g_date_get_julian (date), color, text, appGUI);
		} else {
			cal_remove_note (g_date_get_julian (date), appGUI);
		}

		g_free (text);
	}

	cal_set_day_info (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_update_note (guint uday, guint umonth, guint uyear, gchar *color, GUI *appGUI)
{
GtkTextBuffer *textbuffer;
gchar *text;
GDate *date;

	date = g_date_new_dmy (uday, umonth + 1, uyear);

	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));
	text = utl_gui_text_buffer_get_text_with_tags (GTK_TEXT_BUFFER (textbuffer));

	if (text != NULL) {

		if (strlen (text)) {
			cal_refresh_marks (appGUI);
			update_aux_calendars (appGUI);
			cal_add_note (g_date_get_julian (date), color, text, appGUI);
		} else {
			cal_remove_note (g_date_get_julian (date), appGUI);
		}

		g_free (text);
	}

	cal_set_day_info (appGUI);
	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void
cal_mark_events (GtkWidget *calendar, GDate *date, GUI *appGUI)
{
guint month, year;
guint i, days;
GDate *tmpdate;

	if (appGUI->calendar_only == TRUE) return;

	tmpdate = g_date_new_julian (g_date_get_julian (date));

	month = g_date_get_month (tmpdate) - 1;
	year = g_date_get_year (tmpdate);

	gui_calendar_select_day (GUI_CALENDAR (calendar), 1);
	gui_calendar_select_month (GUI_CALENDAR (calendar), month, year);
	gui_calendar_clear_marks (GUI_CALENDAR (calendar), DAY_NOTE_MARK);
	gui_calendar_clear_marks (GUI_CALENDAR (calendar), EVENT_MARK);
	gui_calendar_clear_marks (GUI_CALENDAR (calendar), BIRTHDAY_MARK);

	if (config.enable_day_mark == FALSE) return;

	days = utl_date_get_days_in_month (tmpdate);

	for (i = 1; i <= days; i++) {
		g_date_set_day (tmpdate, i);

		if (cal_check_note (g_date_get_julian (tmpdate), appGUI) == TRUE) {
			gui_calendar_set_day_color (GUI_CALENDAR (calendar), i, cal_get_note_color (g_date_get_julian (tmpdate), appGUI));
		}

		calendar_mark_events (calendar, g_date_get_julian (tmpdate), i, appGUI);
	}

	g_date_free (tmpdate);
}

/*------------------------------------------------------------------------------*/

void
mark_events (GtkWidget *calendar, guint month, guint year, GUI *appGUI)
{
	guint32 julian;
	guint i, days;

	if (appGUI->calendar_only == TRUE) return;

	gui_calendar_select_month (GUI_CALENDAR (calendar), month, year);
	gui_calendar_select_day (GUI_CALENDAR (calendar), 1);
	gui_calendar_clear_marks (GUI_CALENDAR (calendar), DAY_NOTE_MARK);
	gui_calendar_clear_marks (GUI_CALENDAR (calendar), EVENT_MARK);
	gui_calendar_clear_marks (GUI_CALENDAR (calendar), BIRTHDAY_MARK);

	if (config.enable_day_mark == FALSE) return;

	days = g_date_get_days_in_month (month + 1, year);

	for (i = 1; i <= days; i++) {
		julian = utl_date_dmy_to_julian (i, month + 1, year);
		if (cal_check_note (julian, appGUI) == TRUE) {
			gui_calendar_set_day_color (GUI_CALENDAR (calendar), i, cal_get_note_color (julian, appGUI));
		}
		calendar_mark_events (calendar, julian, i, appGUI);
	}
}

/*------------------------------------------------------------------------------*/

void
update_aux_calendars (GUI *appGUI)
{
gchar buffer[BUFFER_SIZE];
GDate *tmpdate;

	if (appGUI->calendar_only == TRUE) return;

	if (!config.gui_layout) {
		if (appGUI->calendar_only == TRUE || config.enable_auxilary_calendars == FALSE ||
			config.auxilary_calendars_state == FALSE) return;
	}

	tmpdate = g_date_new ();
	g_return_if_fail (tmpdate != NULL);

	g_date_set_julian (tmpdate, g_date_get_julian (appGUI->cal->date));
	g_date_subtract_months (tmpdate, 1);
	g_date_strftime (buffer, BUFFER_SIZE, "%B %Y", tmpdate);
	gtk_label_set_text (GTK_LABEL (appGUI->cal->prev_month_label), buffer);
	cal_mark_events (appGUI->cal->calendar_prev, tmpdate, appGUI);

	g_date_set_julian (tmpdate, g_date_get_julian (appGUI->cal->date));
	g_date_add_months (tmpdate, 1);
	g_date_strftime (buffer, BUFFER_SIZE, "%B %Y", tmpdate);
	gtk_label_set_text (GTK_LABEL (appGUI->cal->next_month_label), buffer);
	cal_mark_events (appGUI->cal->calendar_next, tmpdate, appGUI);

	g_date_free (tmpdate);
}

/*------------------------------------------------------------------------------*/

#ifdef TASKS_ENABLED

gint
check_add_tasks (GDate *date, gboolean count, GUI *appGUI)
{
GtkTreePath *path;
GtkTreeIter iter;
GtkTreeModel *model;
guint32 julian, sjulian;
gboolean done;
gint time;
gchar *summary, *category;
gchar tmpbuf[BUFFER_SIZE];
gint i;

    model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
	g_return_val_if_fail (model != NULL, 0);

	path = gtk_tree_path_new_first ();
	sjulian = g_date_get_julian (date);
	i = 0;

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {
		gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_DATE_JULIAN, &julian, TA_COLUMN_CATEGORY, &category, -1);

		if (julian == sjulian && tsk_get_category_state (category, STATE_CALENDAR, appGUI) == TRUE) {
			if (count == FALSE) {
				gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_TIME, &time, TA_COLUMN_DONE, &done,
				                    TA_COLUMN_SUMMARY, &summary, -1);

				if (time >= 0) {
					g_snprintf (tmpbuf, BUFFER_SIZE, "%d. [%02d:%02d] %s", i + 1, time / 3600, time / 60 % 60, summary);
				} else {
					g_snprintf (tmpbuf, BUFFER_SIZE, "%d. %s", i + 1, summary);
				}

				if (done == TRUE) {
					gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer,
					                                          &appGUI->cal->day_desc_iter, tmpbuf, -1, "strike", NULL);
				} else {
					gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, tmpbuf, -1);
				}
				gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n", -1);
				g_free (summary);
			}
			i++;
		}

		g_free (category);
		gtk_tree_path_next (path);
	}
	gtk_tree_path_free (path);

	return i;
}

#endif  /* TASKS_ENABLED */

/*------------------------------------------------------------------------------*/

#ifdef CONTACTS_ENABLED

gint
check_add_contacts (GDate *sdate, gboolean count, GUI *appGUI)
{
GtkTreePath *path;
GtkTreeIter iter;
GtkTreeModel *model;
guint32 julian;
gchar *first_name, *last_name;
gchar tmpbuf[BUFFER_SIZE], buffer[BUFFER_SIZE];
GDate *date;
gint i, age, syear;

	model = GTK_TREE_MODEL (appGUI->cnt->contacts_list_store);
	g_return_val_if_fail (model != NULL, 0);

	date = g_date_new ();
	g_return_val_if_fail (date != NULL, 0);

	syear = g_date_get_year (sdate);
	path = gtk_tree_path_new_first ();
	i = 0;

	while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {
		gtk_tree_model_get (model, &iter, COLUMN_BIRTH_DAY_DATE, &julian, -1);

		if (g_date_valid_julian (julian)) {
			g_date_set_julian (date, julian);
			age = syear - g_date_get_year (date);

			if (age >= 0) {
				if (g_date_valid_dmy (g_date_get_day (date), g_date_get_month (date), syear) == FALSE) {
					g_date_subtract_days (date, 1);
				}
				g_date_set_year (date, syear);

				if (g_date_compare (date, sdate) == 0) {

					if (count == FALSE) {
						gtk_tree_model_get (model, &iter, COLUMN_FIRST_NAME, &first_name, COLUMN_LAST_NAME, &last_name, -1);
						utl_name_strcat (first_name, last_name, buffer);

						if (age == 0) {
							g_snprintf (tmpbuf, BUFFER_SIZE, "%s %s\n", buffer, _("was born"));
						} else {
							g_snprintf (tmpbuf, BUFFER_SIZE, "%s (%d %s)\n", buffer, age,
							            ngettext ("year old", "years old", age));
						}

						gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, tmpbuf, -1);
					}
					i++;
				}
			}
		}
		gtk_tree_path_next (path);
	}
	gtk_tree_path_free (path);
	g_date_free (date);

	return i;
}

#endif /* CONTACTS_ENABLED */

/*------------------------------------------------------------------------------*/

gchar *
cal_get_day_category (GDate *date, GUI *appGUI)
{
GtkTreeIter iter;
static gchar buffer[BUFFER_SIZE];
gchar *color_val, *color_name, *color_sel;
gint i;

	i = 0;
	buffer[0] = '\0';
	color_sel = cal_get_note_color (g_date_get_julian (date), appGUI);
	if (color_sel == NULL) return buffer;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, 1, &color_val, 2, &color_name, -1);

		if (!strcmp (color_val, color_sel)) {
			g_snprintf (buffer, BUFFER_SIZE, "%s", color_name);
			g_free (color_val);
			g_free (color_name);
			break;
		}

		g_free (color_val);
		g_free (color_name);
	}

	return buffer;
}

/*------------------------------------------------------------------------------*/

void
update_clock (GUI *appGUI)
{
	gchar *tstr, *text;

	tstr = utl_time_print_default (utl_time_get_current_seconds (), TRUE);
	text = g_strdup_printf ("<tt>%s</tt>", tstr);
	if (appGUI->cal->time_label) {
		gtk_label_set_markup (GTK_LABEL (appGUI->cal->time_label), text);
	}
	g_free (tstr);
	g_free (text);
}

/*------------------------------------------------------------------------------*/

void
cal_set_day_info (GUI *appGUI)
{
static guint cmonth = 0;
GtkTextChildAnchor *anchor = NULL;
GtkWidget *table = NULL, *label;
gchar *text, *day_category;
gchar tmpbuf[BUFFER_SIZE];
gboolean current_date;
gint i, rows;
guint dday, dmonth, dyear;
gint edays;
GDate *date;
gchar *stripped;

	date = appGUI->cal->date;
	utl_gui_clear_text_buffer (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter);

	dday = g_date_get_day (date);
	dmonth = g_date_get_month (date) - 1;
	dyear = g_date_get_year (date);

	rows = config.di_show_current_time + config.di_show_day_number + config.di_show_marked_days + 
	       config.di_show_week_number + config.di_show_weekend_days + config.di_show_moon_phase +
	       config.di_show_zodiac_sign + config.di_show_current_day_distance + config.di_show_day_category;

	if (rows != 0) {
		anchor = gtk_text_buffer_create_child_anchor (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter);
		table = gtk_table_new (rows, 3, FALSE);
		gtk_widget_show (table);
		gtk_table_set_row_spacings (GTK_TABLE (table), 4);
		gtk_table_set_col_spacings (GTK_TABLE (table), 8);
	}

	current_date = (g_date_get_julian (date) == utl_date_get_current_julian ());
	day_category = cal_get_day_category (date, appGUI);

	i = 0;

	if (current_date && config.di_show_current_time) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Current time"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_day_number) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Day number"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
    }

    if (config.di_show_current_day_distance) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Today distance"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_week_number) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Week number"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_marked_days) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Marked days"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_weekend_days) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Weekend days"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_day_category && strlen(day_category) && config.enable_day_mark) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Day category"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_moon_phase) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Moon phase"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		i++;
	}

	if (config.di_show_zodiac_sign) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s:</b>", _("Zodiac sign"));
		label = gtk_label_new (tmpbuf);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	}

	i = 0;

	appGUI->cal->time_label = gtk_label_new (NULL);
	if (current_date && config.di_show_current_time) {
		update_clock (appGUI);
		gtk_widget_show (appGUI->cal->time_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->time_label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->time_label), 0, 0.5);
		i++;
	}

	if (config.di_show_day_number) {
		appGUI->cal->day_number_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->day_number_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->day_number_label, 1, 2, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->day_number_label), 0, 0.5);

		appGUI->cal->day_number_year_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->day_number_year_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->day_number_year_label, 2, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->day_number_year_label), 0, 0.5);
		i++;
	}

	if (config.di_show_current_day_distance) {
		label = gtk_label_new (NULL);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		gtk_label_set_text (GTK_LABEL (label), get_current_date_distance_str (g_date_get_julian (date)));
		i++;
	}

	if (config.di_show_week_number) {
		appGUI->cal->week_number_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->week_number_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->week_number_label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->week_number_label), 0, 0.5);
		i++;
	}

	if (config.di_show_marked_days) {
		appGUI->cal->marked_days_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->marked_days_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->marked_days_label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->marked_days_label), 0, 0.5);
		i++;
	}

	if (config.di_show_weekend_days) {
		appGUI->cal->weekend_days_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->weekend_days_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->weekend_days_label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->weekend_days_label), 0, 0.5);
		i++;
	}

	if (config.di_show_day_category && strlen(day_category) && config.enable_day_mark) {
		appGUI->cal->day_category_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->day_category_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->day_category_label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->day_category_label), 0, 0.5);
		i++;
	}

	if (config.di_show_moon_phase) {
		appGUI->cal->moon_icon = gtk_image_new();
		gtk_widget_show (appGUI->cal->moon_icon);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->moon_icon, 1, 2, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->moon_icon), 0, 0.5);

		appGUI->cal->moon_phase_label = gtk_label_new (NULL);
		gtk_widget_show (appGUI->cal->moon_phase_label);
		gtk_table_attach (GTK_TABLE (table), appGUI->cal->moon_phase_label, 2, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (appGUI->cal->moon_phase_label), 0, 0.5);

		cal_set_moon_icon (utl_calc_moon_phase (date), appGUI);
		i++;
	}

    if (config.di_show_zodiac_sign) {
		label = gtk_label_new (NULL);
		gtk_widget_show (label);
		gtk_table_attach (GTK_TABLE (table), label, 1, 3, i, i+1,
		                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		gtk_label_set_text (GTK_LABEL (label), utl_get_zodiac_name (g_date_get_day (date), g_date_get_month (date)));
	}

	if (config.di_show_day_number) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "%d", g_date_get_day_of_year (date));
		gtk_label_set_text (GTK_LABEL (appGUI->cal->day_number_label), tmpbuf);

		edays = utl_get_days_per_year (g_date_get_year (date)) - g_date_get_day_of_year (date);
		if (edays) {
			g_snprintf (tmpbuf, BUFFER_SIZE, "(%d %s)", edays,
			            ngettext ("day till end of year", "days till end of year", edays));
		} else {
			g_snprintf (tmpbuf, BUFFER_SIZE, "(%s)", _("the last day of the year"));
		}
		gtk_label_set_text (GTK_LABEL (appGUI->cal->day_number_year_label), tmpbuf);
	}

    if (config.di_show_week_number) {
        if (utl_get_week_number (dyear, dmonth + 1, dday) > utl_weeks_in_year (dyear)) {
            g_snprintf (tmpbuf, BUFFER_SIZE, "1 / %d", utl_weeks_in_year (dyear+1));
        } else {
            g_snprintf (tmpbuf, BUFFER_SIZE, "%d / %d", utl_get_week_number (dyear, dmonth+1, dday), utl_weeks_in_year (dyear));
        }
        gtk_label_set_text (GTK_LABEL (appGUI->cal->week_number_label), tmpbuf);
    }

	if (config.di_show_marked_days) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "%d", get_marked_days (g_date_get_month (date)-1, g_date_get_year (date), appGUI));
		gtk_label_set_text (GTK_LABEL (appGUI->cal->marked_days_label), tmpbuf);
	}

	if (config.di_show_weekend_days) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "%d", utl_get_weekend_days_in_month (date));
		gtk_label_set_text (GTK_LABEL (appGUI->cal->weekend_days_label), tmpbuf);
	}

	if (config.di_show_day_category && strlen(day_category) && config.enable_day_mark) {
		gtk_label_set_text (GTK_LABEL (appGUI->cal->day_category_label), day_category);
	}

	if (rows != 0) {
		gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW(appGUI->cal->day_desc_textview), table, anchor);
		gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n\n", -1);
	}

	if (config.di_show_notes && config.enable_day_mark) {
		text = cal_get_note (g_date_get_julian (date), appGUI);

		if (text != NULL) {
			g_snprintf (tmpbuf, BUFFER_SIZE, "%s:\n", _("Day notes"));
			gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter,
			                                          tmpbuf, -1, "bold", NULL);

			if (g_date_get_julian (date) < utl_date_get_current_julian () && config.strikethrough_past_notes == TRUE) {
				stripped = utl_gui_text_strip_tags (text);
				gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer,
				                                          &appGUI->cal->day_desc_iter, stripped, -1, "strike", NULL);
				g_free (stripped);
			} else {
				utl_gui_text_buffer_set_text_with_tags (appGUI->cal->day_desc_text_buffer, text, FALSE);
                gtk_text_buffer_get_end_iter (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter);
			}

			if (text[strlen(text)-1] != '\n') {
				gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n\n", -1);
			} else {
				gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n", -1);
			}
		}

	}

	if (appGUI->calendar_only != TRUE && appGUI->all_pages_added != FALSE && config.enable_day_mark) {

#ifdef TASKS_ENABLED
		/* check tasks */
		i = check_add_tasks (date, TRUE, appGUI);

		if (i) {
			g_snprintf (tmpbuf, BUFFER_SIZE, "%s:\n", _("Day tasks"));
			gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer,
			                                          &appGUI->cal->day_desc_iter, tmpbuf, -1, "bold", NULL);
			check_add_tasks (date, FALSE, appGUI);
			gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n", -1);
		}
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
		/* check contacts */
		i = check_add_contacts (date, TRUE, appGUI);

		if (i) {
			g_snprintf (tmpbuf, BUFFER_SIZE, "%s:\n", _("Birthday"));
			gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer,
			                                          &appGUI->cal->day_desc_iter, tmpbuf, -1, "bold", NULL);
			check_add_contacts (date, FALSE, appGUI);
			gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n", -1);
		}
#endif  /* CONTACTS_ENABLED */

	}

#ifdef HAVE_LIBICAL
	calendar_display_ics (date, appGUI);
#endif  /* HAVE_LIBICAL */

	if (cmonth != g_date_get_month (date)) {
		cmonth = g_date_get_month (date);
		update_aux_calendars (appGUI);
	}
}

/*------------------------------------------------------------------------------*/

void
cal_set_note (GDate *date, GUI *appGUI)
{
GtkTextBuffer *textbuffer;
GtkTextIter iter_start, iter_end;
gchar *t;

	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));
	t = cal_get_note (g_date_get_julian (date), appGUI);

	if (t != NULL) {
		utl_gui_text_buffer_set_text_with_tags (GTK_TEXT_BUFFER (textbuffer), t, TRUE);
	} else {
		gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (textbuffer), &iter_start);
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (textbuffer), &iter_end);
		gtk_text_buffer_delete (GTK_TEXT_BUFFER (textbuffer), &iter_start, &iter_end);
	}
}

/*------------------------------------------------------------------------------*/

void
day_notes_toggled_cb (GtkToggleToolButton *togglebutton, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	config.day_notes_visible = gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (appGUI->cal->notes_button));

	if (!config.day_notes_visible) {
		config.enable_day_mark = TRUE;
		cal_update_note (appGUI->cal->date, cal_get_note_color (g_date_get_julian (appGUI->cal->date), appGUI), appGUI);
		cal_mark_days_with_notes (appGUI->cal->date, appGUI);
		gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), FALSE);
		gtk_text_view_set_editable (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), FALSE);
		gtk_widget_hide (appGUI->cal->notes_vbox);
		gtk_widget_show (appGUI->cal->day_info_vbox);
		gui_systray_tooltip_update (appGUI);
		update_aux_calendars (appGUI);
		if (config.save_data_after_modification) {
			cal_write_notes (appGUI);
		}
	} else {
		gtk_widget_show (appGUI->cal->notes_vbox);
		gtk_widget_hide (appGUI->cal->day_info_vbox);
		gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), TRUE);
		gtk_text_view_set_editable (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), TRUE);
		gtk_widget_grab_focus (GTK_WIDGET (appGUI->cal->calendar_note_textview));
	}
}

/*------------------------------------------------------------------------------*/

void
cal_set_date (GDate *date, GUI *appGUI)
{
	g_date_set_julian (appGUI->cal->date, g_date_get_julian (date));
	gui_calendar_select_day (GUI_CALENDAR (appGUI->cal->calendar), 1);     /* Trick: always select valid day number */
	gui_calendar_select_month (GUI_CALENDAR (appGUI->cal->calendar), g_date_get_month (date) - 1, g_date_get_year (date));
	gui_calendar_select_day (GUI_CALENDAR (appGUI->cal->calendar), g_date_get_day (date));
	gtk_label_set_text (GTK_LABEL (appGUI->cal->date_label), utl_get_date_name_format (date, config.date_header_format));
}

/*------------------------------------------------------------------------------*/

void
cal_jump_to_date (GDate *date, GUI *appGUI)
{
	appGUI->cal->dont_update = TRUE;
	cal_set_date (date, appGUI);

	if (appGUI->calendar_only == FALSE) {
		cal_mark_days_with_notes (date, appGUI);
		cal_set_note (date, appGUI);
		cal_set_day_info (appGUI);
		enable_disable_note_buttons (appGUI);
	}

	appGUI->cal->dont_update = FALSE;
}

/*------------------------------------------------------------------------------*/

void
calendar_set_today (GUI *appGUI)
{
	g_date_set_time_t (appGUI->cal->date, time (NULL));
	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_close_text_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	gtk_widget_hide (appGUI->cal->notes_vbox);
	gtk_widget_show (appGUI->cal->day_info_vbox);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (appGUI->cal->notes_button), FALSE);
	gtk_widget_grab_focus (GTK_WIDGET (appGUI->cal->calendar));
}

/*------------------------------------------------------------------------------*/

gchar *
calendar_get_note_text (GUI *appGUI)
{
GtkTextBuffer *textbuffer;

	if (appGUI->calendar_only == TRUE) return NULL;

	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));
	return utl_gui_text_buffer_get_text_with_tags (GTK_TEXT_BUFFER (textbuffer));
}

/*------------------------------------------------------------------------------*/

void
calendar_clear_text_cb (GtkWidget *widget, gpointer user_data)
{
	gchar *text = NULL;
	gchar tmpbuf[BUFFER_SIZE];
	gint response;

	GUI *appGUI = (GUI *) user_data;
	text = calendar_get_note_text (appGUI);
	if (text == NULL) return;
	if (!strlen (text)) {
		g_free (text);
		return;
	}

	g_snprintf (tmpbuf, BUFFER_SIZE, "%s\n\n%s", _("Selected day note will be removed."), _("Continue?"));
	response = utl_gui_create_dialog (GTK_MESSAGE_QUESTION, tmpbuf, GTK_WINDOW (appGUI->main_window));

	if (response == GTK_RESPONSE_YES) {
		gtk_text_buffer_set_text (GTK_TEXT_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview))), "", -1);
		enable_disable_note_buttons (appGUI);
		cal_update_note (appGUI->cal->date, NULL, appGUI);
		cal_mark_days_with_notes (appGUI->cal->date, appGUI);
		gui_systray_tooltip_update (appGUI);
        if (config.save_data_after_modification) {
            cal_write_notes (appGUI);
        }
	}
	g_free (text);
}

/*------------------------------------------------------------------------------*/

void
calendar_insert_timeline_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	calendar_create_insert_timeline_window (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_select_color_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	calendar_create_color_selector_window (FALSE, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_update_date (guint day, guint month, guint year, GUI *appGUI)
{
guint max_day;
GDate *date;

	max_day = g_date_get_days_in_month (month, year);

	if (day > max_day)
		day = max_day;

	date = g_date_new_dmy (day, month, year);
	g_return_if_fail (date != NULL);

	cal_jump_to_date (date, appGUI);
	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_day (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);
	if (g_date_get_julian (appGUI->cal->date) <= 1) return;

	if ((g_date_get_day (appGUI->cal->date) > 1) || (config.display_options & GUI_CALENDAR_NO_MONTH_CHANGE) == FALSE)
		g_date_subtract_days (appGUI->cal->date, 1);

	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_day (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);

	if ((g_date_get_day (appGUI->cal->date) < utl_date_get_days_in_month (appGUI->cal->date) ||
	    (config.display_options & GUI_CALENDAR_NO_MONTH_CHANGE) == FALSE)) {
		g_date_add_days (appGUI->cal->date, 1);
	}
	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_today (GUI *appGUI)
{
	calendar_store_note (appGUI);
	calendar_set_today (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_week (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);
	if (g_date_get_julian (appGUI->cal->date) <= 7) return;

	if ((g_date_get_day (appGUI->cal->date) > 7) || (config.display_options & GUI_CALENDAR_NO_MONTH_CHANGE) == FALSE)
		g_date_subtract_days (appGUI->cal->date, 7);

	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_week (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);

	if ((g_date_get_day (appGUI->cal->date) + 7 <= utl_date_get_days_in_month (appGUI->cal->date)) ||
	    (config.display_options & GUI_CALENDAR_NO_MONTH_CHANGE) == FALSE) {
		g_date_add_days (appGUI->cal->date, 7);
	}
	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_month (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);
	if (g_date_get_julian (appGUI->cal->date) <= 31) return;

	g_date_subtract_months (appGUI->cal->date, 1);
	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_month (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);

	g_date_add_months (appGUI->cal->date, 1);
	cal_jump_to_date (appGUI->cal->date, appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_year (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);
	if (g_date_get_year (appGUI->cal->date) == 1) return;

	g_date_subtract_years (appGUI->cal->date, 1);
	cal_jump_to_date (appGUI->cal->date, appGUI);
	update_aux_calendars (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_year (GUI *appGUI)
{
	calendar_store_note (appGUI);
	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);

	g_date_add_years (appGUI->cal->date, 1);
	cal_jump_to_date (appGUI->cal->date, appGUI);
	update_aux_calendars (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_store_note (GUI *appGUI)
{
	gchar *text = NULL;
	guint32 julian = g_date_get_julian (appGUI->cal->date);

	text = calendar_get_note_text (appGUI);

	if (config.day_notes_visible && strlen (text))
		cal_add_note (julian, cal_get_note_color (julian, appGUI), text, appGUI);

	g_free (text);
}

/*------------------------------------------------------------------------------*/

void
calendar_day_selected_cb (GuiCalendar *calendar, GUI *appGUI)
{
	GDate *prev_date = g_date_new_julian (g_date_get_julian (appGUI->cal->date));

	if (appGUI->cal->dont_update == FALSE)
		gui_calendar_get_gdate (GUI_CALENDAR (calendar), appGUI->cal->date);

	gtk_label_set_text (GTK_LABEL (appGUI->cal->date_label), utl_get_date_name_format (appGUI->cal->date, config.date_header_format));

	if (appGUI->cal->dont_update == FALSE) {
		if (appGUI->calendar_only == FALSE) {
			cal_update_note (prev_date, cal_get_note_color (g_date_get_julian (prev_date), appGUI), appGUI);
			cal_mark_days_with_notes (appGUI->cal->date, appGUI);
			cal_set_note (appGUI->cal->date, appGUI);
			cal_set_day_info (appGUI);
			enable_disable_note_buttons (appGUI);
		}
	}

	g_date_free (prev_date);

	if (appGUI->calendar_only == FALSE) {
		/* enable/disable 'select day color' popup entry */
		if (cal_check_note (g_date_get_julian (appGUI->cal->date), appGUI) == TRUE) {
			gtk_widget_show (appGUI->cal->popup_menu_select_day_color_entry);
		} else {
			gtk_widget_hide (appGUI->cal->popup_menu_select_day_color_entry);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
calendar_dc_day_selected_cb (GuiCalendar *calendar, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    if (appGUI->calendar_only == FALSE) {
        if (!config.day_notes_visible) {
                enable_disable_note_buttons(appGUI);
                gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button), 
                                             !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button)));
        }
    }
}

/*------------------------------------------------------------------------------*/

void
add_calendar_toolbar_widget (GtkUIManager *uim_widget, GtkWidget *widget, gpointer user_data) {

GtkWidget *handle_box;

    GUI *appGUI = (GUI *)user_data;

    if (GTK_IS_TOOLBAR (widget)) {

        appGUI->cal->calendar_toolbar = GTK_TOOLBAR (widget);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->calendar_toolbar, GTK_CAN_FOCUS);

        handle_box = gtk_handle_box_new ();
        gtk_widget_show (handle_box);
        gtk_container_add (GTK_CONTAINER (handle_box), widget);
        gtk_box_pack_start (GTK_BOX(appGUI->cal->vbox), handle_box, FALSE, FALSE, 0);
        g_signal_connect_swapped (widget, "destroy", 
                G_CALLBACK (gtk_widget_destroy), handle_box);

    } else {
        gtk_box_pack_start (GTK_BOX(appGUI->cal->vbox), widget, FALSE, FALSE, 0);
    }

    gtk_widget_show (widget);
}

/*------------------------------------------------------------------------------*/

void
enable_disable_note_buttons (GUI *appGUI)
{
	gboolean state = FALSE;
	gchar *text = NULL;

	text = calendar_get_note_text (appGUI);

	if (strlen (text))
		state = TRUE;

	gtk_widget_set_sensitive (appGUI->cal->n_clear_button, state);
	gtk_widget_set_sensitive (appGUI->cal->n_select_color_button, state);

	g_free (text);
}

/*------------------------------------------------------------------------------*/

gint
calendar_textview_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	enable_disable_note_buttons (appGUI);

	return FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
click_handler_cb (GtkWidget * widget, GdkEventButton * event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {    /* RMB */
        gtk_menu_popup(GTK_MENU(appGUI->cal->month_selector_menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    } else if (event->type == GDK_BUTTON_PRESS && event->button == 2) {     /* MMB */
        calendar_update_date (g_date_get_day (appGUI->cal->date), utl_date_get_current_month (),
		                      g_date_get_year (appGUI->cal->date), appGUI);
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
calendar_create_jumpto_window_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	calendar_create_jumpto_window (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_create_fullyear_window_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_create_fullyear_window (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_create_datecalc_window_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_create_calc_window (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_year_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_prev_year (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_year_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_next_year (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_month_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_prev_month (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_month_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_next_month (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_prev_day_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_prev_day (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_next_day_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_next_day (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_btn_today_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_btn_today (appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_edit_note_cb (GtkToggleToolButton *toggle_tool_button, gpointer data) {

    GUI *appGUI = (GUI *)data;
    day_notes_toggled_cb (toggle_tool_button, appGUI);
}

/*------------------------------------------------------------------------------*/ 

void
aux_cal_expander_cb (GObject *object, GParamSpec *param_spec, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;
    config.auxilary_calendars_state = gtk_expander_get_expanded (GTK_EXPANDER(appGUI->cal->aux_cal_expander));
	update_aux_calendars (appGUI);
}

/*------------------------------------------------------------------------------*/ 

gboolean
mouse_button_click_handler_cb (GtkWidget * widget, GdkEventButton * event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->type == GDK_BUTTON_PRESS && event->button == 3 && appGUI->calendar_only == FALSE) {    /* RMB */
        gtk_menu_popup(GTK_MENU(appGUI->cal->popup_menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

#ifdef TASKS_ENABLED
void
popup_add_task_entry_selected_cb (gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	GDate *date = g_date_new ();

	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), date);
	tasks_add_edit_dialog_show (FALSE, g_date_get_julian (date), utl_time_get_current_seconds (), appGUI);
	g_date_free (date);
}
#endif  /* TASKS_ENABLED */

/*------------------------------------------------------------------------------*/

void
popup_select_day_color_entry_selected_cb (gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	calendar_create_color_selector_window (TRUE, appGUI);
}

/*------------------------------------------------------------------------------*/

void
popup_browse_notes_cb (gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	cal_notes_browser (appGUI);
}

/*------------------------------------------------------------------------------*/

#ifdef PRINTING_SUPPORT
void
calendar_print_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    calendar_create_print_window (appGUI);
}
#endif /* PRINTING_SUPPORT */

/*------------------------------------------------------------------------------*/

#ifdef HAVE_LIBICAL
void
popup_ical_export_cb (gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
    ical_export (appGUI);
}

void
popup_ical_browse_cb (gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	ical_events_browser (appGUI);
}
#endif  /* HAVE_LIBICAL */

/*------------------------------------------------------------------------------*/

void
calendar_set_text_attribute_cb (GtkWidget *widget, gpointer user_data) {

GtkTextBuffer *buffer;
gchar *tagname;

	GUI *appGUI = (GUI *)user_data;

	tagname = (gchar*) g_object_get_data (G_OBJECT (widget), "tag");
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));
	utl_gui_text_buffer_toggle_tags (buffer, tagname);
	g_signal_emit_by_name(G_OBJECT(buffer), "changed");
}

/*------------------------------------------------------------------------------*/

void
date_selected_cb (GtkCalendar *calendar, gpointer user_data)
{
	GUI *appGUI = (GUI *)user_data;
	GDate *date = g_date_new ();

	if (!config.gui_layout) {
        if (gtk_expander_get_expanded (GTK_EXPANDER(appGUI->cal->aux_cal_expander)) == FALSE) {
			g_date_free (date);
			return;
		}
	}

	gui_calendar_get_gdate (GUI_CALENDAR (calendar), date);
	cal_jump_to_date (date, appGUI);
	update_aux_calendars (appGUI);
	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void
gui_create_calendar(GtkWidget *notebook, GUI *appGUI) {

GtkWidget       *vbox1;
GtkWidget       *vbox2;
GtkWidget       *vbox3;
GtkWidget       *hbox1 = NULL;
GtkWidget       *hbox2;
GtkWidget       *hbox3;
GtkWidget       *eventbox;
GtkWidget       *hseparator;
GtkWidget       *alignment;
GtkWidget       *label;
GtkWidget       *frame;
GtkWidget       *vseparator;
GtkUIManager    *uim_widget = NULL;
GError          *error = NULL;
GtkActionGroup  *action_group = NULL;
GtkWidget       *note_scrolledwindow;
#ifdef TASKS_ENABLED
GtkWidget       *popup_menu_add_task_entry;
#endif  /* TASKS_ENABLED */
GtkWidget       *popup_menu_separator;
GtkWidget       *popup_menu_browse_notes;
GtkTextBuffer   *buffer;

#ifdef HAVE_LIBICAL
GtkWidget       *popup_menu_ical_browse;
GtkWidget       *popup_menu_ical_export;
#endif  /* HAVE_LIBICAL */

gchar tmpbuf[BUFFER_SIZE];

const gchar *ui_info =
"  <toolbar name=\"toolbar\">\n"
"    <toolitem name=\"previous_year\" action=\"previous_year\" />\n"
"    <toolitem name=\"previous_month\" action=\"previous_month\" />\n"
"    <toolitem name=\"previous_day\" action=\"previous_day\" />\n"
"    <toolitem name=\"today\" action=\"today\" />\n"
"    <toolitem name=\"next_day\" action=\"next_day\" />\n"
"    <toolitem name=\"next_month\" action=\"next_month\" />\n"
"    <toolitem name=\"next_year\" action=\"next_year\" />\n"
"    <separator/>\n"
"    <toolitem name=\"jump_to_date\" action=\"jump_to_date\" />\n"
"    <toolitem name=\"full_year\" action=\"full_year\" />\n"
"    <toolitem name=\"date_calc\" action=\"date_calc\" />\n"
#ifdef PRINTING_SUPPORT
"    <toolitem name=\"print\" action=\"print\" />\n"
#endif /* PRINTING_SUPPORT */
"    <separator/>\n"
"    <toolitem name=\"edit_note\" action=\"edit_note\" />\n"
"    <separator expand=\"true\" />\n"
"    <toolitem name=\"preferences\" action=\"preferences\" />\n"
"    <toolitem name=\"about\" action=\"about\" />\n"
"  </toolbar>\n";

GtkActionEntry entries[] = {
    { "previous_year", OSMO_STOCK_PREV_YEAR, _("Previous year"), NULL, _("Previous year"), G_CALLBACK(calendar_btn_prev_year_cb)},
    { "previous_month", OSMO_STOCK_PREV_MONTH, _("Previous month"), NULL, _("Previous month"), G_CALLBACK(calendar_btn_prev_month_cb)},
    { "previous_day", OSMO_STOCK_PREV_DAY, _("Previous day"), NULL, _("Previous day"), G_CALLBACK(calendar_btn_prev_day_cb)},
    { "today", OSMO_STOCK_TODAY, _("Today"), NULL, _("Today"), G_CALLBACK(calendar_btn_today_cb)},
    { "next_day", OSMO_STOCK_NEXT_DAY, _("Next day"), NULL, _("Next day"), G_CALLBACK(calendar_btn_next_day_cb)},
    { "next_month", OSMO_STOCK_NEXT_MONTH, _("Next month"), NULL, _("Next month"), G_CALLBACK(calendar_btn_next_month_cb)},
    { "next_year", OSMO_STOCK_NEXT_YEAR, _("Next year"), NULL, _("Next year"), G_CALLBACK(calendar_btn_next_year_cb)},
    { "jump_to_date", OSMO_STOCK_JUMPTO, _("Jump to date"), NULL, _("Jump to date"), G_CALLBACK(calendar_create_jumpto_window_cb)},
    { "full_year", OSMO_STOCK_FULLYEAR, _("Full-year calendar"), NULL, _("Full-year calendar"), G_CALLBACK(calendar_create_fullyear_window_cb)},   
#ifdef PRINTING_SUPPORT
    { "print", OSMO_STOCK_PRINT, _("Print calendar"), NULL, _("Print calendar"), G_CALLBACK(calendar_print_cb)},
#endif /* PRINTING_SUPPORT */
    { "date_calc", OSMO_STOCK_CALCULATOR, _("Date calculator"), NULL, _("Date calculator"), G_CALLBACK(calendar_create_datecalc_window_cb)},
	{ "preferences", OSMO_STOCK_PREFERENCES, _("Preferences"), NULL, _("Preferences"), G_CALLBACK(show_preferences_window_cb)},
	{ "about", OSMO_STOCK_ABOUT, _("About"), NULL, _("About"), G_CALLBACK (show_about_window_cb)},
};

GtkToggleActionEntry t_entries[] = {
    { "edit_note", OSMO_STOCK_EDIT_NOTE, _("Toggle day note panel"), NULL, _("Toggle day note panel"), G_CALLBACK(calendar_edit_note_cb), FALSE }
};

guint n_entries = G_N_ELEMENTS (entries);
guint n_t_entries = G_N_ELEMENTS (t_entries);


    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox1), 8);
    g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>", _("Calendar"));
    gui_add_to_notebook (vbox1, tmpbuf, appGUI);

    appGUI->cal->vbox = GTK_BOX(vbox1);

    if (config.hide_calendar == TRUE) {
        gtk_widget_hide(GTK_WIDGET(appGUI->cal->vbox));
    }

    /*-------------------------------------------------------------------------------------*/

    action_group = gtk_action_group_new ("_actions");
    gtk_action_group_add_actions (action_group, entries, n_entries, appGUI);
    gtk_action_group_add_toggle_actions (action_group, t_entries, n_t_entries, appGUI);
    gtk_action_group_set_sensitive(action_group, TRUE);

    uim_widget = gtk_ui_manager_new ();

    gtk_ui_manager_insert_action_group (uim_widget, action_group, 0);
    g_signal_connect (uim_widget, "add_widget", G_CALLBACK (add_calendar_toolbar_widget), appGUI);

    if (!gtk_ui_manager_add_ui_from_string (uim_widget, ui_info, -1, &error)) {
        g_message ("building toolbar failed: %s", error->message);
        g_error_free (error);
    }
    gtk_ui_manager_ensure_update (uim_widget);

    gtk_toolbar_set_style (appGUI->cal->calendar_toolbar, GTK_TOOLBAR_ICONS);

    appGUI->cal->notes_button = gtk_ui_manager_get_widget (uim_widget, "/toolbar/edit_note");

    if (appGUI->calendar_only == TRUE) {
        gtk_widget_hide (appGUI->cal->notes_button);
        gtk_widget_set_sensitive (appGUI->cal->notes_button, FALSE);
#ifdef PRINTING_SUPPORT
        gtk_widget_hide (gtk_ui_manager_get_widget (uim_widget, "/toolbar/print"));
#endif /* PRINTING_SUPPORT */
        gtk_widget_hide (gtk_ui_manager_get_widget (uim_widget, "/toolbar/preferences"));
        gtk_widget_hide (gtk_ui_manager_get_widget (uim_widget, "/toolbar/about"));
    }

    /*-------------------------------------------------------------------------------------*/

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_box_pack_start (GTK_BOX (vbox1), vbox2, TRUE, TRUE, 0);

	if (config.gui_layout && appGUI->calendar_only == FALSE) {
		hbox1 = gtk_hbox_new (FALSE, 6);
		gtk_widget_show (hbox1);
		gtk_box_pack_start (GTK_BOX (vbox2), hbox1, TRUE, TRUE, 0);
	}

	vbox3 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox3);
    if (appGUI->calendar_only == TRUE) {
			gtk_box_pack_start (GTK_BOX (vbox2), vbox3, TRUE, TRUE, 0);
	} else {
		if (!config.gui_layout) {
			gtk_box_pack_start (GTK_BOX (vbox2), vbox3, TRUE, TRUE, 0);
		} else {
			gtk_box_pack_start (GTK_BOX (hbox1), vbox3, FALSE, FALSE, 0);
		}
	}

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox3), hseparator, FALSE, FALSE, 6);

    eventbox = gtk_event_box_new ();
    gtk_widget_show (eventbox);
    gtk_box_pack_start (GTK_BOX (vbox3), eventbox, FALSE, FALSE, 0);
  	g_signal_connect (G_OBJECT(eventbox), "button_press_event", G_CALLBACK(click_handler_cb), appGUI);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox2);
    gtk_container_add (GTK_CONTAINER (eventbox), hbox2);

    /* Calendar popup menu */

    if (appGUI->calendar_only == FALSE) {

        appGUI->cal->popup_menu = gtk_menu_new();

#ifdef TASKS_ENABLED
        popup_menu_add_task_entry = gtk_menu_item_new_with_label(_("Add task"));
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), popup_menu_add_task_entry);
        g_signal_connect_swapped(G_OBJECT(popup_menu_add_task_entry), "activate", 
                                 G_CALLBACK(popup_add_task_entry_selected_cb), appGUI);
        gtk_widget_show(popup_menu_add_task_entry);
#endif  /* TASKS_ENABLED */

        appGUI->cal->popup_menu_select_day_color_entry = gtk_menu_item_new_with_label(_("Select day color"));
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), appGUI->cal->popup_menu_select_day_color_entry);
        g_signal_connect_swapped(G_OBJECT(appGUI->cal->popup_menu_select_day_color_entry), "activate", 
                                 G_CALLBACK(popup_select_day_color_entry_selected_cb), appGUI);
        gtk_widget_show(appGUI->cal->popup_menu_select_day_color_entry);

        popup_menu_separator = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), popup_menu_separator);
        gtk_widget_show(popup_menu_separator);

        popup_menu_browse_notes = gtk_menu_item_new_with_label(_("Browse notes"));
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), popup_menu_browse_notes);
        g_signal_connect_swapped(G_OBJECT(popup_menu_browse_notes), "activate", 
                                 G_CALLBACK(popup_browse_notes_cb), appGUI);
        gtk_widget_show(popup_menu_browse_notes);

#ifdef HAVE_LIBICAL

        popup_menu_ical_browse = gtk_menu_item_new_with_label(_("Browse iCal events"));
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), popup_menu_ical_browse);
        g_signal_connect_swapped(G_OBJECT(popup_menu_ical_browse), "activate", 
                                 G_CALLBACK(popup_ical_browse_cb), appGUI);
        gtk_widget_show(popup_menu_ical_browse);

        popup_menu_separator = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), popup_menu_separator);
        gtk_widget_show(popup_menu_separator);

        popup_menu_ical_export = gtk_menu_item_new_with_label(_("Export to iCal file"));
        gtk_menu_shell_append(GTK_MENU_SHELL(appGUI->cal->popup_menu), popup_menu_ical_export);
        g_signal_connect_swapped(G_OBJECT(popup_menu_ical_export), "activate", 
                                 G_CALLBACK(popup_ical_export_cb), appGUI);
        gtk_widget_show(popup_menu_ical_export);

#endif  /* HAVE_LIBICAL */

    }

    appGUI->cal->date_label = gtk_label_new (NULL);
    gtk_widget_show (appGUI->cal->date_label);
    gtk_box_pack_start (GTK_BOX (hbox2), appGUI->cal->date_label, TRUE, FALSE, 8);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox3), hseparator, FALSE, FALSE, 6);

    appGUI->cal->calendar = gui_calendar_new ();
    gtk_widget_show (appGUI->cal->calendar);
    gui_calendar_set_cursor_type(GUI_CALENDAR(appGUI->cal->calendar), config.cursor_type);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cal->calendar, GTK_CAN_FOCUS);
    gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->calendar), config.display_options);
    gtk_widget_modify_font (GTK_WIDGET(appGUI->cal->calendar), appGUI->cal->fd_cal_font);
    gtk_box_pack_start (GTK_BOX (vbox3), appGUI->cal->calendar, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (appGUI->cal->calendar), "day-selected", 
                      G_CALLBACK (calendar_day_selected_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->calendar), "day-selected-double-click", 
                      G_CALLBACK (calendar_dc_day_selected_cb), appGUI);
  	g_signal_connect (G_OBJECT(appGUI->cal->calendar), "button_press_event", 
                      G_CALLBACK(mouse_button_click_handler_cb), appGUI);
    gui_calendar_enable_cursor (GUI_CALENDAR (appGUI->cal->calendar), TRUE);

    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.mark_color, 0, EVENT_MARKER_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.selection_color, config.selector_alpha, SELECTOR_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.header_color, 0,  HEADER_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.weekend_color, 0, WEEKEND_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
    gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->calendar), config.day_note_marker);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar), config.event_marker_type, EVENT_MARKER);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar), config.today_marker_type, TODAY_MARKER);

    appGUI->cal->month_selector_menu = gtk_menu_new();
    calendar_create_popup_menu (appGUI->cal->month_selector_menu, appGUI);

    /*-------------------------------------------------------------------------------------*/

    if (appGUI->calendar_only == FALSE) {

		if (!config.gui_layout) {
			appGUI->cal->aux_cal_expander = gtk_expander_new (_("Previous and next month"));
			GTK_WIDGET_UNSET_FLAGS(appGUI->cal->aux_cal_expander, GTK_CAN_FOCUS);
			gtk_box_pack_start (GTK_BOX (vbox3), appGUI->cal->aux_cal_expander, FALSE, FALSE, 0);
		}

        appGUI->cal->aux_calendars_table = gtk_table_new (2, 2, FALSE);
		if (!config.gui_layout) {
            gtk_container_add (GTK_CONTAINER (appGUI->cal->aux_cal_expander), appGUI->cal->aux_calendars_table);
		} else {
			gtk_box_pack_start (GTK_BOX (vbox3), appGUI->cal->aux_calendars_table, FALSE, FALSE, 0);
		}
        gtk_table_set_row_spacings (GTK_TABLE (appGUI->cal->aux_calendars_table), 4);
        gtk_table_set_col_spacings (GTK_TABLE (appGUI->cal->aux_calendars_table), 4);

		if (config.enable_auxilary_calendars == TRUE) {
			if (!config.gui_layout) {
                gtk_widget_show (appGUI->cal->aux_cal_expander);
			}
            gtk_widget_show (appGUI->cal->aux_calendars_table);
        }

        appGUI->cal->prev_month_label = gtk_label_new ("");
        gtk_widget_show (appGUI->cal->prev_month_label);
        gtk_table_attach (GTK_TABLE (appGUI->cal->aux_calendars_table), appGUI->cal->prev_month_label, 0, 1, 0, 1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

        appGUI->cal->next_month_label = gtk_label_new ("");
        gtk_widget_show (appGUI->cal->next_month_label);
        gtk_table_attach (GTK_TABLE (appGUI->cal->aux_calendars_table), appGUI->cal->next_month_label, 1, 2, 0, 1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

        appGUI->cal->calendar_prev = gui_calendar_new ();
        gtk_widget_show (appGUI->cal->calendar_prev);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->calendar_prev, GTK_CAN_FOCUS);
        gtk_table_attach (GTK_TABLE (appGUI->cal->aux_calendars_table), appGUI->cal->calendar_prev, 0, 1, 1, 2,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
        gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->calendar_prev), 
                                          (config.display_options & (GUI_CALENDAR_SHOW_DAY_NAMES | GUI_CALENDAR_WEEK_START_MONDAY)) | GUI_CALENDAR_NO_MONTH_CHANGE);
        gui_calendar_enable_cursor (GUI_CALENDAR (appGUI->cal->calendar_prev), FALSE);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.mark_color, 0, EVENT_MARKER_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.selection_color, 0, SELECTOR_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.header_color, 0, HEADER_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.weekend_color, 0, WEEKEND_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
        gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->calendar_prev), config.day_note_marker);
        gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_prev), config.event_marker_type, EVENT_MARKER);
        gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_prev), config.today_marker_type, TODAY_MARKER);

		g_signal_connect (G_OBJECT (appGUI->cal->calendar_prev), "day_selected_double_click",
				                  G_CALLBACK (date_selected_cb), appGUI);

        appGUI->cal->calendar_next = gui_calendar_new ();
        gtk_widget_show (appGUI->cal->calendar_next);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->calendar_next, GTK_CAN_FOCUS);
        gtk_table_attach (GTK_TABLE (appGUI->cal->aux_calendars_table), appGUI->cal->calendar_next, 1, 2, 1, 2,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 0, 0);
        gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->calendar_next), 
                                          (config.display_options & (GUI_CALENDAR_SHOW_DAY_NAMES | GUI_CALENDAR_WEEK_START_MONDAY)) | GUI_CALENDAR_NO_MONTH_CHANGE);
        gui_calendar_enable_cursor (GUI_CALENDAR (appGUI->cal->calendar_next), FALSE);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.mark_color, 0, EVENT_MARKER_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.selection_color, 0, SELECTOR_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.header_color, 0, HEADER_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.weekend_color, 0, WEEKEND_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
        gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
        gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->calendar_next), config.day_note_marker);
        gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_next), config.event_marker_type, EVENT_MARKER);
        gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_next), config.today_marker_type, TODAY_MARKER);

		g_signal_connect (G_OBJECT (appGUI->cal->calendar_next), "day_selected_double_click",
				                  G_CALLBACK (date_selected_cb), appGUI);

    /*-------------------------------------------------------------------------------------*/
    /* notes */

        appGUI->cal->notes_vbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (appGUI->cal->notes_vbox);
		if (!config.gui_layout) {
	        gtk_box_pack_start (GTK_BOX (vbox3), appGUI->cal->notes_vbox, TRUE, TRUE, 0);
		} else {
	        gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->notes_vbox, TRUE, TRUE, 0);
		}

        hbox3 = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox3);
        gtk_box_pack_start (GTK_BOX (appGUI->cal->notes_vbox), hbox3, FALSE, FALSE, 0);

        alignment = gtk_alignment_new (0.0, 0.5, 0, 1);
        gtk_widget_show (alignment);
        gtk_box_pack_start (GTK_BOX (hbox3), alignment, FALSE, FALSE, 4);

        g_snprintf (tmpbuf, BUFFER_SIZE, "%s:", _("Notes"));
        label = gtk_label_new (tmpbuf);
        gtk_widget_show (label);
        gtk_container_add (GTK_CONTAINER (alignment), label);

        if (config.default_stock_icons) {
            appGUI->cal->n_close_button = utl_gui_stock_button (GTK_STOCK_CLOSE, FALSE);
        } else {
            appGUI->cal->n_close_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLOSE, FALSE);
        }
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->n_close_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->n_close_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->n_close_button, _("Close note panel"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->n_close_button, FALSE, FALSE, 1);
        g_signal_connect (G_OBJECT (appGUI->cal->n_close_button), "clicked",
                            G_CALLBACK (calendar_close_text_cb), appGUI);

        vseparator = gtk_vseparator_new ();
        gtk_widget_show (vseparator);
        gtk_box_pack_end (GTK_BOX (hbox3), vseparator, FALSE, TRUE, 0);

		appGUI->cal->ta_highlight_button = utl_gui_stock_button (OSMO_STOCK_EDITOR_HIGHLIGHT_S, FALSE);
        gtk_widget_show (appGUI->cal->ta_highlight_button);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->ta_highlight_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->ta_highlight_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->ta_highlight_button, _("Highlight"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->ta_highlight_button, FALSE, FALSE, 1);
		g_object_set_data (G_OBJECT (appGUI->cal->ta_highlight_button), "tag", "mark_color");
		g_signal_connect (G_OBJECT (appGUI->cal->ta_highlight_button), "clicked", 
						  G_CALLBACK (calendar_set_text_attribute_cb), appGUI);

		appGUI->cal->ta_strikethrough_button = utl_gui_stock_button (OSMO_STOCK_EDITOR_STRIKETHROUGH_S, FALSE);
        gtk_widget_show (appGUI->cal->ta_strikethrough_button);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->ta_strikethrough_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->ta_strikethrough_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->ta_strikethrough_button, _("Strikethrough"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->ta_strikethrough_button, FALSE, FALSE, 1);
		g_object_set_data (G_OBJECT (appGUI->cal->ta_strikethrough_button), "tag", "strike");
		g_signal_connect (G_OBJECT (appGUI->cal->ta_strikethrough_button), "clicked", 
						  G_CALLBACK (calendar_set_text_attribute_cb), appGUI);

		appGUI->cal->ta_underline_button = utl_gui_stock_button (OSMO_STOCK_EDITOR_UNDERLINE_S, FALSE);
        gtk_widget_show (appGUI->cal->ta_underline_button);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->ta_underline_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->ta_underline_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->ta_underline_button, _("Underline"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->ta_underline_button, FALSE, FALSE, 1);
		g_object_set_data (G_OBJECT (appGUI->cal->ta_underline_button), "tag", "underline");
		g_signal_connect (G_OBJECT (appGUI->cal->ta_underline_button), "clicked", 
						  G_CALLBACK (calendar_set_text_attribute_cb), appGUI);

        appGUI->cal->ta_italic_button = utl_gui_stock_button (OSMO_STOCK_EDITOR_ITALIC_S, FALSE);
        gtk_widget_show (appGUI->cal->ta_italic_button);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->ta_italic_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->ta_italic_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->ta_italic_button, _("Italic"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->ta_italic_button, FALSE, FALSE, 1);
		g_object_set_data (G_OBJECT (appGUI->cal->ta_italic_button), "tag", "italic");
		g_signal_connect (G_OBJECT (appGUI->cal->ta_italic_button), "clicked", 
						  G_CALLBACK (calendar_set_text_attribute_cb), appGUI);

        appGUI->cal->ta_bold_button = utl_gui_stock_button (OSMO_STOCK_EDITOR_BOLD_S, FALSE);
        gtk_widget_show (appGUI->cal->ta_bold_button);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->ta_bold_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->ta_bold_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->ta_bold_button, _("Bold"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->ta_bold_button, FALSE, FALSE, 1);
		g_object_set_data (G_OBJECT (appGUI->cal->ta_bold_button), "tag", "bold");
		g_signal_connect (G_OBJECT (appGUI->cal->ta_bold_button), "clicked", 
						  G_CALLBACK (calendar_set_text_attribute_cb), appGUI);

		vseparator = gtk_vseparator_new ();
        gtk_widget_show (vseparator);
        gtk_box_pack_end (GTK_BOX (hbox3), vseparator, FALSE, TRUE, 0);

        appGUI->cal->n_timeline_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_INSERT_TIMELINE, FALSE);
        gtk_widget_show (appGUI->cal->n_timeline_button);
        GTK_WIDGET_UNSET_FLAGS(appGUI->cal->n_timeline_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->n_timeline_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->n_timeline_button, _("Insert timeline"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->n_timeline_button, FALSE, FALSE, 1);
        g_signal_connect (G_OBJECT (appGUI->cal->n_timeline_button), "clicked",
                            G_CALLBACK (calendar_insert_timeline_cb), appGUI);

        appGUI->cal->n_clear_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLEAR, FALSE);
        gtk_widget_show (appGUI->cal->n_clear_button);
        GTK_WIDGET_UNSET_FLAGS (appGUI->cal->n_clear_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->n_clear_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->n_clear_button, _("Clear text"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->n_clear_button, FALSE, FALSE, 1);
        g_signal_connect (G_OBJECT (appGUI->cal->n_clear_button), "clicked",
                            G_CALLBACK (calendar_clear_text_cb), appGUI);

        appGUI->cal->n_select_color_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_COLOR, FALSE);
        gtk_widget_show (appGUI->cal->n_select_color_button);
        GTK_WIDGET_UNSET_FLAGS (appGUI->cal->n_select_color_button, GTK_CAN_FOCUS);
        gtk_button_set_relief (GTK_BUTTON(appGUI->cal->n_select_color_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (appGUI->cal->n_select_color_button, _("Select day color"));
		}
        gtk_box_pack_end (GTK_BOX (hbox3), appGUI->cal->n_select_color_button, FALSE, FALSE, 1);
        g_signal_connect (G_OBJECT (appGUI->cal->n_select_color_button), "clicked",
                            G_CALLBACK (calendar_select_color_cb), appGUI);

        note_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (note_scrolledwindow);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (note_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start (GTK_BOX (appGUI->cal->notes_vbox), note_scrolledwindow, TRUE, TRUE, 0);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (note_scrolledwindow), GTK_SHADOW_IN);

        appGUI->cal->calendar_note_textview = gtk_text_view_new ();
        gtk_widget_show (appGUI->cal->calendar_note_textview);
        g_signal_connect (G_OBJECT (appGUI->cal->calendar_note_textview), "key_press_event",
                          G_CALLBACK (calendar_textview_key_press_cb), appGUI);
        gtk_container_add (GTK_CONTAINER (note_scrolledwindow), appGUI->cal->calendar_note_textview);
        gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), TRUE);
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), GTK_WRAP_WORD_CHAR);
        gtk_widget_modify_font (GTK_WIDGET(appGUI->cal->calendar_note_textview), appGUI->cal->fd_notes_font);
        gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), 4);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), 4);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), 4);
	
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));
		gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
		gtk_text_buffer_create_tag (buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
		gtk_text_buffer_create_tag (buffer, "strike", "strikethrough", TRUE, NULL);
		gtk_text_buffer_create_tag (buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
		gtk_text_buffer_create_tag (buffer, "mark_color", "background", "#FFFF00", NULL);

    /*-------------------------------------------------------------------------------------*/
    /* day info */

        appGUI->cal->day_info_vbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (appGUI->cal->day_info_vbox);
		if (!config.gui_layout) {
	        gtk_box_pack_start (GTK_BOX (vbox3), appGUI->cal->day_info_vbox, TRUE, TRUE, 0);
		} else {
	        gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->day_info_vbox, TRUE, TRUE, 0);
		}

        frame = gtk_frame_new (NULL);
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (appGUI->cal->day_info_vbox), frame, TRUE, TRUE, 0);
        gtk_frame_set_label_align (GTK_FRAME (frame), 0.98, 0.5);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

        alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
        gtk_widget_show (alignment);
        gtk_container_add (GTK_CONTAINER (frame), alignment);
        gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 8, 0);

        g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>", _("Info"));
        label = gtk_label_new (tmpbuf);
        gtk_widget_show (label);
        gtk_frame_set_label_widget (GTK_FRAME (frame), label);
        gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

        appGUI->cal->day_info_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (appGUI->cal->day_info_scrolledwindow);
        gtk_container_add (GTK_CONTAINER (alignment), appGUI->cal->day_info_scrolledwindow);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (appGUI->cal->day_info_scrolledwindow), GTK_SHADOW_NONE);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->cal->day_info_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        appGUI->cal->day_desc_textview = gtk_text_view_new ();
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->cal->day_desc_textview), GTK_WRAP_WORD);
        gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW(appGUI->cal->day_desc_textview), 4);
        gtk_text_view_set_left_margin (GTK_TEXT_VIEW(appGUI->cal->day_desc_textview), 4);
        gtk_text_view_set_right_margin (GTK_TEXT_VIEW(appGUI->cal->day_desc_textview), 4);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(appGUI->cal->day_desc_textview), FALSE);
        gtk_text_view_set_editable (GTK_TEXT_VIEW(appGUI->cal->day_desc_textview), FALSE);
        gtk_widget_show (appGUI->cal->day_desc_textview);
        gtk_container_add (GTK_CONTAINER (appGUI->cal->day_info_scrolledwindow), appGUI->cal->day_desc_textview);
        gtk_widget_realize (appGUI->cal->day_desc_textview);
        gtk_widget_modify_base (appGUI->cal->day_desc_textview, GTK_STATE_NORMAL, &appGUI->main_window->style->bg[GTK_STATE_NORMAL]);

        appGUI->cal->day_desc_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appGUI->cal->day_desc_textview));
		gtk_text_buffer_create_tag (appGUI->cal->day_desc_text_buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
		gtk_text_buffer_create_tag (appGUI->cal->day_desc_text_buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
		gtk_text_buffer_create_tag (appGUI->cal->day_desc_text_buffer, "strike", "strikethrough", TRUE, NULL);
		gtk_text_buffer_create_tag (appGUI->cal->day_desc_text_buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
		gtk_text_buffer_create_tag (appGUI->cal->day_desc_text_buffer, "mark_color", "background", "#FFFF00", NULL);

    /*-------------------------------------------------------------------------------------*/

    }

    gtk_widget_modify_font (GTK_WIDGET(appGUI->cal->date_label), appGUI->cal->fd_day_name_font);

	gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->calendar), appGUI->cal->date);

    gui_calendar_set_frame_cursor_thickness (GUI_CALENDAR (appGUI->cal->calendar), config.frame_cursor_thickness);

    if (appGUI->calendar_only == FALSE) {
        if (!config.day_notes_visible) {
            gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button), FALSE);
            gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), FALSE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), FALSE);
            gtk_widget_show(appGUI->cal->day_info_vbox);
            gtk_widget_hide(appGUI->cal->notes_vbox);
        } else {
            gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(appGUI->cal->notes_button), TRUE);
            gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), TRUE);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview), TRUE);
            gtk_widget_hide(appGUI->cal->day_info_vbox);
            gtk_widget_show(appGUI->cal->notes_vbox);
        }

        gtk_widget_realize(GTK_WIDGET(appGUI->cal->calendar_note_textview));
        gtk_widget_grab_focus(GTK_WIDGET(appGUI->cal->calendar_note_textview));
    }

    if (appGUI->calendar_only == FALSE) {
		if (!config.gui_layout) {
			g_signal_connect (G_OBJECT(appGUI->cal->aux_cal_expander), "notify::expanded", 
							  G_CALLBACK(aux_cal_expander_cb), appGUI);
			gtk_expander_set_expanded (GTK_EXPANDER(appGUI->cal->aux_cal_expander), config.auxilary_calendars_state);
		} else {
			update_aux_calendars (appGUI);
		}
    }

}

/*------------------------------------------------------------------------------*/

void
select_bg_color_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	gtk_widget_destroy (appGUI->cal->select_bg_color_window);
}

/*------------------------------------------------------------------------------*/

void
button_select_bg_color_close_cb (GtkWidget *widget, gpointer user_data)
{
	select_bg_color_close_cb (widget, NULL, user_data);
}

/*------------------------------------------------------------------------------*/

void
colors_category_selected (GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *color_val = NULL;
	gchar *text = NULL;
	gint n;

	if (gtk_tree_selection_get_selected (appGUI->cal->colors_category_select, &model, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cal->colors_category_store), &iter, 1, &color_val, 3, &n, -1);

		text = calendar_get_note_text (appGUI);
		cal_add_note (g_date_get_julian (appGUI->cal->date), color_val, text, appGUI);
		cal_mark_days_with_notes (appGUI->cal->date, appGUI);

		select_bg_color_close_cb (NULL, NULL, appGUI);
		update_aux_calendars (appGUI);

		g_free (color_val);
		g_free (text);
	}

}

/*------------------------------------------------------------------------------*/

void
select_bg_color_apply_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	config.enable_day_mark = TRUE;
	cal_refresh_marks (appGUI);
	update_aux_calendars (appGUI);
	colors_category_selected (appGUI);
}

/*------------------------------------------------------------------------------*/

gint
select_bg_color_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	switch (event->keyval) {

		case GDK_Return:
			config.enable_day_mark = TRUE;
			cal_refresh_marks (appGUI);
			update_aux_calendars (appGUI);
			colors_category_selected (appGUI);
			return TRUE;

		case GDK_Escape:
			select_bg_color_close_cb (NULL, NULL, appGUI);
			return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

gint
colors_list_dbclick_cb (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	if ((event->type == GDK_2BUTTON_PRESS) && (event->button == 1)) {
		config.enable_day_mark = TRUE;
		cal_refresh_marks (appGUI);
		update_aux_calendars (appGUI);
		colors_category_selected (appGUI);
		return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

void
calendar_create_color_selector_window (gboolean window_pos, GUI *appGUI) {

GtkWidget *vbox1;
GtkWidget *scrolledwindow;
GtkWidget *colors_category_treeview;
GtkWidget *hseparator;
GtkWidget *hbuttonbox;
GtkWidget *ok_button;
GtkWidget *cancel_button;
GtkTreeIter iter;
GtkCellRenderer     *renderer;
GtkTreeViewColumn   *column;
GtkTreePath     *path;
GdkPixbuf *image;
gchar *color_val, *color_name, *color_sel;
gchar tmpbuf[BUFFER_SIZE];
gint i;
GdkColor color;

	if (cal_check_note (g_date_get_julian (appGUI->cal->date), appGUI) == FALSE && !config.day_notes_visible) return;
	
	config.enable_day_mark = TRUE;
	cal_refresh_marks (appGUI);
	update_aux_calendars (appGUI);

    appGUI->cal->select_bg_color_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->select_bg_color_window), 4);
    gtk_window_set_title (GTK_WINDOW (appGUI->cal->select_bg_color_window), _("Select color"));
    gtk_window_set_default_size (GTK_WINDOW(appGUI->cal->select_bg_color_window), 200, 280);
    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cal->select_bg_color_window), GTK_WINDOW(appGUI->main_window));
    if (window_pos == TRUE) {
        gtk_window_set_position(GTK_WINDOW(appGUI->cal->select_bg_color_window), GTK_WIN_POS_CENTER_ON_PARENT);
    } else {
        gtk_window_set_position(GTK_WINDOW(appGUI->cal->select_bg_color_window), GTK_WIN_POS_MOUSE);
    }
    gtk_window_set_modal(GTK_WINDOW(appGUI->cal->select_bg_color_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->cal->select_bg_color_window), "delete_event",
                      G_CALLBACK(select_bg_color_close_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->select_bg_color_window), "key_press_event",
                      G_CALLBACK (select_bg_color_key_press_cb), appGUI);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cal->select_bg_color_window), vbox1);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

    appGUI->cal->colors_category_store = gtk_list_store_new(4, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);

    colors_category_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(appGUI->cal->colors_category_store));
    g_signal_connect(G_OBJECT(colors_category_treeview), "button_press_event",
                     G_CALLBACK(colors_list_dbclick_cb), appGUI);
    appGUI->cal->colors_category_select = gtk_tree_view_get_selection(GTK_TREE_VIEW(colors_category_treeview));
    gtk_widget_show (colors_category_treeview);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), colors_category_treeview);
    gtk_container_set_border_width (GTK_CONTAINER (colors_category_treeview), 4);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (colors_category_treeview), FALSE);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (colors_category_treeview), FALSE);

    renderer = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "pixbuf", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(colors_category_treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(colors_category_treeview), column);
    gtk_tree_view_column_set_visible (column, FALSE);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(colors_category_treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(colors_category_treeview), column);
    gtk_tree_view_column_set_visible (column, FALSE);

    color = (appGUI->cal->calendar)->style->base[GTK_WIDGET_STATE (appGUI->cal->calendar)];
    g_snprintf (tmpbuf, BUFFER_SIZE, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
    image = utl_gui_create_color_swatch (tmpbuf);
    gtk_list_store_append(appGUI->cal->colors_category_store, &iter);
    gtk_list_store_set(appGUI->cal->colors_category_store, &iter, 0, image, 1, tmpbuf, 2, _("None"), 3, 0, -1);
    g_object_unref (image);

    i = 0;

    color_sel = cal_get_note_color (g_date_get_julian (appGUI->cal->date), appGUI);

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->calendar_category_store), &iter, NULL, i++)) {
        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->calendar_category_store), &iter, 1, &color_val, 2, &color_name, -1);
        image = utl_gui_create_color_swatch (color_val);
        gtk_list_store_append(appGUI->cal->colors_category_store, &iter);
        gtk_list_store_set(appGUI->cal->colors_category_store, &iter, 0, image, 1, color_val, 2, color_name, 3, i, -1);

        if (color_sel != NULL) {
            if (!strcmp(color_val, color_sel)) {
                path = gtk_tree_model_get_path (GTK_TREE_MODEL(appGUI->cal->colors_category_store), &iter);
                if (path != NULL) {
                    gtk_tree_view_set_cursor (GTK_TREE_VIEW (colors_category_treeview), path, NULL, FALSE);
                    gtk_tree_path_free(path);
                }
            }
        }

        g_object_unref (image);
        g_free(color_val);
        g_free(color_name);
    }

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, FALSE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

    if (config.default_stock_icons) {
        cancel_button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    } else {
        cancel_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_CANCEL);
    }
    GTK_WIDGET_UNSET_FLAGS(cancel_button, GTK_CAN_FOCUS);
    gtk_widget_show (cancel_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
    g_signal_connect (G_OBJECT (cancel_button), "clicked",
                        G_CALLBACK (button_select_bg_color_close_cb), appGUI);

    if (config.default_stock_icons) {
        ok_button = gtk_button_new_from_stock (GTK_STOCK_OK);
    } else {
        ok_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_OK);
    }
    GTK_WIDGET_UNSET_FLAGS(ok_button, GTK_CAN_FOCUS);
    gtk_widget_show (ok_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), ok_button);
    g_signal_connect (G_OBJECT (ok_button), "clicked",
                        G_CALLBACK (select_bg_color_apply_cb), appGUI);

    gtk_widget_show (appGUI->cal->select_bg_color_window);
}

/*------------------------------------------------------------------------------*/

void
menu_entry_selected_cb (gpointer user_data)
{
    MESSAGE *msg = (MESSAGE *) user_data;
    calendar_update_date (g_date_get_day (msg->appGUI->cal->date), (gint) msg->data,
	                      g_date_get_year (msg->appGUI->cal->date), msg->appGUI);
}

/*------------------------------------------------------------------------------*/

void
calendar_create_popup_menu (GtkWidget *menu, GUI *appGUI)
{
static MESSAGE msg_month[MAX_MONTHS];
GtkWidget *menu_entry;
gchar buffer[BUFFER_SIZE];
GDate *tmpdate;
gint i;

	tmpdate = g_date_new_dmy (1, 1, utl_date_get_current_year ());
	g_return_if_fail (tmpdate != NULL);

	for (i = G_DATE_JANUARY; i <= G_DATE_DECEMBER; i++) {
		g_date_set_month (tmpdate, i);
		g_date_strftime (buffer, BUFFER_SIZE, "%B", tmpdate);

		menu_entry = gtk_menu_item_new_with_label (buffer);
		gtk_menu_shell_append (GTK_MENU_SHELL (appGUI->cal->month_selector_menu), menu_entry);
		msg_month[i-1].data = (gpointer) i;
		msg_month[i-1].appGUI = appGUI;
		g_signal_connect_swapped (G_OBJECT (menu_entry), "activate",
		                          G_CALLBACK (menu_entry_selected_cb), &msg_month[i-1]);
		gtk_widget_show (menu_entry);
	}

	g_date_free (tmpdate);
}

/*------------------------------------------------------------------------------*/

void
calendar_mark_events (GtkWidget *calendar, guint32 julian_day, guint i, GUI *appGUI) {

gboolean flag = FALSE;

	if (appGUI->calendar_only == TRUE || appGUI->all_pages_added == FALSE) 
		return;

#ifdef TASKS_ENABLED
	if (tsk_check_tasks (julian_day, julian_day, STATE_CALENDAR, appGUI) == TRUE) {
		gui_calendar_mark_day (GUI_CALENDAR (calendar), i, EVENT_MARK);
		flag = TRUE;
	}
#endif  /* TASKS_ENABLED */

#ifdef HAVE_LIBICAL
	if (flag == FALSE) {
		if (ics_check_event (julian_day, appGUI) == TRUE) {
			gui_calendar_mark_day (GUI_CALENDAR (calendar), i, EVENT_MARK);
		}
	}
#endif  /* HAVE_LIBICAL */

#ifdef CONTACTS_ENABLED
	if (check_contacts (julian_day, appGUI) == TRUE) {
		gui_calendar_mark_day (GUI_CALENDAR (calendar), i, BIRTHDAY_MARK);
	}
#endif  /* CONTACTS_ENABLED */

}

/*------------------------------------------------------------------------------*/

