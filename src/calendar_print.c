
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

#include "calendar_notes.h"
#include "calendar_widget.h"
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "tasks_utils.h"
#include "utils.h"
#include "utils_gui.h"

#ifdef PRINTING_SUPPORT

/*------------------------------------------------------------------------------*/

void
window_cal_print_opt_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	gtk_widget_destroy (appGUI->cal->window_print_options);
}

/*------------------------------------------------------------------------------*/

void
window_button_cal_print_opt_close_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
	window_cal_print_opt_close_cb (widget, NULL, appGUI);
}

/*------------------------------------------------------------------------------*/

gint
cal_print_opt_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	if (event->keyval == GDK_Escape) {
		window_cal_print_opt_close_cb (widget, NULL, appGUI);
		return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

void
calendar_begin_print (GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	GtkPageSetup *setup;

	if (config.cal_print_page_orientation == LANDSCAPE) {
		setup = gtk_page_setup_new ();
		gtk_page_setup_set_orientation (setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
		gtk_print_operation_set_default_page_setup (operation, setup);
		g_object_unref (setup);
	}

	appGUI->print_lines_per_page = 1;
	appGUI->print_nlines = 1;
	appGUI->print_npages = 1;
	gtk_print_operation_set_n_pages (operation, appGUI->print_npages);
}

/*------------------------------------------------------------------------------*/

void
cal_print_get_events (gchar *buffer, guint32 julian, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GSList *lnode;
	struct note *a;
	gint i;

	gchar *wbuf1, *wbuf2, *stripped;
	gchar buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];
	GDate *date, *sdate;
	gint age, syear;
	guint32 tsk_julian;
	gint time;
	gint max_event_length;
	GRegex *reg;

	buffer[0] = '\0';
	max_event_length = (config.cal_print_event_length + 2 > BUFFER_SIZE) ? BUFFER_SIZE : config.cal_print_event_length + 2;


#ifdef TASKS_ENABLED

	/* tasks */
	if (config.cal_print_tasks) {

		model = GTK_TREE_MODEL (appGUI->tsk->tasks_list_store);
		g_return_if_fail (model != NULL);

		path = gtk_tree_path_new_first ();

		while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {
			gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_DATE_JULIAN, &tsk_julian, TA_COLUMN_CATEGORY, &wbuf1, -1);

			if (tsk_julian == julian && tsk_get_category_state (wbuf1, STATE_CALENDAR, appGUI) == TRUE) {
				gtk_tree_model_get (model, &iter, TA_COLUMN_DUE_TIME, &time, TA_COLUMN_SUMMARY, &wbuf2, -1);

				if (time >= 0) {
					g_snprintf (buf1, max_event_length, "\n[%02d:%02d] %s", time / 3600, time / 60 % 60, wbuf2);
				} else {
					g_snprintf (buf1, max_event_length, "\n%s", wbuf2);
				}

				g_strlcat (buffer, buf1, BUFFER_SIZE);
				g_free (wbuf2);
			}

			g_free (wbuf1);
			gtk_tree_path_next (path);
		}

		gtk_tree_path_free (path);

	}

#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED

	/* birthdays */
	if (config.cal_print_birthdays) {

		model = GTK_TREE_MODEL (appGUI->cnt->contacts_list_store);
		g_return_if_fail (model != NULL);

		date = g_date_new ();
		g_return_if_fail (date != NULL);

		sdate = g_date_new_julian (julian);
		g_return_if_fail (sdate != NULL);

		syear = g_date_get_year (sdate);
		path = gtk_tree_path_new_first ();

		while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {
			gtk_tree_model_get (model, &iter, COLUMN_BIRTH_DAY_DATE, &tsk_julian, -1);

			if (g_date_valid_julian (tsk_julian)) {
				g_date_set_julian (date, tsk_julian);
				age = syear - g_date_get_year (date);

				if (age >= 0) {

					if (g_date_valid_dmy (g_date_get_day (date), g_date_get_month (date), syear) == FALSE) {
						g_date_subtract_days (date, 1);
					}
					g_date_set_year (date, syear);

					if (g_date_compare (date, sdate) == 0) {

						gtk_tree_model_get (model, &iter, COLUMN_FIRST_NAME, &wbuf1, COLUMN_LAST_NAME, &wbuf2, -1);
						utl_name_strcat (wbuf1, wbuf2, buf2);

						g_snprintf (buf1, max_event_length, "\n%s (%d)", buf2, age);
						g_strlcat (buffer, buf1, BUFFER_SIZE);
					}
				}
			}
			gtk_tree_path_next (path);
		}

		gtk_tree_path_free (path);
		g_date_free (sdate);
		g_date_free (date);

	}

	/* name days */
	if (config.cal_print_namedays) {

		model = GTK_TREE_MODEL (appGUI->cnt->contacts_list_store);
		g_return_if_fail (model != NULL);

		date = NULL;
		date = g_date_new ();
		g_return_if_fail (date != NULL);

		sdate = NULL;
		sdate = g_date_new_julian (julian);
		g_return_if_fail (sdate != NULL);

		syear = g_date_get_year (sdate);
		path = gtk_tree_path_new_first ();

		while (gtk_tree_model_get_iter (model, &iter, path) == TRUE) {
			gtk_tree_model_get (model, &iter, COLUMN_NAME_DAY_DATE, &tsk_julian, -1);

			if (g_date_valid_julian (tsk_julian)) {
				g_date_set_julian (date, tsk_julian);

				if (g_date_valid_dmy (g_date_get_day (date), g_date_get_month (date), syear) == TRUE) {
					g_date_set_year (date, syear);

					if (g_date_compare (date, sdate) == 0) {

						gtk_tree_model_get (model, &iter, COLUMN_FIRST_NAME, &wbuf1, COLUMN_LAST_NAME, &wbuf2, -1);
						utl_name_strcat (wbuf1, wbuf2, buf1);

						g_snprintf (buf2, max_event_length, "\n%s (%s)", buf1, _("Name day"));
						g_strlcat (buffer, buf2, BUFFER_SIZE);
					}
				}
			}
			gtk_tree_path_next (path);
		}

		gtk_tree_path_free (path);
		g_date_free (sdate);
		g_date_free (date);

	}

#endif  /* CONTACTS_ENABLED */

	/* day note */
	if (config.cal_print_day_notes) {

		wbuf1 = NULL;

		if (appGUI->cal->notes_list != NULL) {

			reg = g_regex_new ("\n", 0, 0, NULL);

			for (i = 0, lnode = appGUI->cal->notes_list; lnode != NULL; lnode = lnode->next, i++) {
				a = g_slist_nth_data (appGUI->cal->notes_list, i);
				stripped = utl_gui_text_strip_tags (a->note);

				if (a->date == julian) {
					wbuf1 = g_regex_replace_literal (reg, stripped, -1, 0, " ", 0, NULL);
					g_free (stripped);
					break;
				}

				g_free (stripped);
			}

			g_regex_unref (reg);
		}

		if (wbuf1 != NULL) {
			g_strstrip (wbuf1);
			g_snprintf (buf1, max_event_length, "\n%s", wbuf1);
			g_strlcat (buffer, buf1, BUFFER_SIZE);
			g_free (wbuf1);
		}

	}

#ifdef HAVE_LIBICAL

	/* ical */
	if (config.cal_print_ical) {


	}

#endif  /* HAVE_LIBICAL */

	g_strstrip (buffer);

}

/*------------------------------------------------------------------------------*/

void
calendar_draw_page (GtkPrintOperation *operation, GtkPrintContext *context, gint npage, gpointer user_data)
{
	PangoLayout *layout;
	PangoFontDescription *month_name_font, *day_name_font, *day_num_font, *event_font;
	cairo_t *cr;
	GDate *date;
	gdouble page_width, page_height, day_width, day_height;
	gint text_width, text_height, header_height, event_height, mnf_height, dnf_height, duf_height;
	gint day, month, i, j;
	guint32 julian;
	gboolean monday, actual;

	gchar buffer[BUFFER_SIZE];

	gint padding = config.cal_print_padding;

	GUI *appGUI = (GUI *) user_data;

	date = g_date_new_julian (g_date_get_julian (appGUI->cal->date));
	g_return_if_fail (date != NULL);

	cr = gtk_print_context_get_cairo_context (context);
	layout = gtk_print_context_create_pango_layout (context);

	month_name_font = pango_font_description_from_string (config.cal_print_month_name_font);
	day_name_font = pango_font_description_from_string (config.cal_print_day_name_font);
	day_num_font = pango_font_description_from_string (config.cal_print_day_num_font);
	event_font = pango_font_description_from_string (config.cal_print_event_font);

	pango_layout_set_text (layout, "Aj", -1);

	pango_layout_set_font_description (layout, month_name_font);
	pango_layout_get_pixel_size (layout, NULL, &mnf_height);
	mnf_height *= 1.2;

	pango_layout_set_font_description (layout, day_name_font);
	pango_layout_get_pixel_size (layout, NULL, &dnf_height);
	dnf_height *= 1.2;

	pango_layout_set_font_description (layout, day_num_font);
	pango_layout_get_pixel_size (layout, NULL, &duf_height);

	page_width = gtk_print_context_get_width (context);
	day_width = page_width / 7;

	page_height = gtk_print_context_get_height (context);
	header_height = mnf_height + dnf_height;
	day_height = (page_height - header_height) / 6;
	event_height = day_height - duf_height - padding * 3;

	cairo_set_line_width (cr, 1);
	monday = (config.display_options & GUI_CALENDAR_WEEK_START_MONDAY) ? TRUE : FALSE;


	/* Month and year */
	pango_layout_set_font_description (layout, month_name_font);
	g_date_strftime (buffer, BUFFER_SIZE, "%B %Y", date);
	pango_layout_set_text (layout, buffer, -1);
	pango_layout_get_pixel_size (layout, &text_width, NULL);

	cairo_move_to (cr, (page_width - text_width) / 2, 0);
	pango_cairo_show_layout (cr, layout);


	/* Day names */
	pango_layout_set_font_description (layout, day_name_font);

	for (i = 0; i < 7; i++) {
		g_snprintf (buffer, BUFFER_SIZE, "%s", utl_get_day_name (i + 7 + monday, FALSE));
		pango_layout_set_text (layout, buffer, -1);
		pango_layout_get_pixel_size (layout, &text_width, NULL);
		cairo_move_to (cr, day_width * i + (day_width - text_width) / 2, mnf_height);
		pango_cairo_show_layout (cr, layout);
	}


	/* Day */
	g_date_set_day (date, 1);
	day = g_date_get_weekday (date);
	month = g_date_get_month (date);

	day = monday ? day - 1 : day % 7;

	if (day > 0)
		g_date_subtract_days (date, day);

	day = g_date_get_day (date);
	julian = g_date_get_julian (date);

	pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
	pango_layout_set_width (layout, (day_width - padding * 2) * PANGO_SCALE);
#ifdef HAVE_PANGO_1_20
	pango_layout_set_height (layout, event_height * PANGO_SCALE);
#endif /* HAVE_PANGO_1_20 */
	pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
	pango_layout_set_indent (layout, -4 * PANGO_SCALE);

	for (i = 0; i < 6; i++) {

		for (j = 0; j < 7; j++) {

			actual = (month == g_date_get_month (date)) ? TRUE : FALSE;
			day = g_date_get_day (date);

			cairo_rectangle (cr, day_width * j, header_height + day_height * i, day_width, day_height);

			if (actual) {
				cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
			} else {
				cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
			}

			cairo_fill_preserve (cr);
			cairo_set_source_rgb (cr, 0, 0, 0);
			cairo_stroke (cr);

			pango_layout_set_font_description (layout, day_num_font);

			if (actual) {

				cairo_move_to (cr, day_width * j + padding, header_height + day_height * i + padding);

				if ((j == 0 && !monday) || (j == 5 && monday) || j == 6) {
					g_snprintf (buffer, BUFFER_SIZE, "<span color=\"red\">%d</span>", day);
				} else {
					g_snprintf (buffer, BUFFER_SIZE, "%d", day);
				}

				pango_layout_set_markup (layout, buffer, -1);
				pango_cairo_show_layout (cr, layout);

				cal_print_get_events (buffer, julian, appGUI);

				pango_layout_set_markup (layout, "", -1);
				pango_layout_set_text (layout, buffer, -1);
				pango_layout_set_font_description (layout, event_font);
				pango_layout_get_pixel_size (layout, NULL, &text_height);
				cairo_move_to (cr, day_width * j + padding, header_height + day_height * (i + 1) - text_height - padding);
				pango_cairo_show_layout (cr, layout);

			} else {

				cairo_move_to (cr, day_width * j + padding, header_height + day_height * i + padding);
				g_snprintf (buffer, BUFFER_SIZE, "<span color=\"white\">%d</span>", day);
				pango_layout_set_markup (layout, buffer, -1);
				pango_cairo_show_layout (cr, layout);

			}

			g_date_add_days (date, 1);
			julian++;

		}
	}

	g_date_free (date);
	pango_font_description_free (month_name_font);
	pango_font_description_free (day_name_font);
	pango_font_description_free (day_num_font);
	pango_font_description_free (event_font);
	g_object_unref (layout);
}

/*------------------------------------------------------------------------------*/

void
calendar_print_start_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

	GtkPrintOperation *print;
	GtkPrintOperationResult result;
	GError *error = NULL;
	gchar buffer[BUFFER_SIZE];

	g_strlcpy (config.cal_print_month_name_font, gtk_entry_get_text
	    (GTK_ENTRY (appGUI->cal->print_month_name_font_entry)), MAXFONTNAME);
	g_strlcpy (config.cal_print_day_name_font, gtk_entry_get_text
	    (GTK_ENTRY (appGUI->cal->print_day_name_font_entry)), MAXFONTNAME);
	g_strlcpy (config.cal_print_day_num_font, gtk_entry_get_text
	    (GTK_ENTRY (appGUI->cal->print_day_num_font_entry)), MAXFONTNAME);
	g_strlcpy (config.cal_print_event_font, gtk_entry_get_text
	    (GTK_ENTRY (appGUI->cal->print_event_font_entry)), MAXFONTNAME);
	config.cal_print_event_length = gtk_spin_button_get_value
	    (GTK_SPIN_BUTTON (appGUI->cal->print_event_length_spinbutton));
	config.cal_print_padding = gtk_spin_button_get_value
	    (GTK_SPIN_BUTTON (appGUI->cal->print_padding_spinbutton));
	config.cal_print_page_orientation = gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (appGUI->cal->print_portrait_radiobutton)) ? PORTRAIT : LANDSCAPE;
	config.cal_print_tasks = gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (appGUI->cal->print_tasks_checkbutton));
	config.cal_print_birthdays = gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (appGUI->cal->print_birthdays_checkbutton));
	config.cal_print_namedays = gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (appGUI->cal->print_namedays_checkbutton));
	config.cal_print_day_notes = gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (appGUI->cal->print_day_notes_checkbutton));
	/*
	config.cal_print_ical = gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (appGUI->cal->print_ical_checkbutton));
	*/

	gtk_widget_destroy (appGUI->cal->window_print_options);

    print = gtk_print_operation_new ();

    appGUI->print_lines_per_page = 0;
    appGUI->print_nlines = 0;
    appGUI->print_npages = 0;

    g_signal_connect (print, "begin_print", G_CALLBACK (calendar_begin_print), appGUI);
    g_signal_connect (print, "draw_page", G_CALLBACK (calendar_draw_page), appGUI);

    result = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                      GTK_WINDOW (appGUI->main_window), &error);

    if (result == GTK_PRINT_OPERATION_RESULT_ERROR) {
	    g_snprintf (buffer, BUFFER_SIZE, "%s: %s", _("Error printing"), error->message);
        utl_gui_create_dialog (GTK_MESSAGE_ERROR, buffer, GTK_WINDOW (appGUI->main_window));
        g_error_free (error);
    }

    g_object_unref (print);
}

/*------------------------------------------------------------------------------*/

void
calendar_create_print_window (GUI *appGUI)
{
	GtkWidget *vbox1;
	GtkWidget *frame1;
	GtkWidget *alignment;
	GtkWidget *table_fonts;
	GtkWidget *month_name_font_button;
	GtkWidget *day_name_font_button;
	GtkWidget *day_number_font_button;
	GtkWidget *event_font_button;
	GtkWidget *label;
	GtkWidget *hbox1;
	GtkWidget *frame2;
	GtkWidget *vbox2;
	GtkWidget *vseparator1;
	GtkWidget *frame3;
	GtkWidget *table5;
	GtkObject *spinbutton_adj;
	GtkWidget *hseparator1;
	GtkWidget *hbuttonbox;
	GtkWidget *cancel_button;
	GtkWidget *ok_button;
	GtkWidget *landscape_radiobutton;
	GSList *radiobutton_group = NULL;
	gchar buffer[BUFFER_SIZE];

	static FONT_SEL sel1, sel2, sel3, sel4;

	sel1.appGUI = sel2.appGUI = sel3.appGUI = sel4.appGUI = appGUI;
	sel1.font = sel2.font = sel3.font = sel4.font = NULL;
	sel1.widget = sel2.widget = sel3.widget = sel4.widget = NULL;
	sel1.save = sel2.save = sel3.save = sel4.save = FALSE;

	appGUI->cal->window_print_options = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (appGUI->cal->window_print_options), _("Printing properties"));
	gtk_window_set_position (GTK_WINDOW (appGUI->cal->window_print_options), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_modal (GTK_WINDOW (appGUI->cal->window_print_options), TRUE);
	g_signal_connect (G_OBJECT (appGUI->cal->window_print_options), "delete_event",
	                  G_CALLBACK (window_cal_print_opt_close_cb), appGUI);
	g_signal_connect (G_OBJECT (appGUI->cal->window_print_options), "key_press_event",
	                  G_CALLBACK (cal_print_opt_key_press_cb), appGUI);
	gtk_window_set_transient_for (GTK_WINDOW (appGUI->cal->window_print_options), GTK_WINDOW (appGUI->main_window));
	gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->window_print_options), 4);

	/*---------------------------------------------------------------------------------*/

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (appGUI->cal->window_print_options), vbox1);

	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox1), frame1, FALSE, FALSE, 2);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame1), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	table_fonts = gtk_table_new (4, 3, FALSE);
	gtk_widget_show (table_fonts);
	gtk_container_add (GTK_CONTAINER (alignment), table_fonts);
	gtk_table_set_row_spacings (GTK_TABLE (table_fonts), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table_fonts), 4);

	/* table_fonts: row 1 */
	g_snprintf (buffer, BUFFER_SIZE, "%s:", _("Month name"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table_fonts), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	appGUI->cal->print_month_name_font_entry = gtk_entry_new ();
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_month_name_font_entry, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_month_name_font_entry);
	gtk_table_attach (GTK_TABLE (table_fonts), appGUI->cal->print_month_name_font_entry, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->print_month_name_font_entry), config.cal_print_month_name_font);

	sel1.config = config.cal_print_month_name_font;
	sel1.entry = appGUI->cal->print_month_name_font_entry;

	if (config.default_stock_icons) {
		month_name_font_button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	} else {
		month_name_font_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS (month_name_font_button, GTK_CAN_FOCUS);
	gtk_widget_show (month_name_font_button);
	g_signal_connect (G_OBJECT (month_name_font_button), "clicked",
	                  G_CALLBACK (utl_gui_font_select_cb), &sel1);
	gtk_table_attach (GTK_TABLE (table_fonts), month_name_font_button, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/* table_fonts: row 2 */
	g_snprintf (buffer, BUFFER_SIZE, "%s:", _("Day name"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table_fonts), label, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	appGUI->cal->print_day_name_font_entry = gtk_entry_new ();
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_day_name_font_entry, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_day_name_font_entry);
	gtk_table_attach (GTK_TABLE (table_fonts), appGUI->cal->print_day_name_font_entry, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->print_day_name_font_entry), config.cal_print_day_name_font);

	sel2.config = config.cal_print_day_name_font;
	sel2.entry = appGUI->cal->print_day_name_font_entry;

	if (config.default_stock_icons) {
		day_name_font_button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	} else {
		day_name_font_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS (day_name_font_button, GTK_CAN_FOCUS);
	gtk_widget_show (day_name_font_button);
	g_signal_connect (G_OBJECT (day_name_font_button), "clicked",
	                  G_CALLBACK (utl_gui_font_select_cb), &sel2);
	gtk_table_attach (GTK_TABLE (table_fonts), day_name_font_button, 2, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/* table_fonts: row 3 */
	g_snprintf (buffer, BUFFER_SIZE, "%s:", _("Day number"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table_fonts), label, 0, 1, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	appGUI->cal->print_day_num_font_entry = gtk_entry_new ();
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_day_num_font_entry, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_day_num_font_entry);
	gtk_table_attach (GTK_TABLE (table_fonts), appGUI->cal->print_day_num_font_entry, 1, 2, 2, 3,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->print_day_num_font_entry), config.cal_print_day_num_font);

	sel3.config = config.cal_print_day_num_font;
	sel3.entry = appGUI->cal->print_day_num_font_entry;

	if (config.default_stock_icons) {
		day_number_font_button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	} else {
		day_number_font_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS (day_number_font_button, GTK_CAN_FOCUS);
	gtk_widget_show (day_number_font_button);
	g_signal_connect (G_OBJECT (day_number_font_button), "clicked",
	                  G_CALLBACK (utl_gui_font_select_cb), &sel3);
	gtk_table_attach (GTK_TABLE (table_fonts), day_number_font_button, 2, 3, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/* table_fonts: row 4 */
	g_snprintf (buffer, BUFFER_SIZE, "%s:", _("Events"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table_fonts), label, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	appGUI->cal->print_event_font_entry = gtk_entry_new ();
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_event_font_entry, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_event_font_entry);
	gtk_table_attach (GTK_TABLE (table_fonts), appGUI->cal->print_event_font_entry, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->print_event_font_entry), config.cal_print_event_font);

	sel4.config = config.cal_print_event_font;
	sel4.entry = appGUI->cal->print_event_font_entry;

	if (config.default_stock_icons) {
		event_font_button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	} else {
		event_font_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	}
	GTK_WIDGET_UNSET_FLAGS (event_font_button, GTK_CAN_FOCUS);
	gtk_widget_show (event_font_button);
	g_signal_connect (G_OBJECT (event_font_button), "clicked",
	                  G_CALLBACK (utl_gui_font_select_cb), &sel4);
	gtk_table_attach (GTK_TABLE (table_fonts), event_font_button, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	/* table_fonts: END */

	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("Fonts"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_frame_set_label_widget (GTK_FRAME (frame1), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	frame2 = gtk_frame_new (NULL);
	gtk_widget_show (frame2);
	gtk_box_pack_start (GTK_BOX (hbox1), frame2, TRUE, TRUE, 2);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame2), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (alignment), vbox2);

	appGUI->cal->print_tasks_checkbutton = gtk_check_button_new_with_mnemonic (_("Tasks"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_tasks_checkbutton, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_tasks_checkbutton);
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->print_tasks_checkbutton, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->print_tasks_checkbutton),
	                              config.cal_print_tasks);

	appGUI->cal->print_birthdays_checkbutton = gtk_check_button_new_with_mnemonic (_("Birthdays"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_birthdays_checkbutton, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_birthdays_checkbutton);
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->print_birthdays_checkbutton, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->print_birthdays_checkbutton),
	                              config.cal_print_birthdays);

	appGUI->cal->print_namedays_checkbutton = gtk_check_button_new_with_mnemonic (_("Namedays"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_namedays_checkbutton, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_namedays_checkbutton);
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->print_namedays_checkbutton, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->print_namedays_checkbutton),
	                              config.cal_print_namedays);

	appGUI->cal->print_day_notes_checkbutton = gtk_check_button_new_with_mnemonic (_("Day notes"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_day_notes_checkbutton, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_day_notes_checkbutton);
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->print_day_notes_checkbutton, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->print_day_notes_checkbutton),
	                              config.cal_print_day_notes);

	/*
	appGUI->cal->print_ical_checkbutton = gtk_check_button_new_with_mnemonic (_("ICal"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_ical_checkbutton, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_ical_checkbutton);
	gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->print_ical_checkbutton, FALSE, FALSE, 0);
	gtk_widget_set_sensitive (appGUI->cal->print_ical_checkbutton, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->print_ical_checkbutton),
	                              config.cal_print_ical);
	*/

	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("Visible events"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_frame_set_label_widget (GTK_FRAME (frame2), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	vseparator1 = gtk_vseparator_new ();
	gtk_widget_show (vseparator1);
	gtk_box_pack_start (GTK_BOX (hbox1), vseparator1, TRUE, TRUE, 0);

	frame3 = gtk_frame_new (NULL);
	gtk_widget_show (frame3);
	gtk_box_pack_start (GTK_BOX (hbox1), frame3, TRUE, TRUE, 2);
	gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);

	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("Options"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_frame_set_label_widget (GTK_FRAME (frame3), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame3), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	table5 = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (table5);
	gtk_container_add (GTK_CONTAINER (alignment), table5);
	gtk_table_set_row_spacings (GTK_TABLE (table5), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table5), 4);

	g_snprintf (buffer, BUFFER_SIZE, "%s:", _("Padding"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table5), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	g_snprintf (buffer, BUFFER_SIZE, "%s:", _("Event maximum length"));
	label = gtk_label_new (buffer);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table5), label, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	spinbutton_adj = gtk_adjustment_new (config.cal_print_padding, 1, 16, 1, 10, 0);
	appGUI->cal->print_padding_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (appGUI->cal->print_padding_spinbutton);
	gtk_table_attach (GTK_TABLE (table5), appGUI->cal->print_padding_spinbutton, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	spinbutton_adj = gtk_adjustment_new (config.cal_print_event_length, 6, 256, 1, 10, 0);
	appGUI->cal->print_event_length_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (appGUI->cal->print_event_length_spinbutton);
	gtk_table_attach (GTK_TABLE (table5), appGUI->cal->print_event_length_spinbutton, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new (_("Page orientation:"));
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table5), label, 0, 1, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	appGUI->cal->print_portrait_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Portrait"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->cal->print_portrait_radiobutton, GTK_CAN_FOCUS);
	gtk_widget_show (appGUI->cal->print_portrait_radiobutton);
	gtk_table_attach (GTK_TABLE (table5), appGUI->cal->print_portrait_radiobutton, 1, 2, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (appGUI->cal->print_portrait_radiobutton), radiobutton_group);
	radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (appGUI->cal->print_portrait_radiobutton));

	landscape_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Landscape"));
	GTK_WIDGET_UNSET_FLAGS (landscape_radiobutton, GTK_CAN_FOCUS);
	gtk_widget_show (landscape_radiobutton);
	gtk_table_attach (GTK_TABLE (table5), landscape_radiobutton, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (landscape_radiobutton), radiobutton_group);
	radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (landscape_radiobutton));

	if (config.cal_print_page_orientation == PORTRAIT) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cal->print_portrait_radiobutton), TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (landscape_radiobutton), TRUE);
	}

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, FALSE, TRUE, 4);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox), 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 16);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
	gtk_widget_show (cancel_button);
	g_signal_connect (cancel_button, "clicked", G_CALLBACK (window_button_cal_print_opt_close_cb), appGUI);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);

	ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
	gtk_widget_show (ok_button);
	g_signal_connect (ok_button, "clicked", G_CALLBACK (calendar_print_start_cb), appGUI);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), ok_button);

	gtk_widget_show (appGUI->cal->window_print_options);
}

/*------------------------------------------------------------------------------*/

#endif /* PRINTING_SUPPORT */

