
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
#include "notes.h"
#include "i18n.h"
#include "calendar.h"
#include "calendar_notes.h"
#include "options_prefs.h"
#include "preferences_gui.h"
#include "notes_items.h"
#include "calendar_utils.h"
#include "utils.h"
#include "utils_date.h"
#include "utils_date_time.h"
#include "utils_gui.h"
#include "stock_icons.h"

#ifdef NOTES_ENABLED

#ifdef HAVE_LIBGRINGOTTS
#include <libgringotts.h>
#endif /* HAVE_LIBGRINGOTTS */

#ifdef HAVE_GTKSPELL
#include <gtkspell/gtkspell.h>
#endif /* HAVE_GTKSPELL */

/*============================================================================*/

static void
show_about_window_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkWidget *window = opt_create_about_window (appGUI);
	gtk_widget_show (window);
}

/*============================================================================*/

static void
show_preferences_window_cb (GtkWidget *widget, GUI *appGUI)
{
	appGUI->opt->window = opt_create_preferences_window (appGUI);
	gtk_widget_show (appGUI->opt->window);

	gint page = gtk_notebook_page_num (GTK_NOTEBOOK (appGUI->opt->notebook), appGUI->opt->notes);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->opt->notebook), page);
}

/*------------------------------------------------------------------------------*/

void
notes_show_selector_editor (gint mode, GUI *appGUI) {

GtkTextBuffer *buffer;
GtkTextMark *cursor;
GtkTextIter iter;
GtkTreeIter n_iter;
GtkTreePath *path, *sort_path, *filter_path;
gchar *name, *fontname;
gint line;
gboolean remember_cursor, readonly = FALSE;
gchar tmpbuf[BUFFER_SIZE];

	if (mode == SELECTOR) {

		gtk_widget_show (appGUI->nte->vbox_selector);
		gtk_widget_hide (appGUI->nte->vbox_editor);
		gtk_widget_grab_focus (appGUI->nte->notes_find_entry);
		appGUI->nte->editor_active = FALSE;

	} else if (mode == EDITOR) {

		gtk_widget_hide (appGUI->nte->find_hbox);
		gtk_widget_hide (appGUI->nte->vbox_selector);
		gtk_widget_show (appGUI->nte->vbox_editor);
		gtk_entry_set_text(GTK_ENTRY(appGUI->nte->find_entry), "");
		gtk_widget_grab_focus (appGUI->nte->editor_textview);
		appGUI->nte->editor_active = TRUE;

		gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->nte->notes_list), &sort_path, NULL);

		if (sort_path == NULL) return;
		filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->nte->notes_sort), sort_path);
		gtk_tree_path_free (sort_path);

		if (filter_path == NULL) return;
		path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), filter_path);
		gtk_tree_path_free (filter_path);

		gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &n_iter, path);
		gtk_tree_model_get (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &n_iter,
                            N_COLUMN_NAME, &name,
                            N_COLUMN_EDITOR_LINE, &line,
							N_COLUMN_REMEMBER_EDITOR_LINE, &remember_cursor, 
							N_COLUMN_EDITOR_READONLY, &readonly,
							N_COLUMN_FONTNAME, &fontname, -1);
		sprintf (tmpbuf, "<big><i><b>%s</b></i></big>", name);
		gtk_label_set_markup (GTK_LABEL (appGUI->nte->title_label), tmpbuf);
        appGUI->nte->fd_notes_font = pango_font_description_from_string(fontname);
	    gtk_widget_modify_font (GTK_WIDGET(appGUI->nte->editor_textview), appGUI->nte->fd_notes_font);
		gtk_font_button_set_font_name (GTK_FONT_BUTTON(appGUI->nte->font_picker), fontname);
		g_free(name);
		g_free(fontname);
		gtk_tree_path_free (path);

		/* restore cursor position */

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));

		if (remember_cursor == TRUE) {
			gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (buffer), &iter, line);
			gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (buffer), &iter);
			cursor = gtk_text_buffer_get_mark (buffer, "insert");
			gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (appGUI->nte->editor_textview), cursor, 0.0, TRUE, 0.0, 0.0);
		} else {
			gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &iter);
			gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (buffer), &iter);
			gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (appGUI->nte->editor_textview), &iter, 0.0, FALSE, 0.0, 0.0);
		}

		/* read-only */
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->nte->readonly_checkbutton), readonly);
		gtk_text_view_set_editable (GTK_TEXT_VIEW (appGUI->nte->editor_textview), !readonly);
		gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (appGUI->nte->editor_textview), !readonly);
		gtk_widget_set_sensitive (appGUI->nte->font_picker, !readonly);
		appGUI->nte->note_read_only = readonly;

    }
}

/*------------------------------------------------------------------------------*/

void
editor_save_buffer_cb (GtkWidget *widget, gpointer user_data) {

GtkTextBuffer *buffer;
guchar *text;
guint32 current_date;
gint current_time;
GtkTreeIter iter;
GtkTreeIter sort_child_iter, filter_child_iter;
GtkTreeModel *model;

	GUI *appGUI = (GUI *)user_data;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));

	text = (unsigned char*) utl_gui_text_buffer_get_text_with_tags (buffer);

	if (appGUI->nte->encrypted == TRUE) {

#ifdef HAVE_LIBGRINGOTTS
	if (appGUI->nte->keyholder != NULL && appGUI->nte->context != NULL && appGUI->nte->filename != NULL) {
        grg_encrypt_file (appGUI->nte->context, appGUI->nte->keyholder,
						  (unsigned char*) notes_get_full_filename(appGUI->nte->filename, appGUI), 
						  (guchar *) text, -1);

        grg_free (appGUI->nte->context, text, -1);
	}
#endif /* HAVE_LIBGRINGOTTS */

	} else {

		g_file_set_contents (notes_get_full_filename(appGUI->nte->filename, appGUI), 
							 (gchar *) text, strlen((gchar *) text), NULL);
	}

	appGUI->nte->changed = FALSE;
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/save"), FALSE);

	if (gtk_tree_selection_get_selected (appGUI->nte->notes_list_selection, &model, &iter)) {

		current_date = utl_date_get_current_julian ();
		current_time = utl_time_get_current_seconds ();

		gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT(appGUI->nte->notes_sort), 
														  &sort_child_iter, &iter);
		gtk_tree_model_filter_convert_iter_to_child_iter (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), 
														  &filter_child_iter, &sort_child_iter);

		gtk_list_store_set (appGUI->nte->notes_list_store, &filter_child_iter, 
						    N_COLUMN_LAST_CHANGES_DATE, get_date_time_str (current_date, current_time),
						    N_COLUMN_LAST_CHANGES_DATE_JULIAN, current_date,
						    N_COLUMN_LAST_CHANGES_TIME, current_time, -1);
	}
}

/*------------------------------------------------------------------------------*/

void
editor_find_text_show_cb (GtkWidget *widget, gpointer user_data) {

	GUI *appGUI = (GUI *)user_data;

	gtk_widget_show (appGUI->nte->find_hbox);
	gtk_widget_grab_focus (appGUI->nte->find_entry);

	appGUI->nte->find_hbox_visible = TRUE;
}

/*------------------------------------------------------------------------------*/

void
editor_find_text_hide_cb (GtkWidget *widget, gpointer user_data) {

	GUI *appGUI = (GUI *)user_data;

	utl_gui_change_bg_widget_state (appGUI->nte->find_entry, NULL, appGUI);

	gtk_widget_hide (appGUI->nte->find_hbox);
	gtk_widget_grab_focus (appGUI->nte->editor_textview);

	appGUI->nte->find_hbox_visible = FALSE;
}

/*------------------------------------------------------------------------------*/

void
editor_close_cb (GtkWidget *widget, gpointer user_data) {

GtkTextBuffer *buffer;
GtkTextIter iter_s, iter_e, l_iter;
GtkTreeIter n_iter;
GtkTreePath *path, *sort_path, *filter_path;
gint response, line;
GtkTextMark *cursor;
gchar tmpbuf[BUFFER_SIZE];

	GUI *appGUI = (GUI *)user_data;

	if (appGUI->nte->changed == TRUE) {

		sprintf (tmpbuf, "%s\n\n%s", _("The note has changed."), _("Do you want to save it?"));

		response = utl_gui_create_dialog (GTK_MESSAGE_QUESTION, tmpbuf, GTK_WINDOW(appGUI->main_window));

		if (response == GTK_RESPONSE_YES) {
			editor_save_buffer_cb (NULL, appGUI);
		} else if (response != GTK_RESPONSE_NO) {
			return;
		}
	}

#ifdef HAVE_GTKSPELL
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/spell_check")), FALSE);
#endif /* HAVE_GTKSPELL */

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));

#ifdef HAVE_LIBGRINGOTTS
	if (appGUI->nte->keyholder != NULL && appGUI->nte->context != NULL) {
		grg_key_free (appGUI->nte->context, appGUI->nte->keyholder);
		appGUI->nte->keyholder = NULL;
		grg_context_free (appGUI->nte->context);
		appGUI->nte->context = NULL;
	}
#endif /* HAVE_LIBGRINGOTTS */

	if (appGUI->nte->filename != NULL) {
		g_free(appGUI->nte->filename);
		appGUI->nte->filename = NULL;
	}

	cursor = gtk_text_buffer_get_mark (buffer, "insert");
	gtk_text_buffer_get_iter_at_mark (buffer, &l_iter, cursor);
	line = gtk_text_iter_get_line (&l_iter);

	gtk_text_buffer_get_bounds(buffer, &iter_s, &iter_e);
	gtk_text_buffer_delete(buffer, &iter_s, &iter_e);
	appGUI->nte->buffer_check_modify_enable = FALSE;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->nte->notes_list), &sort_path, NULL);

	if (sort_path == NULL) return;
	filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->nte->notes_sort), sort_path);
	gtk_tree_path_free (sort_path);

	if (filter_path == NULL) return;
	path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), filter_path);
	gtk_tree_path_free (filter_path);

	gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &n_iter, path);
	gtk_list_store_set (appGUI->nte->notes_list_store, &n_iter,
						N_COLUMN_EDITOR_LINE, line, 
						N_COLUMN_EDITOR_READONLY, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->nte->readonly_checkbutton)),
						N_COLUMN_FONTNAME, gtk_font_button_get_font_name (GTK_FONT_BUTTON(appGUI->nte->font_picker)), -1);
	gtk_tree_path_free (path);

	notes_show_selector_editor (SELECTOR, appGUI);

	gtk_widget_grab_focus (appGUI->nte->notes_list);
}

/*------------------------------------------------------------------------------*/

void
set_text_attribute_cb (GtkWidget *widget, gpointer user_data) {

GtkTextBuffer *buffer;
gchar *tagname;

	GUI *appGUI = (GUI *)user_data;

	tagname = (gchar*) g_object_get_data (G_OBJECT (widget), "tag");
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));
	utl_gui_text_buffer_toggle_tags (buffer, tagname);
	g_signal_emit_by_name(G_OBJECT(buffer), "changed");
}

/*------------------------------------------------------------------------------*/

void
clear_text_attributes_cb (GtkButton *button, gpointer user_data) {

GtkTextIter start, end;
GtkTextBuffer *buffer;

	GUI *appGUI = (GUI *)user_data;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));
	gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
	gtk_text_buffer_remove_all_tags (buffer, &start, &end);
}

/*------------------------------------------------------------------------------*/

void
add_notes_toolbar_selector_widget (GtkUIManager *notes_uim_widget, GtkWidget *widget, gpointer user_data) {

GtkWidget *handle_box;

    GUI *appGUI = (GUI *)user_data;

    if (GTK_IS_TOOLBAR (widget)) {

        appGUI->nte->notes_toolbar_selector = GTK_TOOLBAR (widget);

        handle_box = gtk_handle_box_new ();
        gtk_widget_show (handle_box);
        gtk_container_add (GTK_CONTAINER (handle_box), widget);
        gtk_box_pack_start (GTK_BOX(appGUI->nte->vbox_selector), handle_box, FALSE, FALSE, 0);
        g_signal_connect_swapped (widget, "destroy", 
                                  G_CALLBACK (gtk_widget_destroy), handle_box);

    } else {
        gtk_box_pack_start (GTK_BOX(appGUI->nte->vbox_selector), widget, FALSE, FALSE, 0);
    }

    gtk_widget_show (widget);
}

/*------------------------------------------------------------------------------*/

void
add_notes_toolbar_editor_widget (GtkUIManager *notes_uim_widget, GtkWidget *widget, gpointer user_data) {

GtkWidget *handle_box;

    GUI *appGUI = (GUI *)user_data;

    if (GTK_IS_TOOLBAR (widget)) {

        appGUI->nte->notes_toolbar_editor = GTK_TOOLBAR (widget);

        handle_box = gtk_handle_box_new ();
        gtk_widget_show (handle_box);
        gtk_container_add (GTK_CONTAINER (handle_box), widget);
        gtk_box_pack_start (GTK_BOX(appGUI->nte->vbox_editor), handle_box, FALSE, FALSE, 0);
        g_signal_connect_swapped (widget, "destroy", 
                                  G_CALLBACK (gtk_widget_destroy), handle_box);

    } else {
        gtk_box_pack_start (GTK_BOX(appGUI->nte->vbox_editor), widget, FALSE, FALSE, 0);
    }

    gtk_widget_show (widget);
}

/*------------------------------------------------------------------------------*/

void
notes_add_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    notes_add_entry (appGUI);
}

/*------------------------------------------------------------------------------*/

void
notes_edit_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    notes_edit_dialog_show(appGUI->nte->notes_list, appGUI->nte->notes_filter, appGUI);
}

/*------------------------------------------------------------------------------*/

void
notes_delete_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    notes_remove_dialog_show(appGUI->nte->notes_list, appGUI->nte->notes_list_store, appGUI);
}

/*------------------------------------------------------------------------------*/

void
insert_current_date_and_time_cb (GtkWidget *widget, GUI *appGUI)
{
	time_t tmm;
	gchar *datestr;

	if (config.use_system_date_in_notes == TRUE) {
		tmm = time (NULL);
		gtk_text_buffer_insert_at_cursor (gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview)), 
		                                  asctime (localtime (&tmm)), -1);
	} else {
		datestr = utl_date_time_print_default (utl_date_get_current_julian (), utl_time_get_current_seconds (), FALSE);
		gtk_text_buffer_insert_at_cursor (gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview)), 
		                                  datestr, -1);
		g_free (datestr);
	}
}

/*------------------------------------------------------------------------------*/

void
insert_separator_cb (GtkWidget *widget, gpointer data) {

gint chars, i;
gchar tmpbuf[BUFFER_SIZE];
PangoLayout *layout;
PangoRectangle logical_rect;
GtkWidget *vscrollbar;

    GUI *appGUI = (GUI *)data;

    memset (tmpbuf, 0, BUFFER_SIZE);

    tmpbuf[0] = config.text_separator;
    layout = gtk_widget_create_pango_layout (appGUI->nte->editor_textview, NULL);
    pango_layout_set_font_description (layout, appGUI->nte->fd_notes_font);
    pango_layout_set_text (layout, tmpbuf, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    vscrollbar = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (appGUI->nte->editor_scrolledwindow));

	if (GTK_WIDGET_VISIBLE(vscrollbar) == TRUE) {
        chars = ((appGUI->nte->editor_textview)->allocation.width) / logical_rect.width;
    } else {
        chars = ((appGUI->nte->editor_textview)->allocation.width - utl_gui_get_sw_vscrollbar_width (appGUI->nte->editor_scrolledwindow)) / logical_rect.width;
    }
    chars = (chars > BUFFER_SIZE) ? BUFFER_SIZE - 2 : chars;

    g_object_unref (G_OBJECT(layout));

    i = 0;
    tmpbuf[i++] = '\n';
    while (i < chars) tmpbuf[i++] = config.text_separator;
    tmpbuf[i++] = '\n';

    gtk_text_buffer_insert_at_cursor (gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview)), 
                                      tmpbuf, -1);
}

/*------------------------------------------------------------------------------*/

gchar *
get_buffer_info_string (GtkTextBuffer *buffer, gboolean selection) 
{
	GtkTextIter start_iter, end_iter;
	gchar tmpbuf[BUFFER_SIZE];
	gchar *text;
	PangoLogAttr *attrs;
	gint i, words, lines, chars, white_chars, bytes;

	if (selection == TRUE && gtk_text_buffer_get_has_selection (buffer) == FALSE)
		return g_strdup("");

    if (selection == TRUE && gtk_text_buffer_get_has_selection (buffer) == TRUE) {
        gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter);
    } else {
        gtk_text_buffer_get_start_iter (buffer, &start_iter);
        gtk_text_buffer_get_end_iter (buffer, &end_iter);
    }

    text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer), &start_iter, &end_iter, FALSE);
    if (text == NULL) return g_strdup("");

    words = 0;
    white_chars = 0;
    lines = gtk_text_buffer_get_line_count (buffer);
    bytes = strlen (text);
    chars = g_utf8_strlen (text, -1);

    attrs = g_new0 (PangoLogAttr, chars + 1);   /* based on code by Paolo Maggi */
    pango_get_log_attrs (text, -1, 0, pango_language_from_string ("C"), attrs, chars + 1);

    for (i=0; i < chars; i++) {
        if (attrs[i].is_white) white_chars++;
        if (attrs[i].is_word_start) words++;
    }

    if (chars == 0) lines = 0;

	g_snprintf (tmpbuf, BUFFER_SIZE, "<b>%s</b>: %d\n<b>%s</b>: %d\n<b>%s</b>: %d\n<b>%s</b>: %d\n<b>%s</b>: %d\n",
                _("Words"), words,
                _("Lines"), lines,
                _("Characters"), chars,
                _("White characters"), white_chars,
                _("Bytes"), bytes);

	g_free (text);
    g_free (attrs);

	return g_strdup(tmpbuf);
}

/*------------------------------------------------------------------------------*/

void
text_info_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path, *sort_path, *filter_path;
	GtkTreeIter n_iter;
	GtkTextBuffer *buffer;

	guint32 modified_date, created_date;
	gint modified_time, created_time;
	gchar *modified_date_str, *created_date_str, *date_str, *day_str;
	gchar *txtinfo_normal, *txtinfo_selection;
	gchar tmpbuf[BUFFER_SIZE];

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->nte->notes_list), &sort_path, NULL);

    if (sort_path == NULL) return;
    filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT(appGUI->nte->notes_sort), sort_path);
    gtk_tree_path_free (sort_path);

    if (filter_path == NULL) return;
    path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), filter_path);
    gtk_tree_path_free (filter_path);

    gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &n_iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &n_iter, 
                        N_COLUMN_LAST_CHANGES_DATE_JULIAN, &modified_date, 
                        N_COLUMN_LAST_CHANGES_TIME, &modified_time,
                        N_COLUMN_CREATE_DATE_JULIAN, &created_date,
                        N_COLUMN_CREATE_TIME, &created_time,
                        -1);

	date_str = utl_date_time_print_default (modified_date, modified_time, FALSE);
	day_str = utl_date_print_j (modified_date, DATE_DAY_OF_WEEK_NAME, config.override_locale_settings);
	modified_date_str = g_strdup_printf ("%s (%s)", date_str, day_str);
	g_free (date_str);
	g_free (day_str);

	date_str = utl_date_time_print_default (created_date, created_time, FALSE);
	day_str = utl_date_print_j (created_date, DATE_DAY_OF_WEEK_NAME, config.override_locale_settings);
	created_date_str = g_strdup_printf ("%s (%s)", date_str, day_str);
	g_free (date_str);
	g_free (day_str);

	txtinfo_normal = get_buffer_info_string (buffer, FALSE);
	txtinfo_selection = get_buffer_info_string (buffer, TRUE);

	if (strlen(txtinfo_selection)) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<u>%s:</u>\n\n%s\n<u>%s:</u>\n\n%s\n\n<b>%s</b>: %s\n<b>%s</b>: %s",
					_("Document"), txtinfo_normal, _("Selection"), txtinfo_selection,
					_("Created"), created_date_str,
					_("Modified"), modified_date_str);
	} else {
		g_snprintf (tmpbuf, BUFFER_SIZE, "<u>%s:</u>\n\n%s\n\n<b>%s</b>: %s\n<b>%s</b>: %s",
					_("Document"), txtinfo_normal,
					_("Created"), created_date_str,
					_("Modified"), modified_date_str);
	}

	g_free (txtinfo_selection);
	g_free (txtinfo_normal);

    g_free (modified_date_str);
    g_free (created_date_str);
    gtk_tree_path_free (path);

	utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW (appGUI->main_window));
}

/*------------------------------------------------------------------------------*/

gint
notes_list_dbclick_cb(GtkWidget * widget, GdkEventButton * event, gpointer func_data) {

    GUI *appGUI = (GUI *)func_data;

    if ((event->type==GDK_2BUTTON_PRESS) && (event->button == 1)) {
        if (gtk_tree_selection_get_selected (appGUI->nte->notes_list_selection, NULL, NULL)) {
            notes_enter_password (appGUI);
        }
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
text_buffer_modified_cb (GtkTextBuffer *textbuffer, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    if (appGUI->nte->buffer_check_modify_enable == FALSE) {
        appGUI->nte->changed = FALSE;
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/save"), FALSE);
    } else {
        appGUI->nte->changed = TRUE;
        gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/save"), TRUE);
    }
}

/*------------------------------------------------------------------------------*/

void
notes_item_selected (GtkTreeSelection *selection, gpointer data) {

    GUI *appGUI = (GUI *)data;

    gboolean state = gtk_tree_selection_get_selected (selection, NULL, NULL);

    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_selector_widget, "/toolbar/edit"), state);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_selector_widget, "/toolbar/delete"), state);

}

/*------------------------------------------------------------------------------*/

void
find (gchar *find_text, GtkTextIter *iter, GUI *appGUI) {

GtkTextBuffer *buffer;
GtkTextIter match_start, match_end;
GtkTextMark *found_pos;
gchar c;
gint i;

    if (strlen(find_text)) {

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(appGUI->nte->find_case_checkbutton)) == FALSE) {
            i = 0;
            while((c = find_text[i])) {
                find_text[i++] =  g_unichar_tolower (c);
            }
        }

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));

        if (gtk_text_iter_forward_search (iter, find_text, GTK_TEXT_SEARCH_TEXT_ONLY, 
                                          &match_start, &match_end, NULL)) {

            utl_gui_change_bg_widget_state (appGUI->nte->find_entry, COLOR_BG_OK, appGUI);
            gtk_text_buffer_select_range (buffer, &match_start, &match_end);
            gtk_text_buffer_create_mark (buffer, "last_pos", &match_end, FALSE);

            found_pos = gtk_text_buffer_create_mark (buffer, "found_pos", &match_end, FALSE);
            gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (appGUI->nte->editor_textview), found_pos); 
            gtk_text_buffer_delete_mark (buffer, found_pos);

        } else {
            utl_gui_change_bg_widget_state (appGUI->nte->find_entry, COLOR_BG_FAIL, appGUI);
            if (gtk_text_buffer_get_mark (buffer, "last_pos") != NULL) {
                gtk_text_buffer_delete_mark_by_name (buffer, "last_pos");
            }
        }
    }

}

/*------------------------------------------------------------------------------*/

void
find_entry_action (GUI *appGUI) {

GtkTextBuffer *buffer;
GtkTextMark *last_pos;
GtkTextIter iter;
gchar *find_text;

    find_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->find_entry)));
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));

    if (appGUI->nte->find_next_flag == TRUE) {

        last_pos = gtk_text_buffer_get_mark (buffer, "last_pos");

        if (last_pos == NULL) {
            gtk_text_buffer_get_start_iter (buffer, &iter);
            find (find_text, &iter, appGUI);
            g_free(find_text);
            return;
        }

        gtk_text_buffer_get_iter_at_mark (buffer, &iter, last_pos);
        find (find_text, &iter, appGUI);

    } else {
        gtk_text_buffer_get_start_iter (buffer, &iter);
        find (find_text, &iter, appGUI);
    }

    g_free(find_text);
}

/*------------------------------------------------------------------------------*/

gint
find_entry_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (event->keyval == GDK_Escape) {
            editor_find_text_hide_cb (widget, appGUI);
            appGUI->nte->find_next_flag = FALSE;
            return TRUE;
    } else if (event->keyval == GDK_Return) {
            find_entry_action (appGUI);
            appGUI->nte->find_next_flag = TRUE;
            return TRUE;
    } else {
        appGUI->nte->find_next_flag = FALSE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
case_sensitive_toggle_cb (GtkToggleButton *togglebutton, gpointer data) {

    GUI *appGUI = (GUI *)data;

    appGUI->nte->find_next_flag = FALSE;
}

/*------------------------------------------------------------------------------*/

gboolean
notes_category_combo_box_focus_cb (GtkWidget *widget, GtkDirectionType *arg1, gpointer user_data) {
    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
update_notes_items (GUI *appGUI) {

GtkTreeIter iter;
gint i;
gchar tmpbuf[BUFFER_SIZE];

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->nte->notes_filter), &iter, NULL, i++));
    --i;

    if (!i) {
        sprintf (tmpbuf, "<i>%s</i>", _("no entries"));
    } else {
        sprintf (tmpbuf, "<i>%4d %s</i>", i, ngettext ("entry", "entries", i));
    }

    gtk_label_set_markup (GTK_LABEL (appGUI->nte->n_items_label), tmpbuf);

}

/*------------------------------------------------------------------------------*/

void
refresh_notes (GUI *appGUI) {

GtkTreeIter iter;
gint i;
guint32 current_date;
gint current_time;

	i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, NULL, i++)) {

		gtk_tree_model_get (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, 
							N_COLUMN_LAST_CHANGES_DATE_JULIAN, &current_date, 
							N_COLUMN_LAST_CHANGES_TIME, &current_time, -1);

		gtk_list_store_set (appGUI->nte->notes_list_store, &iter, 
						    N_COLUMN_LAST_CHANGES_DATE, get_date_time_str (current_date, current_time), -1);
	}

	update_notes_items (appGUI);
}

/*------------------------------------------------------------------------------*/

void
notes_category_filter_cb (GtkComboBox *widget, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    appGUI->nte->filter_index = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    if (appGUI->nte->filter_index != -1) {
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter));
        update_notes_items (appGUI);
    }
}

/*------------------------------------------------------------------------------*/

gboolean
notes_list_filter_cb (GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {

gchar *category;
const gchar *text;
gchar *value;
gint text_len, len, j;

    GUI *appGUI = (GUI *)data;

    text = gtk_entry_get_text(GTK_ENTRY(appGUI->nte->notes_find_entry));

    if (text == NULL) {
        return TRUE;
    }

    text_len = strlen(text);

	gtk_tree_model_get(model, iter, N_COLUMN_NAME, &value, N_COLUMN_CATEGORY, &category, -1);

	if(appGUI->nte->filter_index) {

		if (utl_gui_list_store_get_text_index (appGUI->opt->notes_category_store, category) == appGUI->nte->filter_index) {

			if (value != NULL) {

				len = strlen(value) - text_len;

				if (len >= 0) {
					for(j=0; j <= len; j++) {
						if(!g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value+j, -1), text_len)) {
							g_free (value);
							g_free (category);
							return TRUE;
						}
					}
				}

				g_free(value);
			}

			g_free (category);
			return FALSE;
		} else {
			g_free (value);
			g_free (category);
			return FALSE;
		}

	} else {
		/* all items */

		if (value != NULL) {

			len = strlen(value) - text_len;

			if (len >= 0) {
				for(j=0; j <= len; j++) {
					if(!g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value+j, -1), text_len)) {
						g_free (value);
						g_free (category);
						return TRUE;
					}
				}
			}

			g_free(value);
		}
	}

	g_free (category);

    return FALSE;
}

/*------------------------------------------------------------------------------*/

gint 
custom_notes_sort_function (GtkTreeModel *model, GtkTreeIter *iter_a, GtkTreeIter *iter_b, gpointer user_data) {

gchar *name_a, *name_b;
gchar *category_a, *category_b;
guint32 last_changes_date_a, last_changes_date_b;
gint category_s, name_s;
guint32 last_changes_date_s;

    if(iter_a == NULL || iter_b == NULL) {
        return 0;
    }

    gtk_tree_model_get (model, iter_a, N_COLUMN_NAME, &name_a,
                        N_COLUMN_LAST_CHANGES_DATE_JULIAN, &last_changes_date_a, N_COLUMN_CATEGORY, &category_a, -1);
    gtk_tree_model_get (model, iter_b, N_COLUMN_NAME, &name_b,
                        N_COLUMN_LAST_CHANGES_DATE_JULIAN, &last_changes_date_b, N_COLUMN_CATEGORY, &category_b, -1);


    if (name_a == NULL) {
        name_s = -1;
    } else if (name_b == NULL) {
        name_s = 1;
    } else if (name_a != NULL && name_b != NULL) {
        name_s = g_utf8_collate(name_a, name_b);
    } else {
        name_s = 0;
    }
    if (name_a != NULL) {
        g_free(name_a);
    }
    if (name_b != NULL) {
        g_free(name_b);
    }

    if (category_a == NULL) {
        category_s = -1;
    } else if (category_b == NULL) {
        category_s = 1;
    } else if (category_a != NULL && category_b != NULL) {
        category_s = g_utf8_collate(category_a, category_b);
    } else {
        category_s = 0;
    }
    if (category_a != NULL) {
        g_free(category_a);
    }
    if (category_b != NULL) {
        g_free(category_b);
    }

    if (!last_changes_date_a) last_changes_date_a = 1 << 31;
    if (!last_changes_date_b) last_changes_date_b = 1 << 31;
    last_changes_date_s = last_changes_date_a - last_changes_date_b;

    switch(config.notes_sorting_mode) {

        /* Name, Last changes, Category */
        case 0:
            if (name_s != 0)
                return name_s;
            if (last_changes_date_s != 0)
                return last_changes_date_s;
            if (category_s != 0)
                return category_s;
            break;

        /* Name, Category, Last changes */
        case 1:
            if (name_s != 0)
                return name_s;
            if (category_s != 0)
                return category_s;
            if (last_changes_date_s != 0)
                return last_changes_date_s;
            break;

        /* Category, Last changes, Name */
        case 2:
            if (category_s != 0)
                return category_s;
            if (last_changes_date_s != 0)
                return last_changes_date_s;
            if (name_s != 0)
                return name_s;
            break;

        /* Category, Name, Last changes */
        case 3:
            if (category_s != 0)
                return category_s;
            if (name_s != 0)
                return name_s;
            if (last_changes_date_s != 0)
                return last_changes_date_s;
            break;

        /* Last changes, Category, Name */
        case 4:
            if (last_changes_date_s != 0)
                return last_changes_date_s;
            if (category_s != 0)
                return category_s;
            if (name_s != 0)
                return name_s;
            break;

        /* Last changes, Name, Category */
        case 5:
            if (last_changes_date_s != 0)
                return last_changes_date_s;
            if (name_s != 0)
                return name_s;
            if (category_s != 0)
                return category_s;
            break;

        default:
            break;
    }

    return 0;
}

/*------------------------------------------------------------------------------*/

#ifdef HAVE_GTKSPELL
void
notes_spell_check_cb (GtkToggleToolButton *toggle_tool_button, gpointer data) {

    GUI *appGUI = (GUI *)data;
    GtkSpell *edSpell;

	if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (toggle_tool_button)) == TRUE) {
        edSpell = gtkspell_new_attach (GTK_TEXT_VIEW(appGUI->nte->editor_textview), NULL, NULL);
		if (config.override_locale_settings == TRUE) {
	        gtkspell_set_language (edSpell, config.spell_lang, NULL);
		} else {
	        gtkspell_set_language (edSpell, g_getenv ("LANG"), NULL);
		}
    } else {
        gtkspell_detach (gtkspell_get_from_text_view (GTK_TEXT_VIEW(appGUI->nte->editor_textview)));
    }
}
#endif /* HAVE_GTKSPELL */

/*------------------------------------------------------------------------------*/

void
store_note_columns_info (GUI *appGUI) {

	gint n;

	config.notes_column_idx_0 = utl_gui_get_column_position (appGUI->nte->notes_columns[N_COLUMN_TYPE], 
															 GTK_TREE_VIEW(appGUI->nte->notes_list), MAX_VISIBLE_NOTE_COLUMNS, appGUI);
	config.notes_column_idx_1 = utl_gui_get_column_position (appGUI->nte->notes_columns[N_COLUMN_NAME], 
															 GTK_TREE_VIEW(appGUI->nte->notes_list), MAX_VISIBLE_NOTE_COLUMNS, appGUI);
	config.notes_column_idx_2 = utl_gui_get_column_position (appGUI->nte->notes_columns[N_COLUMN_CATEGORY], 
															 GTK_TREE_VIEW(appGUI->nte->notes_list), MAX_VISIBLE_NOTE_COLUMNS, appGUI);
	config.notes_column_idx_3 = utl_gui_get_column_position (appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE], 
															 GTK_TREE_VIEW(appGUI->nte->notes_list), MAX_VISIBLE_NOTE_COLUMNS, appGUI);
	config.notes_column_idx_4 = utl_gui_get_column_position (appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE], 
															 GTK_TREE_VIEW(appGUI->nte->notes_list), MAX_VISIBLE_NOTE_COLUMNS, appGUI);

	n = gtk_tree_view_column_get_width (gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 0));
	if (n > 1) {
		config.notes_column_idx_0_width = n;
	}
	n = gtk_tree_view_column_get_width (gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 1));
	if (n > 1) {
		config.notes_column_idx_1_width = n;
	}
	n = gtk_tree_view_column_get_width (gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 2));
	if (n > 1) {
		config.notes_column_idx_2_width = n;
	}
	n = gtk_tree_view_column_get_width (gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 3));
	if (n > 1) {
		config.notes_column_idx_3_width = n;
	}
	n = gtk_tree_view_column_get_width (gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 4));
	if (n > 1) {
		config.notes_column_idx_4_width = n;
	}
}

/*------------------------------------------------------------------------------*/

void
set_note_columns_width (GUI *appGUI) {

GtkTreeViewColumn   *col;
gint w;

	w = 2 * utl_gui_get_sw_vscrollbar_width (appGUI->nte->scrolled_win);

	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 0);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.notes_column_idx_0_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.notes_column_idx_0_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 1);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.notes_column_idx_1_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.notes_column_idx_1_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 2);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.notes_column_idx_2_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.notes_column_idx_2_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 3);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.notes_column_idx_3_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.notes_column_idx_3_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->nte->notes_list), 4);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.notes_column_idx_4_width > w) {
		gtk_tree_view_column_set_fixed_width (col, config.notes_column_idx_4_width - w);
	}
}

/*------------------------------------------------------------------------------*/

void
editor_cursor_move_cb (GtkTextBuffer *buffer, GtkTextIter *new_location, 
					   GtkTextMark *mark, gpointer user_data)
{
GtkTextIter iter;
gint row, col, nrow, ncol;
gchar tmpbuf[BUFFER_SIZE];

	GUI *appGUI = (GUI *)user_data;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->nte->readonly_checkbutton))) {
		return;
	}

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	row = gtk_text_iter_get_line (&iter) + 1;
	nrow = gtk_text_buffer_get_line_count (buffer);
    sprintf(tmpbuf, "<tt>%d/%d</tt>", row, nrow);
    gtk_label_set_markup (GTK_LABEL (appGUI->nte->nrow_label), tmpbuf);

	col = gtk_text_iter_get_line_offset (&iter) + 1;
	ncol = gtk_text_iter_get_chars_in_line (&iter);
	if (ncol < col) ncol = col;
    sprintf(tmpbuf, "<tt>%d/%d</tt>", col, ncol);
    gtk_label_set_markup (GTK_LABEL (appGUI->nte->ncol_label), tmpbuf);

}

/*------------------------------------------------------------------------------*/

void
readonly_toggle_cb (GtkToggleButton *togglebutton, gpointer data) {

    GUI *appGUI = (GUI *)data;

	gchar *btnlist[] = {
		"/toolbar/bold", "/toolbar/italic", "/toolbar/underline", "/toolbar/strike",
		"/toolbar/insert_date_time", "/toolbar/insert_separator", "/toolbar/clear", "/toolbar/mark_color"
	};

	guint nbtns = G_N_ELEMENTS (btnlist), i;

	gboolean s = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->nte->readonly_checkbutton));

	gtk_text_view_set_editable (GTK_TEXT_VIEW (appGUI->nte->editor_textview), !s);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (appGUI->nte->editor_textview), !s);
	gtk_widget_set_sensitive (appGUI->nte->font_picker, !s);

	if (s == TRUE) {
		gtk_label_set_text(GTK_LABEL(appGUI->nte->nrow_label), "-");
		gtk_label_set_text(GTK_LABEL(appGUI->nte->ncol_label), "-");
	} else {
		editor_cursor_move_cb (gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview)),
							   NULL, NULL, appGUI);
	}

	appGUI->nte->note_read_only = s;

	for (i=0; i < nbtns; i++) { 
		gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, btnlist[i]), !s);
	}

}

/*------------------------------------------------------------------------------*/

gboolean
notes_search_entry_changed_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

GtkTreePath *path;
GtkTreeIter iter;
gint i;

    GUI *appGUI = (GUI *)user_data;

    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter));

    if(strlen(gtk_entry_get_text (GTK_ENTRY(appGUI->nte->notes_find_entry)))) {

		i = 0;
		while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->nte->notes_filter), &iter, NULL, i++));

		if (i-1 != 0) {

			path = gtk_tree_path_new_first();
			if (path != NULL) {
				gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->nte->notes_list), path, NULL, FALSE);
				gtk_tree_path_free (path);
			}
		} 
    }
		
	update_notes_items (appGUI);

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
nte_clear_find_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
    if (strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->nte->notes_find_entry)))) {
        gtk_entry_set_text(GTK_ENTRY(appGUI->nte->notes_find_entry), "");
		notes_search_entry_changed_cb (NULL, NULL, appGUI);
	}
}

/*------------------------------------------------------------------------------*/

void
nte_font_selected (GtkFontButton *widget, gpointer user_data) {

	GUI *appGUI = (GUI *) user_data;

    pango_font_description_free(appGUI->nte->fd_notes_font);
    appGUI->nte->fd_notes_font = pango_font_description_from_string(gtk_font_button_get_font_name (GTK_FONT_BUTTON(widget)));
    gtk_widget_modify_font (GTK_WIDGET(appGUI->nte->editor_textview), appGUI->nte->fd_notes_font);

}

/*------------------------------------------------------------------------------*/

void 
gui_create_notes (GUI *appGUI) {

GtkWidget           *vbox1;
GtkWidget           *hbox1;
GtkTextBuffer       *buffer;
GtkWidget           *viewport;
GError              *error = NULL;
GtkActionGroup      *action_group_selector = NULL;
GtkActionGroup      *action_group_editor = NULL;
GtkCellRenderer     *renderer;
GtkWidget           *hseparator;
GtkWidget           *label;
GtkWidget           *vseparator;
GtkWidget           *close_button;
GtkWidget           *table;
gint i, n;

gchar tmpbuf[BUFFER_SIZE];

 const gchar *ui_info_selector =
"  <toolbar name=\"toolbar\">\n"
"    <toolitem name=\"new\" action=\"new\" />\n"
"    <toolitem name=\"edit\" action=\"edit\" />\n"
"    <separator/>\n"
"    <toolitem name=\"delete\" action=\"delete\" />\n"
"    <separator expand=\"true\" />\n"
"    <toolitem name=\"preferences\" action=\"preferences\" />\n"
"    <toolitem name=\"about\" action=\"about\" />\n"
"  </toolbar>\n";

GtkActionEntry entries_selector[] = {
	{ "new", OSMO_STOCK_NOTES_ADD, _("New note"), NULL, _("Add note"), G_CALLBACK(notes_add_item_cb)},
	{ "edit", OSMO_STOCK_NOTES_EDIT, _("Edit note"), NULL, _("Edit note"), G_CALLBACK(notes_edit_item_cb)},
	{ "delete", OSMO_STOCK_NOTES_REMOVE, _("Delete note"), NULL, _("Remove note"), G_CALLBACK(notes_delete_item_cb)},
	{ "preferences", OSMO_STOCK_PREFERENCES, _("Preferences"), NULL, _("Preferences"), G_CALLBACK (show_preferences_window_cb)},
	{ "about", OSMO_STOCK_ABOUT, _("About"), NULL, _("About"), G_CALLBACK (show_about_window_cb)},
};

guint n_entries_selector = G_N_ELEMENTS (entries_selector);

const gchar *ui_info_editor =
"  <toolbar name=\"toolbar\">\n"
"    <toolitem name=\"save\" action=\"save\" />\n"
"    <toolitem name=\"find\" action=\"find\" />\n"
#ifdef HAVE_GTKSPELL
"    <toolitem name=\"spell_check\" action=\"spell_check\" />\n"
#endif /* HAVE_GTKSPELL */
"    <separator/>\n"
"    <toolitem name=\"bold\" action=\"bold\" />\n"
"    <toolitem name=\"italic\" action=\"italic\" />\n"
"    <toolitem name=\"underline\" action=\"underline\" />\n"
"    <toolitem name=\"strike\" action=\"strike\" />\n"
"    <toolitem name=\"mark_color\" action=\"mark_color\" />\n"
"    <toolitem name=\"clear\" action=\"clear\" />\n"
"    <separator/>\n"
"    <toolitem name=\"insert_date_time\" action=\"insert_date_time\" />\n"
"    <toolitem name=\"insert_separator\" action=\"insert_separator\" />\n"
"    <separator/>\n"
"    <toolitem name=\"text_info\" action=\"text_info\" />\n"
"    <separator expand=\"true\" />\n"
"    <toolitem name=\"close\" action=\"close\" />\n"
"  </toolbar>\n";

GtkActionEntry entries_editor[] = {
  { "find", OSMO_STOCK_EDITOR_FIND, _("Find"), NULL, _("Find"), G_CALLBACK (editor_find_text_show_cb) },
  { "bold", OSMO_STOCK_EDITOR_BOLD, _("Bold"), NULL, _("Bold"), NULL },
  { "italic", OSMO_STOCK_EDITOR_ITALIC, _("Italic"), NULL, _("Italic"), NULL },
  { "underline", OSMO_STOCK_EDITOR_UNDERLINE, _("Underline"), NULL, _("Underline"), NULL },
  { "strike", OSMO_STOCK_EDITOR_STRIKETHROUGH, _("Strikethrough"), NULL, _("Strikethrough"), NULL },
  { "mark_color", OSMO_STOCK_EDITOR_HIGHLIGHT, _("Highlight"), NULL, _("Highlight"), NULL },
  { "clear", OSMO_STOCK_EDITOR_CLEAR, _("Clear attributes"), NULL, _("Clear attributes"), G_CALLBACK (clear_text_attributes_cb)},
  { "save", OSMO_STOCK_EDITOR_SAVE, _("Save note"), NULL, _("Save note"), G_CALLBACK (editor_save_buffer_cb)},
  { "insert_date_time", OSMO_STOCK_EDITOR_INSERT_DATE_TIME, _("Insert current date and time"), NULL, _("Insert current date and time"), G_CALLBACK(insert_current_date_and_time_cb)},
  { "insert_separator", OSMO_STOCK_EDITOR_INSERT_SEPARATOR, _("Insert separator"), NULL, _("Insert separator"), G_CALLBACK(insert_separator_cb)},
  { "text_info", OSMO_STOCK_EDITOR_INFO, _("Statistics"), NULL, _("Statistics"), G_CALLBACK(text_info_cb)},
  { "close", OSMO_STOCK_CLOSE, _("Close editor"), NULL, _("Close editor"), G_CALLBACK (editor_close_cb)},
};

guint n_entries_editor = G_N_ELEMENTS (entries_editor);

gint columns_order[MAX_VISIBLE_NOTE_COLUMNS];

gint nt_columns[MAX_VISIBLE_NOTE_COLUMNS] = { 
		N_COLUMN_TYPE, N_COLUMN_NAME, N_COLUMN_CATEGORY, N_COLUMN_LAST_CHANGES_DATE, N_COLUMN_CREATE_DATE
};

#ifdef HAVE_GTKSPELL

GtkToggleActionEntry t_entries_editor[] = {
    { "spell_check", OSMO_STOCK_EDITOR_SPELL_CHECKER, _("Toggle spell checker"), NULL, _("Toggle spell checker"), NULL, FALSE }
};

guint n_t_entries_editor = G_N_ELEMENTS (t_entries_editor);

#endif /* HAVE_GTKSPELL */

    vbox1 = gtk_vbox_new (FALSE, 1);
    gtk_widget_show (vbox1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox1), 8);
    sprintf(tmpbuf, "<b>%s</b>", _("Notes"));
    gui_add_to_notebook (vbox1, tmpbuf, appGUI);

    appGUI->nte->vbox = GTK_BOX(vbox1);

    if (config.hide_notes == TRUE) {
        gtk_widget_hide(GTK_WIDGET(appGUI->nte->vbox));
    }

    appGUI->nte->vbox_selector = gtk_vbox_new (FALSE, 1);
    gtk_widget_show (appGUI->nte->vbox_selector);
    gtk_box_pack_start (GTK_BOX(appGUI->nte->vbox), appGUI->nte->vbox_selector, TRUE, TRUE, 0);
    appGUI->nte->vbox_editor = gtk_vbox_new (FALSE, 1);
    gtk_widget_show (appGUI->nte->vbox_editor);
    gtk_box_pack_start (GTK_BOX(appGUI->nte->vbox), appGUI->nte->vbox_editor, TRUE, TRUE, 0);

    /*-------------------------------------------------------------------------------------*/

    action_group_selector = gtk_action_group_new ("_actions");
    gtk_action_group_add_actions (action_group_selector, entries_selector, n_entries_selector, appGUI);
    gtk_action_group_set_sensitive(action_group_selector, TRUE);

    appGUI->nte->notes_uim_selector_widget = gtk_ui_manager_new ();

    gtk_ui_manager_insert_action_group (appGUI->nte->notes_uim_selector_widget, action_group_selector, 0);
    g_signal_connect (appGUI->nte->notes_uim_selector_widget, "add_widget", 
                      G_CALLBACK (add_notes_toolbar_selector_widget), appGUI);

    if (!gtk_ui_manager_add_ui_from_string (appGUI->nte->notes_uim_selector_widget, ui_info_selector, -1, &error)) {
        g_message ("building toolbar failed: %s", error->message);
        g_error_free (error);
    }
    gtk_ui_manager_ensure_update (appGUI->nte->notes_uim_selector_widget);

    gtk_toolbar_set_style (appGUI->nte->notes_toolbar_selector, GTK_TOOLBAR_ICONS);

    /*-------------------------------------------------------------------------------------*/

    action_group_editor = gtk_action_group_new ("_actions");
    gtk_action_group_add_actions (action_group_editor, entries_editor, n_entries_editor, appGUI);
#ifdef HAVE_GTKSPELL
    gtk_action_group_add_toggle_actions (action_group_editor, t_entries_editor, n_t_entries_editor, appGUI);
#endif /* HAVE_GTKSPELL */
    gtk_action_group_set_sensitive(action_group_editor, TRUE);

    appGUI->nte->notes_uim_editor_widget = gtk_ui_manager_new ();

    gtk_ui_manager_insert_action_group (appGUI->nte->notes_uim_editor_widget, action_group_editor, 0);
    g_signal_connect (appGUI->nte->notes_uim_editor_widget, "add_widget", 
                      G_CALLBACK (add_notes_toolbar_editor_widget), appGUI);

    if (!gtk_ui_manager_add_ui_from_string (appGUI->nte->notes_uim_editor_widget, ui_info_editor, -1, &error)) {
        g_message ("building toolbar failed: %s", error->message);
        g_error_free (error);
    }
    gtk_ui_manager_ensure_update (appGUI->nte->notes_uim_editor_widget);

    gtk_toolbar_set_style (appGUI->nte->notes_toolbar_editor, GTK_TOOLBAR_ICONS);

    /*-------------------------------------------------------------------------------------*/

    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_selector_widget, "/toolbar/edit"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_selector_widget, "/toolbar/delete"), FALSE);

    /* selector */

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_selector), hseparator, FALSE, TRUE, 6);

	if (!config.gui_layout) {   /* vertical */
	    table = gtk_table_new (2, 4, FALSE);
	} else {
	    table = gtk_table_new (1, 6, FALSE);
	}
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_selector), table, FALSE, TRUE, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);

    sprintf(tmpbuf, "<b>%s:</b>", _("Category"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    appGUI->nte->cf_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->nte->cf_combobox);
    gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (appGUI->nte->cf_combobox), FALSE);
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->cf_combobox, GTK_CAN_FOCUS);
    g_signal_connect(appGUI->nte->cf_combobox, "changed", 
                     G_CALLBACK(notes_category_filter_cb), appGUI);
    g_signal_connect(G_OBJECT(appGUI->nte->cf_combobox), "focus", 
                     G_CALLBACK(notes_category_combo_box_focus_cb), NULL);

    appGUI->nte->n_items_label = gtk_label_new ("");
    gtk_widget_show (appGUI->nte->n_items_label);
    if (appGUI->tiny_gui == FALSE) {
        gtk_widget_set_size_request (appGUI->nte->n_items_label, 100, -1);
    }
    gtk_label_set_use_markup (GTK_LABEL (appGUI->nte->n_items_label), TRUE);

	if (!config.gui_layout) {   /* vertical */

		gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);

		if (appGUI->tiny_gui == TRUE) {
			gtk_table_attach (GTK_TABLE (table), appGUI->nte->cf_combobox, 1, 2, 0, 1,
							 (GtkAttachOptions) (0),
							 (GtkAttachOptions) (0), 0, 0);
		} else {
			gtk_table_attach (GTK_TABLE (table), appGUI->nte->cf_combobox, 1, 2, 0, 1,
							 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
							 (GtkAttachOptions) (GTK_FILL), 0, 0);
		}

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->n_items_label, 3, 4, 0, 1,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);
	} else {

		gtk_table_attach (GTK_TABLE (table), label, 3, 4, 0, 1,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->cf_combobox, 4, 5, 0, 1,
						 (GtkAttachOptions) (0),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->n_items_label, 5, 6, 0, 1,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);
	}

	sprintf(tmpbuf, "<b>%s:</b>", _("Search"));
	label = gtk_label_new (tmpbuf);
	gtk_widget_show (label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	appGUI->nte->notes_find_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(appGUI->nte->notes_find_entry), 128);
	gtk_widget_show (appGUI->nte->notes_find_entry);
	g_signal_connect (G_OBJECT(appGUI->nte->notes_find_entry), "key_release_event",
					  G_CALLBACK(notes_search_entry_changed_cb), appGUI);

	appGUI->nte->notes_find_clear_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLEAR, FALSE);
	gtk_widget_show (appGUI->nte->notes_find_clear_button);
	GTK_WIDGET_UNSET_FLAGS (appGUI->nte->notes_find_clear_button, GTK_CAN_FOCUS);
	gtk_button_set_relief (GTK_BUTTON(appGUI->nte->notes_find_clear_button), GTK_RELIEF_NONE);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (appGUI->nte->notes_find_clear_button, _("Clear"));
	}
	g_signal_connect (G_OBJECT (appGUI->nte->notes_find_clear_button), "clicked",
						G_CALLBACK (nte_clear_find_cb), appGUI);

	if (!config.gui_layout) {   /* vertical */

		gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->notes_find_entry, 1, 5, 1, 2,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->notes_find_clear_button, 5, 6, 1, 2,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_set_row_spacings (GTK_TABLE (table), 4);

	} else {

		gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->notes_find_entry, 1, 2, 0, 1,
						 (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
						 (GtkAttachOptions) (0), 0, 0);

		gtk_table_attach (GTK_TABLE (table), appGUI->nte->notes_find_clear_button, 2, 3, 0, 1,
						 (GtkAttachOptions) (GTK_FILL),
						 (GtkAttachOptions) (0), 0, 0);
	}

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_selector), hseparator, FALSE, TRUE, 6);

    /*-------------------------------------------------------------------------------------*/

    viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (viewport);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_IN);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_selector), viewport, TRUE, TRUE, 0);

    appGUI->nte->scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (appGUI->nte->scrolled_win);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->nte->scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (viewport), appGUI->nte->scrolled_win);

    appGUI->nte->notes_list_store = gtk_list_store_new(NOTES_NUM_COLUMNS,
                                                       GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING,
                                                       G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
                                                       G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
                                                       G_TYPE_BOOLEAN, G_TYPE_UINT, G_TYPE_BOOLEAN, 
													   G_TYPE_STRING, G_TYPE_STRING);

    appGUI->nte->notes_filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(appGUI->nte->notes_list_store), NULL);
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER(appGUI->nte->notes_filter), 
                                            (GtkTreeModelFilterVisibleFunc)notes_list_filter_cb, 
                                            appGUI, NULL);

    appGUI->nte->notes_sort = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(appGUI->nte->notes_filter));

    appGUI->nte->notes_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(appGUI->nte->notes_sort));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(appGUI->nte->notes_list), config.rules_hint);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW(appGUI->nte->notes_list), FALSE);
    gtk_widget_show (appGUI->nte->notes_list);
    GTK_WIDGET_SET_FLAGS (appGUI->nte->notes_list, GTK_CAN_DEFAULT);
    gtk_container_add (GTK_CONTAINER (appGUI->nte->scrolled_win), appGUI->nte->notes_list);

    g_signal_connect(G_OBJECT(appGUI->nte->notes_list), "button_press_event",
                     G_CALLBACK(notes_list_dbclick_cb), appGUI);

    appGUI->nte->notes_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (appGUI->nte->notes_list));
    g_signal_connect(G_OBJECT(appGUI->nte->notes_list_selection), "changed",
                     G_CALLBACK(notes_item_selected), appGUI);

    /* create columns */

	renderer = gtk_cell_renderer_pixbuf_new();  /* icon */
    appGUI->nte->notes_columns[N_COLUMN_TYPE] = gtk_tree_view_column_new_with_attributes(_("Type"),
																						 renderer,
																						 "pixbuf", N_COLUMN_TYPE,
																						 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_TYPE]);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_TYPE], config.nte_visible_type_column);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_TYPE]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_TYPE]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_TYPE]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    appGUI->nte->notes_columns[N_COLUMN_NAME] = gtk_tree_view_column_new_with_attributes(_("Note name"), 
																						 renderer,
																						 "text", N_COLUMN_NAME,
																						 NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_NAME], TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_NAME]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_NAME]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_NAME]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_NAME]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_CATEGORY] = gtk_tree_view_column_new_with_attributes(_("Category"),
																							 renderer,
																							 "text", N_COLUMN_CATEGORY,
																							 NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_CATEGORY], config.nte_visible_category_column);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_CATEGORY]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_CATEGORY]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_CATEGORY]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_CATEGORY]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE] = gtk_tree_view_column_new_with_attributes(_("Last changes"),
																									  renderer,
																									  "text", N_COLUMN_LAST_CHANGES_DATE,
																									  NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE], config.nte_visible_last_changes_column);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE_JULIAN] = gtk_tree_view_column_new_with_attributes(NULL,
																											 renderer, 
																											 "text", N_COLUMN_LAST_CHANGES_DATE_JULIAN, 
																											 NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE_JULIAN], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE_JULIAN]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_TIME] = gtk_tree_view_column_new_with_attributes(NULL,
																									  renderer,
																									  "text", N_COLUMN_LAST_CHANGES_TIME,
																									  NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_TIME], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_TIME]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE] = gtk_tree_view_column_new_with_attributes(_("Created"),
																								renderer,
																								"text", N_COLUMN_CREATE_DATE,
																								NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE], config.nte_visible_created_column);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE]);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE]), TRUE);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE]), TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE]), GTK_TREE_VIEW_COLUMN_FIXED);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE_JULIAN] = gtk_tree_view_column_new_with_attributes(NULL,
																									   renderer,
																									   "text", N_COLUMN_CREATE_DATE_JULIAN,
																									   NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE_JULIAN], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE_JULIAN]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_CREATE_TIME] = gtk_tree_view_column_new_with_attributes(NULL,
																								renderer, 
																								"text", N_COLUMN_CREATE_TIME,
																								NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_CREATE_TIME], FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_CREATE_TIME]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_REMEMBER_EDITOR_LINE] = gtk_tree_view_column_new_with_attributes(NULL,
																										 renderer,
																										 "text", N_COLUMN_REMEMBER_EDITOR_LINE,
																										 NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_REMEMBER_EDITOR_LINE], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_REMEMBER_EDITOR_LINE]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_EDITOR_LINE] = gtk_tree_view_column_new_with_attributes(NULL,
																								renderer, 
																								"text", N_COLUMN_EDITOR_LINE,
																								NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_EDITOR_LINE], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_EDITOR_LINE]);

	renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_EDITOR_READONLY] = gtk_tree_view_column_new_with_attributes(NULL,
																								    renderer, 
																								    "text", N_COLUMN_EDITOR_READONLY,
																								    NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_EDITOR_READONLY], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_EDITOR_READONLY]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_FILENAME] = gtk_tree_view_column_new_with_attributes(NULL,
																							 renderer,
																							 "text", N_COLUMN_FILENAME,
																							 NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_FILENAME], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_FILENAME]);

    renderer = gtk_cell_renderer_text_new();
    appGUI->nte->notes_columns[N_COLUMN_FONTNAME] = gtk_tree_view_column_new_with_attributes(NULL,
																							 renderer,
																							 "text", N_COLUMN_FONTNAME,
																							 NULL);
    gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_FONTNAME], FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->nte->notes_list), appGUI->nte->notes_columns[N_COLUMN_FONTNAME]);

	/* restore columns order */

	columns_order[0] = config.notes_column_idx_0;
	columns_order[1] = config.notes_column_idx_1;
	columns_order[2] = config.notes_column_idx_2;
	columns_order[3] = config.notes_column_idx_3;
	columns_order[4] = config.notes_column_idx_4;

	n = MAX_VISIBLE_NOTE_COLUMNS-1;

	while (n >= 0) {
		for (i = 0; i < MAX_VISIBLE_NOTE_COLUMNS; i++) {
			if (n == columns_order[i]) {
				gtk_tree_view_move_column_after(GTK_TREE_VIEW(appGUI->nte->notes_list),
												appGUI->nte->notes_columns[nt_columns[i]], NULL);
				n--;
			}
		}
	}

	set_note_columns_width (appGUI);

    /* configure sorting */

    gtk_tree_sortable_set_sort_func ((GtkTreeSortable *)appGUI->nte->notes_sort, N_COLUMN_NAME, 
                                     (GtkTreeIterCompareFunc)custom_notes_sort_function, NULL, NULL);
    gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *)appGUI->nte->notes_sort, N_COLUMN_NAME, 
                                          config.notes_sorting_order);

    /*-------------------------------------------------------------------------------------*/
    /* editor */

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_editor), hseparator, FALSE, TRUE, 4);

    appGUI->nte->title_label = gtk_label_new ("");
    gtk_widget_show (appGUI->nte->title_label);
    gtk_label_set_ellipsize (GTK_LABEL(appGUI->nte->title_label), PANGO_ELLIPSIZE_END);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_editor), appGUI->nte->title_label, FALSE, FALSE, 0);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_editor), hseparator, FALSE, TRUE, 4);

    viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (viewport);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_IN);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_editor), viewport, TRUE, TRUE, 0);
  
    appGUI->nte->editor_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (appGUI->nte->editor_scrolledwindow);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->nte->editor_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (viewport), appGUI->nte->editor_scrolledwindow);

    appGUI->nte->editor_textview = gtk_text_view_new ();
    gtk_widget_show (appGUI->nte->editor_textview);
    gtk_container_add (GTK_CONTAINER (appGUI->nte->editor_scrolledwindow), appGUI->nte->editor_textview);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->nte->editor_textview), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(appGUI->nte->editor_textview), 4);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(appGUI->nte->editor_textview), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(appGUI->nte->editor_textview), 4);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (appGUI->nte->editor_textview));
    appGUI->nte->buffer_check_modify_enable = FALSE;
    g_signal_connect (G_OBJECT (buffer), "changed", G_CALLBACK (text_buffer_modified_cb), appGUI);
	g_signal_connect (G_OBJECT (buffer), "mark_set", G_CALLBACK(editor_cursor_move_cb), appGUI);

    gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    g_object_set_data (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/bold")), 
                       "tag", "bold");
    g_signal_connect (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/bold")), 
                      "clicked", G_CALLBACK (set_text_attribute_cb), appGUI);

    gtk_text_buffer_create_tag (buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
    g_object_set_data (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/italic")), 
                       "tag", "italic");
    g_signal_connect (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/italic")), 
                      "clicked", G_CALLBACK (set_text_attribute_cb), appGUI);

    gtk_text_buffer_create_tag (buffer, "strike", "strikethrough", TRUE, NULL);
    g_object_set_data (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/strike")), 
                       "tag", "strike");
    g_signal_connect (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/strike")), 
                      "clicked", G_CALLBACK (set_text_attribute_cb), appGUI);

    gtk_text_buffer_create_tag (buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
    g_object_set_data (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/underline")), 
                       "tag", "underline");
    g_signal_connect (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/underline")), 
                      "clicked", G_CALLBACK (set_text_attribute_cb), appGUI);

    gtk_text_buffer_create_tag (buffer, "mark_color", "background", "#FFFF00", NULL);
    g_object_set_data (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/mark_color")), 
                       "tag", "mark_color");
    g_signal_connect (G_OBJECT (gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/mark_color")), 
                      "clicked", G_CALLBACK (set_text_attribute_cb), appGUI);

#ifdef HAVE_GTKSPELL
    g_signal_connect (G_OBJECT(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/spell_check")), "toggled", 
                      G_CALLBACK(notes_spell_check_cb), appGUI);
#endif /* HAVE_GTKSPELL */

	hbox1 = gtk_hbox_new (FALSE, 1);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_editor), hbox1, FALSE, FALSE, 2);

    appGUI->nte->readonly_checkbutton = gtk_check_button_new_with_mnemonic (_("Read-only"));
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->readonly_checkbutton, GTK_CAN_FOCUS);
    gtk_widget_show (appGUI->nte->readonly_checkbutton);
    gtk_box_pack_end (GTK_BOX (hbox1), appGUI->nte->readonly_checkbutton, FALSE, FALSE, 4);
    g_signal_connect (GTK_TOGGLE_BUTTON(appGUI->nte->readonly_checkbutton), "toggled",
                      G_CALLBACK (readonly_toggle_cb), appGUI);

	appGUI->nte->font_picker = gtk_font_button_new ();
    gtk_widget_show (appGUI->nte->font_picker);
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->font_picker, GTK_CAN_FOCUS);
    g_signal_connect (GTK_FONT_BUTTON(appGUI->nte->font_picker), "font-set",
                      G_CALLBACK (nte_font_selected), appGUI);
    gtk_box_pack_end (GTK_BOX (hbox1), appGUI->nte->font_picker, FALSE, FALSE, 4);

    sprintf(tmpbuf, "<b>%s</b>:", _("Line"));
    appGUI->nte->nrow_label_t = gtk_label_new (tmpbuf);
    gtk_widget_show (appGUI->nte->nrow_label_t);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->nte->nrow_label_t, FALSE, FALSE, 0);
    gtk_label_set_use_markup (GTK_LABEL (appGUI->nte->nrow_label_t), TRUE);

    sprintf(tmpbuf, "<tt>%s</tt>", "1/1");
    appGUI->nte->nrow_label = gtk_label_new (tmpbuf);
    gtk_widget_show (appGUI->nte->nrow_label);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->nte->nrow_label, FALSE, FALSE, 8);
    gtk_label_set_use_markup (GTK_LABEL (appGUI->nte->nrow_label), TRUE);

    sprintf(tmpbuf, "<b>%s</b>:", _("Column"));
    appGUI->nte->ncol_label_t = gtk_label_new (tmpbuf);
    gtk_widget_show (appGUI->nte->ncol_label_t);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->nte->ncol_label_t, FALSE, FALSE, 8);
    gtk_label_set_use_markup (GTK_LABEL (appGUI->nte->ncol_label_t), TRUE);

    sprintf(tmpbuf, "<tt>%s</tt>", "1/1");
    appGUI->nte->ncol_label = gtk_label_new (tmpbuf);
    gtk_widget_show (appGUI->nte->ncol_label);
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->nte->ncol_label, FALSE, FALSE, 0);
    gtk_label_set_use_markup (GTK_LABEL (appGUI->nte->ncol_label), TRUE);

    appGUI->nte->find_hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (appGUI->nte->find_hbox);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->vbox_editor), appGUI->nte->find_hbox, FALSE, FALSE, 0);

    appGUI->nte->find_next_flag = FALSE;

    sprintf(tmpbuf, "<b>%s</b>:", _("Find"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->find_hbox), label, FALSE, FALSE, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    appGUI->nte->find_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->nte->find_entry);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->find_hbox), appGUI->nte->find_entry, TRUE, TRUE, 4);
    g_signal_connect (G_OBJECT (appGUI->nte->find_entry), "key_press_event",
                      G_CALLBACK (find_entry_key_press_cb), appGUI);

    appGUI->nte->find_case_checkbutton = gtk_check_button_new_with_mnemonic (_("case sensitive"));
    GTK_WIDGET_UNSET_FLAGS(appGUI->nte->find_case_checkbutton, GTK_CAN_FOCUS);
    gtk_widget_show (appGUI->nte->find_case_checkbutton);
    gtk_box_pack_start (GTK_BOX (appGUI->nte->find_hbox), appGUI->nte->find_case_checkbutton, FALSE, FALSE, 4);
    g_signal_connect (GTK_TOGGLE_BUTTON(appGUI->nte->find_case_checkbutton), "toggled",
                      G_CALLBACK (case_sensitive_toggle_cb), appGUI);


    if (config.default_stock_icons) {
        close_button = utl_gui_stock_button (GTK_STOCK_CLOSE, FALSE);
    } else {
        close_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLOSE, FALSE);
    }
    GTK_WIDGET_UNSET_FLAGS(close_button, GTK_CAN_FOCUS);
    gtk_button_set_relief (GTK_BUTTON(close_button), GTK_RELIEF_NONE);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (close_button, _("Close find entry"));
	}
    gtk_box_pack_end (GTK_BOX (appGUI->nte->find_hbox), close_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (close_button), "clicked",
                      G_CALLBACK (editor_find_text_hide_cb), appGUI);

    vseparator = gtk_vseparator_new ();
    gtk_widget_show (vseparator);
    gtk_box_pack_end (GTK_BOX (appGUI->nte->find_hbox), vseparator, FALSE, TRUE, 0);

    gtk_widget_hide (appGUI->nte->find_hbox);

    appGUI->nte->changed = FALSE;
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->nte->notes_uim_editor_widget, "/toolbar/save"), FALSE);

    /*-------------------------------------------------------------------------------------*/

    notes_show_selector_editor (SELECTOR, appGUI);

    appGUI->nte->filename = NULL;

#ifdef HAVE_LIBGRINGOTTS
    appGUI->nte->context = NULL;
    appGUI->nte->keyholder = NULL;
#endif /* HAVE_LIBGRINGOTTS */

}

/*------------------------------------------------------------------------------*/

gboolean
check_if_encrypted (gchar *filename, GUI *appGUI) {

gchar *contents;
gboolean result = FALSE;

	if (g_file_get_contents (notes_get_full_filename(filename, appGUI), &contents, NULL, NULL) == TRUE) {

		if (contents[0] == 'O' && contents[1] == 'S' && contents[2] == 'M' && g_ascii_isalnum(contents[3])) {
			result = TRUE;
		}

		g_free (contents);
	}

	return result;
}

/*------------------------------------------------------------------------------*/

void
check_notes_type (GUI *appGUI) {

GtkTreeIter iter;
gint i;
gchar *filename;
GdkPixbuf *image;
GtkWidget *helper;

    helper = gtk_image_new ();
	i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, NULL, i++)) {

        gtk_tree_model_get (GTK_TREE_MODEL(appGUI->nte->notes_list_store), &iter, 
                            N_COLUMN_FILENAME, &filename, -1);

		if (check_if_encrypted (filename, appGUI) == TRUE) {
			image = gtk_widget_render_icon (helper, OSMO_STOCK_TYPE_ENCRYPTED, GTK_ICON_SIZE_MENU, NULL);
		} else {
			image = gtk_widget_render_icon (helper, OSMO_STOCK_TYPE_NORMAL, GTK_ICON_SIZE_MENU, NULL);
		}

		g_free (filename);
		gtk_list_store_set (appGUI->nte->notes_list_store, &iter, N_COLUMN_TYPE, image, -1);
		g_object_unref (image);
	}
}

/*------------------------------------------------------------------------------*/

#endif  /* NOTES_ENABLED */

