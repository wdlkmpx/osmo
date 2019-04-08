
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

#include "contacts_import.h"
#include "contacts_import_csv.h"
#include "i18n.h"
#include "utils.h"
#include "utils_date.h"
#include "utils_gui.h"
#include "contacts.h"
#include "calendar_utils.h"
#include "stock_icons.h"
#include "options_prefs.h"

#ifdef CONTACTS_ENABLED

/*-------------------------------------------------------------------------------------*/

void
import_store_values (GUI *appGUI) {

    config.import_type = gtk_combo_box_get_active (GTK_COMBO_BOX (appGUI->cnt->import_type_combobox));

    config.import_binary_xml = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->cnt->use_wbxml_checkbutton));

}

/*-------------------------------------------------------------------------------------*/

gboolean
import_contacts_select_file (GUI *appGUI) {

GtkWidget *dialog;
GtkFileFilter *filter_1, *filter_2;
gboolean ret = FALSE;

    dialog = gtk_file_chooser_dialog_new(_("Select CSV file"),
                                         GTK_WINDOW(appGUI->cnt->import_sel_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

    filter_1 = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter_1, "*");
    gtk_file_filter_set_name(GTK_FILE_FILTER(filter_1), _("All Files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_1);

    filter_2 = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter_2, "*.[cC][sS][vV]");
    gtk_file_filter_set_name(GTK_FILE_FILTER(filter_2), _("CSV (comma-separated values) files (*.csv)"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_2);

    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter_2);


    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

        gtk_widget_hide(dialog);
        while (g_main_context_iteration(NULL, FALSE));

        ret = add_csv_records(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)), appGUI);

    }

    gtk_widget_destroy(dialog);

    return ret;
}

/*-------------------------------------------------------------------------------------*/

gboolean
import_contacts_from_csv_file (GUI *appGUI) {
    return import_contacts_select_file (appGUI);
}

/*-------------------------------------------------------------------------------------*/

void
import_sel_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    gdk_window_get_root_origin (GDK_WINDOW(appGUI->cnt->import_sel_window->window),
                                &config.contacts_import_sel_win_x, &config.contacts_import_sel_win_y);

    import_store_values (appGUI);

    gtk_widget_destroy(appGUI->cnt->import_sel_window);
}

/*-------------------------------------------------------------------------------------*/

gint
import_sel_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    switch(event->keyval) {
        case GDK_Escape:
            import_sel_window_close_cb (widget, NULL, appGUI);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
button_import_sel_window_close_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    import_sel_window_close_cb (widget, NULL, appGUI);

}

/*------------------------------------------------------------------------------*/

void
import_type_selected_cb (GtkComboBox *widget, gpointer user_data) {

gint import_type;

    GUI *appGUI = (GUI *)user_data;

    import_type = gtk_combo_box_get_active (widget);

    switch (import_type) {
        case IMPORT_TYPE_FILE:
            gtk_widget_set_sensitive(appGUI->cnt->file_import_vbox, TRUE);
            break;
    }

}

/*------------------------------------------------------------------------------*/

gboolean
import_sel_combo_box_focus_cb (GtkWidget *widget, GtkDirectionType *arg1, gpointer user_data) {
    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
import_sel_cb (GtkWidget *widget, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;
    gchar *text;
    gchar tmpbuf[BUFFER_SIZE];

    import_store_values (appGUI);

    if (config.import_type == IMPORT_TYPE_FILE) {

        /* FILE */

        text = (gchar *)gtk_entry_get_text (GTK_ENTRY(appGUI->cnt->input_file_entry));

        if (!strlen(text)) {
            sprintf (tmpbuf, "%s", _("Please select file first."));
            utl_gui_create_dialog (GTK_MESSAGE_ERROR, tmpbuf, GTK_WINDOW(appGUI->cnt->import_sel_window));
            return;
        }
        add_csv_records(text, appGUI);

    }

    import_sel_window_close_cb (NULL, NULL, appGUI);
}

/*------------------------------------------------------------------------------*/

void
import_contacts_show_dialog (GUI *appGUI) {

GtkWidget *vbox1;
GtkWidget *hseparator;
GtkWidget *hbox1;
GtkWidget *label;
GtkWidget *frame;
GtkWidget *alignment;
GtkWidget *hbox4;
GtkWidget *vbox4;
GtkWidget *hbox6;
GSList *radiobutton_group = NULL;
GtkWidget *hbox7;
GtkWidget *cancel_button;
GtkWidget *hbuttonbox;
gchar tmpbuf[BUFFER_SIZE];


    appGUI->cnt->import_sel_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cnt->import_sel_window), 6);
    gtk_window_set_title (GTK_WINDOW (appGUI->cnt->import_sel_window), _("Import contacts"));

    gtk_window_move (GTK_WINDOW (appGUI->cnt->import_sel_window), 
                     config.contacts_import_sel_win_x, config.contacts_import_sel_win_y);
    gtk_window_set_default_size (GTK_WINDOW(appGUI->cnt->import_sel_window), 450, -1);

    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cnt->import_sel_window), GTK_WINDOW(appGUI->main_window));
    gtk_window_set_modal(GTK_WINDOW(appGUI->cnt->import_sel_window), TRUE);

    g_signal_connect (G_OBJECT (appGUI->cnt->import_sel_window), "key_press_event", 
                      G_CALLBACK (import_sel_key_press_cb), appGUI);

    g_signal_connect (G_OBJECT (appGUI->cnt->import_sel_window), "delete_event", 
                      G_CALLBACK(import_sel_window_close_cb), appGUI);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cnt->import_sel_window), vbox1);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, FALSE, 4);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

    sprintf (tmpbuf, "%s:", _("Import type"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 5);

    appGUI->cnt->import_type_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->cnt->import_type_combobox);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cnt->import_type_combobox, FALSE, FALSE, 8);
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->import_type_combobox), _("File"));
    g_signal_connect (G_OBJECT (appGUI->cnt->import_type_combobox), "changed",
                      G_CALLBACK (import_type_selected_cb), appGUI);
    g_signal_connect(G_OBJECT(appGUI->cnt->import_type_combobox), "focus",
                     G_CALLBACK(import_sel_combo_box_focus_cb), appGUI);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    appGUI->cnt->file_import_vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (appGUI->cnt->file_import_vbox);
    gtk_box_pack_start (GTK_BOX (vbox1), appGUI->cnt->file_import_vbox, FALSE, FALSE, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (appGUI->cnt->file_import_vbox), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    hbox4 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox4);
    gtk_container_add (GTK_CONTAINER (alignment), hbox4);

    appGUI->cnt->input_file_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->cnt->input_file_entry);
    gtk_box_pack_start (GTK_BOX (hbox4), appGUI->cnt->input_file_entry, TRUE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->input_file_entry, GTK_CAN_FOCUS);
    gtk_editable_set_editable (GTK_EDITABLE(appGUI->cnt->input_file_entry), FALSE);

	appGUI->cnt->contacts_browse_button = utl_gui_create_button (GTK_STOCK_OPEN, OSMO_STOCK_BUTTON_OPEN, _("Browse"));
    GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->contacts_browse_button, GTK_CAN_FOCUS);
    gtk_widget_show (appGUI->cnt->contacts_browse_button);
    gtk_box_pack_start (GTK_BOX (hbox4), appGUI->cnt->contacts_browse_button, FALSE, TRUE, 4);

    sprintf (tmpbuf, "<b>%s:</b>", _("Input filename"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    vbox4 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox4);
    gtk_container_add (GTK_CONTAINER (alignment), vbox4);

    hbox6 = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox6);
    gtk_box_pack_start (GTK_BOX (vbox4), hbox6, TRUE, TRUE, 2);

    hbox7 = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox7);
    gtk_box_pack_start (GTK_BOX (vbox4), hbox7, TRUE, TRUE, 2);

    sprintf (tmpbuf, "<b>%s</b>", _("Interface"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 2, 2, 12, 0);

    appGUI->cnt->use_wbxml_checkbutton = gtk_check_button_new_with_mnemonic (_("Use binary XML (WBXML)"));
    gtk_widget_show (appGUI->cnt->use_wbxml_checkbutton);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->cnt->use_wbxml_checkbutton);

    sprintf (tmpbuf, "<b>%s</b>", _("Options"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 8);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
    gtk_widget_show (cancel_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
    g_signal_connect(cancel_button, "clicked", 
                     G_CALLBACK(button_import_sel_window_close_cb), appGUI);

	appGUI->cnt->import_button = utl_gui_create_button (OSMO_STOCK_BUTTON_CONTACTS_IMPORT, OSMO_STOCK_BUTTON_CONTACTS_IMPORT, _("Import"));
    gtk_widget_show (appGUI->cnt->import_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->cnt->import_button);
    g_signal_connect(appGUI->cnt->import_button, "clicked",
                     G_CALLBACK(import_sel_cb), appGUI);

    /* setup fields */

    gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->import_type_combobox), config.import_type);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->cnt->use_wbxml_checkbutton), config.import_binary_xml);

    gtk_widget_show (appGUI->cnt->import_sel_window);

}

/*-------------------------------------------------------------------------------------*/

void
import_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {
 
    GUI *appGUI = (GUI *)user_data;

    gtk_window_get_size (GTK_WINDOW(appGUI->cnt->import_window),
                        &config.contacts_import_win_w, &config.contacts_import_win_h);
    gdk_window_get_root_origin (GDK_WINDOW(appGUI->cnt->import_window->window),
                                &config.contacts_import_win_x, &config.contacts_import_win_y);

    gtk_widget_destroy(appGUI->cnt->import_window);

    if(appGUI->cnt->file_buffer != NULL) {
        g_free(appGUI->cnt->file_buffer);
        appGUI->cnt->file_buffer = NULL;
        appGUI->cnt->file_length = 0;
    }
}

/*------------------------------------------------------------------------------*/

void
button_import_window_close_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    import_window_close_cb (widget, NULL, appGUI);

}

/*------------------------------------------------------------------------------*/

gint
import_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    switch(event->keyval) {
        case GDK_Escape:
            import_window_close_cb (widget, NULL, appGUI);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
record_changed_cb (GtkSpinButton *spinbutton, gpointer user_data) {

gint i;
gchar *str, *field_str;

    GUI *appGUI = (GUI *)user_data;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->cnt->first_row_as_header_check_button)) == TRUE) {
        str = csv_get_line(gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton))+1, appGUI);
    } else {
        str = csv_get_line(gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton)), appGUI);
    }
    if (str != NULL) {
        i = 1;

        while ((field_str = csv_get_field(str, i)) != NULL) {
            gtk_label_set_text (GTK_LABEL(appGUI->cnt->value_labels[i-1]), field_str);
            g_free(field_str);
            i++;
        }

        g_free(str);
    }
}

/*------------------------------------------------------------------------------*/

void
type_changed_cb (GtkComboBox *widget, gpointer user_data) {

gint i, k, n;

    MESSAGE *msg = (MESSAGE *)user_data;

    n = (gint)msg->data;

    k = gtk_combo_box_get_active (GTK_COMBO_BOX(msg->appGUI->cnt->field_type_comboboxes[n]));
    msg->appGUI->cnt->field_type[n] = k;

    for(i=0; i < msg->appGUI->cnt->max_fields; i++) {
        if (i != n) {
            if (k == gtk_combo_box_get_active (GTK_COMBO_BOX(msg->appGUI->cnt->field_type_comboboxes[i]))) {
                gtk_combo_box_set_active (GTK_COMBO_BOX (msg->appGUI->cnt->field_type_comboboxes[i]), 0);
                msg->appGUI->cnt->field_type[i] = 0;
            }
        }
    }
}

/*------------------------------------------------------------------------------*/

void
start_import_cb (GtkWidget *widget, gpointer data) {

gint i, j, k, n, m, p, w;
guint n_records;
gchar *str, *field_str, *item, *text;
gchar ch[2] = { 0, 0 };
GtkTreeIter iter, g_iter;
gboolean g_flag;
gchar tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)data;

    n_records = get_number_of_records (appGUI);

    for(i = k = 0; i < appGUI->cnt->max_fields; i++) {
        k += appGUI->cnt->field_type[i];
    }
    if (!k) {
        utl_gui_create_dialog(GTK_MESSAGE_WARNING, _("Nothing to import."), GTK_WINDOW(appGUI->cnt->import_window));
        return;
    }

    /* add records */

    n = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->cnt->first_row_as_header_check_button)) == TRUE) ? 1:0;

    for(j=n; j < n_records; j++) {

        str = csv_get_line(j+1, appGUI);
        if (str != NULL) {
            gtk_list_store_append(appGUI->cnt->contacts_list_store, &iter);

            for(i=0; i < appGUI->cnt->max_fields; i++) {
                if (appGUI->cnt->field_type[i]) {
                    if ((field_str = csv_get_field(str, i+1)) != NULL) {

                        if (g_utf8_validate(field_str, -1, NULL) == FALSE) {
                            text = g_convert_with_fallback(field_str, -1, "utf-8", "iso-8859-1", 
                                                           "?", NULL, NULL, NULL);
                            g_free(field_str);
                            field_str = text;
                        }

                        p = strlen(field_str);

						for (w = 0; w < p; w++) {      /* remove control chars */
							ch[0] = field_str[w];
							if (g_unichar_iscntrl(g_utf8_get_char(ch))) field_str[w] = ' ';
						}

                        if (appGUI->cnt->field_type[i]-1 == COLUMN_FIRST_NAME){
                            if (p) {
                                gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, COLUMN_FIRST_NAME, field_str, -1);
                            }
                        } else if (appGUI->cnt->field_type[i]-1 == COLUMN_LAST_NAME) {
                            if (p) {
                                gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, COLUMN_LAST_NAME, field_str, -1);
                            }
                        } else if (appGUI->cnt->field_type[i]-1 == COLUMN_GROUP) {
                            /* set group */
                            if(p) {
                                m = 0;
                                g_flag = TRUE;
                                while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->contacts_group_store), &g_iter, NULL, m++)) {
                                    gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->contacts_group_store), &g_iter, 0, &item, -1);
                                    if (!strcmp(field_str, item)) {
                                        g_free(item);
                                        g_flag = FALSE;
                                        break;
                                    }
                                    g_free(item);
                                }
                                if (g_flag == TRUE) {
                                    gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->contacts_group_store), &g_iter, NULL, 0);
                                    gtk_list_store_append(appGUI->opt->contacts_group_store, &g_iter);
                                    gtk_list_store_set(appGUI->opt->contacts_group_store, &g_iter, 0, field_str, -1);
                                }
                                gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, appGUI->cnt->field_type[i]-1, field_str, -1);
                            }
                        } else if (appGUI->cnt->field_type[i]-1 == COLUMN_BIRTH_DAY_DATE) {
                            /* convert birth day date field */
                            if (p) {
                                gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter,
                                                   appGUI->cnt->field_type[i]-1, str_to_julian(field_str, DATE_FULL), -1);
                            }
                        } else if (appGUI->cnt->field_type[i]-1 == COLUMN_NAME_DAY_DATE) {
                            /* convert name day date field */
                            if (p) {
                                gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter,
                                                   appGUI->cnt->field_type[i]-1, str_to_julian(field_str, DATE_NAME_DAY), -1);
                            }
                        } else if (p) {
                            /* remaining fields */
                            gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, appGUI->cnt->field_type[i]-1, field_str, -1);
                        }

                        g_free(field_str);
                    }
                } 

                gtk_tree_model_get(GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, 
                                   COLUMN_FIRST_NAME, &item, -1);
                if (item == NULL) {
                    gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, 
                                       COLUMN_FIRST_NAME, _("None"), -1);
                }
                g_free(item);
                gtk_tree_model_get(GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, 
                                   COLUMN_LAST_NAME, &item, -1);
                if (item == NULL) {
                    gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, 
                                       COLUMN_LAST_NAME, _("None"), -1);
                }
                g_free(item);
                gtk_tree_model_get(GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, 
                                   COLUMN_GROUP, &item, -1);
                if (item == NULL) {
                    gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter, 
                                       COLUMN_GROUP, _("None"), -1);
                }
                g_free(item);
            }
        }
        g_free(str);
    }

    sprintf (tmpbuf, "%d %s.\n", n_records-n, ngettext ("contact added", "contacts added", n_records-n));
    utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW (appGUI->cnt->import_window));
    import_window_close_cb (NULL, NULL, appGUI);
}

/*------------------------------------------------------------------------------*/

void
first_row_as_header_cb (GtkToggleButton *button, gpointer user_data) {

gint i, j;
guint n_records;
gchar *str, *field_str;
gchar tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)user_data;

    n_records = get_number_of_records (appGUI);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)) == TRUE) {
        if(n_records > 1) {
            --n_records;
        }
    } 

    GTK_ADJUSTMENT(appGUI->cnt->current_record_spinbutton_adj)->upper = n_records;
    gtk_adjustment_set_value (GTK_ADJUSTMENT(appGUI->cnt->current_record_spinbutton_adj), 1.0);
    sprintf(tmpbuf, "%d", n_records);
    gtk_label_set_text (GTK_LABEL(appGUI->cnt->n_records_label), tmpbuf);

    record_changed_cb (GTK_SPIN_BUTTON (appGUI->cnt->current_record_spinbutton), appGUI);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)) == TRUE) {
        str = csv_get_line(1, appGUI);

        if (str != NULL) {

            for(i=0; i < appGUI->cnt->max_fields; i++) {

                if ((field_str = csv_get_field(str, i+1)) != NULL) {

                    for(j=0; j < CONTACTS_NUM_COLUMNS; j++) {
                        if (j != COLUMN_PHOTO) {
                            if (!strcmp(field_str, appGUI->cnt->contact_fields_tags_name[j*2])) {
                                gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->field_type_comboboxes[i]), j+1);
                                appGUI->cnt->field_type[i] = j+1;
                            }
                        }
                    }

                    g_free(field_str);
                }

            }

            g_free(str);
        }
    } else {
        for(i=0; i < appGUI->cnt->max_fields; i++) {
            gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->field_type_comboboxes[i]), 0);
            appGUI->cnt->field_type[i] = 0;
        }
    }
}

/*------------------------------------------------------------------------------*/

gboolean
add_csv_records (gchar *filename, GUI *appGUI) {

GtkWidget *vbox1;
GtkWidget *vbox2;
GtkWidget *hbox1;
GtkWidget *label;
GtkWidget *max_fields_label;
GtkWidget *fields_scrolledwindow;
GtkWidget *fields_table;
GtkWidget *hseparator;
GtkWidget *hbuttonbox;
GtkWidget *cancel_button;
GtkWidget *import_button;

gchar *str, *field_str, *text;
guint record, fields, n_records, i, j;
gchar tmpbuf[BUFFER_SIZE];
static MESSAGE msg_fields[CONTACTS_NUM_COLUMNS];

    if (g_access (filename, R_OK) != -1) {

        appGUI->cnt->file_buffer = NULL;
        appGUI->cnt->file_length = 0;

        if (g_file_get_contents (filename, &appGUI->cnt->file_buffer, &appGUI->cnt->file_length, NULL) == TRUE) {

            /* get maximum number of fields */

            n_records = get_number_of_records (appGUI);

            if (n_records) {

                record = 1;
                appGUI->cnt->max_fields = 0;

                for (record = 1; record <= n_records; record++) {

                    str = csv_get_line(record, appGUI);

                    if (str != NULL) {
                        fields = 0;

                        while ((field_str = csv_get_field(str, fields+1)) != NULL) {
                            g_free(field_str);
                            fields++;
                        }

                        g_free(str);

                        if (fields > appGUI->cnt->max_fields) {
                            appGUI->cnt->max_fields = fields;
                        }
                    }
                }

                if (appGUI->cnt->max_fields > CONTACTS_NUM_COLUMNS) {
                    appGUI->cnt->max_fields = CONTACTS_NUM_COLUMNS;
                }

                /* create gui */

                appGUI->cnt->import_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
                gtk_window_set_title (GTK_WINDOW (appGUI->cnt->import_window), _("Import contacts"));
                gtk_container_set_border_width (GTK_CONTAINER (appGUI->cnt->import_window), 6);

                gtk_window_move (GTK_WINDOW (appGUI->cnt->import_window), 
                                 config.contacts_import_win_x, config.contacts_import_win_y);
                gtk_window_set_default_size (GTK_WINDOW(appGUI->cnt->import_window), 
                                             config.contacts_import_win_w, config.contacts_import_win_h);

                gtk_window_set_transient_for(GTK_WINDOW(appGUI->cnt->import_window), GTK_WINDOW(appGUI->main_window));
                gtk_window_set_modal(GTK_WINDOW(appGUI->cnt->import_window), TRUE);

                g_signal_connect (G_OBJECT (appGUI->cnt->import_window), "key_press_event", 
                                  G_CALLBACK (import_key_press_cb), appGUI);

                g_signal_connect (G_OBJECT (appGUI->cnt->import_window), "delete_event", 
                                  G_CALLBACK(import_window_close_cb), appGUI);

                vbox1 = gtk_vbox_new (FALSE, 0);
                gtk_widget_show (vbox1);
                gtk_container_add (GTK_CONTAINER (appGUI->cnt->import_window), vbox1);

                vbox2 = gtk_vbox_new (FALSE, 0);
                gtk_widget_show (vbox2);
                gtk_box_pack_start (GTK_BOX (vbox1), vbox2, TRUE, TRUE, 0);

                hbox1 = gtk_hbox_new (FALSE, 0);
                gtk_widget_show (hbox1);
                gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 4);

                sprintf(tmpbuf, "%d", n_records);
                appGUI->cnt->n_records_label = gtk_label_new (tmpbuf);
                gtk_widget_show (appGUI->cnt->n_records_label);
                gtk_box_pack_end (GTK_BOX (hbox1), appGUI->cnt->n_records_label, FALSE, FALSE, 0);
                gtk_misc_set_padding (GTK_MISC (appGUI->cnt->n_records_label), 6, 0);

                sprintf(tmpbuf, "<b>%s</b>", _("of"));
                label = gtk_label_new (tmpbuf);
                gtk_widget_show (label);
                gtk_box_pack_end (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
                gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
                gtk_misc_set_padding (GTK_MISC (label), 6, 0);

                appGUI->cnt->current_record_spinbutton_adj = gtk_adjustment_new (1, 1, n_records, 1, 10, 0);
                appGUI->cnt->current_record_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (appGUI->cnt->current_record_spinbutton_adj), 1, 0);
                gtk_widget_show (appGUI->cnt->current_record_spinbutton);
                g_signal_connect (G_OBJECT (appGUI->cnt->current_record_spinbutton), "value-changed", 
                                  G_CALLBACK(record_changed_cb), appGUI);
                gtk_box_pack_end (GTK_BOX (hbox1), appGUI->cnt->current_record_spinbutton, FALSE, FALSE, 0);

                sprintf(tmpbuf, "<b>%s:</b>", _("Record"));
                label = gtk_label_new (tmpbuf);
                gtk_widget_show (label);
                gtk_box_pack_end (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
                gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
                gtk_misc_set_padding (GTK_MISC (label), 6, 0);

                sprintf(tmpbuf, "<b>%s:</b>", _("Number fields per record"));
                label = gtk_label_new (tmpbuf);
                gtk_widget_show (label);
                gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
                gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
                gtk_misc_set_padding (GTK_MISC (label), 6, 0);

                sprintf(tmpbuf, "%d", appGUI->cnt->max_fields);
                max_fields_label = gtk_label_new (tmpbuf);
                gtk_widget_show (max_fields_label);
                gtk_box_pack_start (GTK_BOX (hbox1), max_fields_label, FALSE, FALSE, 0);
                gtk_misc_set_padding (GTK_MISC (max_fields_label), 6, 0);

                hseparator = gtk_hseparator_new ();
                gtk_widget_show (hseparator);
                gtk_box_pack_start (GTK_BOX (vbox2), hseparator, FALSE, TRUE, 8);

                fields_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
                gtk_widget_show (fields_scrolledwindow);
                gtk_box_pack_start (GTK_BOX (vbox2), fields_scrolledwindow, TRUE, TRUE, 0);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (fields_scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

                fields_table = gtk_table_new (appGUI->cnt->max_fields, 4, FALSE);
                gtk_widget_show (fields_table);
                gtk_container_set_border_width (GTK_CONTAINER (fields_table), 4);
                gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (fields_scrolledwindow), fields_table);
                gtk_table_set_row_spacings (GTK_TABLE (fields_table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (fields_table), 4);

                sprintf(tmpbuf, "%s", _("Use first record as header"));
                appGUI->cnt->first_row_as_header_check_button = gtk_check_button_new_with_mnemonic (tmpbuf);
                gtk_widget_show (appGUI->cnt->first_row_as_header_check_button);
                gtk_box_pack_start (GTK_BOX (vbox1), appGUI->cnt->first_row_as_header_check_button, FALSE, TRUE, 4);
                GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->first_row_as_header_check_button, GTK_CAN_FOCUS);
                g_signal_connect (G_OBJECT (appGUI->cnt->first_row_as_header_check_button), "toggled",
                                  G_CALLBACK (first_row_as_header_cb), appGUI);

                hseparator = gtk_hseparator_new ();
                gtk_widget_show (hseparator);
                gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

                hbuttonbox = gtk_hbutton_box_new ();
                gtk_widget_show (hbuttonbox);
                gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, TRUE, 0);
                gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
                gtk_box_set_spacing (GTK_BOX (hbuttonbox), 8);

                if (config.default_stock_icons) {
                    cancel_button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
                } else {
                    cancel_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_CANCEL);
                }
                gtk_widget_show (cancel_button);
                gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
                GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);
                g_signal_connect(cancel_button, "clicked", G_CALLBACK(button_import_window_close_cb), appGUI);

                import_button = gtk_button_new_from_stock (OSMO_STOCK_BUTTON_CONTACTS_IMPORT);
                gtk_widget_show (import_button);
                gtk_container_add (GTK_CONTAINER (hbuttonbox), import_button);
                GTK_WIDGET_SET_FLAGS (import_button, GTK_CAN_DEFAULT);
                g_signal_connect(import_button, "clicked", G_CALLBACK(start_import_cb), appGUI);

                gtk_widget_show(appGUI->cnt->import_window);

                gtk_widget_set_sensitive (fields_table, FALSE);
                gtk_widget_set_sensitive (appGUI->cnt->current_record_spinbutton, FALSE);
                gtk_widget_set_sensitive (appGUI->cnt->first_row_as_header_check_button, FALSE);
                gtk_widget_set_sensitive (cancel_button, FALSE);
                gtk_widget_set_sensitive (import_button, FALSE);

                while (g_main_context_iteration(NULL, FALSE));

                str = csv_get_line(1, appGUI);

                for(i=0; i < appGUI->cnt->max_fields; i++) {

                    sprintf(tmpbuf, "<b>%s:</b>", _("Field type"));
                    label = gtk_label_new (tmpbuf);
                    gtk_widget_show (label);
                    gtk_table_attach (GTK_TABLE (fields_table), label, 0, 1, i, i+1,
                                      (GtkAttachOptions) (GTK_FILL),
                                      (GtkAttachOptions) (0), 0, 0);
                    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
                    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
                    gtk_misc_set_padding (GTK_MISC (label), 8, 0);

                    sprintf(tmpbuf, "<b>%s:</b>", _("Value"));
                    label = gtk_label_new (tmpbuf);
                    gtk_widget_show (label);
                    gtk_table_attach (GTK_TABLE (fields_table), label, 2, 3, i, i+1,
                                      (GtkAttachOptions) (GTK_FILL),
                                      (GtkAttachOptions) (0), 0, 0);
                    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
                    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
                    gtk_misc_set_padding (GTK_MISC (label), 8, 0);

                    appGUI->cnt->value_labels[i] = gtk_label_new ("");
                    gtk_widget_show (appGUI->cnt->value_labels[i]);
                    gtk_table_attach (GTK_TABLE (fields_table), appGUI->cnt->value_labels[i], 3, 4, i, i+1,
                                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                                      (GtkAttachOptions) (0), 0, 0);
                    gtk_widget_set_size_request (appGUI->cnt->value_labels[i], 50, -1);
                    gtk_misc_set_alignment (GTK_MISC (appGUI->cnt->value_labels[i]), 0, 0.5);
                    gtk_misc_set_padding (GTK_MISC (appGUI->cnt->value_labels[i]), 8, 0);

                    appGUI->cnt->field_type_comboboxes[i] = gtk_combo_box_new_text ();
                    gtk_widget_show (appGUI->cnt->field_type_comboboxes[i]);
                    gtk_table_attach (GTK_TABLE (fields_table), appGUI->cnt->field_type_comboboxes[i], 1, 2, i, i+1,
                                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                                      (GtkAttachOptions) (GTK_FILL), 0, 0);
                    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cnt->field_type_comboboxes[i]), 4);
                    sprintf(tmpbuf, "[%s]", _("None"));
                    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->field_type_comboboxes[i]), tmpbuf);

                    for(j = 0; j < CONTACTS_NUM_COLUMNS; j++) {
                        gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->field_type_comboboxes[i]), 
                                                   gettext(appGUI->cnt->contact_fields_tags_name[j*2]));
                    }

                    gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->field_type_comboboxes[i]), 0);
                    appGUI->cnt->field_type[i] = 0;
                    gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (appGUI->cnt->field_type_comboboxes[i]), 4);

                    msg_fields[i].data = (gpointer)i;
                    msg_fields[i].appGUI = appGUI;
                    g_signal_connect (G_OBJECT (appGUI->cnt->field_type_comboboxes[i]), "changed", 
                                      G_CALLBACK(type_changed_cb), &msg_fields[i]);

                    if (str != NULL) {
                        if ((field_str = csv_get_field(str, i+1)) != NULL) {

                            if (g_utf8_validate(field_str, -1, NULL) == FALSE) {
                                text = g_convert_with_fallback(field_str, -1, "utf-8", "iso-8859-1", 
                                                               "?", NULL, NULL, NULL);
                                g_free(field_str);
                                field_str = text;
                            }

                            gtk_label_set_text (GTK_LABEL(appGUI->cnt->value_labels[i]), field_str);
                            g_free(field_str);
                        }
                    }

                    while (g_main_context_iteration(NULL, FALSE));
                }

                if (str != NULL) {
                    g_free(str);
                }

                gtk_widget_set_sensitive (fields_table, TRUE);
                gtk_widget_set_sensitive (appGUI->cnt->current_record_spinbutton, TRUE);
                gtk_widget_set_sensitive (appGUI->cnt->first_row_as_header_check_button, TRUE);
                gtk_widget_set_sensitive (cancel_button, TRUE);
                gtk_widget_set_sensitive (import_button, TRUE);

            } else {
                utl_gui_create_dialog(GTK_MESSAGE_ERROR, _("No records found in selected file."), GTK_WINDOW(appGUI->main_window));
                return FALSE;
            }

        } else {
            utl_gui_create_dialog(GTK_MESSAGE_ERROR, _("Cannot read file."), GTK_WINDOW(appGUI->main_window));
            return FALSE;
        }
    } else {
        utl_gui_create_dialog(GTK_MESSAGE_ERROR, _("Cannot open file."), GTK_WINDOW(appGUI->main_window));
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

#endif  /* CONTACTS_ENABLED */

