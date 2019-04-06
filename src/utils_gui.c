/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Mąka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Mąka <silloz@users.sourceforge.net>
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

#include "gui.h"
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "utils.h"
#include "utils_gui.h"

static gunichar TAG_CHAR = 0xe000;      /* Unicode chars in the Private Use Area. */

/* ========================================================================= */

GtkWidget *
utl_gui_create_label (const gchar *format, const gchar *name)
{
	gchar *str = g_strdup_printf (format, name);
	GtkWidget *label = gtk_label_new (str);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	g_free (str);

	return label;
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_window (const gchar *name, gint width, gint height, GUI *appGUI)
{
	GtkWidget *window;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), name);
	gtk_window_set_default_size (GTK_WINDOW (window), width, height);
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (appGUI->main_window));
	gtk_container_set_border_width (GTK_CONTAINER (window), WINDOW_BORDER);

	return window;
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_button (const gchar *gtk_name, const gchar *osmo_name, gchar *label)
{
	if (config.default_stock_icons) {
        return gtk_button_new_from_stock (gtk_name);
	} else {
		return utl_gui_stock_name_button (osmo_name, label);
	}

	/*return utl_gui_stock_name_button (config.default_stock_icons ? gtk_name : osmo_name, label);*/
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_frame (GtkWidget *container, const gchar *name)
{
	GtkWidget *frame, *label, *alignment;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
/*	gtk_container_add (GTK_CONTAINER (container), frame);*/
	gtk_box_pack_start (GTK_BOX (container), frame, FALSE, FALSE, 0);

	label = utl_gui_create_label ("<b>%s</b>", name);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment),
	                           ALIGNMENT_PADDING_TOP, ALIGNMENT_PADDING_BOTTOM,
	                           ALIGNMENT_PADDING_LEFT, ALIGNMENT_PADDING_RIGHT);

	return alignment;
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_vbox_in_frame (GtkWidget *container, const gchar *name)
{
	GtkWidget *frame, *box;

	frame = utl_gui_create_frame (container, name);

	box = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), box);

	return box;
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_hbox_in_frame (GtkWidget *container, const gchar *name)
{
	GtkWidget *frame, *box;

	frame = utl_gui_create_frame (container, name);

	box = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), box);

	return box;
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_table_in_frame (GtkWidget *container, const gchar *name, guint rows, guint columns)
{
	GtkWidget *frame, *table;

	frame = utl_gui_create_frame (container, name);

	table = gtk_table_new (rows, columns, FALSE);
	gtk_container_add (GTK_CONTAINER (frame), table);

	return table;
}

/* ========================================================================= */

GtkWidget *
utl_gui_create_icon_with_label (const gchar *stock_id, const gchar *label_str)
{
	GtkWidget *vbox_icon, *image, *label;

	vbox_icon = gtk_vbox_new (FALSE, 0);

	image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_box_pack_start (GTK_BOX (vbox_icon), image, TRUE, TRUE, 0);

	label = gtk_label_new (label_str);
	gtk_box_pack_start (GTK_BOX (vbox_icon), label, FALSE, FALSE, 0);
	gtk_widget_show_all (vbox_icon);

	return vbox_icon;
}

/* ========================================================================= */

GtkWidget *
utl_gui_insert_in_scrolled_window (GtkWidget *widget, GtkShadowType type)
{
	GtkWidget *scrolledwindow, *viewport;

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
	                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), type);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), viewport);

	gtk_container_add (GTK_CONTAINER (viewport), widget);

	return scrolledwindow;
}

/* ========================================================================= */

void
utl_gui_font_select_cb (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *font_selector;
	gchar *font_name;
	gint response;

	FONT_SEL *sel = (FONT_SEL *) user_data;

	g_return_if_fail (sel->config != NULL);

	font_selector = gtk_font_selection_dialog_new (_("Select a font..."));
	gtk_window_set_modal (GTK_WINDOW (font_selector), TRUE);
	gtk_window_set_position (GTK_WINDOW (font_selector), GTK_WIN_POS_MOUSE);
	gtk_window_set_transient_for (GTK_WINDOW (font_selector), GTK_WINDOW (sel->appGUI->main_window));
	gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG (font_selector),
	                                         sel->config);
	gtk_widget_show (font_selector);

	response = gtk_dialog_run (GTK_DIALOG (font_selector));

	if (response == GTK_RESPONSE_OK) {

		font_name = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG (font_selector));

		if (sel->save == TRUE)
			g_strlcpy (sel->config, font_name, MAXFONTNAME);

		if (sel->entry != NULL)
			gtk_entry_set_text (GTK_ENTRY (sel->entry), font_name);

		if (sel->font != NULL) {

			pango_font_description_free (*sel->font);
			*sel->font = pango_font_description_from_string (font_name);

			if (sel->widget != NULL)
				gtk_widget_modify_font (GTK_WIDGET (sel->widget), *sel->font);

		}

		g_free (font_name);

	}

	gtk_widget_destroy (font_selector);
}

/*------------------------------------------------------------------------------*/

gint
utl_gui_get_sw_vscrollbar_width (GtkWidget *scrolled_window)
{
	GtkWidget *vscrollbar;
	GValue *value;
	gint width;

	value = g_new0 (GValue, 1);
	value = g_value_init (value, G_TYPE_INT);

	vscrollbar = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (scrolled_window));
	gtk_widget_style_get_property (vscrollbar, "slider-width", value);
	width = g_value_get_int (value);

	g_value_unset (value);
	g_free (value);

	return width;
}

/*------------------------------------------------------------------------------*/

gint
utl_gui_get_combobox_items (GtkComboBox *combo_box) {
    return gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)), NULL);
}

/*------------------------------------------------------------------------------*/

void
utl_gui_create_category_combobox (GtkComboBox *combo_box, GtkListStore *store, gboolean none) {

GtkTreeIter iter;
gchar *category;
gint i, n;

    n = utl_gui_get_combobox_items(combo_box);

    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), -1);

    for (i = n-1; i >= 0; i--) {
        gtk_combo_box_remove_text (GTK_COMBO_BOX (combo_box), i);
    }

    if (none == TRUE) {
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("None"));
    } else {
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("All items"));
    }

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, i++)) {

        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &category, -1);
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), category);
        g_free(category);
    }

}

/*------------------------------------------------------------------------------*/

gint
utl_gui_get_column_position (GtkTreeViewColumn *column, GtkTreeView *treeview, gint M, GUI *appGUI) {

gint i, n = -1;

	for (i = 0; i < M; i++) {
		if (gtk_tree_view_get_column (GTK_TREE_VIEW(treeview), i) == column) n = i;
	}

	return n;
}

/*------------------------------------------------------------------------------*/

gchar *
utl_gui_text_buffer_get_text_with_tags (GtkTextBuffer *buffer) {

GtkTextIter start, prev;
GSList *tags = NULL, *i;
gchar tag_char_utf8[7] = {0};
gchar *text = g_strdup (""), *oldtext = NULL, *tmp;
gboolean done = FALSE;

    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &start);

    g_unichar_to_utf8 (TAG_CHAR, tag_char_utf8);

    prev = start;

    while (!done)
    {
        tmp = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer), &prev, &start, TRUE);
        oldtext = text;
        text = g_strconcat (text, tmp, NULL);
        g_free (oldtext);
        g_free (tmp);

        tags = gtk_text_iter_get_toggled_tags (&start, TRUE);
        for (i = tags; i; i = i->next)
        {
            gchar *name;
            g_object_get (G_OBJECT (i->data), "name", &name, NULL);
            oldtext = text;
            text = g_strconcat (text, tag_char_utf8, name, tag_char_utf8, NULL);
            g_free (oldtext);
            g_free (name);
        }
        g_slist_free (tags);

        tags = gtk_text_iter_get_toggled_tags (&start, FALSE);
        for (i = tags; i; i = i->next)
        {
            gchar *name;
            g_object_get (G_OBJECT (i->data), "name", &name, NULL);
            oldtext = text;
            text = g_strconcat (text, tag_char_utf8, "/", name, tag_char_utf8, NULL);
            g_free (oldtext);
            g_free (name);
        }
        g_slist_free (tags);

        if (gtk_text_iter_is_end (&start))
            done = TRUE;
        prev = start;
        gtk_text_iter_forward_to_tag_toggle (&start, NULL);
    }

    return text;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_text_buffer_set_text_with_tags (GtkTextBuffer *buffer, const gchar *text, gboolean clear) {

GtkTextIter start, end;
GList *tags = NULL;
gchar **tokens;
gint count;
gchar tag_char_utf8[7] = {0};

    if (!text)
        return;
			
    gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

	if (clear == TRUE) {
        gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);
	    gtk_text_buffer_delete (GTK_TEXT_BUFFER (buffer), &start, &end);
	}
    gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);

    g_unichar_to_utf8 (TAG_CHAR, tag_char_utf8);

    tokens = g_strsplit (text, tag_char_utf8, 0);

    for (count = 0; tokens[count]; count++)
    {
        if (count % 2 == 0)
        {
            gint offset;
            GList *j;

            offset = gtk_text_iter_get_offset (&end);
            gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, tokens[count], -1);
            gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &start, offset);

            for (j = tags; j; j = j->next)
            {
                gtk_text_buffer_apply_tag_by_name (GTK_TEXT_BUFFER (buffer), j->data, &start, &end);
            }
        }
        else
        {
            if (tokens[count][0] != '/')
            {
                tags = g_list_prepend (tags, tokens[count]);
            }
            else
            {
                GList *element = g_list_find_custom (tags, &(tokens[count][1]), (GCompareFunc) g_ascii_strcasecmp);

                if (element)
                {
                    tags = g_list_delete_link (tags, element);
                }
            }
        }
    }

    gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));

    g_strfreev (tokens);
}

/*------------------------------------------------------------------------------*/

void
utl_gui_text_buffer_toggle_tags (GtkTextBuffer *buffer, const gchar *tag_name) {

GtkTextTagTable *tag_table;
GtkTextTag *tag;
GtkTextIter start, end, titer;
gboolean itagged;
	
	tag_table = gtk_text_buffer_get_tag_table (buffer);
	tag = gtk_text_tag_table_lookup (tag_table, tag_name);
	
	g_return_if_fail (tag != NULL);

	gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
	
	itagged = TRUE;

	for (titer = start; !gtk_text_iter_equal (&titer, &end); gtk_text_iter_forward_char (&titer)) {
		if ((itagged = gtk_text_iter_has_tag (&titer, tag)) == FALSE) {
			break;
		}
	}
	
	if (itagged) {
		gtk_text_buffer_remove_tag (buffer, tag, &start, &end);
	} else {
		gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
	}
}

/*------------------------------------------------------------------------------*/

gchar *
utl_gui_text_strip_tags (gchar *text) {

gchar *output = g_strdup(""), *oldoutput = NULL;
gchar tag_char_utf8[7] = {0};
GList *tags = NULL;
gchar **tokens;
gint count;

	g_return_val_if_fail (output != NULL, NULL);

    g_unichar_to_utf8 (TAG_CHAR, tag_char_utf8);

    tokens = g_strsplit (text, tag_char_utf8, 0);

    for (count = 0; tokens[count]; count++)
    {
        if (count % 2 == 0)
        {
			oldoutput = output;
	        output = g_strconcat (output, tokens[count], NULL);
            g_free (oldoutput);
        }
        else
        {
            if (tokens[count][0] != '/')
            {
                tags = g_list_prepend (tags, tokens[count]);
            }
            else
            {
                GList *element = g_list_find_custom (tags, &(tokens[count][1]), (GCompareFunc) g_ascii_strcasecmp);

                if (element)
                {
                    tags = g_list_delete_link (tags, element);
                }
            }
        }
    }

    g_strfreev (tokens);

	return output;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_text_buffer_remove_all_tags (GtkTextBuffer *buffer) {

GtkTextIter start, end;

	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_remove_all_tags (buffer, &start, &end);
}

/*------------------------------------------------------------------------------*/

void
utl_gui_change_bg_widget_state (GtkWidget *widget, gchar *color_str, GUI *appGUI) {

GdkColor color;

    if (color_str != NULL) {

        gdk_color_parse (color_str, &color);
        gtk_widget_modify_base (widget, GTK_STATE_NORMAL, &color);

    } else {

        gtk_widget_modify_base (widget, GTK_STATE_NORMAL, 
                                &(appGUI->main_window)->style->base[GTK_WIDGET_STATE (appGUI->main_window)]);
    }

}

/*------------------------------------------------------------------------------*/

GdkPixbuf*
utl_gui_create_color_swatch (gchar *color) {

gchar *swatch[] = {
    "32 14 2 1", ".      c #FFFFFF", "-      c #000000",
    "--------------------------------", "-..............................-", "-..............................-",
    "-..............................-", "-..............................-", "-..............................-",
    "-..............................-", "-..............................-", "-..............................-",
    "-..............................-", "-..............................-", "-..............................-",
    "-..............................-", "--------------------------------"
};

gchar color_str[] = ".      c #000000";

    sprintf (color_str, ".      c %s", color);
    swatch[1] = color_str;
    return gdk_pixbuf_new_from_xpm_data ((gchar const **)swatch);
}

/*------------------------------------------------------------------------------*/

void
utl_gui_fill_iconlabel (GtkWidget *dialog, gchar *stock_icon, gchar *message) {

GtkWidget *hbox1;
GtkWidget *image;
GtkWidget *label;

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), hbox1, TRUE, TRUE, 16);
    image = gtk_image_new_from_stock (stock_icon, GTK_ICON_SIZE_DIALOG);
    gtk_widget_show (image);
    gtk_box_pack_start (GTK_BOX (hbox1), image, FALSE, TRUE, 16);
    label = gtk_label_new (NULL);
    gtk_widget_show (label);
    gtk_label_set_markup (GTK_LABEL(label), message);
    gtk_box_pack_start (GTK_BOX (hbox1), label, TRUE, TRUE, 16);
}

/*------------------------------------------------------------------------------*/

gint
utl_gui_create_dialog (gint dialog_type, gchar *message, GtkWindow *parent) {

GtkWidget *info_dialog = NULL;
gint response = -1;

    switch (dialog_type) {

        case GTK_MESSAGE_QUESTION:
            if (config.default_stock_icons) {
                info_dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(parent), 
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message, NULL);
                gtk_window_set_title (GTK_WINDOW(info_dialog), _("Question"));
            } else {
                info_dialog = gtk_dialog_new_with_buttons (_("Question"), GTK_WINDOW(parent),
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      OSMO_STOCK_BUTTON_NO, GTK_RESPONSE_NO,
                                                      OSMO_STOCK_BUTTON_YES, GTK_RESPONSE_YES, NULL);
                utl_gui_fill_iconlabel (info_dialog, "gtk-dialog-question", message);
            }
            break;

        case GTK_MESSAGE_INFO:
            if (config.default_stock_icons) {
                info_dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(parent), 
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message, NULL);
                gtk_window_set_title (GTK_WINDOW(info_dialog), _("Information"));
            } else {
                info_dialog = gtk_dialog_new_with_buttons (_("Information"), GTK_WINDOW(parent),
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      OSMO_STOCK_BUTTON_CLOSE, GTK_RESPONSE_NO, NULL);
                utl_gui_fill_iconlabel (info_dialog, "gtk-dialog-info", message);
            }
            break;

        case GTK_MESSAGE_ERROR:
            if (config.default_stock_icons) {
                info_dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(parent), 
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, message, NULL);
                gtk_window_set_title (GTK_WINDOW(info_dialog), _("Error"));
            } else {
                info_dialog = gtk_dialog_new_with_buttons (_("Error"), GTK_WINDOW(parent),
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      OSMO_STOCK_BUTTON_CLOSE, GTK_RESPONSE_NO, NULL);
                utl_gui_fill_iconlabel (info_dialog, "gtk-dialog-error", message);
            }
            break;

        case GTK_MESSAGE_WARNING:
            if (config.default_stock_icons) {
                info_dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(parent), 
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, message, NULL);
                gtk_window_set_title (GTK_WINDOW(info_dialog), _("Warning"));
            } else {
                info_dialog = gtk_dialog_new_with_buttons (_("Warning"), GTK_WINDOW(parent),
                                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                      OSMO_STOCK_BUTTON_CLOSE, GTK_RESPONSE_NO, NULL);
                utl_gui_fill_iconlabel (info_dialog, "gtk-dialog-warning", message);
            }
            break;
    };

    if (info_dialog != NULL) {
        gtk_widget_show (info_dialog);
        response = gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy (info_dialog);
    }

    return response;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_update_command_status (GtkWidget *entry_widget, GtkWidget *icon_widget, GUI *appGUI) {

GdkPixbuf *image;
gchar *cmd;
gint i;

    if (strlen(gtk_entry_get_text(GTK_ENTRY(entry_widget)))) {

        cmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_widget)));

        for (i=0; i < strlen(cmd); i++) {
            if (cmd[i] == ' ') cmd[i] = 0;
        }
	    gtk_widget_show (icon_widget);

        if (utl_is_valid_command (cmd) == TRUE) {
            image = gtk_widget_render_icon (icon_widget, OSMO_STOCK_LIST_VALID, GTK_ICON_SIZE_MENU, NULL);
        } else {
            image = gtk_widget_render_icon (icon_widget, OSMO_STOCK_LIST_INVALID, GTK_ICON_SIZE_MENU, NULL);
        }

	    gtk_image_set_from_pixbuf (GTK_IMAGE(icon_widget), image);
	    g_object_unref (image);
        g_free (cmd);

    } else {
	    gtk_widget_hide (icon_widget);
    }
}

/*------------------------------------------------------------------------------*/

gint
utl_gui_check_overwrite_file (gchar *filename, GtkWidget *window, GUI *appGUI) {

gint response;

    if (g_file_test(filename, G_FILE_TEST_IS_REGULAR) == TRUE) {

        response = utl_gui_create_dialog(GTK_MESSAGE_QUESTION, 
										 _("Selected file exist! Overwrite?"), GTK_WINDOW(window));

        if (response == GTK_RESPONSE_NO || response == GTK_RESPONSE_DELETE_EVENT) {
            return -1;
        }
    }

    return 0;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_clear_text_buffer (GtkTextBuffer *buffer, GtkTextIter *iter)
{
	GtkTextIter iter_s, iter_e;

	gtk_text_buffer_get_bounds (buffer, &iter_s, &iter_e);
	gtk_text_buffer_delete (buffer, &iter_s, &iter_e);
	if (iter != NULL)
		gtk_text_buffer_get_iter_at_offset (buffer, iter, 0);
}

/*------------------------------------------------------------------------------*/

gint
utl_gui_list_store_get_text_index (GtkListStore *store, gchar *text) {

GtkTreeIter iter;
gint i, f;
gchar *category;

    i = f = 0;

    if (text != NULL) {
        while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, i++)) {

            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &category, -1);
            if (category != NULL) {
                if (!strcmp(category, text)) {
                    f = 1;
                    break;
                }
                g_free(category);
            }
        }

        if (f) {
            g_free(category);
        }
    }

    return (f ? i:0);
}

/*------------------------------------------------------------------------------*/

void
utl_gui_sw_vscrollbar_move_position (GtkWidget *scrolled_window, gint direction) {

GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scrolled_window));

    if (direction == SW_MOVE_UP) {

        if (adj->value > 0.0) {
            adj->value -= adj->step_increment;
            adj->value = adj->value < 0.0 ? 0.0 : adj->value;
            gtk_adjustment_value_changed (GTK_ADJUSTMENT(adj));
        }

    } else if (direction == SW_MOVE_DOWN) {

        if (adj->value+adj->page_size < adj->upper) {
            adj->value += adj->step_increment;
            gtk_adjustment_value_changed (GTK_ADJUSTMENT(adj));
        }
    }
}

/*------------------------------------------------------------------------------*/

GtkWidget* 
utl_gui_stock_button (const gchar *bstock, gboolean toggle) {

GtkWidget *button;
GtkWidget *alignment;
GtkWidget *hbox;
GtkWidget *image;

    if(toggle == FALSE) {
        button = g_object_new (GTK_TYPE_BUTTON, "visible", TRUE, NULL);
    } else {
        button = g_object_new (GTK_TYPE_TOGGLE_BUTTON, "visible", TRUE, NULL);
    }

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    hbox = gtk_hbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    image = gtk_image_new_from_stock (bstock, GTK_ICON_SIZE_BUTTON);
    if (image)
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);

    gtk_widget_show_all (alignment);
    gtk_container_add (GTK_CONTAINER (button), alignment);

    return button;
}

/*------------------------------------------------------------------------------*/

GtkWidget* 
utl_gui_stock_name_button (const gchar *bstock, gchar *blabel) {

GtkWidget *button;
GtkWidget *alignment;
GtkWidget *hbox;
GtkWidget *image;
GtkWidget *label;

    button = g_object_new (GTK_TYPE_BUTTON, "visible", TRUE, NULL);

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    hbox = gtk_hbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    image = gtk_image_new_from_stock (bstock, GTK_ICON_SIZE_BUTTON);
    if (image)
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);

	label = gtk_label_new (blabel);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

    gtk_widget_show_all (alignment);
    gtk_container_add (GTK_CONTAINER (button), alignment);

    return button;
}

/*------------------------------------------------------------------------------*/

GtkWidget* 
utl_gui_image_label_radio_button (gchar *label, const guint8 *pix) {

GtkWidget *button;
GtkWidget *alignment;
GtkWidget *hbox;
GtkWidget *image;
GdkPixbuf *icon;

    button = g_object_new (GTK_TYPE_RADIO_BUTTON, "visible", TRUE, NULL);

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    hbox = gtk_hbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    icon = gdk_pixbuf_new_from_inline (-1, pix, FALSE, NULL);
    image = gtk_image_new_from_pixbuf(icon);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);

    if (label != NULL && label != (gchar *)-1) {
        gtk_box_pack_start (GTK_BOX (hbox),
        g_object_new (GTK_TYPE_LABEL, "label", label, "use_underline", TRUE, NULL), FALSE, TRUE, 0);
    }

    gtk_widget_show_all (alignment);
    gtk_container_add (GTK_CONTAINER (button), alignment);

    return button;
}

/*------------------------------------------------------------------------------*/

GtkWidget* 
utl_gui_stock_label_radio_button (gchar *label, const gchar *stock, GtkIconSize size) {

GtkWidget *button;
GtkWidget *alignment;
GtkWidget *hbox;
GtkWidget *image;

    button = g_object_new (GTK_TYPE_RADIO_BUTTON, "visible", TRUE, NULL);

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    hbox = gtk_hbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    image = gtk_image_new_from_stock (stock, size);

    if (image) {
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
    }

    if (label != NULL && label != (gchar *)-1) {
        gtk_box_pack_start (GTK_BOX (hbox),
        g_object_new (GTK_TYPE_LABEL, "label", label, "use_underline", TRUE, NULL), FALSE, TRUE, 0);
    }

    gtk_widget_show_all (alignment);
    gtk_container_add (GTK_CONTAINER (button), alignment);

    return button;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_url_initialize (GUI *appGUI) {

    appGUI->hand_cursor = gdk_cursor_new (GDK_HAND2);
    appGUI->regular_cursor = gdk_cursor_new (GDK_XTERM);

    appGUI->hovering_over_link = FALSE;
    appGUI->gui_url_tag = NULL;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_url_insert_link (GSList **links_list, gint *link_index, GtkWidget *textview, GtkTextIter *iter,
                         gchar *color, gchar *font, gchar *text, gboolean center, GUI *appGUI)
{
	GtkJustification justify = center ? GTK_JUSTIFY_CENTER : GTK_JUSTIFY_LEFT;
	PangoUnderline underline = config.disable_underline_links ? PANGO_UNDERLINE_NONE : PANGO_UNDERLINE_SINGLE;

	appGUI->gui_url_tag = gtk_text_buffer_create_tag (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), NULL,
	                                                  "foreground", color,
	                                                  "font", font,
	                                                  "justification", justify,
	                                                  "underline", underline,
	                                                  NULL);

	g_object_set_data (G_OBJECT (appGUI->gui_url_tag), "link", GINT_TO_POINTER (*link_index));

	*links_list = g_slist_append (*links_list, g_strdup (text));
	gtk_text_buffer_insert_with_tags (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)),
	                                  iter, text, -1, appGUI->gui_url_tag, NULL);
	(*link_index)++;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_url_remove_links (GSList **links_list, gint *link_index)
{
	if (link_index) *link_index = 1;

	if (*links_list != NULL) {
		g_slist_foreach (*links_list, (GFunc) g_free, NULL);
		g_slist_free (*links_list);
		*links_list = NULL;
	}
}

/*------------------------------------------------------------------------------*/

void
utl_gui_url_set_cursor_if_appropriate (GtkTextView *textview, gint x, gint y, GUI *appGUI) {

GSList *tags = NULL, *tagp = NULL;
GtkTextBuffer *buffer;
GtkTextIter iter;
gboolean hovering = FALSE;
GtkTextTag *tag;
int *slink;

    buffer = gtk_text_view_get_buffer (textview);

    gtk_text_view_get_iter_at_location (textview, &iter, x, y);

    tags = gtk_text_iter_get_tags (&iter);

    for (tagp = tags;  tagp != NULL;  tagp = tagp->next) {
        tag = tagp->data;
        slink = g_object_get_data (G_OBJECT (tag), "link");

        if (slink != 0) {
            hovering = TRUE;
            break;
        }
    }

    if (hovering != appGUI->hovering_over_link) {
        appGUI->hovering_over_link = hovering;

        if (appGUI->hovering_over_link) {
            gdk_window_set_cursor (gtk_text_view_get_window (textview, GTK_TEXT_WINDOW_TEXT), appGUI->hand_cursor);
        } else {
            gdk_window_set_cursor (gtk_text_view_get_window (textview, GTK_TEXT_WINDOW_TEXT), appGUI->regular_cursor);
        }
    }

    if (tags) {
        g_slist_free (tags);
    }
}

/*------------------------------------------------------------------------------*/

gboolean
utl_gui_url_event_after (GtkWidget *textview, GdkEvent *ev, GSList **links_list)
{
	GtkTextIter start, end, iter;
	GtkTextBuffer *buffer;
	GdkEventButton *event;
	gint x, y;
	GSList *tags = NULL, *tagp = NULL;
	GtkTextTag *tag;
	gchar *link;
	gint slink;
	gchar tmpbuf[BUFFER_SIZE];

	if (ev->type != GDK_BUTTON_RELEASE)
		return FALSE;

	event = (GdkEventButton *) ev;
	if (event->button != 1)
		return FALSE;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
	if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
		return FALSE;

	gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (textview), GTK_TEXT_WINDOW_WIDGET,
	                                       event->x, event->y, &x, &y);
	gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (textview), &iter, x, y);

	tags = gtk_text_iter_get_tags (&iter);
	for (tagp = tags; tagp != NULL; tagp = tagp->next) {
		tag = tagp->data;
		slink = (gint) g_object_get_data (G_OBJECT (tag), "link");

		if (slink != 0) {
			link = g_slist_nth_data (*links_list, slink-1);
			g_snprintf (tmpbuf, BUFFER_SIZE, "\"%s\"", link);
			utl_run_helper (tmpbuf, utl_get_link_type (link));
			break;
		}
	}

	if (tags) g_slist_free (tags);

	return FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
utl_gui_url_motion_notify_event (GtkWidget *textview, GdkEventMotion *event, gpointer data) {

gint x, y;

    GUI *appGUI = (GUI *)data;

    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (textview), GTK_TEXT_WINDOW_WIDGET, 
                                           event->x, event->y, &x, &y);
    utl_gui_url_set_cursor_if_appropriate (GTK_TEXT_VIEW (textview), x, y, appGUI);
    gdk_window_get_pointer (textview->window, NULL, NULL, NULL);
    return FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
utl_gui_url_visibility_notify_event (GtkWidget *textview, GdkEventVisibility *event, gpointer data) {

gint wx, wy, bx, by;

    GUI *appGUI = (GUI *)data;

    gdk_window_get_pointer (textview->window, &wx, &wy, NULL);
    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (textview), GTK_TEXT_WINDOW_WIDGET, wx, wy, &bx, &by);
    utl_gui_url_set_cursor_if_appropriate (GTK_TEXT_VIEW (textview), bx, by, appGUI);
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
utl_gui_url_setup (GSList **links_list, gint *link_index, GtkWidget *textview, GUI *appGUI) {

    utl_gui_url_remove_links (links_list, link_index);

    *link_index = 1;
    appGUI->hovering_over_link = FALSE;
    appGUI->gui_url_tag = NULL;

    g_signal_connect (textview, "event-after", G_CALLBACK (utl_gui_url_event_after), links_list);
    g_signal_connect (textview, "motion-notify-event", G_CALLBACK (utl_gui_url_motion_notify_event), appGUI);
    g_signal_connect (textview, "visibility-notify-event", G_CALLBACK (utl_gui_url_visibility_notify_event), appGUI);

}

/*------------------------------------------------------------------------------*/

