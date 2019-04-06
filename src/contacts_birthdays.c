
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

#include "contacts_birthdays.h"
#include "i18n.h"
#include "calendar.h"
#include "contacts.h"
#include "calendar_notes.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "calendar_utils.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"

#ifdef CONTACTS_ENABLED

/*------------------------------------------------------------------------------*/

void
birthdays_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;

    gtk_window_get_size (GTK_WINDOW(appGUI->cnt->birthdays_window),
                        &config.contacts_birthdays_win_w, &config.contacts_birthdays_win_h);

    gtk_widget_destroy (appGUI->cnt->birthdays_window);
}

/*------------------------------------------------------------------------------*/

void
button_birthdays_window_close_cb (GtkButton *button, gpointer user_data)
{
	birthdays_window_close_cb (GTK_WIDGET (button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

gint 
birthdays_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	switch (event->keyval) {

		case GDK_Escape:
	        birthdays_window_close_cb (NULL, NULL, user_data);
			return TRUE;

	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

gint
birthdays_list_dbclick_cb (GtkWidget * widget, GdkEventButton * event, gpointer user_data)
{
GtkTreeIter iter;
GtkTreeModel *model, *contacts_model;
GtkTreePath *sort_path, *filter_path, *path;
gint id, id_c;

    GUI *appGUI = (GUI *) user_data;
	contacts_model = GTK_TREE_MODEL (appGUI->cnt->contacts_list_store);

    if (((event->type == GDK_2BUTTON_PRESS) && (event->button == 1)) == FALSE)
	    return FALSE;

	if (gtk_tree_selection_get_selected (appGUI->cnt->birthdays_list_selection, &model, &iter)) {

		if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->cnt->contacts_find_entry)))) {
			gtk_entry_set_text (GTK_ENTRY (appGUI->cnt->contacts_find_entry), "");
			gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->cnt->contacts_filter));
		}

		gtk_tree_model_get (model, &iter, B_COLUMN_ID, &id, -1);
		gtk_widget_destroy (appGUI->cnt->birthdays_window);

		contacts_selection_activate (FALSE, appGUI);

		if (gtk_tree_model_get_iter_first (contacts_model, &iter) == TRUE) {
			sort_path = gtk_tree_model_get_path (contacts_model, &iter);

			while (sort_path != NULL) {
				gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->cnt->contacts_list), sort_path, NULL, FALSE);
				filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->cnt->contacts_sort), sort_path);

				if (filter_path != NULL) {
					path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (appGUI->cnt->contacts_filter), filter_path);

					if (path != NULL) {
						gtk_tree_model_get_iter (contacts_model, &iter, path);
						gtk_tree_model_get (contacts_model, &iter, COLUMN_ID, &id_c, -1);

						if (id == id_c) {
							contacts_selection_activate (TRUE, appGUI);
							gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->cnt->contacts_list), sort_path, NULL, FALSE);
							show_contacts_desc_panel (TRUE, appGUI);
							gtk_tree_path_free (path);
							gtk_tree_path_free (filter_path);
							g_signal_emit_by_name (G_OBJECT (appGUI->cnt->contacts_list_selection), "changed");
							break;
						}

						gtk_tree_path_free (path);
					}

					gtk_tree_path_free (filter_path);
				}

				gtk_tree_path_next (sort_path);
				if (gtk_tree_model_get_iter (contacts_model, &iter, sort_path) == FALSE) break;
			}

			gtk_tree_path_free (sort_path);
		}

	}

	return TRUE;
}

/*------------------------------------------------------------------------------*/

void
contacts_create_birthdays_window (GUI *appGUI)
{
GtkWidget           *vbox1;
GtkWidget           *hseparator;
GtkWidget           *hbuttonbox;
GtkWidget           *close_button;
GtkTreeViewColumn   *column;
GtkCellRenderer     *renderer;
GtkWidget           *scrolledwindow;
gint i, n, id, age;
guint32 date;
gchar *text, buffer[BUFFER_SIZE], buff[BUFFER_SIZE];
GtkTreeIter iter, n_iter;
GDate *cdate_birthday, *cdate_current;
guint b_day, b_month, b_year;
guint c_day, c_month, c_year;
gboolean flag, leap;

	cdate_birthday = g_date_new ();
	g_return_if_fail (cdate_birthday != NULL);

	cdate_current = g_date_new ();
	g_return_if_fail (cdate_current != NULL);

	i = n = 0;
	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, COLUMN_BIRTH_DAY_DATE, &date, -1);
		if (date) n++;
	}

	if (n == 0) {
		utl_gui_create_dialog (GTK_MESSAGE_INFO, _("No birthdays defined"), GTK_WINDOW (appGUI->main_window));
		return;
	}

	appGUI->cnt->birthdays_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (appGUI->cnt->birthdays_window), _("Birthdays list"));
	gtk_window_set_position (GTK_WINDOW (appGUI->cnt->birthdays_window), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size (GTK_WINDOW (appGUI->cnt->birthdays_window), 
                                 config.contacts_birthdays_win_w, config.contacts_birthdays_win_h);
	gtk_window_set_modal (GTK_WINDOW (appGUI->cnt->birthdays_window), TRUE);
	g_signal_connect (G_OBJECT (appGUI->cnt->birthdays_window), "delete_event",
	                  G_CALLBACK (birthdays_window_close_cb), appGUI);
	gtk_window_set_transient_for (GTK_WINDOW (appGUI->cnt->birthdays_window), GTK_WINDOW (appGUI->main_window));
	gtk_container_set_border_width (GTK_CONTAINER (appGUI->cnt->birthdays_window), 8);
	g_signal_connect (G_OBJECT (appGUI->cnt->birthdays_window), "key_press_event",
	                  G_CALLBACK (birthdays_key_press_cb), appGUI);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (appGUI->cnt->birthdays_window), vbox1);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow, TRUE, TRUE, 0);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	appGUI->cnt->birthdays_list_store = gtk_list_store_new (BIRTHDAYS_NUM_COLUMNS,
	                                                        G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING,
	                                                        G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);

	appGUI->cnt->birthdays_list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (appGUI->cnt->birthdays_list_store));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), config.rules_hint);
	gtk_widget_show (appGUI->cnt->birthdays_list);
	GTK_WIDGET_SET_FLAGS (appGUI->cnt->birthdays_list, GTK_CAN_DEFAULT);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), appGUI->cnt->birthdays_list);

	appGUI->cnt->birthdays_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (appGUI->cnt->birthdays_list));

	g_signal_connect (G_OBJECT (appGUI->cnt->birthdays_list), "button_press_event",
	                  G_CALLBACK (birthdays_list_dbclick_cb), appGUI);

	/* create columns */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "xpad", 8, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer, "text", B_COLUMN_NAME, NULL);
	gtk_tree_view_column_set_visible (column, TRUE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);
	gtk_tree_view_column_set_sort_column_id (column, B_COLUMN_NAME);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Days to birthday"), renderer, "text", B_COLUMN_DAYS_NUM, NULL);
	gtk_tree_view_column_set_visible (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "xalign", 1.0, NULL);
	g_object_set (G_OBJECT (renderer), "xpad", 8, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Days to birthday"), renderer, "text", B_COLUMN_DAYS, NULL);
	gtk_tree_view_column_set_visible (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);
	gtk_tree_view_column_set_sort_column_id (column, B_COLUMN_DAYS_NUM);
	g_signal_emit_by_name (column, "clicked");

	column = gtk_tree_view_column_new_with_attributes (_("Age"), renderer, "text", B_COLUMN_AGE, NULL);
	gtk_tree_view_column_set_visible (column, config.cnt_visible_age_column);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);
	gtk_tree_view_column_set_sort_column_id (column, B_COLUMN_AGE);

	column = gtk_tree_view_column_new_with_attributes (_("Birthday date"), renderer, "text", B_COLUMN_DATE, NULL);
	gtk_tree_view_column_set_visible (column, config.cnt_visible_birthday_date_column);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Zodiac sign"), renderer, "text", B_COLUMN_ZODIAC, NULL);
	gtk_tree_view_column_set_visible (column, config.cnt_visible_zodiac_sign_column);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("ID", renderer, "text", B_COLUMN_ID, NULL);
	gtk_tree_view_column_set_visible (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), column);

	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (appGUI->cnt->birthdays_list), FALSE);

	i = 0;

	g_date_set_julian (cdate_current, utl_date_get_current_julian ());
	c_day = g_date_get_day (cdate_current);
	c_month = g_date_get_month (cdate_current);
	c_year = g_date_get_year (cdate_current);

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, COLUMN_BIRTH_DAY_DATE, &date, -1);

		if (g_date_valid_julian (date)) {

			/* calculate age */
			g_date_set_julian (cdate_birthday, date);
			b_day = g_date_get_day (cdate_birthday);
			b_month = g_date_get_month (cdate_birthday);
			b_year = g_date_get_year (cdate_birthday);

			age = (gint) c_year - b_year;
			if (b_month < c_month || (b_month == c_month && b_day < c_day)) age++;

			if (age < 1) continue;

			/* name */
			flag = FALSE;
			g_snprintf (buff, BUFFER_SIZE, "(%s)", _("None"));

			gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, COLUMN_LAST_NAME, &text, -1);
			if (text != NULL) {
				flag = TRUE;
				if (strcmp (text, buff) == 0) {
					text[0] = '\0';
					flag = FALSE;
				}
				g_strlcpy (buffer, text, BUFFER_SIZE);
				g_free (text);
			}

			gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, COLUMN_FIRST_NAME, &text, -1);
			if (text != NULL) {
				if (strcmp (text, buff) == 0) {
					text[0] = '\0';
				}
				if (flag == TRUE) {
					g_strlcat (buffer, " ", BUFFER_SIZE);
					g_strlcat (buffer, text, BUFFER_SIZE);
				} else {
					g_strlcpy (buffer, text, BUFFER_SIZE);
				}
				g_free (text);
			}

			gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cnt->contacts_list_store), &iter, COLUMN_ID, &id, -1);
			gtk_list_store_append (appGUI->cnt->birthdays_list_store, &n_iter);
			gtk_list_store_set (appGUI->cnt->birthdays_list_store, &n_iter,
			                    B_COLUMN_ID, id, B_COLUMN_NAME, buffer, B_COLUMN_AGE, age, -1);

			/* calculate days to birthday */
			b_year = c_year;
			if ((b_month < c_month) || (b_month == c_month && b_day < c_day)) b_year++;

			leap = FALSE;
			if (g_date_valid_dmy (b_day, b_month, b_year) == FALSE) {
				g_date_set_day (cdate_birthday, b_day - 1);
				leap = TRUE;
			}
			g_date_set_year (cdate_birthday, b_year);

			date = g_date_days_between (cdate_current, cdate_birthday);
			if (date == 0) {
				g_snprintf (buffer, BUFFER_SIZE, "%s", _("today"));
			} else {
				g_snprintf (buffer, BUFFER_SIZE, leap ? "%d + 1" : "%d", date);
			}

			g_date_strftime (buff, BUFFER_SIZE, "%A, ", cdate_birthday);
			g_strlcat (buff, julian_to_str (g_date_get_julian (cdate_birthday), config.date_format, config.override_locale_settings), BUFFER_SIZE);

			gtk_list_store_set (appGUI->cnt->birthdays_list_store, &n_iter,
			                    B_COLUMN_DAYS_NUM, date, B_COLUMN_DAYS, buffer,
			                    B_COLUMN_DATE, buff, B_COLUMN_ZODIAC, utl_get_zodiac_name (b_day, b_month), -1);
		}

	}
	g_date_free (cdate_birthday);
	g_date_free (cdate_current);

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
	g_signal_connect (close_button, "clicked", G_CALLBACK (button_birthdays_window_close_cb), appGUI);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), close_button);

	gtk_widget_show (appGUI->cnt->birthdays_window);
	gtk_widget_grab_focus (close_button);
}

/*------------------------------------------------------------------------------*/

#endif  /* CONTACTS_ENABLED */

