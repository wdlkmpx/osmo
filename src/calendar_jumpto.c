
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

#include "calendar_jumpto.h"
#include "i18n.h"
#include "calendar.h"
#include "calendar_utils.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"
#include "options_prefs.h"
#include "stock_icons.h"

/*------------------------------------------------------------------------------*/

gboolean
check_number (const gchar *str) {

gint i, n;

    n = strlen(str);
    if (!n) return FALSE;

    for(i=0;i<n;i++)
        if (!isdigit(str[i])) return FALSE;

    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
jumpto_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;
    gtk_widget_destroy (appGUI->cal->jumpto_window);
}

/*------------------------------------------------------------------------------*/

gint 
jumpto_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
            jumpto_window_close_cb (NULL, NULL, appGUI);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

gint 
day_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

char tmpbuf[BUFFER_SIZE];
gint a;

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Return) {

        if (check_number(gtk_entry_get_text(GTK_ENTRY(appGUI->cal->day_entry))) == FALSE) {
            g_snprintf (tmpbuf, BUFFER_SIZE, "%d", utl_date_get_current_day ());
            gtk_entry_set_text (GTK_ENTRY(appGUI->cal->day_entry), tmpbuf);
        }

        a = atoi(gtk_entry_get_text(GTK_ENTRY(appGUI->cal->day_entry)));

        if (a > 31)
            gtk_entry_set_text (GTK_ENTRY(appGUI->cal->day_entry), "31");
        if (a < 1)
            gtk_entry_set_text (GTK_ENTRY(appGUI->cal->day_entry), "1");

        gtk_widget_grab_focus(GTK_WIDGET(appGUI->cal->month_entry));
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

gint 
month_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

char tmpbuf[BUFFER_SIZE];
gint a;

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Return) {

        if (check_number(gtk_entry_get_text(GTK_ENTRY(appGUI->cal->month_entry))) == FALSE) {
            g_snprintf (tmpbuf, BUFFER_SIZE, "%d", utl_date_get_current_month ());
            gtk_entry_set_text (GTK_ENTRY(appGUI->cal->month_entry), tmpbuf);
        }

        a = atoi(gtk_entry_get_text(GTK_ENTRY(appGUI->cal->month_entry)));

        if (a > 12)
            gtk_entry_set_text (GTK_ENTRY(appGUI->cal->month_entry), "12");
        if (a < 1)
            gtk_entry_set_text (GTK_ENTRY(appGUI->cal->month_entry), "1");

        gtk_widget_grab_focus(GTK_WIDGET(appGUI->cal->year_entry));
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/
/* FIXME */
void
jumpto_go_window_close_cb (GtkButton *button, gpointer user_data)
{
GDate *cdate;

	GUI *appGUI = (GUI *)user_data;

	appGUI->cal->jday = atoi (gtk_entry_get_text (GTK_ENTRY (appGUI->cal->day_entry)));
	appGUI->cal->jmonth = atoi (gtk_entry_get_text (GTK_ENTRY (appGUI->cal->month_entry)));
	appGUI->cal->jyear = atoi (gtk_entry_get_text (GTK_ENTRY (appGUI->cal->year_entry)));

	if ((appGUI->cal->jyear > JULIAN_GREGORIAN_YEAR) &&
	    (appGUI->cal->jmonth > 0 && appGUI->cal->jmonth <= 12)) {
		if (appGUI->cal->jday > 0 &&
		    appGUI->cal->jday <= g_date_get_days_in_month (appGUI->cal->jmonth, appGUI->cal->jyear)) {

			cdate = g_date_new_dmy (appGUI->cal->jday, appGUI->cal->jmonth, appGUI->cal->jyear);
			g_return_if_fail (cdate != NULL);

			cal_jump_to_date (cdate, appGUI);
			update_aux_calendars (appGUI);
			g_date_free (cdate);
		}
	}

	jumpto_window_close_cb (GTK_WIDGET (button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

void
button_jumpto_window_close_cb (GtkButton *button, gpointer user_data) {
 
    jumpto_window_close_cb (GTK_WIDGET(button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

gint
year_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Return) {

        jumpto_go_window_close_cb (NULL, appGUI); 
        return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
calendar_create_jumpto_window (GUI *appGUI) {

GtkWidget   *vbox1;
GtkWidget   *hbox1;
GtkWidget   *label;
GtkWidget   *hseparator;
GtkWidget   *hbuttonbox;
GtkWidget   *cancel_button;
GtkWidget   *jumpto_button;
gint        win_xpos, win_ypos;
char tmpbuf[BUFFER_SIZE];

    appGUI->cal->jumpto_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (appGUI->cal->jumpto_window), _("Jump to date"));
    gtk_window_set_position (GTK_WINDOW (appGUI->cal->jumpto_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal (GTK_WINDOW (appGUI->cal->jumpto_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->cal->jumpto_window), "delete_event",
                      G_CALLBACK(jumpto_window_close_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->jumpto_window), "key_press_event",
                        G_CALLBACK (jumpto_key_press_cb), appGUI);
    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cal->jumpto_window), GTK_WINDOW(appGUI->main_window));
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->jumpto_window), 8);
    gtk_window_set_resizable (GTK_WINDOW (appGUI->cal->jumpto_window), FALSE);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cal->jumpto_window), vbox1);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);

    sprintf(tmpbuf, "%s:", _("Day"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
    gtk_misc_set_padding (GTK_MISC (label), 8, 0);

    appGUI->cal->day_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->cal->day_entry);
    gtk_widget_set_size_request (appGUI->cal->day_entry, 50, -1);
    gtk_entry_set_max_length (GTK_ENTRY(appGUI->cal->day_entry), 2);
    g_signal_connect (G_OBJECT (appGUI->cal->day_entry), "key_press_event",
                        G_CALLBACK (day_key_press_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->day_entry, FALSE, TRUE, 1);

    sprintf(tmpbuf, "%s:", _("Month"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
    gtk_misc_set_padding (GTK_MISC (label), 8, 0);

    appGUI->cal->month_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->cal->month_entry);
    gtk_widget_set_size_request (appGUI->cal->month_entry, 50, -1);
    gtk_entry_set_max_length (GTK_ENTRY(appGUI->cal->month_entry), 2);
    g_signal_connect (G_OBJECT (appGUI->cal->month_entry), "key_press_event",
                        G_CALLBACK (month_key_press_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->month_entry, FALSE, TRUE, 1);

    sprintf(tmpbuf, "%s:", _("Year"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
    gtk_misc_set_padding (GTK_MISC (label), 8, 0);

    appGUI->cal->year_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->cal->year_entry);
    gtk_widget_set_size_request (appGUI->cal->year_entry, 50, -1);
    gtk_entry_set_max_length (GTK_ENTRY(appGUI->cal->year_entry), 4);
    g_signal_connect (G_OBJECT (appGUI->cal->year_entry), "key_press_event",
                        G_CALLBACK (year_key_press_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->year_entry, FALSE, TRUE, 1);

	g_snprintf (tmpbuf, BUFFER_SIZE, "%d", utl_date_get_current_day ());
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->day_entry), tmpbuf);
	g_snprintf (tmpbuf, BUFFER_SIZE, "%d", utl_date_get_current_month ());
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->month_entry), tmpbuf);
	g_snprintf (tmpbuf, BUFFER_SIZE, "%d", utl_date_get_current_year ());
	gtk_entry_set_text (GTK_ENTRY (appGUI->cal->year_entry), tmpbuf);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
    gtk_widget_show (cancel_button);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(button_jumpto_window_close_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), cancel_button);

	jumpto_button = utl_gui_create_button (GTK_STOCK_JUMP_TO, OSMO_STOCK_BUTTON_JUMPTO, _("Jump to"));
    gtk_widget_show (jumpto_button);
    g_signal_connect(jumpto_button, "clicked", G_CALLBACK(jumpto_go_window_close_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), jumpto_button);

    gtk_widget_grab_focus(GTK_WIDGET(appGUI->cal->day_entry));

    gtk_window_get_position (GTK_WINDOW(appGUI->cal->jumpto_window), &win_xpos, &win_ypos);
    gtk_window_move (GTK_WINDOW (appGUI->cal->jumpto_window), win_xpos-5, win_ypos-40);
    gtk_widget_show(appGUI->cal->jumpto_window);

}

/*------------------------------------------------------------------------------*/

