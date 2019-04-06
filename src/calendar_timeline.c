
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


#include "calendar_timeline.h"
#include "i18n.h"
#include "calendar.h"
#include "options_prefs.h"
#include "stock_icons.h"

/*------------------------------------------------------------------------------*/

void
insert_timeline_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;
    gtk_widget_destroy (appGUI->cal->insert_timeline_window);
}

/*------------------------------------------------------------------------------*/

void
button_insert_timeline_window_close_cb (GtkWidget *widget, gpointer data) {

    insert_timeline_window_close_cb (widget, NULL, data);
}

/*------------------------------------------------------------------------------*/

gint 
insert_timeline_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
            insert_timeline_window_close_cb (NULL, NULL, appGUI);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
insert_timeline_action_cb (GtkButton *button, gpointer user_data) {

gint ts, te, m;
GtkTextBuffer *text_buffer;
GtkTextIter titer;
char tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)user_data;

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appGUI->cal->calendar_note_textview));
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &titer, -1);

    ts = config.timeline_start;
    te = config.timeline_end + config.timeline_step;

    do {

        m = ts / 60;
        sprintf(tmpbuf, "%02d:%02d - \n", m, ts-m*60);
        gtk_text_buffer_insert(text_buffer, &titer, tmpbuf, -1);

        ts += config.timeline_step;

    } while (ts < te);

    insert_timeline_window_close_cb (NULL, NULL, appGUI);

    enable_disable_note_buttons(appGUI);
}

/*------------------------------------------------------------------------------*/

void
fill_timeline(GUI *appGUI) {

gint i;

    i = config.timeline_start / 60;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(appGUI->cal->tl_start_h_spinbutton), i);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(appGUI->cal->tl_start_m_spinbutton), config.timeline_start-i*60);
    i = config.timeline_end / 60;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(appGUI->cal->tl_end_h_spinbutton), i);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(appGUI->cal->tl_end_m_spinbutton), config.timeline_end-i*60);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(appGUI->cal->tl_step_spinbutton), (gdouble)config.timeline_step);
}

/*------------------------------------------------------------------------------*/

void
timeline_changed_cb (GtkSpinButton *spinbutton, gpointer user_data) {

gint i, j;

    GUI *appGUI = (GUI *)user_data;

    i = (gint) gtk_spin_button_get_value (GTK_SPIN_BUTTON(appGUI->cal->tl_start_h_spinbutton)) * 60 +
        (gint) gtk_spin_button_get_value (GTK_SPIN_BUTTON(appGUI->cal->tl_start_m_spinbutton));
    j = (gint) gtk_spin_button_get_value (GTK_SPIN_BUTTON(appGUI->cal->tl_end_h_spinbutton)) * 60 +
        (gint) gtk_spin_button_get_value (GTK_SPIN_BUTTON(appGUI->cal->tl_end_m_spinbutton));

    if (i < j) {
        config.timeline_start = i;
        config.timeline_end = j;
        config.timeline_step = (gint) gtk_spin_button_get_value (GTK_SPIN_BUTTON(appGUI->cal->tl_step_spinbutton));
    } else {
        fill_timeline(appGUI);
    }
}

/*------------------------------------------------------------------------------*/

void
calendar_create_insert_timeline_window (GUI *appGUI) {

GtkWidget   *vbox1;
GtkWidget   *label;
GtkWidget   *hseparator;
GtkWidget   *hbuttonbox;
GtkWidget   *timeline_table;
GtkWidget   *frame;
GtkWidget   *alignment;
GtkObject   *tl_start_h_spinbutton_adj;
GtkObject   *tl_start_m_spinbutton_adj;
GtkObject   *tl_end_h_spinbutton_adj;
GtkObject   *tl_end_m_spinbutton_adj;
GtkObject   *tl_step_spinbutton_adj;
gint        win_xpos, win_ypos;
char tmpbuf[BUFFER_SIZE];

    appGUI->cal->insert_timeline_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (appGUI->cal->insert_timeline_window), _("Insert timeline"));
    gtk_window_set_position (GTK_WINDOW (appGUI->cal->insert_timeline_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal (GTK_WINDOW (appGUI->cal->insert_timeline_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->cal->insert_timeline_window), "delete_event",
                      G_CALLBACK(insert_timeline_window_close_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->insert_timeline_window), "key_press_event",
                        G_CALLBACK (insert_timeline_key_press_cb), appGUI);
    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cal->insert_timeline_window), GTK_WINDOW(appGUI->main_window));
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->insert_timeline_window), 8);
    gtk_window_set_resizable (GTK_WINDOW (appGUI->cal->insert_timeline_window), FALSE);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cal->insert_timeline_window), vbox1);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    sprintf(tmpbuf, "<b>%s</b>", _("Timeline"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_padding (GTK_MISC (label), 0, 4);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 12, 12, 0);

    timeline_table = gtk_table_new (1, 9, FALSE);
    gtk_widget_show (timeline_table);
    gtk_container_set_border_width (GTK_CONTAINER (timeline_table), 4);
    gtk_table_set_col_spacings (GTK_TABLE (timeline_table), 8);
    gtk_table_set_row_spacings (GTK_TABLE (timeline_table), 4);
    gtk_container_add (GTK_CONTAINER (alignment), timeline_table);

    sprintf(tmpbuf, "%s:", _("From (hour)"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (timeline_table), label, 0, 1, 0, 1,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    tl_start_h_spinbutton_adj = gtk_adjustment_new (1, 0, 24, 1, 10, 0);
    appGUI->cal->tl_start_h_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (tl_start_h_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->cal->tl_start_h_spinbutton);
    gtk_table_attach (GTK_TABLE (timeline_table), appGUI->cal->tl_start_h_spinbutton, 1, 2, 0, 1,
                     (GtkAttachOptions) (0),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (appGUI->cal->tl_start_h_spinbutton), TRUE);

    sprintf(tmpbuf, "%s:", _("Step (minutes)"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (timeline_table), label, 0, 1, 2, 3,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    tl_step_spinbutton_adj = gtk_adjustment_new (1, 1, 120, 1, 10, 0);
    appGUI->cal->tl_step_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (tl_step_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->cal->tl_step_spinbutton);
    gtk_table_attach (GTK_TABLE (timeline_table), appGUI->cal->tl_step_spinbutton, 1, 2, 2, 3,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (appGUI->cal->tl_step_spinbutton), TRUE);

    sprintf(tmpbuf, "%s:", _("To (hour)"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (timeline_table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    tl_end_h_spinbutton_adj = gtk_adjustment_new (1, 1, 23, 1, 10, 0);
    appGUI->cal->tl_end_h_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (tl_end_h_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->cal->tl_end_h_spinbutton);
    gtk_table_attach (GTK_TABLE (timeline_table), appGUI->cal->tl_end_h_spinbutton, 1, 2, 1, 2,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (appGUI->cal->tl_end_h_spinbutton), TRUE);

    label = gtk_label_new (":");
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (timeline_table), label, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    label = gtk_label_new (":");
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (timeline_table), label, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    tl_start_m_spinbutton_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
    appGUI->cal->tl_start_m_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (tl_start_m_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->cal->tl_start_m_spinbutton);
    gtk_table_attach (GTK_TABLE (timeline_table), appGUI->cal->tl_start_m_spinbutton, 3, 4, 0, 1,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (appGUI->cal->tl_start_m_spinbutton), TRUE);

    tl_end_m_spinbutton_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
    appGUI->cal->tl_end_m_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (tl_end_m_spinbutton_adj), 1, 0);
    gtk_widget_show (appGUI->cal->tl_end_m_spinbutton);
    gtk_table_attach (GTK_TABLE (timeline_table), appGUI->cal->tl_end_m_spinbutton, 3, 4, 1, 2,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (appGUI->cal->tl_end_m_spinbutton), TRUE);

    fill_timeline(appGUI);

    g_signal_connect (G_OBJECT (appGUI->cal->tl_start_h_spinbutton), "value-changed",
                      G_CALLBACK (timeline_changed_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->tl_step_spinbutton), "value-changed",
                      G_CALLBACK (timeline_changed_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->tl_end_h_spinbutton), "value-changed",
                      G_CALLBACK (timeline_changed_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->tl_start_m_spinbutton), "value-changed",
                      G_CALLBACK (timeline_changed_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->tl_end_m_spinbutton), "value-changed",
                      G_CALLBACK (timeline_changed_cb), appGUI);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

    if (config.default_stock_icons) {
        appGUI->cal->cancel_button = gtk_button_new_from_stock (GTK_STOCK_CANCEL); 
    } else {
        appGUI->cal->cancel_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_CANCEL); 
    }
    gtk_widget_show (appGUI->cal->cancel_button);
    g_signal_connect(appGUI->cal->cancel_button, "clicked", G_CALLBACK(button_insert_timeline_window_close_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), appGUI->cal->cancel_button);   

    if (config.default_stock_icons) {
        appGUI->cal->insert_timeline_button = gtk_button_new_from_stock (GTK_STOCK_PASTE); 
    } else {
        appGUI->cal->insert_timeline_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_INSERT_TIMELINE); 
    }
    gtk_widget_show (appGUI->cal->insert_timeline_button);
    g_signal_connect(appGUI->cal->insert_timeline_button, "clicked", G_CALLBACK(insert_timeline_action_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), appGUI->cal->insert_timeline_button);   

    gtk_window_get_position (GTK_WINDOW(appGUI->cal->insert_timeline_window), &win_xpos, &win_ypos);
    gtk_window_move (GTK_WINDOW (appGUI->cal->insert_timeline_window), win_xpos-5, win_ypos-40);
    gtk_widget_show(appGUI->cal->insert_timeline_window);

}

/*------------------------------------------------------------------------------*/

