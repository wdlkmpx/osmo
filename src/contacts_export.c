
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

#include "contacts_export.h"
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
export_check (GUI *appGUI) {

gint i, k;
const gchar *text;

    if (appGUI->cnt->output_file_entry != NULL && appGUI->cnt->export_button != NULL) {
        k = 0;

        for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {
            if(config.export_fields[i] == '+') k++;
        }

        text = gtk_entry_get_text (GTK_ENTRY(appGUI->cnt->output_file_entry));

        if (text == NULL) {
            gtk_widget_set_sensitive(appGUI->cnt->export_button, FALSE);
        } else if (strlen(text) && k) {
            gtk_widget_set_sensitive(appGUI->cnt->export_button, TRUE);
        } else {
            gtk_widget_set_sensitive(appGUI->cnt->export_button, FALSE);
        }
    }
}

/*-------------------------------------------------------------------------------------*/

void
export_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    gtk_window_get_size (GTK_WINDOW(appGUI->cnt->export_window),
                        &config.contacts_export_win_w, &config.contacts_export_win_h);
    gdk_window_get_root_origin (GDK_WINDOW(appGUI->cnt->export_window->window),
                                &config.contacts_export_win_x, &config.contacts_export_win_y);

    gtk_widget_destroy(appGUI->cnt->export_window);
    appGUI->cnt->output_file_entry = NULL;
    appGUI->cnt->export_button = NULL;
}

/*-------------------------------------------------------------------------------------*/

void
export_cb (GtkWidget *widget, gpointer data) {

gint n;
gboolean header;
gchar tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)data;

    if (strlen(gtk_entry_get_text (GTK_ENTRY(appGUI->cnt->output_file_entry)))) {

        header = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->cnt->first_row_header_check_button));

        n = export_contacts_to_file((gchar *) gtk_entry_get_text (GTK_ENTRY(appGUI->cnt->output_file_entry)), header, appGUI);

        if (n != -1) {

            sprintf(tmpbuf, "%s %d %s.\n", _("Done!"), n,
                ngettext ("contact exported", "contacts exported", n));

            utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW(appGUI->cnt->export_window));
            export_window_close_cb (widget, NULL, appGUI);

        }
    }
}

/*------------------------------------------------------------------------------*/

void
button_export_window_close_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    export_window_close_cb (widget, NULL, appGUI);

}

/*------------------------------------------------------------------------------*/

gint
export_contacts_to_file(gchar *filename, gboolean header, GUI *appGUI) {

GtkTreePath *sort_path, *filter_path, *path;
gint i, j, a, b, c, e, n, m, max_field, exported;
gchar *text;
GtkTreeIter iter;
FILE *filehandle;
guint32 date;
gchar tmpbuf[BUFFER_SIZE];
gchar tmp_buffer_1[BUFFER_SIZE], tmp_buffer_2[BUFFER_SIZE];

    exported = 0;

    if (utl_gui_check_overwrite_file (filename, appGUI->cnt->export_window, appGUI) != 0) {
        return -1;
    }

    for(max_field = CONTACTS_NUM_COLUMNS-1; max_field >= 0; --max_field) {
        if(config.export_fields[max_field] == '+') break;
    }
    if (max_field == -1) {
        max_field = CONTACTS_NUM_COLUMNS-1;
    }

    filehandle = g_fopen (filename, "w");
    if(filehandle) {

        if (config.export_format == EXPORT_TO_XHTML) {
            fprintf(filehandle, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
            fprintf(filehandle, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
            fprintf(filehandle, "<head>\n");
            fprintf(filehandle, "\t<title>Contact List</title>\n");
            fprintf(filehandle, "\t<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n");
            fprintf(filehandle, "\t<meta name=\"generator\" content=\"OSMO - http://clay.ll.pl/osmo\" />\n");
            fprintf(filehandle, "\t<style type=\"text/css\">\n");
            fprintf(filehandle, "\t\tbody { color:black; background-color:white; }\n");
            fprintf(filehandle, "\t\ta { color:#0000ff; }\n");
            fprintf(filehandle, "\t\ta:visited { color:#000055; }\n");
            fprintf(filehandle, "\t\ttable { border-collapse:collapse; }\n");
            fprintf(filehandle, "\t\ttr.header { background-color:#c0c0c0; }\n");
            fprintf(filehandle, "\t\ttr.evenrow { background-color:#f0f0f0; }\n");
            fprintf(filehandle, "\t\ttr.oddrow { background-color:#fafafa; }\n");
            fprintf(filehandle, "\t\tth, td { border:1px solid #555555; padding:3px; }\n");
            fprintf(filehandle, "\t</style>\n");
            fprintf(filehandle, "</head>\n\n");
            fprintf(filehandle, "<body>\n\n");
            fprintf(filehandle, "<h1>Contact List</h1>\n\n");

            fprintf(filehandle, "<table>\n");

            fprintf(filehandle, "<tr class=\"header\">\n");

            for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {
                if(config.export_fields[i] == '+') {
                    fprintf(filehandle, "\t<th>");

                    if (appGUI->cnt->contact_fields_tags_name[i*2] != NULL) {
                        for(a=b=0; a < strlen(appGUI->cnt->contact_fields_tags_name[i*2]); a++) {
                            if(appGUI->cnt->contact_fields_tags_name[i*2][a] == ' ') {
                                tmpbuf[b] = '\0';
                                strcat(tmpbuf, "&nbsp;");
                                b += 6;
                            } else {
                                tmpbuf[b++] = appGUI->cnt->contact_fields_tags_name[i*2][a];
                            }
                        }
                        tmpbuf[b] = '\0';
                        fprintf(filehandle, "%s", tmpbuf);
                    }

                    fprintf(filehandle, "</th>\n");
                }
            }

            fprintf(filehandle, "</tr>\n\n");
        }

        j = 0;
        sort_path = gtk_tree_path_new_first ();

        while (gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, sort_path) == TRUE) {

            if (sort_path != NULL) {

                filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->cnt->contacts_sort), sort_path);

                if (filter_path != NULL) {

                    path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->cnt->contacts_filter), filter_path);

                    if (path != NULL) {

                        gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, path);

                        if (config.export_format == EXPORT_TO_XHTML) {
                            if(j & 1) {
                                fprintf(filehandle, "<tr class=\"evenrow\">\n");
                            } else {
                                fprintf(filehandle, "<tr class=\"oddrow\">\n");
                            }
                        } else if (config.export_format == EXPORT_TO_CSV && header == TRUE && j == 0) {

                            for(i=n=0; i < CONTACTS_NUM_COLUMNS; i++) {
                                if (i != COLUMN_PHOTO && i != COLUMN_ID && config.export_fields[i] == '+') {
                                    n++;
                                }
                            }
                            for(i=m=0; i < CONTACTS_NUM_COLUMNS; i++) {
                                if (i != COLUMN_PHOTO && i != COLUMN_ID && config.export_fields[i] == '+') {
                                    fprintf(filehandle, "%s", appGUI->cnt->contact_fields_tags_name[i*2]);
                                    if (m != n-1) {
                                        fprintf(filehandle, ",");
                                    }
                                    m++;
                                }
                            }
                            fprintf(filehandle, "\n");
                        }

                        for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {

                            if(config.export_fields[i] == '+') {

                                if (i == COLUMN_BIRTH_DAY_DATE || i == COLUMN_NAME_DAY_DATE) {
                                    gtk_tree_model_get (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, i, &date, -1);
                                    if (date == 0) {
                                        text = NULL;
                                    } else {
                                        if (i == COLUMN_BIRTH_DAY_DATE) {
                                            text = g_strdup((const gchar *)julian_to_str(date, DATE_FULL, config.override_locale_settings));
                                        } else {
                                            text = g_strdup((const gchar *)julian_to_str(date, DATE_NAME_DAY, config.override_locale_settings));
                                        }
                                    }
                                } else {
                                    gtk_tree_model_get (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter, i, &text, -1);
                                }

                                if (config.export_format == EXPORT_TO_XHTML) {
                                    fprintf(filehandle, "\t<td>");

                                    if (text != NULL) {

                                        for(a=b=0; a < strlen(text); a++) {
                                            if(text[a] == ' ') {
                                                tmpbuf[b] = '\0';
                                                strcat(tmpbuf, "&nbsp;");
                                                b += 6;
                                            } else {
                                                tmpbuf[b++] = text[a];
                                            }
                                        }
                                        tmpbuf[b] = '\0';

                                        switch (i) {
                                            case COLUMN_EMAIL_1:
                                            case COLUMN_EMAIL_2:
                                            case COLUMN_EMAIL_3:
                                            case COLUMN_EMAIL_4:
                                                fprintf(filehandle, "<a href=\"mailto:%s\">%s</a>", tmpbuf, tmpbuf);
                                                break;
                                            case COLUMN_WWW_1:
                                            case COLUMN_WWW_2:
                                            case COLUMN_WWW_3:
                                            case COLUMN_WWW_4:
                                            case COLUMN_BLOG:
                                                fprintf(filehandle, "<a href=\"%s\">%s</a>", tmpbuf, tmpbuf);
                                                break;
                                            default:
                                                fprintf(filehandle, "%s", tmpbuf);
                                        }
                                        g_free(text);
                                    }

                                    fprintf(filehandle, "</td>\n");

                                } else {
                                    /* CSV */

                                    if (text != NULL) {

                                        /* 1 */

                                        for(a = b = e = 0; a < strlen(text); a++) {
                                            if(text[a]=='"') {
                                                b = 1;
                                            }
                                            if(text[a]=='\n') {
                                                e = 1;              /* found new line */
                                            }
                                        }

                                        c = a = 0;

                                        if (b) {
                                            tmp_buffer_1[c++] = '"';
                                        }

                                        do {
                                            if(text[a]=='"') {
                                                tmp_buffer_1[c++] = '"';
                                            }
                                            tmp_buffer_1[c++] = text[a];
                                            a++;
                                        } while (text[a]!='\0');

                                        if(b) {
                                            tmp_buffer_1[c++] = '"';
                                        }
                                        tmp_buffer_1[c] = '\0';

                                        /* 2 */

                                        for(a = b = 0; a < strlen(tmp_buffer_1); a++) {
                                            if(tmp_buffer_1[a]==',') {
                                                b = 1;
                                            }
                                        }

                                        c = a = 0;

                                        if (b) {
                                            tmp_buffer_2[c++] = '"';
                                        }

                                        do {
                                            tmp_buffer_2[c++] = tmp_buffer_1[a];
                                            a++;
                                        } while (tmp_buffer_1[a]!='\0');

                                        if (b) {
                                            tmp_buffer_2[c++] = '"';
                                        }
                                        tmp_buffer_2[c] = '\0';

                                        if (e) {
                                            fprintf(filehandle, "\"%s\"", tmp_buffer_2);
                                        } else {
                                            fprintf(filehandle, "%s", tmp_buffer_2);
                                        }

                                        g_free(text);
                                    }

                                    if (i != max_field) {
                                        fprintf(filehandle, ",");
                                    }

                                }

                            }
                        }

                        if (config.export_format == EXPORT_TO_XHTML) {
                            fprintf(filehandle, "</tr>\n\n");
                        } else {
                            /* CSV */
                            fprintf(filehandle, "\n");
                        }

                        j++;
                        exported++;
                        gtk_tree_path_free(path);

                    }

                    gtk_tree_path_free(filter_path);
                }

            }

            gtk_tree_path_next (sort_path);
        }

        if (config.export_format == EXPORT_TO_XHTML) {
            fprintf(filehandle, "</table>\n");

            fprintf(filehandle, "</body>\n");
            fprintf(filehandle, "</html>\n");
        }

        fclose(filehandle);

    } else {

        utl_gui_create_dialog(GTK_MESSAGE_ERROR, _("Cannot create file."), GTK_WINDOW(appGUI->cnt->export_window));
        return -1;
    }

    return exported;
}

/*------------------------------------------------------------------------------*/

void
browse_dir_cb (GtkWidget *widget, gpointer data) {

GtkWidget *dialog;
gchar f_filename[PATH_MAX];

        GUI *appGUI = (GUI *)data;

        dialog = gtk_file_chooser_dialog_new(_("Select output file"),
                                             GTK_WINDOW(appGUI->main_window),
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                             NULL);

        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

            g_strlcpy (f_filename, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)), PATH_MAX-1);

            if (config.export_format == EXPORT_TO_XHTML) {
                if (g_str_has_suffix (f_filename, ".html") == FALSE &&
                    g_str_has_suffix (f_filename, ".HTML") == FALSE) {
                    g_strlcat(f_filename, ".html", PATH_MAX-1);
                }
            } else {
                if (g_str_has_suffix (f_filename, ".csv") == FALSE &&
                    g_str_has_suffix (f_filename, ".CSV") == FALSE) {
                    g_strlcat(f_filename, ".csv", PATH_MAX-1);
                }
            }
            gtk_entry_set_text (GTK_ENTRY(appGUI->cnt->output_file_entry), f_filename);
        }

        export_check(appGUI);
        gtk_widget_destroy(dialog);
}


/*------------------------------------------------------------------------------*/

gint
export_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    switch(event->keyval) {
        case GDK_Escape:
            export_window_close_cb (widget, NULL, appGUI);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
export_field_selected_cb (GtkToggleButton *button, gpointer user_data) {

gint i;

    MESSAGE *msg = (MESSAGE *)user_data;

    i = (int) msg->data;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)) == TRUE) {
        config.export_fields[i] = '+';
    } else {
        config.export_fields[i] = '-';
    }

    export_check(msg->appGUI);
}

/*------------------------------------------------------------------------------*/

void
format_changed_cb (GtkButton *button, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->cnt->format_csv_radiobutton)) == TRUE) {
        config.export_format = EXPORT_TO_CSV;
    } else {
        config.export_format = EXPORT_TO_XHTML;
    }
}

/*------------------------------------------------------------------------------*/

void
select_action_cb (GtkWidget *widget, gpointer user_data) {

    gint i;
    gboolean state;

    MESSAGE *msg = (MESSAGE *)user_data;

    for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {

        if (i != COLUMN_PHOTO && i != COLUMN_ID) {
                switch((gint) msg->data) {

                    case SELECT_ALL:
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(msg->appGUI->cnt->check_buttons[i]), TRUE);
                        break;

                    case SELECT_NONE:
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(msg->appGUI->cnt->check_buttons[i]), FALSE);
                        break;

                    case SELECT_INVERT:
                        state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(msg->appGUI->cnt->check_buttons[i]));
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(msg->appGUI->cnt->check_buttons[i]), !state);
                        break;

                    default:
                        break;
                };
        }
    }
}

/*--------------------------------------------------------------------*/

void
contacts_create_export_window(GUI *appGUI) {

GtkWidget *vbox1;
GtkWidget *hbox1;
GtkWidget *vbox2;
GtkWidget *hbox2;
GtkWidget *vbox3;
GtkWidget *frame;
GtkWidget *alignment;
GtkWidget *vbox4;
GtkWidget *vbox5;
GtkWidget *format_xhtml_radiobutton;
GSList    *format_radiobutton_group = NULL;
GtkWidget *label;
GtkWidget *vseparator;
GtkWidget *scrolledwindow;
GtkWidget *viewport;
GtkWidget *fields_table;
GtkWidget *hbox3;
GtkWidget *browse_dir_button;
GtkWidget *hseparator;
GtkWidget *hbuttonbox;
GtkWidget *hbuttonbox_s;
GtkWidget *cancel_button;
GtkWidget *select_all_button;
GtkWidget *select_none_button;
GtkWidget *invert_selection_button;
gint i;
gchar tmpbuf[BUFFER_SIZE];
static MESSAGE msg_export_field[CONTACTS_NUM_COLUMNS];
static MESSAGE msg_select[3]; /* select all, select none, select invert */

    appGUI->cnt->export_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (appGUI->cnt->export_window), _("Export contacts"));
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cnt->export_window), 6);

    gtk_window_move (GTK_WINDOW (appGUI->cnt->export_window), 
                     config.contacts_export_win_x, config.contacts_export_win_y);
    gtk_window_set_default_size (GTK_WINDOW(appGUI->cnt->export_window), 
                                 config.contacts_export_win_w, config.contacts_export_win_h);

    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cnt->export_window), GTK_WINDOW(appGUI->main_window));
    gtk_window_set_modal(GTK_WINDOW(appGUI->cnt->export_window), TRUE);

    g_signal_connect (G_OBJECT (appGUI->cnt->export_window), "key_press_event", 
                      G_CALLBACK (export_key_press_cb), appGUI);

    g_signal_connect (G_OBJECT (appGUI->cnt->export_window), "delete_event", 
                      G_CALLBACK(export_window_close_cb), appGUI);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cnt->export_window), vbox1);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox2);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox2, TRUE, TRUE, 0);

    vbox3 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox3);
    gtk_box_pack_start (GTK_BOX (hbox2), vbox3, FALSE, TRUE, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox3), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    vbox4 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox4);
    gtk_container_add (GTK_CONTAINER (alignment), vbox4);
    gtk_container_set_border_width (GTK_CONTAINER (vbox4), 8);

    appGUI->cnt->format_csv_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, "CSV");
    gtk_widget_show (appGUI->cnt->format_csv_radiobutton);
    g_signal_connect (G_OBJECT (appGUI->cnt->format_csv_radiobutton), "clicked",
                      G_CALLBACK(format_changed_cb), appGUI);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->format_csv_radiobutton, GTK_CAN_FOCUS);
    gtk_box_pack_start (GTK_BOX (vbox4), appGUI->cnt->format_csv_radiobutton, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (appGUI->cnt->format_csv_radiobutton), format_radiobutton_group);
    format_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (appGUI->cnt->format_csv_radiobutton));

    format_xhtml_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, "XHTML");
    gtk_widget_show (format_xhtml_radiobutton);
    g_signal_connect (G_OBJECT (format_xhtml_radiobutton), "clicked",
                      G_CALLBACK(format_changed_cb), appGUI);
    GTK_WIDGET_UNSET_FLAGS(format_xhtml_radiobutton, GTK_CAN_FOCUS);
    gtk_box_pack_start (GTK_BOX (vbox4), format_xhtml_radiobutton, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (format_xhtml_radiobutton), format_radiobutton_group);
    format_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (format_xhtml_radiobutton));

    if (config.export_format) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(format_xhtml_radiobutton), TRUE);
    }

    sprintf(tmpbuf, "<b>%s:</b>", _("Output format"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox3), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    vbox4 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox4);
    gtk_container_add (GTK_CONTAINER (alignment), vbox4);
    gtk_container_set_border_width (GTK_CONTAINER (vbox4), 8);

    sprintf(tmpbuf, "<b>%s:</b>", _("Options"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    sprintf(tmpbuf, "%s", _("Add header"));
    appGUI->cnt->first_row_header_check_button = gtk_check_button_new_with_mnemonic (tmpbuf);
    gtk_widget_show (appGUI->cnt->first_row_header_check_button);
    gtk_box_pack_start (GTK_BOX (vbox4), appGUI->cnt->first_row_header_check_button, TRUE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->first_row_header_check_button, GTK_CAN_FOCUS);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(appGUI->cnt->first_row_header_check_button), TRUE);

    vseparator = gtk_vseparator_new ();
    gtk_widget_show (vseparator);
    gtk_box_pack_start (GTK_BOX (hbox2), vseparator, FALSE, TRUE, 4);

    vbox5 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox5);
    gtk_box_pack_start (GTK_BOX (hbox2), vbox5, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox4), 8);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox5), frame, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_container_add (GTK_CONTAINER (alignment), scrolledwindow);
    gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 8);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (viewport);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), viewport);

    fields_table = gtk_table_new (CONTACTS_NUM_COLUMNS, 1, FALSE);
    gtk_widget_show (fields_table);
    gtk_container_add (GTK_CONTAINER (viewport), fields_table);

    sprintf(tmpbuf, "<b>%s:</b>", _("Fields to export"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox5), frame, FALSE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    hbuttonbox_s = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox_s);
    gtk_box_pack_start (GTK_BOX (vbox5), hbuttonbox_s, FALSE, TRUE, 0);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox_s), 4);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox_s), GTK_BUTTONBOX_END);

    select_all_button = gtk_button_new_with_label (_("All"));
    GTK_WIDGET_UNSET_FLAGS(select_all_button, GTK_CAN_FOCUS);
    gtk_widget_show (select_all_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox_s), select_all_button);
    msg_select[0].appGUI = appGUI;
    msg_select[0].data = (gpointer) SELECT_ALL;
    g_signal_connect (G_OBJECT (select_all_button), "clicked",
                        G_CALLBACK (select_action_cb), &msg_select[0]);

    select_none_button = gtk_button_new_with_label (_("None"));
    GTK_WIDGET_UNSET_FLAGS(select_none_button, GTK_CAN_FOCUS);
    gtk_widget_show (select_none_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox_s), select_none_button);
    msg_select[1].appGUI = appGUI;
    msg_select[1].data = (gpointer) SELECT_NONE;
    g_signal_connect (G_OBJECT (select_none_button), "clicked",
                        G_CALLBACK (select_action_cb), &msg_select[1]);

    invert_selection_button = gtk_button_new_with_label (_("Invert"));
    GTK_WIDGET_UNSET_FLAGS(invert_selection_button, GTK_CAN_FOCUS);
    gtk_widget_show (select_none_button);
    gtk_widget_show (invert_selection_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox_s), invert_selection_button);
    msg_select[2].appGUI = appGUI;
    msg_select[2].data = (gpointer) SELECT_INVERT;
    g_signal_connect (G_OBJECT (invert_selection_button), "clicked",
                        G_CALLBACK (select_action_cb), &msg_select[2]);

    sprintf(tmpbuf, "<b>%s:</b>", _("Select fields"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    hbox3 = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox3);
    gtk_container_add (GTK_CONTAINER (alignment), hbox3);

    appGUI->cnt->output_file_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->cnt->output_file_entry);
    gtk_box_pack_start (GTK_BOX (hbox3), appGUI->cnt->output_file_entry, TRUE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->output_file_entry, GTK_CAN_FOCUS);
    gtk_editable_set_editable (GTK_EDITABLE(appGUI->cnt->output_file_entry), FALSE);

    for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {

        if (i != COLUMN_PHOTO && i != COLUMN_ID) {

            appGUI->cnt->check_buttons[i] = gtk_check_button_new_with_mnemonic (appGUI->cnt->contact_fields_tags_name[i*2]);

            gtk_widget_show (appGUI->cnt->check_buttons[i]);
            msg_export_field[i].appGUI = appGUI;
            msg_export_field[i].data = (gpointer) i;
            g_signal_connect (G_OBJECT (appGUI->cnt->check_buttons[i]), "toggled",
                              G_CALLBACK (export_field_selected_cb), &msg_export_field[i]);
            GTK_WIDGET_UNSET_FLAGS(appGUI->cnt->check_buttons[i], GTK_CAN_FOCUS);
            gtk_table_attach (GTK_TABLE (fields_table), appGUI->cnt->check_buttons[i], 0, 1, i, i+1,
                              (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                              (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

            if(config.export_fields[i] == '+') {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(appGUI->cnt->check_buttons[i]), TRUE);
            }
        }
    }

	browse_dir_button = utl_gui_create_button (GTK_STOCK_OPEN, OSMO_STOCK_BUTTON_OPEN, _("Browse"));
    gtk_widget_show (browse_dir_button);
    GTK_WIDGET_UNSET_FLAGS(browse_dir_button, GTK_CAN_FOCUS);
    g_signal_connect(browse_dir_button, "clicked", G_CALLBACK(browse_dir_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox3), browse_dir_button, FALSE, FALSE, 0);

    sprintf(tmpbuf, "<b>%s:</b>", _("Output filename"));
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
    GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(button_export_window_close_cb), appGUI);

    appGUI->cnt->export_button = utl_gui_create_button (OSMO_STOCK_BUTTON_CONTACTS_EXPORT, OSMO_STOCK_BUTTON_CONTACTS_EXPORT, _("Export"));
    gtk_widget_show (appGUI->cnt->export_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->cnt->export_button);
    GTK_WIDGET_SET_FLAGS (appGUI->cnt->export_button, GTK_CAN_DEFAULT);
    g_signal_connect(appGUI->cnt->export_button, "clicked", G_CALLBACK(export_cb), appGUI);
    gtk_widget_set_sensitive(appGUI->cnt->export_button, FALSE);

    gtk_widget_show(appGUI->cnt->export_window);

}

/*-------------------------------------------------------------------------------------*/

#endif  /* CONTACTS_ENABLED */

