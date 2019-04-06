
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

#include "notes_items.h"
#include "i18n.h"
#include "options_prefs.h"
#include "notes.h"
#include "notes_preferences_gui.h"
#include "calendar.h"
#include "calendar_notes.h"
#include "calendar_widget.h"
#include "calendar_utils.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"
#include "stock_icons.h"

#ifdef NOTES_ENABLED

#ifdef HAVE_LIBGRINGOTTS
#include <libgringotts.h>
#endif  /* HAVE_LIBGRINGOTTS */

/*------------------------------------------------------------------------------*/

gboolean
notes_get_active_state (GUI *appGUI) {

gint b, c;

    gint a = strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->note_name_entry)));
    if (appGUI->nte->password_entry != NULL) {
        b = strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->password_entry)));
    } else {
        b = 1;
    }
    if (appGUI->nte->spassword_entry != NULL) {
        c = strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->spassword_entry)));
    } else {
        c = 1;
    }

	if (appGUI->nte->encrypted == TRUE) {
	    return (a && b && c);
	} else {
		return a;
	}
}

/*------------------------------------------------------------------------------*/

void
notes_edit_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    appGUI->nte->password_entry = NULL;
    appGUI->nte->spassword_entry = NULL;

    gtk_widget_destroy(appGUI->nte->edit_entry_window);
}

/*------------------------------------------------------------------------------*/

void
button_notes_edit_close_cb (GtkWidget *widget, gpointer data) {

    notes_edit_close_cb (widget, NULL, data);
}

/*------------------------------------------------------------------------------*/

void
notes_edit_action_cb (GtkWidget *widget, gpointer data) {

GtkTreePath *path, *sort_path, *filter_path;
GtkTreeIter iter;
gchar *category;
gboolean flag, remember_cursor;
gint i, n;

    GUI *appGUI = (GUI *)data;

    gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->nte->notes_list), &sort_path, NULL);

    if (sort_path == NULL) return;
    filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->nte->notes_sort), sort_path);
    gtk_tree_path_free (sort_path);

    if (filter_path == NULL) return;
    path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), filter_path);
    gtk_tree_path_free (filter_path);

    if (path != NULL) {

        n = gtk_combo_box_get_active (GTK_COMBO_BOX(appGUI->nte->category_combobox));

        if (n == 0) { 
            category = g_strdup(_("None"));
        } else {
            i = 0;
            flag = FALSE;
            while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, NULL, i++)) {
                gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, 0, &category, -1);
                if (i == n) {
                    flag = TRUE;
                    break;
                }
                g_free(category);
            }
            if (flag != TRUE) {
                category = g_strdup(_("None"));
            }
        }

		remember_cursor = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->nte->remember_cursor_checkbutton));

        gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, path);
        gtk_list_store_set(appGUI->nte->notes_list_store, &iter, 
                           N_COLUMN_NAME, gtk_entry_get_text(GTK_ENTRY(appGUI->nte->note_name_entry)), 
                           N_COLUMN_CATEGORY, category, N_COLUMN_REMEMBER_EDITOR_LINE, remember_cursor, -1);

        g_free (category);
        gtk_tree_path_free (path);

        notes_edit_close_cb (widget, NULL, data);

        if (config.save_data_after_modification) {
            write_notes_entries (appGUI);
        }
    }

}

/*------------------------------------------------------------------------------*/

gint 
notes_edit_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
            notes_edit_close_cb (NULL, NULL, appGUI);
            return TRUE;
    } else if (event->keyval == GDK_Return) {
            notes_edit_action_cb (NULL, appGUI);
            return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

gint 
notes_add_entry_key_release_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (notes_get_active_state (appGUI)) {
        gtk_widget_set_sensitive(appGUI->nte->add_entry_ok_button, TRUE);
    } else {
        gtk_widget_set_sensitive(appGUI->nte->add_entry_ok_button, FALSE);
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
notes_edit_dialog_show (GtkWidget *list, GtkTreeModel *model, GUI *appGUI)
{
	GtkWidget *vbox1;
	GtkWidget *hseparator;
	GtkWidget *hbuttonbox;
	GtkWidget *cancel_button;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *alignment;

	GtkTreePath *sort_path, *filter_path, *path;
	GtkTreeIter iter;
	gchar *c_category, *category, *name;
	gchar tmpbuf[BUFFER_SIZE];
	gboolean remember_cursor;
	gint i, n;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (list), &sort_path, NULL);
	if (sort_path == NULL) return;

	filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->nte->notes_sort), sort_path);
	gtk_tree_path_free (sort_path);
	if (filter_path == NULL) return;

	path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (appGUI->nte->notes_filter), filter_path);
	gtk_tree_path_free (filter_path);
	if (path == NULL) return;

	appGUI->nte->edit_entry_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (appGUI->nte->edit_entry_window), 4);
	gtk_window_set_title (GTK_WINDOW (appGUI->nte->edit_entry_window), _("Edit entry"));
	gtk_window_set_default_size (GTK_WINDOW (appGUI->nte->edit_entry_window), 400, -1);
	gtk_window_set_transient_for (GTK_WINDOW (appGUI->nte->edit_entry_window), GTK_WINDOW (appGUI->main_window));
	gtk_window_set_position (GTK_WINDOW (appGUI->nte->edit_entry_window), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_modal (GTK_WINDOW (appGUI->nte->edit_entry_window), TRUE);
	g_signal_connect (G_OBJECT (appGUI->nte->edit_entry_window), "delete_event",
	                  G_CALLBACK (notes_edit_close_cb), appGUI);
	g_signal_connect (G_OBJECT (appGUI->nte->edit_entry_window), "key_press_event",
	                  G_CALLBACK (notes_edit_key_press_cb), appGUI);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (appGUI->nte->edit_entry_window), vbox1);

	table = gtk_table_new (1, 2, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox1), table, FALSE, TRUE, 0);

	frame = gtk_frame_new (NULL);
	gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	appGUI->nte->note_name_entry = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->note_name_entry);
	gtk_entry_set_invisible_char (GTK_ENTRY (appGUI->nte->note_name_entry), 8226);
	g_signal_connect (G_OBJECT (appGUI->nte->note_name_entry), "key_release_event",
	                  G_CALLBACK (notes_add_entry_key_release_cb), appGUI);

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->nte->notes_list), &sort_path, NULL);
	filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->nte->notes_sort), sort_path);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->nte->notes_filter), &iter, filter_path);
	gtk_tree_path_free (filter_path);
	gtk_tree_model_get (GTK_TREE_MODEL (appGUI->nte->notes_filter), &iter,
	                    N_COLUMN_NAME, &name,
	                    N_COLUMN_CATEGORY, &c_category,
	                    N_COLUMN_REMEMBER_EDITOR_LINE, &remember_cursor,
	                    -1);

	gtk_entry_set_text (GTK_ENTRY (appGUI->nte->note_name_entry), name);
	g_free (name);

	sprintf (tmpbuf, "<b>%s:</b>", _("Note name"));
	label = gtk_label_new (tmpbuf);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	frame = gtk_frame_new (NULL);
	gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	appGUI->nte->category_combobox = gtk_combo_box_new_text ();
	gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->category_combobox);
	gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->nte->category_combobox), _("None"));

	i = n = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->notes_category_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->notes_category_store), &iter, 0, &category, -1);
		gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->nte->category_combobox), category);
		if (!strcmp (category, c_category)) n = i;
		g_free (category);
	}

	g_free (c_category);

	gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->nte->category_combobox), n);

	sprintf (tmpbuf, "<b>%s:</b>", _("Category"));
	label = gtk_label_new (tmpbuf);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	frame = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	appGUI->nte->remember_cursor_checkbutton = gtk_check_button_new_with_mnemonic (_("Remember cursor position"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->nte->remember_cursor_checkbutton, GTK_CAN_FOCUS);
	gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->remember_cursor_checkbutton);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->nte->remember_cursor_checkbutton), remember_cursor);

	sprintf (tmpbuf, "<b>%s:</b>", _("Options"));
	label = gtk_label_new (tmpbuf);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	hseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
	GTK_WIDGET_UNSET_FLAGS (cancel_button, GTK_CAN_FOCUS);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
	g_signal_connect (G_OBJECT (cancel_button), "clicked",
	                  G_CALLBACK (button_notes_edit_close_cb), appGUI);

	appGUI->nte->add_entry_ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->nte->add_entry_ok_button, GTK_CAN_FOCUS);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->nte->add_entry_ok_button);
	g_signal_connect (G_OBJECT (appGUI->nte->add_entry_ok_button), "clicked",
	                  G_CALLBACK (notes_edit_action_cb), appGUI);

	gtk_widget_show_all (appGUI->nte->edit_entry_window);
}

/*------------------------------------------------------------------------------*/

void
notes_remove_dialog_show (GtkWidget *list, GtkListStore *list_store, GUI *appGUI) {

gint response;
GtkTreePath *path, *filter_path, *sort_path;
GtkTreeIter iter;
gchar *filename;
gchar tmpbuf[BUFFER_SIZE];

    gtk_tree_view_get_cursor (GTK_TREE_VIEW (list), &sort_path, NULL);

    if (sort_path == NULL) return;

    filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->nte->notes_sort), sort_path);
    gtk_tree_path_free (sort_path);

    if (filter_path != NULL) {

        path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), filter_path);
        gtk_tree_path_free (filter_path);

        if (path != NULL) {

            sprintf (tmpbuf, "%s\n%s\n\n%s", _("Selected note will be removed."), 
                     _("No further data recovery will be possible."), _("Are you sure?"));

            response = utl_gui_create_dialog(GTK_MESSAGE_QUESTION, tmpbuf, GTK_WINDOW(appGUI->main_window));

            if (response == GTK_RESPONSE_YES) {   
                gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store), &iter, path);
                gtk_tree_model_get(GTK_TREE_MODEL(list_store), &iter, N_COLUMN_FILENAME, &filename, -1);
                g_unlink (notes_get_full_filename(filename, appGUI));
                g_free (filename);
                gtk_list_store_remove(list_store, &iter);
                gtk_tree_path_free(path);
                update_notes_items(appGUI);
				check_notes_type (appGUI);

                if (config.save_data_after_modification) {
                    write_notes_entries (appGUI);
                }
            }
        }
    }
}

/*------------------------------------------------------------------------------*/

void
notes_add_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

#ifdef HAVE_LIBGRINGOTTS
    appGUI->nte->password_entry = NULL;
    appGUI->nte->spassword_entry = NULL;
#endif  /* HAVE_LIBGRINGOTTS */

    gtk_widget_destroy(appGUI->nte->add_entry_window);
}

/*------------------------------------------------------------------------------*/

void
button_notes_add_close_cb (GtkWidget *widget, gpointer data) {

    notes_add_close_cb (widget, NULL, data);
}

/*------------------------------------------------------------------------------*/

gint 
notes_add_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
            notes_add_close_cb (NULL, NULL, appGUI);
            return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
notes_add_entry_action_cb (GtkWidget *widget, gpointer data) {

gchar *name;
gchar *empty_str, *current_filename;
gchar *category;
guint32 current_date;
GtkTreeIter iter;
gint current_time;
gint i, n;
gboolean flag, remember_cursor;

#ifdef HAVE_LIBGRINGOTTS
gchar *pass = NULL, *spass = NULL;
GRG_CTX context;
GRG_KEY keyholder;
gint ret;
#endif /* HAVE_LIBGRINGOTTS */

    GUI *appGUI = (GUI *)data;

#ifdef HAVE_LIBGRINGOTTS
	if (appGUI->nte->encrypted == TRUE) {
		pass = g_strdup(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->password_entry)));
		spass = g_strdup(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->spassword_entry)));

		if (g_utf8_collate (pass, spass) != 0) {
			utl_gui_create_dialog(GTK_MESSAGE_ERROR, _("Passwords do not match!"), GTK_WINDOW(appGUI->main_window));
			gtk_widget_grab_focus (appGUI->nte->password_entry);
			g_free(pass);
			g_free(spass);
			return;
		}
	}
#endif /* HAVE_LIBGRINGOTTS */

    name = g_strdup(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->note_name_entry)));
    n = gtk_combo_box_get_active (GTK_COMBO_BOX(appGUI->nte->category_combobox));
    empty_str = g_strdup("");
	remember_cursor = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->nte->remember_cursor_checkbutton));

    notes_add_close_cb (NULL, NULL, appGUI);

    current_date = utl_date_get_current_julian ();
    current_time = utl_time_get_current_seconds ();
    current_filename = g_strdup (notes_get_new_filename (appGUI));

#ifdef HAVE_LIBGRINGOTTS
	if (appGUI->nte->encrypted == TRUE) {

		context = grg_context_initialize_defaults ((unsigned char*) "OSM");
		keyholder = grg_key_gen ((unsigned char*) pass, -1);

		grg_ctx_set_crypt_algo (context, get_enc_algorithm_value());
		grg_ctx_set_hash_algo (context, get_enc_hashing_value());
		grg_ctx_set_comp_algo (context, get_comp_algorithm_value());
		grg_ctx_set_comp_ratio (context, get_comp_ratio_value());

		ret = grg_encrypt_file (context, keyholder, 
								(unsigned char*) notes_get_full_filename(current_filename, appGUI), 
								(guchar *)empty_str, -1);

		grg_free (context, empty_str, -1);
		grg_key_free (context, keyholder);
		grg_context_free (context);

	} else {

		g_file_set_contents (notes_get_full_filename(current_filename, appGUI), 
							 (gchar *) empty_str, strlen(empty_str), NULL);
	}
#endif /* HAVE_LIBGRINGOTTS */

        if (n == 0) { 
            category = g_strdup(_("None"));
        } else {
            i = 0;
            flag = FALSE;
            while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, NULL, i++)) {
                gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, 0, &category, -1);
                if (i == n) {
                    flag = TRUE;
                    break;
                }
                g_free(category);
            }
            if (flag != TRUE) {
                category = g_strdup(_("None"));
            }
        }

        gtk_list_store_append(appGUI->nte->notes_list_store, &iter);
        gtk_list_store_set(appGUI->nte->notes_list_store, &iter, 
                           N_COLUMN_NAME, name, 
                           N_COLUMN_CATEGORY, category, 
                           N_COLUMN_LAST_CHANGES_DATE, get_date_time_str (current_date, current_time),
                           N_COLUMN_LAST_CHANGES_DATE_JULIAN, current_date,
                           N_COLUMN_LAST_CHANGES_TIME, current_time,
                           N_COLUMN_CREATE_DATE, get_date_time_str (current_date, current_time),
                           N_COLUMN_CREATE_DATE_JULIAN, current_date,
                           N_COLUMN_CREATE_TIME, current_time,
						   N_COLUMN_REMEMBER_EDITOR_LINE, remember_cursor,
                           N_COLUMN_EDITOR_LINE, 0,
                           N_COLUMN_FILENAME, current_filename, -1);
        g_free (category);

        update_notes_items(appGUI);
		check_notes_type (appGUI);

        if (config.save_data_after_modification) {
            write_notes_entries (appGUI);
        }

    g_free(current_filename);
    g_free(name);

#ifdef HAVE_LIBGRINGOTTS
	if (appGUI->nte->encrypted == TRUE) {
		g_free(pass);
		g_free(spass);
	}
#endif /* HAVE_LIBGRINGOTTS */

}

/*------------------------------------------------------------------------------*/

gint 
notes_add_ok_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Return && notes_get_active_state (appGUI) == TRUE) {
            notes_add_entry_action_cb (NULL, appGUI);
            return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
note_type_cb (GtkWidget *widget, gpointer user_data) {

	GUI *appGUI = (GUI *) user_data;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->nte->note_normal_radiobutton)) == TRUE) {
		appGUI->nte->encrypted = FALSE;
	} else {
		appGUI->nte->encrypted = TRUE;
	}

	gtk_widget_set_sensitive (appGUI->nte->password_entry, appGUI->nte->encrypted);
	gtk_widget_set_sensitive (appGUI->nte->spassword_entry, appGUI->nte->encrypted);

}

/*------------------------------------------------------------------------------*/

void
notes_add_entry (GUI *appGUI) {

GtkWidget *vbox1;
GtkWidget *hseparator;
GtkWidget *hbuttonbox;
GtkWidget *cancel_button;
GtkWidget *label;
GtkWidget *frame;
GtkWidget *table;
GtkWidget *alignment;

#ifdef HAVE_LIBGRINGOTTS
GtkWidget *hbox1;
GtkWidget *note_encrypted_radiobutton;
GSList *note_normal_radiobutton_group = NULL;
#endif  /* HAVE_LIBGRINGOTTS */

gchar tmpbuf[BUFFER_SIZE];
gint i;
gchar *category;
GtkTreeIter iter;

	appGUI->nte->encrypted = FALSE;     /* default mode */

    appGUI->nte->add_entry_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->nte->add_entry_window), 4);
    gtk_window_set_title (GTK_WINDOW (appGUI->nte->add_entry_window), _("Add note"));
    gtk_window_set_default_size (GTK_WINDOW(appGUI->nte->add_entry_window), 450, -1);
    gtk_window_set_transient_for(GTK_WINDOW(appGUI->nte->add_entry_window), GTK_WINDOW(appGUI->main_window));
    gtk_window_set_position(GTK_WINDOW(appGUI->nte->add_entry_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal(GTK_WINDOW(appGUI->nte->add_entry_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->nte->add_entry_window), "delete_event",
                      G_CALLBACK(notes_add_close_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->nte->add_entry_window), "key_press_event",
                      G_CALLBACK (notes_add_key_press_cb), appGUI);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->nte->add_entry_window), vbox1);

    table = gtk_table_new (1, 2, FALSE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox1), table, FALSE, TRUE, 0);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    appGUI->nte->note_name_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->nte->note_name_entry);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->note_name_entry);
    gtk_entry_set_invisible_char (GTK_ENTRY (appGUI->nte->note_name_entry), 8226);
    g_signal_connect (G_OBJECT (appGUI->nte->note_name_entry), "key_release_event",
                      G_CALLBACK (notes_add_entry_key_release_cb), appGUI);

    sprintf (tmpbuf, "<b>%s:</b>", _("Note name"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    appGUI->nte->category_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->nte->category_combobox);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->category_combobox);
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->nte->category_combobox), _("None"));

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, NULL, i++)) {
        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, 0, &category, -1);
        gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->nte->category_combobox), category);
        g_free(category);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX(appGUI->nte->category_combobox), 
                              gtk_combo_box_get_active (GTK_COMBO_BOX (appGUI->nte->cf_combobox)));

    sprintf (tmpbuf, "<b>%s:</b>", _("Category"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

#ifdef HAVE_LIBGRINGOTTS

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (alignment), hbox1);

	appGUI->nte->note_normal_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Plain"));
	gtk_widget_show (appGUI->nte->note_normal_radiobutton);
    g_signal_connect (appGUI->nte->note_normal_radiobutton, "toggled", 
					  G_CALLBACK (note_type_cb), appGUI);
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->note_normal_radiobutton, GTK_CAN_FOCUS);
	gtk_box_pack_start (GTK_BOX (hbox1), appGUI->nte->note_normal_radiobutton, TRUE, TRUE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (appGUI->nte->note_normal_radiobutton), note_normal_radiobutton_group);
	note_normal_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (appGUI->nte->note_normal_radiobutton));

	note_encrypted_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Encrypted"));
	gtk_widget_show (note_encrypted_radiobutton);
    GTK_WIDGET_UNSET_FLAGS(note_encrypted_radiobutton, GTK_CAN_FOCUS);
	gtk_box_pack_start (GTK_BOX (hbox1), note_encrypted_radiobutton, TRUE, TRUE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (note_encrypted_radiobutton), note_normal_radiobutton_group);
	note_normal_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (note_encrypted_radiobutton));

    sprintf (tmpbuf, "<b>%s:</b>", _("Note type"));
	label = gtk_label_new (tmpbuf);
	gtk_widget_show (label);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    appGUI->nte->password_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->nte->password_entry);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->password_entry);
    gtk_entry_set_invisible_char (GTK_ENTRY (appGUI->nte->password_entry), 8226);
    gtk_entry_set_visibility (GTK_ENTRY (appGUI->nte->password_entry), FALSE);
    g_signal_connect (G_OBJECT (appGUI->nte->password_entry), "key_release_event",
                      G_CALLBACK (notes_add_entry_key_release_cb), appGUI);
	gtk_widget_set_sensitive (appGUI->nte->password_entry, FALSE);

    sprintf (tmpbuf, "<b>%s:</b>", _("Enter password"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    appGUI->nte->spassword_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->nte->spassword_entry);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->spassword_entry);
    gtk_entry_set_invisible_char (GTK_ENTRY (appGUI->nte->spassword_entry), 8226);
    gtk_entry_set_visibility (GTK_ENTRY (appGUI->nte->spassword_entry), FALSE);
    g_signal_connect (G_OBJECT (appGUI->nte->spassword_entry), "key_release_event",
                      G_CALLBACK (notes_add_entry_key_release_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->nte->spassword_entry), "key_press_event",
                      G_CALLBACK (notes_add_ok_key_press_cb), appGUI);
	gtk_widget_set_sensitive (appGUI->nte->spassword_entry, FALSE);

    sprintf (tmpbuf, "<b>%s:</b>", _("Re-enter password"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

#endif  /* HAVE_LIBGRINGOTTS */

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

	appGUI->nte->remember_cursor_checkbutton = gtk_check_button_new_with_mnemonic (_("Remember cursor position"));
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->remember_cursor_checkbutton, GTK_CAN_FOCUS);
    gtk_widget_show (appGUI->nte->remember_cursor_checkbutton);
    gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->remember_cursor_checkbutton);

    sprintf (tmpbuf, "<b>%s:</b>", _("Options"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, FALSE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
    GTK_WIDGET_UNSET_FLAGS(cancel_button, GTK_CAN_FOCUS);
    gtk_widget_show (cancel_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
    g_signal_connect (G_OBJECT (cancel_button), "clicked",
                        G_CALLBACK (button_notes_add_close_cb), appGUI);
    
    appGUI->nte->add_entry_ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->add_entry_ok_button, GTK_CAN_FOCUS);
    gtk_widget_show (appGUI->nte->add_entry_ok_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->nte->add_entry_ok_button);
    g_signal_connect (G_OBJECT (appGUI->nte->add_entry_ok_button), "clicked",
                        G_CALLBACK (notes_add_entry_action_cb), appGUI);
    gtk_widget_set_sensitive(appGUI->nte->add_entry_ok_button, FALSE);

    gtk_widget_show (appGUI->nte->add_entry_window);

}

/*------------------------------------------------------------------------------*/

void
notes_enter_password_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    appGUI->nte->password_entry = NULL;
    appGUI->nte->spassword_entry = NULL;

    gtk_widget_destroy(appGUI->nte->enter_password_window);
}

/*------------------------------------------------------------------------------*/

void
button_enter_password_close_cb (GtkWidget *widget, gpointer data) {

    notes_enter_password_close_cb (widget, NULL, data);
}

/*------------------------------------------------------------------------------*/

void
notes_enter_editor_action (GUI *appGUI) {

GtkTextBuffer *buffer;
gchar *txtnote;
gchar *current_filename;
GtkTreeIter iter;
GtkTreeModel *model;

    if (gtk_tree_selection_get_selected (appGUI->nte->notes_list_selection, &model, &iter)) {

        gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, N_COLUMN_FILENAME, &current_filename, -1);

		appGUI->nte->filename = g_strdup (current_filename);

		g_file_get_contents (notes_get_full_filename(current_filename, appGUI), &txtnote, NULL, NULL);

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));
		utl_gui_text_buffer_set_text_with_tags (buffer, (const gchar *)txtnote, TRUE);
		notes_show_selector_editor (EDITOR, appGUI);
		appGUI->nte->buffer_check_modify_enable = TRUE;

		g_free (txtnote);
		g_free (current_filename);
	}

}

/*------------------------------------------------------------------------------*/

#ifdef HAVE_LIBGRINGOTTS

void
notes_enter_password_action_cb (GtkWidget *widget, gpointer data) {

gchar *pass, *current_filename;
GtkTreeIter iter;
GtkTreeModel *model;
GtkTextBuffer *buffer;
unsigned char *txtnote;
long txtlen;
gint ret;

    GUI *appGUI = (GUI *)data;

    pass = g_strdup(gtk_entry_get_text (GTK_ENTRY(appGUI->nte->password_entry)));

    notes_enter_password_close_cb (widget, NULL, data);

    if (gtk_tree_selection_get_selected (appGUI->nte->notes_list_selection, &model, &iter)) {

        gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, N_COLUMN_FILENAME, &current_filename, -1);

        appGUI->nte->filename = g_strdup(notes_get_full_filename(current_filename, appGUI));
        appGUI->nte->context = grg_context_initialize_defaults ((unsigned char*) "OSM");
        appGUI->nte->keyholder = grg_key_gen ((unsigned char*) pass, -1);

        grg_ctx_set_crypt_algo (appGUI->nte->context, get_enc_algorithm_value());
        grg_ctx_set_hash_algo (appGUI->nte->context, get_enc_hashing_value());
        grg_ctx_set_comp_algo (appGUI->nte->context, get_comp_algorithm_value());
        grg_ctx_set_comp_ratio (appGUI->nte->context, get_comp_ratio_value());

        ret = grg_decrypt_file (appGUI->nte->context, appGUI->nte->keyholder, 
                                (unsigned char *) appGUI->nte->filename, &txtnote, &txtlen);

        if (ret != GRG_OK) {
            utl_gui_create_dialog(GTK_MESSAGE_ERROR, _("Incorrect password!"), GTK_WINDOW(appGUI->main_window));
            g_free(pass);
            grg_key_free (appGUI->nte->context, appGUI->nte->keyholder);
            appGUI->nte->keyholder = NULL;
            grg_context_free (appGUI->nte->context);
            appGUI->nte->context = NULL;
            g_free(appGUI->nte->filename);
            appGUI->nte->filename = NULL;
            return;
        } else {
			buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));
            utl_gui_text_buffer_set_text_with_tags (buffer, (const gchar *)txtnote, TRUE);
            notes_show_selector_editor (EDITOR, appGUI);
            appGUI->nte->buffer_check_modify_enable = TRUE;
        }

		g_free (current_filename);
    }

    g_free(pass);
}


/*------------------------------------------------------------------------------*/

gint 
notes_enter_password_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
            notes_enter_password_close_cb (NULL, NULL, appGUI);
            return TRUE;
    } else if (event->keyval == GDK_Return) {
            notes_enter_password_action_cb (NULL, appGUI);
            return TRUE;
    }
    return FALSE;
}

#endif /* HAVE_LIBGRINGOTTS */

/*------------------------------------------------------------------------------*/

void
notes_enter_password (GUI *appGUI) {

gchar tmpbuf[BUFFER_SIZE];

GtkTreeIter iter;
GtkTreeModel *model;
gchar *current_filename;

#ifdef HAVE_LIBGRINGOTTS
	GtkWidget *frame;
	GtkWidget *alignment;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;
	GtkWidget *label;
	GtkWidget *vbox1;
	GtkWidget *hseparator;
	GtkWidget *hbuttonbox;
#endif /* HAVE_LIBGRINGOTTS */

	if (gtk_tree_selection_get_selected (appGUI->nte->notes_list_selection, &model, &iter)) {

        gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, N_COLUMN_FILENAME, &current_filename, -1);

#ifndef HAVE_LIBGRINGOTTS

		if (check_if_encrypted (current_filename, appGUI) == TRUE) {

			g_snprintf (tmpbuf, BUFFER_SIZE, "%s\n\n(%s)", _("Cannot open the note."), _("encryption support is disabled"));
			utl_gui_create_dialog (GTK_MESSAGE_ERROR, tmpbuf, GTK_WINDOW(appGUI->main_window));
			g_free (current_filename);
			return;

		} else {
			appGUI->nte->encrypted = FALSE;
            appGUI->nte->changed = FALSE;
			notes_enter_editor_action (appGUI);
		}

#else

		if (check_if_encrypted (current_filename, appGUI) == TRUE) {

			appGUI->nte->encrypted = TRUE;

			appGUI->nte->enter_password_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			gtk_container_set_border_width (GTK_CONTAINER (appGUI->nte->enter_password_window), 4);
			gtk_window_set_title (GTK_WINDOW (appGUI->nte->enter_password_window), _("Authorization"));
			gtk_window_set_default_size (GTK_WINDOW(appGUI->nte->enter_password_window), 350, -1);
			gtk_window_set_transient_for(GTK_WINDOW(appGUI->nte->enter_password_window), GTK_WINDOW(appGUI->main_window));
			gtk_window_set_position(GTK_WINDOW(appGUI->nte->enter_password_window), GTK_WIN_POS_CENTER_ON_PARENT);
			gtk_window_set_modal(GTK_WINDOW(appGUI->nte->enter_password_window), TRUE);
			g_signal_connect (G_OBJECT (appGUI->nte->enter_password_window), "delete_event",
							  G_CALLBACK(notes_enter_password_close_cb), appGUI);
			g_signal_connect (G_OBJECT (appGUI->nte->enter_password_window), "key_press_event",
							  G_CALLBACK (notes_enter_password_key_press_cb), appGUI);

			vbox1 = gtk_vbox_new (FALSE, 0);
			gtk_widget_show (vbox1);
			gtk_container_add (GTK_CONTAINER (appGUI->nte->enter_password_window), vbox1);

			frame = gtk_frame_new (NULL);
			gtk_widget_show (frame);
			gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
			gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

			alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
			gtk_widget_show (alignment);
			gtk_container_add (GTK_CONTAINER (frame), alignment);
			gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
			gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

			appGUI->nte->password_entry = gtk_entry_new ();
			gtk_widget_show (appGUI->nte->password_entry);
			gtk_container_add (GTK_CONTAINER (alignment), appGUI->nte->password_entry);
			gtk_entry_set_invisible_char (GTK_ENTRY (appGUI->nte->password_entry), 8226);
			gtk_entry_set_visibility (GTK_ENTRY (appGUI->nte->password_entry), FALSE);

			sprintf (tmpbuf, "<b>%s:</b>", _("Enter password"));
			label = gtk_label_new (tmpbuf);
			gtk_widget_show (label);
			gtk_frame_set_label_widget (GTK_FRAME (frame), label);
			gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

			hseparator = gtk_hseparator_new ();
			gtk_widget_show (hseparator);
			gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 4);

			hbuttonbox = gtk_hbutton_box_new ();
			gtk_widget_show (hbuttonbox);
			gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox, FALSE, FALSE, 0);
			gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
			gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

			cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
			GTK_WIDGET_UNSET_FLAGS(cancel_button, GTK_CAN_FOCUS);
			gtk_widget_show (cancel_button);
			gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
			g_signal_connect (G_OBJECT (cancel_button), "clicked",
								G_CALLBACK (button_enter_password_close_cb), appGUI);
			
			ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
			GTK_WIDGET_UNSET_FLAGS(ok_button, GTK_CAN_FOCUS);
			gtk_widget_show (ok_button);
			gtk_container_add (GTK_CONTAINER (hbuttonbox), ok_button);
			g_signal_connect (G_OBJECT (ok_button), "clicked",
								G_CALLBACK (notes_enter_password_action_cb), appGUI);

			gtk_widget_show (appGUI->nte->enter_password_window);

		} else {
			appGUI->nte->encrypted = FALSE;
            appGUI->nte->changed = FALSE;
			notes_enter_editor_action (appGUI);
		}

#endif /* HAVE_LIBGRINGOTTS */

		g_free (current_filename);
	}
}

/*------------------------------------------------------------------------------*/

void
read_notes_entries (GUI *appGUI) {

xmlDocPtr doc;
xmlChar *key;
xmlNodePtr node, cnode, category_node, main_node;
GtkTreeIter iter;
gchar *name, *category, *create_date_str, *fontname;
gchar *notes_dir, *str, *dir, *fullpath;
gint last_changes_time, create_time, editor_line;
guint32 last_changes_date_julian, create_date_julian;
gboolean remember_editor_line, editor_readonly;

    notes_dir = g_strdup(prefs_get_config_filename(NOTES_DIRNAME, appGUI));
    if (g_file_test (notes_dir, G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS) == FALSE) {
        g_mkdir (notes_dir, 0755);
    }
    g_free(notes_dir);

    if (g_file_test (prefs_get_config_filename(NOTES_ENTRIES_FILENAME, appGUI), G_FILE_TEST_IS_REGULAR) == FALSE) 
        return;

    if((doc = xmlParseFile(prefs_get_config_filename(NOTES_ENTRIES_FILENAME, appGUI)))) {

        if(!(node = xmlDocGetRootElement(doc))) {
            xmlFreeDoc(doc);
            return;
        }

        if (xmlStrcmp(node->name, (const xmlChar *) NOTES_NAME)) {
            xmlFreeDoc(doc);
            return;
        }

        main_node = node->xmlChildrenNode;

        while (main_node != NULL) {

            if(!xmlStrcmp(main_node->name, (xmlChar *) NOTES_CATEGORY_ENTRIES_NAME)) {

                /* read note */
                category_node = main_node->xmlChildrenNode;

                while (category_node != NULL) {

                    if ((!xmlStrcmp(category_node->name, (const xmlChar *) "name"))) {
                        key = xmlNodeListGetString(doc, category_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            gtk_list_store_append(appGUI->opt->notes_category_store, &iter);
                            gtk_list_store_set(appGUI->opt->notes_category_store, &iter, 0, (gchar *) key, -1);
                        }
                        xmlFree(key);
                    }

                    category_node = category_node->next;
                }
            }

            /*---------------------------------------------------------------------------------------*/

            if(!xmlStrcmp(main_node->name, (xmlChar *) NOTES_ENTRIES_NAME)) {

                /* read note */
                node = main_node->xmlChildrenNode;

                while (node != NULL) {

                    if(!xmlStrcmp(node->name, (xmlChar *) "entry")) {

                        cnode = node->xmlChildrenNode;

						remember_editor_line = FALSE;
						editor_readonly = FALSE;
                        editor_line = 0;
						fontname = NULL;

                        while (cnode != NULL) {

							utl_xml_get_str ("name", &name, cnode);
							utl_xml_get_str ("category", &category, cnode);
							utl_xml_get_uint ("last_changes_date", &last_changes_date_julian, cnode);
							utl_xml_get_int ("last_changes_time", &last_changes_time, cnode);
							utl_xml_get_uint ("create_date", &create_date_julian, cnode);
							utl_xml_get_int ("create_time", &create_time, cnode);
							utl_xml_get_int ("remember_editor_line", &remember_editor_line, cnode);
							utl_xml_get_int ("editor_line", &editor_line, cnode);
							utl_xml_get_int ("read_only", &editor_readonly, cnode);
							utl_xml_get_str ("fontname", &fontname, cnode);

                            if ((!xmlStrcmp(cnode->name, (const xmlChar *) "filename"))) {
                                key = xmlNodeListGetString(doc, cnode->xmlChildrenNode, 1);
                                if (key != NULL) {

									str = g_path_get_basename ((gchar *) key);
									dir = g_path_get_dirname ((gchar *) key);

									if (str && dir) {

										if (dir[0] == '.') {
											fullpath = g_strdup (notes_get_full_filename (str, appGUI));
										} else {
											fullpath = (gchar *) key;
										}

										gtk_list_store_append (appGUI->nte->notes_list_store, &iter);

										create_date_str = g_strdup(get_date_time_str (create_date_julian, create_time));

										if (fontname == NULL) {
											fontname = g_strdup("Sans 12");
										}

										gtk_list_store_set (appGUI->nte->notes_list_store, &iter, 
															N_COLUMN_NAME, name, 
															N_COLUMN_CATEGORY, category, 
															N_COLUMN_LAST_CHANGES_DATE, get_date_time_str (last_changes_date_julian, last_changes_time),
															N_COLUMN_LAST_CHANGES_DATE_JULIAN, last_changes_date_julian,
															N_COLUMN_LAST_CHANGES_TIME, last_changes_time,
															N_COLUMN_CREATE_DATE, create_date_str,
															N_COLUMN_CREATE_DATE_JULIAN, create_date_julian,
															N_COLUMN_CREATE_TIME, create_time,
															N_COLUMN_REMEMBER_EDITOR_LINE, remember_editor_line,
															N_COLUMN_FONTNAME, fontname,
															N_COLUMN_EDITOR_LINE, editor_line,
															N_COLUMN_EDITOR_READONLY, editor_readonly, -1);

										g_free (create_date_str);

										if (dir[0] == '.') {
											gtk_list_store_set (appGUI->nte->notes_list_store, &iter, 
																N_COLUMN_FILENAME, (gchar *)key, -1);
											g_free (fullpath);
										} else {
											gtk_list_store_set (appGUI->nte->notes_list_store, &iter, 
																N_COLUMN_FILENAME, str, -1);
										}

										g_free (str);
										g_free (dir);
										g_free (name);
										g_free (fontname);
										g_free (category);

										fontname = NULL;
									}
                                    xmlFree (key);
                                }
                            }

                            cnode = cnode->next;
                        }

                    }

                    node = node->next;
                }

            }

            /*---------------------------------------------------------------------------------------*/

            main_node = main_node->next;
        }

        xmlFree(node);
        xmlFreeDoc(doc);

        notes_cleanup_files (appGUI);
		check_notes_type (appGUI);
    }

}

/*------------------------------------------------------------------------------*/

void
write_notes_entries (GUI *appGUI) {

gint i;
xmlDocPtr doc;
xmlNodePtr main_node, node, note_node;
xmlAttrPtr attr;
GtkTreeIter iter;
gchar *category, *name, *note_filename, *note_fontname;
gint last_changes_time, create_time, editor_line;
guint32 last_changes_date_julian, create_date_julian;
gboolean remember_editor_line, editor_readonly;
xmlChar *escaped;

    if ((appGUI->save_status & WRT_NOTES) != 0) return;

    appGUI->save_status |= WRT_NOTES;

    doc = xmlNewDoc((const xmlChar *) "1.0");
    attr = xmlNewDocProp (doc, (const xmlChar *) "encoding", (const xmlChar *) "utf-8");

    main_node = xmlNewNode(NULL, (const xmlChar *) NOTES_NAME);
    xmlDocSetRootElement(doc, main_node);

    node = xmlNewChild(main_node, NULL, (const xmlChar *) NOTES_CATEGORY_ENTRIES_NAME, (xmlChar *) NULL);

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, NULL, i++)) {

        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->notes_category_store), &iter, 0, &category, -1);
		escaped = xmlEncodeEntitiesReentrant(doc, (const xmlChar *) category);
        xmlNewChild(node, NULL, (const xmlChar *) "name", (xmlChar *) escaped);
        g_free(category);
        xmlFree (escaped);
    }

    node = xmlNewChild(main_node, NULL, (const xmlChar *) NOTES_ENTRIES_NAME, (xmlChar *) NULL);

    i = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->nte->notes_list_store), &iter, NULL, i++)) {

		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->nte->notes_list_store), &iter,
		                    N_COLUMN_NAME, &name,
		                    N_COLUMN_CATEGORY, &category,
		                    N_COLUMN_LAST_CHANGES_DATE_JULIAN, &last_changes_date_julian,
		                    N_COLUMN_LAST_CHANGES_TIME, &last_changes_time,
		                    N_COLUMN_CREATE_DATE_JULIAN, &create_date_julian,
		                    N_COLUMN_CREATE_TIME, &create_time,
		                    N_COLUMN_REMEMBER_EDITOR_LINE, &remember_editor_line,
		                    N_COLUMN_EDITOR_LINE, &editor_line,
		                    N_COLUMN_EDITOR_READONLY, &editor_readonly,
		                    N_COLUMN_FONTNAME, &note_fontname,
		                    N_COLUMN_FILENAME, &note_filename, -1);

		note_node = xmlNewChild (node, NULL, (const xmlChar *) "entry", (xmlChar *) NULL);

		utl_xml_put_str ("name", name, note_node, doc);
		g_free (name);
		utl_xml_put_str ("category", category, note_node, doc);
		g_free (category);
		utl_xml_put_uint ("last_changes_date", last_changes_date_julian, note_node);
		utl_xml_put_int ("last_changes_time", last_changes_time, note_node);
		utl_xml_put_uint ("create_date", create_date_julian, note_node);
		utl_xml_put_int ("create_time", create_time, note_node);
		utl_xml_put_int ("remember_editor_line", remember_editor_line, note_node);
		utl_xml_put_int ("editor_line", editor_line, note_node);
		utl_xml_put_int ("read_only", editor_readonly, note_node);
		utl_xml_put_str ("fontname", note_fontname, note_node, doc);
		utl_xml_put_str ("filename", note_filename, note_node, doc);
		g_free (note_filename);
	}

	xmlSaveFormatFileEnc (prefs_get_config_filename (NOTES_ENTRIES_FILENAME, appGUI), doc, "utf-8", 1);
	xmlFreeProp (attr);
	xmlFreeDoc (doc);

	appGUI->save_status &= ~WRT_NOTES;
}

/*------------------------------------------------------------------------------*/

gchar *
notes_get_new_filename (GUI *appGUI) {

guint i;
static gchar fullpath[PATH_MAX];

    for(i=0; i < 99999999; i++) {
        g_snprintf(fullpath, PATH_MAX-1, "%s%c%08d.osm", 
                   prefs_get_config_filename(NOTES_DIRNAME, appGUI), G_DIR_SEPARATOR, i);
        if (g_file_test (fullpath, G_FILE_TEST_EXISTS) == FALSE) break;
    }

	g_snprintf(fullpath, PATH_MAX-1, "%08d.osm", i);

    return fullpath;
}

/*------------------------------------------------------------------------------*/

void
notes_cleanup_files (GUI *appGUI) {

gchar fullpath[PATH_MAX];
GDir *dpath;
gboolean found;
gint i;
const gchar *item_name;
gchar *note_filename;
GtkTreeIter iter;

    g_snprintf(fullpath, PATH_MAX-1, "%s", prefs_get_config_filename(NOTES_DIRNAME, appGUI));
    dpath = g_dir_open (fullpath, 0, NULL);

    if (dpath != NULL) {

        while ((item_name = g_dir_read_name (dpath)) != NULL) {

            found = FALSE;

            g_snprintf(fullpath, PATH_MAX-1, "%s%c%s", 
                       prefs_get_config_filename(NOTES_DIRNAME, appGUI), G_DIR_SEPARATOR, item_name);

            i = 0;

            while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, NULL, i++)) {

                gtk_tree_model_get(GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, 
                                   N_COLUMN_FILENAME, &note_filename, -1);
                
                if (!strncmp(notes_get_full_filename(note_filename, appGUI), fullpath, PATH_MAX)) {
                    found = TRUE;
                    g_free(note_filename);
                    break;
                }

                g_free(note_filename);
            }

            if (found == FALSE) {
                g_unlink (fullpath);
            }

        }

        g_dir_close (dpath);
    }

}

/*------------------------------------------------------------------------------*/

gchar *
notes_get_full_filename (gchar *filename, GUI *appGUI) {

static gchar fullname[PATH_MAX];
gchar *str;

    g_snprintf (fullname, PATH_MAX-1, "%s%c%s", 
				prefs_get_config_filename(NOTES_DIRNAME, appGUI), G_DIR_SEPARATOR, filename);

	if (g_file_test (fullname, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS) == FALSE) {

		str = g_path_get_basename (fullname);

		if (str) {
			g_snprintf (fullname, PATH_MAX-1, "%s%c%s", 
						prefs_get_config_filename(NOTES_DIRNAME, appGUI), G_DIR_SEPARATOR, str);
			g_free (str);
		}
	}

	return fullname;
}

/*------------------------------------------------------------------------------*/

#endif  /* NOTES_ENABLED */

