
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
#include "contacts.h"
#include "i18n.h"
#include "contacts_items.h"
#include "utils.h"
#include "utils_date.h"
#include "utils_gui.h"
#include "options_prefs.h"
#include "preferences_gui.h"
#include "stock_icons.h"
#include "contacts_birthdays.h"
#include "contacts_import.h"
#include "contacts_export.h"
#include "calendar_utils.h"

#ifdef CONTACTS_ENABLED

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

	gint page = gtk_notebook_page_num (GTK_NOTEBOOK (appGUI->opt->notebook), appGUI->opt->contacts);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (appGUI->opt->notebook), page);
}

/*------------------------------------------------------------------------------*/

void
set_export_active (GUI *appGUI) {

GtkTreeIter iter;
guint n;
gboolean state;

    n = 0;
    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->cnt->contacts_filter), &iter, NULL, n++));
    state = (n == 1) ? FALSE : TRUE;
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/export"), state);
}

/*------------------------------------------------------------------------------*/

void
show_contacts_desc_panel (gboolean enable, GUI *appGUI) {

GtkTreeIter  iter;
GdkRectangle rect, visible_rect;
GtkTreePath  *visible_path;
GtkTreeModel *model;
GtkAdjustment *adj;

    if(enable == TRUE) {

        if (gtk_tree_selection_get_selected (appGUI->cnt->contacts_list_selection, &model, &iter)) {

            gtk_paned_set_position(GTK_PANED(appGUI->cnt->contacts_paned), config.contacts_pane_pos);

            while (g_main_context_iteration(NULL, FALSE));

            visible_path = gtk_tree_model_get_path (model, &iter);

            if (visible_path) {

                gtk_tree_view_get_cell_area (GTK_TREE_VIEW (appGUI->cnt->contacts_list), visible_path, NULL, &rect);
                gtk_tree_view_get_visible_rect (GTK_TREE_VIEW (appGUI->cnt->contacts_list), &visible_rect);

                if (rect.y < visible_rect.y || rect.y > visible_rect.y + visible_rect.height) {
                        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (appGUI->cnt->contacts_list), visible_path, NULL, TRUE, 0.5, 0.0);
                }

                gtk_tree_path_free(visible_path);
            }

        } else {
            enable = FALSE;
        }

    } else {

        config.contacts_pane_pos = gtk_paned_get_position(GTK_PANED(appGUI->cnt->contacts_paned));

		if (!config.gui_layout) {
			gtk_paned_set_position(GTK_PANED(appGUI->cnt->contacts_paned), 99999);

			adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(appGUI->cnt->contacts_panel_scrolledwindow));
			gtk_adjustment_set_value (adj, 0.0);
			gtk_adjustment_value_changed (adj);
		}

    }

    appGUI->cnt->contacts_panel_status = enable;
}

/*------------------------------------------------------------------------------*/

gboolean
find_combo_box_focus_cb (GtkWidget *widget, GtkDirectionType *arg1, gpointer user_data) {
    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
contacts_panel_close_desc_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    show_contacts_desc_panel(FALSE, appGUI);
}

/*------------------------------------------------------------------------------*/

gboolean
contacts_list_filter_cb (GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {

const gchar *text;
gchar *value = NULL;
gint i, j, len, text_len;
guint32 date;

    GUI *appGUI = (GUI *)data;

    if (appGUI->cnt->contacts_filter_disabled == TRUE) {
        return TRUE;
    }

    text = gtk_entry_get_text(GTK_ENTRY(appGUI->cnt->contacts_find_entry));

    if (text == NULL) {
        return TRUE;
    }

    text_len = strlen(text);

    if (text_len) {

        if(config.find_mode == CONTACTS_FF_FIRST_NAME) {

            gtk_tree_model_get(model, iter, COLUMN_FIRST_NAME, &value, -1);
            if(value != NULL) {
                if (strlen(value)) {
                    if(g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value, -1), text_len)) {
						g_free (value);
                        return FALSE;
					}
                }
            } else {
                return FALSE;
            }

        } else if (config.find_mode == CONTACTS_FF_LAST_NAME) {

            gtk_tree_model_get(model, iter, COLUMN_LAST_NAME, &value, -1);
            if(value != NULL) {
                if(strlen(value)) {
                    if(g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value, -1), text_len)) {
						g_free (value);
                        return FALSE;
					}
                }
            } else {
                return FALSE;
            }

        } else if (config.find_mode == CONTACTS_FF_TAGS) {

			gtk_tree_model_get(model, iter, COLUMN_TAGS, &value, -1);

			if (value != NULL) {

				len = strlen(value) - text_len;

				if (len >= 0) {
					for(j=0; j <= len; j++) {
						if(!g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value+j, -1), text_len)) {
							g_free (value);
							return TRUE;
						}
					}
				}

				g_free(value);
			}

			return FALSE;

		} else if (config.find_mode == CONTACTS_FF_ALL_FIELDS) {

            for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {

                if (i != COLUMN_PHOTO && i != COLUMN_ID) {
                    if (i == COLUMN_BIRTH_DAY_DATE || i == COLUMN_NAME_DAY_DATE) {
                        gtk_tree_model_get (model, iter, i, &date, -1);
                        if (date == 0) {
                            value = NULL;
                        } else {
                            if (i == COLUMN_BIRTH_DAY_DATE) {
                                value = g_strdup((const gchar *)julian_to_str(date, DATE_FULL, config.override_locale_settings));
                            } else {
                                value = g_strdup((const gchar *)julian_to_str(date, DATE_NAME_DAY, config.override_locale_settings));
                            }
                        }
                    } else {
                        gtk_tree_model_get(model, iter, i, &value, -1);
                    }

                    if (value != NULL) {

                        len = strlen(value) - text_len;

                        if (len >= 0) {
                            for(j=0; j <= len; j++) {
                                if(!g_ascii_strncasecmp(g_utf8_casefold(text, -1), g_utf8_casefold(value+j, -1), text_len)) {
									g_free (value);
                                    return TRUE;
								}
                            }
                        }

                        g_free(value);
                    }
                }
            }

            return FALSE;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
contacts_select_first_position_in_list (GUI *appGUI) {

GtkTreeIter     iter;
GtkTreePath     *path;

    /* set cursor at first position */
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter) == TRUE) {
        path = gtk_tree_model_get_path (GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), &iter);
        if (path != NULL) {
            gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->cnt->contacts_list), path, NULL, FALSE);
            gtk_tree_path_free (path);
        }
    }
}

/*------------------------------------------------------------------------------*/

void
contacts_item_selected_cb (GtkTreeSelection *selection, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean address_available = FALSE, additional_info = FALSE;
	guint32 date;
	gchar *fname, *sname, *lname;
	gchar tmpbuf[BUFFER_SIZE], htmpbuf[BUFFER_SIZE];
	gchar *html, *html_buffer;
	gchar *text;
	gint i;

	if (gtk_tree_selection_get_selected (selection, &model, &iter) == FALSE) {

		gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/edit"), FALSE);
		gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/delete"), FALSE);
		gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/map_location"), FALSE);
		set_export_active (appGUI);

		return;
	}

	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/edit"), TRUE);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/delete"), TRUE);

	address_available = check_address (HOME_ADDRESS, appGUI);
	if (address_available == FALSE) {
		address_available = check_address (WORK_ADDRESS, appGUI);
	}

	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/map_location"), address_available);
	set_export_active (appGUI);

	/****************************************************************/

	html = g_strdup_printf (
	"<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
	"<style type=\"text/css\">\n"
	"body { font-size: %dpx; }\n"
/*	"h1 { font-size: 24px; font-weight: bold; text-transform: capitalize; font-style: italic; }\n"*/
	"h1 { font-size: %dpx; font-weight: bold; font-style: italic; }\n"
	/*"p { white-space: pre; margin-top: 0; margin-left: 2px; margin-bottom: 2px; font-family: monospace; }\n"*/
	"pre { white-space: pre; margin-top: 0; margin-left: 2px; margin-bottom: 2px; }\n"
	/*"pre { white-space: pre-wrap; white-space: -moz-pre-wrap !important; white-space: -pre-wrap; white-space: -o-pre-wrap; }\n"*/
	"a { color: %s; }\n"
	"table { width: 100%%; }\n"
	"td.tag { width: 30%%; }\n"
	"td.value { width: 70%%; }\n"
/*	".tag { text-transform: uppercase; font-weight: bold; color: %s; }\n"*/
	".tag { font-weight: bold; color: %s; }\n"
	".value { color: #000; }\n"
	".photo { position: absolute; top: 0; right: 0; width: %dpx; border: 1px solid #000; float: right; }\n"
	"</style>\n</head>\n\n<body>\n", 
	config.contact_item_font_size, config.contact_name_font_size,
	config.contact_link_color, config.contact_tag_color, config.photo_width);

	/****************************************************************/

	gtk_tree_model_get (model, &iter,
	                    COLUMN_FIRST_NAME, &fname,
	                    COLUMN_SECOND_NAME, &sname,
	                    COLUMN_LAST_NAME, &lname,
	                    COLUMN_PHOTO, &text,
	                    -1);

	if (!fname) fname = g_strdup ("");
	if (!sname) sname = g_strdup ("");
	if (!lname) lname = g_strdup ("");

	g_snprintf (htmpbuf, BUFFER_SIZE, "<h1>%s %s %s</h1>\n", fname, sname, lname);
	html_buffer = html;
	html = g_strconcat (html_buffer, htmpbuf, NULL);
	g_free (html_buffer);
	g_free (fname);
	g_free (sname);
	g_free (lname);

	/* insert photo */
	if (text != NULL) {
		g_snprintf (htmpbuf, BUFFER_SIZE, "<img src=\"%s\" alt=\"\" class=\"photo\">", text);
		html_buffer = html;
		html = g_strconcat (html_buffer, htmpbuf, NULL);
		g_free (html_buffer);
		g_free (text);
	}

	g_snprintf (htmpbuf, BUFFER_SIZE, "<table>\n");
	html_buffer = html;
	html = g_strconcat (html_buffer, htmpbuf, NULL);
	g_free (html_buffer);

	for (i = 0; i < CONTACTS_NUM_COLUMNS; i++) {

		if ((i == COLUMN_ID) ||
		    (i == COLUMN_PHOTO) ||
		    (i == COLUMN_FIRST_NAME) ||
		    (i == COLUMN_SECOND_NAME) ||
		    (i == COLUMN_LAST_NAME) ||
		    (i == COLUMN_GROUP))
			continue;

		if (i == COLUMN_BIRTH_DAY_DATE || i == COLUMN_NAME_DAY_DATE) {
			gtk_tree_model_get (model, &iter, i, &date, -1);
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
			gtk_tree_model_get (model, &iter, i, &text, -1);
		}

		if (text == NULL || !strlen (text)) continue;

		g_snprintf (htmpbuf, BUFFER_SIZE, "<tr>");
		html_buffer = html;
		html = g_strconcat (html_buffer, htmpbuf, NULL);
		g_free (html_buffer);

		if ((i >= COLUMN_HOME_PHONE_2 && i <= COLUMN_HOME_PHONE_4) ||
		    (i >= COLUMN_WORK_PHONE_2 && i <= COLUMN_WORK_PHONE_4) ||
		    (i >= COLUMN_CELL_PHONE_2 && i <= COLUMN_CELL_PHONE_4) ||
		    (i >= COLUMN_EMAIL_2 && i <= COLUMN_EMAIL_4) ||
		    (i >= COLUMN_WWW_2 && i <= COLUMN_WWW_4)) {
			tmpbuf[0] = '\0';
		} else {
			sprintf (tmpbuf, "%s:", gettext (appGUI->cnt->contact_fields_tags_name[2*i]));
		}

		if (i == COLUMN_INFO) {

			gchar *tmp = utl_text_to_html (text, TRUE);

			if (tmp != NULL) {
				additional_info = TRUE;
				g_snprintf (htmpbuf, BUFFER_SIZE, "<td colspan=\"2\" class=\"tag\">%s</td></tr></table>\n<pre>%s</pre>\n", tmpbuf, tmp);
				html_buffer = html;
				html = g_strconcat (html_buffer, htmpbuf, NULL);
				g_free (html_buffer);
				g_free (tmp);
			}

		} else {

			g_snprintf (htmpbuf, BUFFER_SIZE, "<td class=\"tag\">%s</td>", tmpbuf);
			html_buffer = html;
			html = g_strconcat (html_buffer, htmpbuf, NULL);
			g_free (html_buffer);

		}

		if (i == COLUMN_BLOG || (i >= COLUMN_EMAIL_1 && i <= COLUMN_EMAIL_4) || (i >= COLUMN_WWW_1 && i <= COLUMN_WWW_4)) {

			g_snprintf (htmpbuf, BUFFER_SIZE, "<td class=\"value\"><a href=\"%s\">%s</a></td></tr>\n", text, text);
			html_buffer = html;
			html = g_strconcat (html_buffer, htmpbuf, NULL);
			g_free (html_buffer);

		} else if (i != COLUMN_INFO) {

			g_snprintf (htmpbuf, BUFFER_SIZE, "<td class=\"value\">%s</td></tr>\n", text);
			html_buffer = html;
			html = g_strconcat (html_buffer, htmpbuf, NULL);
			g_free (html_buffer);

		}

	}

	if (additional_info == TRUE) {
		g_snprintf (htmpbuf, BUFFER_SIZE, "</body>\n</html>\n");
	} else {
		g_snprintf (htmpbuf, BUFFER_SIZE, "</table>\n</body>\n</html>\n");
	}
	html_buffer = html;
	html = g_strconcat (html_buffer, htmpbuf, NULL);
	g_free (html_buffer);

	/* display html code */
	/*printf ("%s\n", html);*/

	if (html_document_open_stream (appGUI->cnt->html_document, "text/html")) {

		html_view_set_document (HTML_VIEW (appGUI->cnt->html_view), NULL);
		html_document_clear (appGUI->cnt->html_document);
		html_document_write_stream (appGUI->cnt->html_document, html, strlen (html));
		html_view_set_document (HTML_VIEW (appGUI->cnt->html_view), appGUI->cnt->html_document);

		html_document_close_stream (appGUI->cnt->html_document);
	}

	g_free (html);
}

/*------------------------------------------------------------------------------*/

void
add_contacts_toolbar_widget (GtkUIManager *contacts_uim_widget, GtkWidget *widget, gpointer user_data) {

GtkWidget *handle_box;

    GUI *appGUI = (GUI *)user_data;

    if (GTK_IS_TOOLBAR (widget)) {

        appGUI->cnt->contacts_toolbar = GTK_TOOLBAR (widget);

        handle_box = gtk_handle_box_new ();
        gtk_widget_show (handle_box);
        gtk_container_add (GTK_CONTAINER (handle_box), widget);
        gtk_box_pack_start (appGUI->cnt->vbox, handle_box, FALSE, FALSE, 0);
        g_signal_connect_swapped (widget, "destroy", G_CALLBACK (gtk_widget_destroy), handle_box);

    } else {
        gtk_box_pack_start (appGUI->cnt->vbox, widget, FALSE, FALSE, 0);
    }

    gtk_widget_show (widget);
}

/*------------------------------------------------------------------------------*/

gboolean
contacts_search_entry_changed_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

GtkTreePath *path;
GtkTreeIter iter;
gint i;

    GUI *appGUI = (GUI *)user_data;

    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->cnt->contacts_filter));
    set_export_active (appGUI);

    if(strlen(gtk_entry_get_text (GTK_ENTRY(appGUI->cnt->contacts_find_entry)))) {

        if (config.show_after_search == TRUE) {

            i = 0;
            while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->cnt->contacts_filter), &iter, NULL, i++));

            if (i-1 != 0) {
                show_contacts_desc_panel(TRUE, appGUI);

                path = gtk_tree_path_new_first();
                if (path != NULL) {
                    gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->cnt->contacts_list), path, NULL, FALSE);
                    gtk_tree_path_free (path);
                }
            } else {
                /*utl_gui_clear_text_buffer (gtk_text_view_get_buffer(GTK_TEXT_VIEW(appGUI->cnt->contacts_desc_textview)), &titer);*/
            }
        }
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
contacts_find_type_selected_cb (GtkComboBox *widget, gpointer user_data) {

GtkTreeIter iter;
GtkTreePath *path;
gint i;

    GUI *appGUI = (GUI *)user_data;

    config.find_mode = gtk_combo_box_get_active (widget);

    if(strlen(gtk_entry_get_text (GTK_ENTRY(appGUI->cnt->contacts_find_entry)))) {
        gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->cnt->contacts_filter));

		i = 0;
		while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->cnt->contacts_filter), &iter, NULL, i++));

        path = gtk_tree_path_new_first();
        if (path != NULL && i-1) {
            gtk_tree_view_set_cursor (GTK_TREE_VIEW (appGUI->cnt->contacts_list), path, NULL, FALSE);
			gtk_tree_path_free (path);
            show_contacts_desc_panel(TRUE, appGUI);
        }
    }
}

/*------------------------------------------------------------------------------*/

gint
custom_contacts_sort_function (GtkTreeModel *model, GtkTreeIter *iter_a, GtkTreeIter *iter_b, gpointer user_data) {

gchar *last_name_a, *last_name_b;
gchar *group_a, *group_b;
gchar *first_name_a, *first_name_b;
gint group_s, first_name_s, last_name_s;

    GUI *appGUI = (GUI *)user_data;

    if (appGUI->cnt->contacts_filter_disabled == TRUE)
        return 0;

    if(iter_a == NULL || iter_b == NULL) {
        return 0;
    }

    gtk_tree_model_get(model, iter_a,
                       COLUMN_GROUP, &group_a, COLUMN_FIRST_NAME, &first_name_a, COLUMN_LAST_NAME, &last_name_a, -1);
    gtk_tree_model_get(model, iter_b,
                       COLUMN_GROUP, &group_b, COLUMN_FIRST_NAME, &first_name_b, COLUMN_LAST_NAME, &last_name_b, -1);

    if (group_a == NULL) {
        group_s = -1;
    } else if (group_b == NULL) {
        group_s = 1;
    } else if (group_a != NULL && group_b != NULL) {
        group_s = g_utf8_collate(group_a, group_b);
    } else {
        group_s = 0;
    }

    if (group_a != NULL) {
        g_free(group_a);
    }
    if (group_b != NULL) {
        g_free(group_b);
    }

    if (first_name_a == NULL) {
        first_name_s = -1;
    } else if (first_name_b == NULL) {
        first_name_s = 1;
    } else if (first_name_a != NULL && first_name_b != NULL) {
        first_name_s = g_utf8_collate(first_name_a, first_name_b);
    } else {
        first_name_s = 0;
    }

    if (first_name_a != NULL) {
        g_free(first_name_a);
    }
    if (first_name_b != NULL) {
        g_free(first_name_b);
    }

    if (last_name_a == NULL) {
        last_name_s = -1;
    } else if (last_name_b == NULL) {
        last_name_s = 1;
    } else if (last_name_a != NULL && last_name_b != NULL) {
        last_name_s = g_utf8_collate(last_name_a, last_name_b);
    } else {
        last_name_s = 0;
    }

    if (last_name_a != NULL) {
        g_free(last_name_a);
    }
    if (last_name_b != NULL) {
        g_free(last_name_b);
    }

    switch(config.contacts_sorting_mode) {

        /* Group, First Name, Last Name */
        case 0:
            if (group_s != 0)
                return group_s;
            if (first_name_s != 0)
                return first_name_s;
            if (last_name_s != 0)
                return last_name_s;
            break;

        /* Group, Last Name, First Name */
        case 1:
            if (group_s != 0)
                return group_s;
            if (last_name_s != 0)
                return last_name_s;
            if (first_name_s != 0)
                return first_name_s;
            break;

        /* Last Name, First Name, Group */
        case 2:
            if (last_name_s != 0)
                return last_name_s;
            if (first_name_s != 0)
                return first_name_s;
            if (group_s != 0)
                return group_s;
            break;

        /* Last Name, Group, First Name */
        case 3:
            if (last_name_s != 0)
                return last_name_s;
            if (group_s != 0)
                return group_s;
            if (first_name_s != 0)
                return first_name_s;
            break;

        /* First Name, Last Name, Group */
        case 4:
            if (first_name_s != 0)
                return first_name_s;
            if (last_name_s != 0)
                return last_name_s;
            if (group_s != 0)
                return group_s;
            break;

        /* First Name, Group, Last Name */
        case 5:
            if (first_name_s != 0)
                return first_name_s;
            if (group_s != 0)
                return group_s;
            if (last_name_s != 0)
                return last_name_s;
            break;

        default:
            break;
    }

    return 0;
}

/*------------------------------------------------------------------------------*/

void
contacts_add_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    contacts_add_edit_dialog_show (FALSE, appGUI);
}

/*------------------------------------------------------------------------------*/

void
contacts_edit_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    if (gtk_tree_selection_get_selected (appGUI->cnt->contacts_list_selection, NULL, NULL)) {
        GUI *appGUI = (GUI *)data;
        contacts_add_edit_dialog_show (TRUE, appGUI);
    }
}

/*------------------------------------------------------------------------------*/

void
contacts_remove_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    contacts_remove_dialog_show(appGUI->cnt->contacts_list, appGUI->cnt->contacts_list_store, appGUI);
}

/*------------------------------------------------------------------------------*/

void
contacts_birthdays_item_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    contacts_create_birthdays_window (appGUI);
}

/*------------------------------------------------------------------------------*/

void
contacts_map_location_cb (GtkWidget *widget, gpointer data) {

gint response = -1;
GtkWidget *info_dialog = NULL;
GtkWidget *hbox;
GtkWidget *home_addr_radiobutton;
GtkWidget *work_addr_radiobutton;
GSList *hw_radiobutton_group = NULL;


    GUI *appGUI = (GUI *)data;

    if (gtk_tree_selection_get_selected (appGUI->cnt->contacts_list_selection, NULL, NULL)) {
    
		if (check_address (HOME_ADDRESS, appGUI) && check_address (WORK_ADDRESS, appGUI)) {

			info_dialog = gtk_message_dialog_new (GTK_WINDOW(appGUI->main_window),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO, GTK_BUTTONS_OK_CANCEL, 
												  "\n%s:", _("Please select address"));

			gtk_window_set_title(GTK_WINDOW(info_dialog), _("Information"));
			gtk_widget_show (info_dialog);

			hbox = gtk_hbox_new (FALSE, 4);
		 	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(info_dialog)->vbox), hbox, FALSE, TRUE, 2);
			gtk_widget_show (hbox);

			work_addr_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Work"));
			gtk_widget_show (work_addr_radiobutton);
		 	gtk_box_pack_end (GTK_BOX(hbox), work_addr_radiobutton, FALSE, TRUE, 2);
		    gtk_radio_button_set_group (GTK_RADIO_BUTTON (work_addr_radiobutton), hw_radiobutton_group);
            hw_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (work_addr_radiobutton));

			home_addr_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Home"));
			gtk_widget_show (home_addr_radiobutton);
		 	gtk_box_pack_end (GTK_BOX(hbox), home_addr_radiobutton, FALSE, TRUE, 2);
		  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (home_addr_radiobutton), hw_radiobutton_group);
			hw_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (home_addr_radiobutton));

			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (home_addr_radiobutton), TRUE);

			response = gtk_dialog_run(GTK_DIALOG(info_dialog));

			if (response == GTK_RESPONSE_OK) {
				if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (home_addr_radiobutton)) == TRUE) {
					show_contact_location_on_map (HOME_ADDRESS, appGUI);
				} else {
					show_contact_location_on_map (WORK_ADDRESS, appGUI);
				}
			}

			gtk_widget_destroy(info_dialog);

		} else {
			if (check_address (HOME_ADDRESS, appGUI)) {
				show_contact_location_on_map (HOME_ADDRESS, appGUI);
			} else {
				show_contact_location_on_map (WORK_ADDRESS, appGUI);
			}
		}
    }
}

/*------------------------------------------------------------------------------*/

void
contacts_export_items_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;
    contacts_create_export_window(appGUI);
}

/*------------------------------------------------------------------------------*/

void
contacts_import_items_cb (GtkWidget *widget, gpointer data) {

    GUI *appGUI = (GUI *)data;

    import_contacts_from_csv_file (appGUI);
}

/*------------------------------------------------------------------------------*/

gint
contacts_list_dbclick_cb(GtkWidget * widget, GdkEventButton * event, gpointer func_data) {

    GUI *appGUI = (GUI *)func_data;

    if ((event->type==GDK_2BUTTON_PRESS) && (event->button == 1)) {

        if (appGUI->cnt->contacts_panel_status == TRUE) {
            contacts_edit_item_cb (NULL, appGUI);
        } else {
            show_contacts_desc_panel(TRUE, appGUI);
        }
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
contacts_selection_activate (gboolean active, GUI *appGUI) {
    if (active == TRUE) {
        g_signal_connect(G_OBJECT(appGUI->cnt->contacts_list_selection), "changed",
                         G_CALLBACK(contacts_item_selected_cb), appGUI);
    } else {
        g_signal_handlers_disconnect_by_func (G_OBJECT (appGUI->cnt->contacts_list_selection),
                                              G_CALLBACK (contacts_item_selected_cb), appGUI);
    }
}

/*------------------------------------------------------------------------------*/

void
store_contact_columns_info (GUI *appGUI) {

	gint n;

	config.contacts_column_idx_0 = utl_gui_get_column_position (appGUI->cnt->contacts_columns[COLUMN_GROUP],
																GTK_TREE_VIEW(appGUI->cnt->contacts_list), MAX_VISIBLE_CONTACT_COLUMNS, appGUI);
	config.contacts_column_idx_1 = utl_gui_get_column_position (appGUI->cnt->contacts_columns[COLUMN_FIRST_NAME], 
																GTK_TREE_VIEW(appGUI->cnt->contacts_list), MAX_VISIBLE_CONTACT_COLUMNS, appGUI);
	config.contacts_column_idx_2 = utl_gui_get_column_position (appGUI->cnt->contacts_columns[COLUMN_LAST_NAME], 
																GTK_TREE_VIEW(appGUI->cnt->contacts_list), MAX_VISIBLE_CONTACT_COLUMNS, appGUI);

	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), 0));
	if (n > 1) {
		config.contacts_column_idx_0_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), 1));
	if (n > 1) {
		config.contacts_column_idx_1_width = n;
	}
	n = gtk_tree_view_column_get_width(gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), 2));
	if (n > 1) {
		config.contacts_column_idx_2_width = n;
	}
}

/*------------------------------------------------------------------------------*/

void
set_contacts_columns_width (GUI *appGUI) {

	GtkTreeViewColumn   *col;
	gint w;

	w = 2 * utl_gui_get_sw_vscrollbar_width (appGUI->cnt->scrolled_win);

	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), COLUMN_GROUP);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.contacts_column_idx_0_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.contacts_column_idx_0_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), COLUMN_FIRST_NAME);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.contacts_column_idx_1_width > 0) {
		gtk_tree_view_column_set_fixed_width (col, config.contacts_column_idx_1_width);
	}
	col = gtk_tree_view_get_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), COLUMN_LAST_NAME);
	if (gtk_tree_view_column_get_visible(col) == TRUE && config.contacts_column_idx_2_width > w) {
		gtk_tree_view_column_set_fixed_width (col, config.contacts_column_idx_2_width - w);
	}
}

/*------------------------------------------------------------------------------*/

gboolean
on_requested_url (HtmlDocument *doc, const gchar *url, HtmlStream *stream, gpointer data) {

FILE *fp;
gint len;
gchar tmp_buffer[8192];

	g_return_val_if_fail (url != NULL, TRUE);
	g_return_val_if_fail (stream != NULL, TRUE);

  	fp = fopen(url, "r");

	if (fp != NULL) {

		while ((len = fread(tmp_buffer, 1, sizeof(tmp_buffer), fp)) > 0) {
            html_stream_write(stream, tmp_buffer, len);
        }
		fclose (fp);

		return TRUE;
	}

	return FALSE;
}

/*------------------------------------------------------------------------------*/

void
on_link_clicked (GtkWidget *html, const gchar *url, gpointer data) {

gchar *link = (gchar *)url;

	utl_run_helper (link, utl_get_link_type (link));
}

/*------------------------------------------------------------------------------*/

void
cnt_clear_find_cb (GtkWidget *widget, gpointer user_data)
{
	GUI *appGUI = (GUI *) user_data;
    if (strlen(gtk_entry_get_text(GTK_ENTRY(appGUI->cnt->contacts_find_entry)))) {
        gtk_entry_set_text(GTK_ENTRY(appGUI->cnt->contacts_find_entry), "");
		contacts_search_entry_changed_cb (NULL, NULL, appGUI);
	}
}

/*------------------------------------------------------------------------------*/

void
gui_create_contacts(GUI *appGUI) {

GtkWidget           *vbox1;
GtkWidget           *vbox2;
GtkWidget           *vbox3;
GtkWidget           *hbox2;
GtkWidget           *hseparator;
GtkWidget           *label;
GtkWidget           *top_viewport;
GtkWidget           *bottom_viewport;
GtkWidget           *close_button;
GtkCellRenderer     *renderer[CONTACTS_NUM_COLUMNS];
GType               contact_columns_types[CONTACTS_NUM_COLUMNS];
gint                i, n;
GError              *error = NULL;
GtkActionGroup      *action_group = NULL;
gchar tmpbuf[BUFFER_SIZE];

gint columns_order[MAX_VISIBLE_CONTACT_COLUMNS];

gint co_columns[MAX_VISIBLE_CONTACT_COLUMNS] = { 
		COLUMN_GROUP, COLUMN_FIRST_NAME, COLUMN_LAST_NAME
};

static gchar *contact_fields_tags_name[] = {
    N_("Group"), "group", 
	N_("First name"), "first_name", N_("Last name"), "last_name", 
	N_("Second name"), "second_name", 
    N_("Nickname"), "nickname", N_("Tags"), "tags", 
	N_("Birthday date"), "birthday_date", N_("Name day date"), "name_day_date",

    /*--------------------------------------------------*/
    N_("Home address"), "home_address", N_("Home postcode"), "home_postcode", N_("Home city"), 
    "home_city", N_("Home state"), "home_state", N_("Home country"), "home_country",
    /*--------------------------------------------------*/

    N_("Organization"), "organization", N_("Department"), "department",

    /*--------------------------------------------------*/
    N_("Work address"), "work_address", N_("Work postcode"), "work_postcode", N_("Work city"), 
    "work_city", N_("Work state"), "work_state", N_("Work country"), "work_country",
    /*--------------------------------------------------*/

    N_("Fax"), "work_fax",

    /*--------------------------------------------------*/
    N_("Home phone"), "home_phone_1", N_("Home phone 2"), "home_phone_2",
    N_("Home phone 3"), "home_phone_3", N_("Home phone 4"), "home_phone_4",
    N_("Work phone"), "work_phone_1", N_("Work phone 2"), "work_phone_2",
    N_("Work phone 3"), "work_phone_3", N_("Work phone 4"), "work_phone_4",
    N_("Cell phone"), "cell_phone_1", N_("Cell phone 2"), "cell_phone_2",
    N_("Cell phone 3"), "cell_phone_3", N_("Cell phone 4"), "cell_phone_4",
    N_("E-Mail"), "email_1", N_("E-Mail 2"), "email_2", N_("E-Mail 3"), "email_3", 
    N_("E-Mail 4"), "email_4", N_("WWW"), "www_1", N_("WWW 2"), "www_2", N_("WWW 3"), "www_3", 
    N_("WWW 4"), "www_4",
    /*--------------------------------------------------*/

    N_("IM Gadu-Gadu"), "im_gg", N_("IM Yahoo"), "im_yahoo", N_("IM MSN"), "im_msn",
    N_("IM ICQ"), "im_icq", N_("IM AOL"), "im_aol",
    N_("IM Jabber"), "im_jabber", N_("IM Skype"), "im_skype", N_("IM Tlen"), "im_tlen",
    N_("Blog"), "blog", N_("Photo"), "photo_path", N_("Additional info"), "additional_info", "ID", "id"
};

const gchar *ui_info =
"  <toolbar name=\"toolbar\">\n"
"    <toolitem name=\"add\" action=\"add\" />\n"
"    <toolitem name=\"edit\" action=\"edit\" />\n"
"    <toolitem name=\"delete\" action=\"delete\" />\n"
"    <separator/>\n"
"    <toolitem name=\"birthdays\" action=\"birthdays\" />\n"
"    <toolitem name=\"map_location\" action=\"map_location\" />\n"
"    <separator/>\n"
"    <toolitem name=\"import\" action=\"import\" />\n"
"    <toolitem name=\"export\" action=\"export\" />\n"
"    <separator expand=\"true\" />\n"
"    <toolitem name=\"preferences\" action=\"preferences\" />\n"
"    <toolitem name=\"about\" action=\"about\" />\n"
"  </toolbar>\n";

GtkActionEntry entries[] = {
    { "add", OSMO_STOCK_CONTACTS_ADD, _("New contact"), NULL, _("New contact"), G_CALLBACK(contacts_add_item_cb)},
    { "delete", OSMO_STOCK_CONTACTS_REMOVE, _("Remove contact"), NULL, _("Remove contact"), G_CALLBACK(contacts_remove_item_cb)},
    { "edit", OSMO_STOCK_CONTACTS_EDIT, _("Edit contact"), NULL, _("Edit contact"), G_CALLBACK(contacts_edit_item_cb)},
    { "birthdays", OSMO_STOCK_CONTACTS_BIRTHDAYS, _("Show birthdays"), NULL, _("Show birthdays"), G_CALLBACK(contacts_birthdays_item_cb)},
    { "map_location", OSMO_STOCK_CONTACTS_MAP_LOCATION, _("Show contact location on the map"), NULL, _("Show contact location on the map"), G_CALLBACK(contacts_map_location_cb)},
    { "import", OSMO_STOCK_CONTACTS_IMPORT, _("Import contacts"), NULL, _("Import contacts"), G_CALLBACK(contacts_import_items_cb)},
    { "export", OSMO_STOCK_CONTACTS_EXPORT, _("Export contacts"), NULL, _("Export contacts"), G_CALLBACK(contacts_export_items_cb)},
	{ "preferences", OSMO_STOCK_PREFERENCES, _("Preferences"), NULL, _("Preferences"), G_CALLBACK (show_preferences_window_cb)},
	{ "about", OSMO_STOCK_ABOUT, _("About"), NULL, _("About"), G_CALLBACK (show_about_window_cb)},
};

guint n_entries = G_N_ELEMENTS (entries);

    appGUI->cnt->contact_fields_tags_name = contact_fields_tags_name;

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox1), 8);
    sprintf(tmpbuf, "<b>%s</b>", _("Contacts"));
    gui_add_to_notebook (vbox1, tmpbuf, appGUI);

    appGUI->cnt->vbox = GTK_BOX(vbox1);

    if (config.hide_contacts == TRUE) {
        gtk_widget_hide(GTK_WIDGET(appGUI->cnt->vbox));
    }

    /*-------------------------------------------------------------------------------------*/

    action_group = gtk_action_group_new ("_actions");
    gtk_action_group_add_actions (action_group, entries, n_entries, appGUI);
    gtk_action_group_set_sensitive(action_group, TRUE);

    appGUI->cnt->contacts_uim_widget = gtk_ui_manager_new ();

    gtk_ui_manager_insert_action_group (appGUI->cnt->contacts_uim_widget, action_group, 0);
    g_signal_connect (appGUI->cnt->contacts_uim_widget, "add_widget", G_CALLBACK (add_contacts_toolbar_widget), appGUI);

    if (!gtk_ui_manager_add_ui_from_string (appGUI->cnt->contacts_uim_widget, ui_info, -1, &error)) {
        g_message ("building toolbar failed: %s", error->message);
        g_error_free (error);
    }
    gtk_ui_manager_ensure_update (appGUI->cnt->contacts_uim_widget);

    gtk_toolbar_set_style (appGUI->cnt->contacts_toolbar, GTK_TOOLBAR_ICONS);

    /*-------------------------------------------------------------------------------------*/

    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/edit"), FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget (appGUI->cnt->contacts_uim_widget, "/toolbar/delete"), FALSE);

/******************************************************************************************/

	if (!config.gui_layout) {
	    appGUI->cnt->contacts_paned = gtk_vpaned_new();
	    gtk_paned_set_position(GTK_PANED(appGUI->cnt->contacts_paned), 99999);
	} else {
        appGUI->cnt->contacts_paned = gtk_hpaned_new();
	}

    gtk_widget_show (appGUI->cnt->contacts_paned);
    gtk_box_pack_start(GTK_BOX(vbox1), appGUI->cnt->contacts_paned, TRUE, TRUE, 0);

    top_viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (top_viewport);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (top_viewport), GTK_SHADOW_NONE);
    gtk_paned_pack1 (GTK_PANED (appGUI->cnt->contacts_paned), top_viewport, FALSE, TRUE);

    vbox3 = gtk_vbox_new (FALSE, 1);
    gtk_widget_show (vbox3);
    gtk_container_add (GTK_CONTAINER (top_viewport), vbox3);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox3), hseparator, FALSE, FALSE, 6);

    sprintf(tmpbuf, "<b>%s:</b>", _("Search"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox3), label, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox2);
    gtk_box_pack_start (GTK_BOX (vbox3), hbox2, FALSE, TRUE, 0);

    appGUI->cnt->contacts_find_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(appGUI->cnt->contacts_find_entry), 128);
    gtk_widget_show (appGUI->cnt->contacts_find_entry);
    g_signal_connect (G_OBJECT(appGUI->cnt->contacts_find_entry), "key_release_event",
                        G_CALLBACK(contacts_search_entry_changed_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox2), appGUI->cnt->contacts_find_entry, TRUE, TRUE, 0);

	appGUI->cnt->contacts_find_clear_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLEAR, FALSE);
	gtk_widget_show (appGUI->cnt->contacts_find_clear_button);
	GTK_WIDGET_UNSET_FLAGS (appGUI->cnt->contacts_find_clear_button, GTK_CAN_FOCUS);
	gtk_button_set_relief (GTK_BUTTON(appGUI->cnt->contacts_find_clear_button), GTK_RELIEF_NONE);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (appGUI->cnt->contacts_find_clear_button, _("Clear"));
	}
	g_signal_connect (G_OBJECT (appGUI->cnt->contacts_find_clear_button), "clicked",
						G_CALLBACK (cnt_clear_find_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox2), appGUI->cnt->contacts_find_clear_button, FALSE, FALSE, 0);

    appGUI->cnt->contacts_find_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->cnt->contacts_find_combobox);
    gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox2), appGUI->cnt->contacts_find_combobox, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (appGUI->cnt->contacts_find_combobox), "changed",
                      G_CALLBACK (contacts_find_type_selected_cb), appGUI);
    g_signal_connect(G_OBJECT(appGUI->cnt->contacts_find_combobox), "focus",
                     G_CALLBACK(find_combo_box_focus_cb), appGUI);
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), _("First Name"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), _("Last Name"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), _("Tags"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), _("All fields"));

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox3), hseparator, FALSE, FALSE, 6);

    appGUI->cnt->scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (appGUI->cnt->scrolled_win);
    gtk_box_pack_start (GTK_BOX (vbox3), appGUI->cnt->scrolled_win, TRUE, TRUE, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (appGUI->cnt->scrolled_win), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->cnt->scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    for(i=0; i< CONTACTS_NUM_COLUMNS; i++) {
        if (i == COLUMN_BIRTH_DAY_DATE || i == COLUMN_NAME_DAY_DATE || i == COLUMN_ID) {
            contact_columns_types[i] = G_TYPE_UINT;
        } else {
            contact_columns_types[i] = G_TYPE_STRING;
        }
    }

    appGUI->cnt->contacts_list_store = gtk_list_store_newv(CONTACTS_NUM_COLUMNS, &contact_columns_types[0]);

    appGUI->cnt->contacts_filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(appGUI->cnt->contacts_list_store), NULL);
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER(appGUI->cnt->contacts_filter),
                                            (GtkTreeModelFilterVisibleFunc)contacts_list_filter_cb,
                                            appGUI, NULL);

    appGUI->cnt->contacts_sort = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(appGUI->cnt->contacts_filter));

    appGUI->cnt->contacts_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(appGUI->cnt->contacts_sort));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(appGUI->cnt->contacts_list), config.rules_hint);
    gtk_widget_show (appGUI->cnt->contacts_list);
    GTK_WIDGET_SET_FLAGS (appGUI->cnt->contacts_list, GTK_CAN_DEFAULT);
    gtk_widget_modify_fg(GTK_WIDGET(appGUI->cnt->contacts_list), GTK_STATE_SELECTED,
                         (& GTK_WIDGET(appGUI->cnt->contacts_list)->style->base[GTK_STATE_SELECTED]));

    g_signal_connect(G_OBJECT(appGUI->cnt->contacts_list), "button_press_event",
                     G_CALLBACK(contacts_list_dbclick_cb), appGUI);

    appGUI->cnt->contacts_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (appGUI->cnt->contacts_list));
    contacts_selection_activate (TRUE, appGUI);

    /* columns setup */

    for (i = COLUMN_GROUP; i < CONTACTS_NUM_COLUMNS; i++) {
        renderer[i] = gtk_cell_renderer_text_new();
        appGUI->cnt->contacts_columns[i] = gtk_tree_view_column_new_with_attributes(gettext(contact_fields_tags_name[i*2]),
                                                              renderer[i], "text", i, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->cnt->contacts_list), appGUI->cnt->contacts_columns[i]);

        if(i != COLUMN_FIRST_NAME && i != COLUMN_LAST_NAME && i != COLUMN_GROUP) {
            gtk_tree_view_column_set_visible (appGUI->cnt->contacts_columns[i], FALSE);
        } else {

			if (config.hide_group_column == TRUE && i == COLUMN_GROUP) {
                gtk_tree_view_column_set_visible (appGUI->cnt->contacts_columns[i], FALSE);
            }

			gtk_tree_view_column_set_reorderable (appGUI->cnt->contacts_columns[i], TRUE);
			gtk_tree_view_column_set_resizable (appGUI->cnt->contacts_columns[i], TRUE);
			gtk_tree_view_column_set_sizing (appGUI->cnt->contacts_columns[i], GTK_TREE_VIEW_COLUMN_FIXED);

        }
    }

    gtk_container_add (GTK_CONTAINER (appGUI->cnt->scrolled_win), appGUI->cnt->contacts_list);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW(appGUI->cnt->contacts_list), FALSE);

	/* restore columns order */

	columns_order[0] = config.contacts_column_idx_0;
	columns_order[1] = config.contacts_column_idx_1;
	columns_order[2] = config.contacts_column_idx_2;

	n = MAX_VISIBLE_CONTACT_COLUMNS-1;

	while (n >= 0) {
		for (i = 0; i < MAX_VISIBLE_CONTACT_COLUMNS; i++) {
			if (n == columns_order[i]) {
				gtk_tree_view_move_column_after(GTK_TREE_VIEW(appGUI->cnt->contacts_list),
												appGUI->cnt->contacts_columns[co_columns[i]], NULL);
				n--;
			}
		}
	}

	set_contacts_columns_width (appGUI);

    /* configure sorting */

    gtk_tree_sortable_set_sort_func((GtkTreeSortable *)appGUI->cnt->contacts_sort, 0,
                                    (GtkTreeIterCompareFunc)custom_contacts_sort_function, appGUI, NULL);

    gtk_tree_sortable_set_sort_column_id((GtkTreeSortable *)appGUI->cnt->contacts_sort, COLUMN_FIRST_NAME, config.contacts_sorting_order);
    gtk_tree_sortable_set_sort_column_id((GtkTreeSortable *)appGUI->cnt->contacts_sort, COLUMN_LAST_NAME, config.contacts_sorting_order);
    gtk_tree_sortable_set_sort_column_id((GtkTreeSortable *)appGUI->cnt->contacts_sort, COLUMN_GROUP, config.contacts_sorting_order);

    bottom_viewport = gtk_viewport_new (NULL, NULL);
    gtk_widget_show (bottom_viewport);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (bottom_viewport), GTK_SHADOW_NONE);
    gtk_paned_pack2 (GTK_PANED (appGUI->cnt->contacts_paned), bottom_viewport, TRUE, TRUE);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), 0);
    gtk_container_add (GTK_CONTAINER (bottom_viewport), vbox2);

    appGUI->cnt->panel_hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cnt->panel_hbox, FALSE, FALSE, 0);
    gtk_widget_show(appGUI->cnt->panel_hbox);

    sprintf(tmpbuf, "%s:", _("Contact details"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (appGUI->cnt->panel_hbox), label, FALSE, FALSE, 0);

	if (!config.gui_layout) {
		if (config.default_stock_icons) {
			close_button = utl_gui_stock_button (GTK_STOCK_CLOSE, FALSE);
		} else {
			close_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLOSE, FALSE);
		}
		GTK_WIDGET_UNSET_FLAGS(close_button, GTK_CAN_FOCUS);
		gtk_button_set_relief (GTK_BUTTON(close_button), GTK_RELIEF_NONE);
		if (config.enable_tooltips) {
			gtk_widget_set_tooltip_text (close_button, _("Close contact panel"));
		}
		gtk_box_pack_end (GTK_BOX (appGUI->cnt->panel_hbox), close_button, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (close_button), "clicked",
							G_CALLBACK (contacts_panel_close_desc_cb), appGUI);
	}

    appGUI->cnt->contacts_panel_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (appGUI->cnt->contacts_panel_scrolledwindow);
    gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cnt->contacts_panel_scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (appGUI->cnt->contacts_panel_scrolledwindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->cnt->contacts_panel_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	appGUI->cnt->html_document = html_document_new ();
	g_signal_connect (appGUI->cnt->html_document, "link-clicked", G_CALLBACK (on_link_clicked), NULL);
	g_signal_connect (appGUI->cnt->html_document, "request-url", G_CALLBACK (on_requested_url), NULL);
	appGUI->cnt->html_view = html_view_new ();
	html_view_set_document (HTML_VIEW(appGUI->cnt->html_view), appGUI->cnt->html_document);
	gtk_widget_show (appGUI->cnt->html_view);
    gtk_container_add (GTK_CONTAINER (appGUI->cnt->contacts_panel_scrolledwindow), appGUI->cnt->html_view);

    gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cnt->contacts_find_combobox), config.find_mode);

    appGUI->cnt->contacts_filter_disabled = FALSE;
    gtk_tree_sortable_sort_column_changed((GtkTreeSortable *)appGUI->cnt->contacts_sort);
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(appGUI->cnt->contacts_filter));

	if (config.gui_layout) {
		gtk_paned_set_position(GTK_PANED(appGUI->cnt->contacts_paned), config.contacts_pane_pos);
	}

	gtk_widget_grab_focus (appGUI->cnt->contacts_find_entry);
}

/*------------------------------------------------------------------------------*/

gboolean
check_address (gint address_type, GUI *appGUI) {

GtkTreeIter iter;
GtkTreeModel *model;
gchar *text = NULL;
gint n = 0;
gint c1, c2, c3;
	
    if (gtk_tree_selection_get_selected (appGUI->cnt->contacts_list_selection, &model, &iter)) {

		if (address_type == HOME_ADDRESS) {
			c1 = COLUMN_HOME_ADDRESS;
			c2 = COLUMN_HOME_CITY;
			c3 = COLUMN_HOME_COUNTRY;
		} else {
			c1 = COLUMN_WORK_ADDRESS;
			c2 = COLUMN_WORK_CITY;
			c3 = COLUMN_WORK_COUNTRY;
		}

		gtk_tree_model_get (model, &iter, c1, &text, -1);
		if (text == NULL) return FALSE;
		if (strlen(text)) n++;
		g_free (text);
		gtk_tree_model_get (model, &iter, c2, &text, -1);
		if (text == NULL) return FALSE;
		if (strlen(text)) n++;
		g_free (text);
		gtk_tree_model_get (model, &iter, c3, &text, -1);
		if (text == NULL) return FALSE;
		if (strlen(text)) n++;
		g_free (text);
	}

	return (n == 3);
}

/*------------------------------------------------------------------------------*/

void
show_contact_location_on_map (gint address_type, GUI *appGUI) {

GtkTreeIter     iter;
GtkTreeModel    *model;
gboolean pn_flag = FALSE;
gint i;
gchar *text = NULL;
gchar google_maps_url[BUFFER_SIZE];
gint cbegin, cend, cskip1, cskip2;

    if (gtk_tree_selection_get_selected (appGUI->cnt->contacts_list_selection, &model, &iter)) {

		if (address_type == HOME_ADDRESS) {
			cbegin = COLUMN_HOME_ADDRESS;
			cend = COLUMN_HOME_COUNTRY;
			cskip1 = COLUMN_HOME_POST_CODE;
			cskip2 = COLUMN_HOME_STATE;
		} else {
			cbegin = COLUMN_WORK_ADDRESS;
			cend = COLUMN_WORK_COUNTRY;
			cskip1 = COLUMN_WORK_POST_CODE;
			cskip2 = COLUMN_WORK_STATE;
		}

        g_snprintf (google_maps_url, BUFFER_SIZE, "%s\"", GOOGLE_MAPS_QUERY);

		for (i = cbegin; i <= cend; i++) {

			if (i == cskip1 || i == cskip2) continue;

				gtk_tree_model_get (model, &iter, i, &text, -1);
				if (text != NULL) {
					if (pn_flag) {
						g_strlcat (google_maps_url, ",", BUFFER_SIZE);
					}
					g_strlcat (google_maps_url, text, BUFFER_SIZE);
					pn_flag = TRUE;
					g_free (text);
				}
		}

		g_strlcat (google_maps_url, "\"", BUFFER_SIZE);

		utl_run_helper (google_maps_url, WWW);
	}
}

/*------------------------------------------------------------------------------*/

#endif  /* CONTACTS_ENABLED */

