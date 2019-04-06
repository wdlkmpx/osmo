
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

#include "calendar_ical.h"
#include "i18n.h"
#include "calendar.h"
#include "utils.h"
#include "utils_gui.h"
#include "utils_date.h"
#include "calendar_notes.h"
#include "calendar_widget.h"
#include "options_prefs.h"
#include "calendar_preferences_gui.h"
#include "stock_icons.h"

#ifdef HAVE_LIBICAL

#include <libical/ical.h>
#include <libical/icalss.h>
#include <libical/icalset.h>
#include <libical/icalclassify.h>

/*------------------------------------------------------------------------------*/

void
ics_initialize_timezone (void) {

gchar *tz_search_paths [] = {
    "/usr/share/zoneinfo/",
    "/opt/usr/share/libical/zoneinfo/",
    "/usr/share/libical/zoneinfo/",
    "/usr/local/share/libical/zoneinfo/"
};

gint i;

#define N_SEARCH_PATHS (sizeof (tz_search_paths) / sizeof (tz_search_paths [0]))

    for (i = 0; i < N_SEARCH_PATHS; i++) {
		if (!g_access (tz_search_paths[i], F_OK | R_OK)) {
            set_zone_directory (tz_search_paths[i]);
			break;
		}
	}
}

/*------------------------------------------------------------------------------*/

gchar* 
ics_read_stream (gchar *s, size_t size, void *d) {   
    return fgets (s, size, (FILE*)d); 
} 

/*------------------------------------------------------------------------------*/

gchar*
str_remove_backslash (gchar *text) {

static gchar tmpbuf[BUFFER_SIZE];
gint i, j;

    i = j = 0;

    while (text[i] != '\0') {
        if (text[i] == '\\' && text[i+1] != 'n') i++;
        tmpbuf[j++] = text[i++];
    }

    tmpbuf[j] = '\0';
    return tmpbuf;
}

/*------------------------------------------------------------------------------*/

void
add_ics_entry_to_day_info_panel (gchar *name, struct ics_entry *item, gboolean desc_flag, GUI *appGUI) {

gchar tmpbuf[BUFFER_SIZE];
gchar *tmp_str = NULL;

	if (name != NULL) {
		g_snprintf (tmpbuf, BUFFER_SIZE, "%s:\n", name);
		gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer, 
												  &appGUI->cal->day_desc_iter, tmpbuf, -1, "bold", NULL);
	}
						
	if (item->description != NULL) {
		tmp_str = utl_text_replace (item->description, "\\\\n", "\n");
	}

	if (item->seconds_begin != 0 && item->seconds_end != 0) {
	    g_snprintf (tmpbuf, BUFFER_SIZE, "[ %s, %s - %s ]\n%s\n", 
					utl_get_julian_day_name (item->julian),
					utl_time_print_s (item->seconds_begin, TIME_HH_MM, config.override_locale_settings),
					utl_time_print_s (item->seconds_end, TIME_HH_MM, config.override_locale_settings),
					str_remove_backslash (item->summary));
	} else if (item->seconds_begin != 0 && item->seconds_end == 0) {
	    g_snprintf (tmpbuf, BUFFER_SIZE, "[ %s, %s ]\n%s\n", 
					utl_get_julian_day_name (item->julian),
					utl_time_print_s (item->seconds_begin, TIME_HH_MM, config.override_locale_settings),
					str_remove_backslash (item->summary));
	} else {
	    g_snprintf (tmpbuf, BUFFER_SIZE, "%s\n", str_remove_backslash (item->summary));
	}
    gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, tmpbuf, -1);

    if (item->description != NULL && desc_flag == TRUE) {
        g_snprintf (tmpbuf, BUFFER_SIZE, "(%s)\n\n", str_remove_backslash (tmp_str));
		gtk_text_buffer_insert_with_tags_by_name (appGUI->cal->day_desc_text_buffer, 
												  &appGUI->cal->day_desc_iter, tmpbuf, -1, "italic", NULL);
    } else {
        gtk_text_buffer_insert (appGUI->cal->day_desc_text_buffer, &appGUI->cal->day_desc_iter, "\n", -1);
    }

	if (item->description != NULL) {
		g_free (tmp_str);
	}
}

/*------------------------------------------------------------------------------*/

void
ics_add_item (struct ics_file *file_entry, icalproperty *dtime_begin, icalproperty *dtime_end,
              icalproperty *summary, icalproperty *description)
{
	icaltimetype tt;
	struct ics_entry *item;

	item = g_malloc (sizeof (struct ics_entry));

	tt = icaltime_from_string (icalproperty_get_value_as_string (dtime_begin));

	if (tt.day == 29 && tt.month == 2 && !g_date_valid_dmy (tt.day, tt.month, tt.year)) {
		item->julian = utl_date_dmy_to_julian (tt.day, tt.month, LEAP_YEAR);
	} else {
		item->julian = utl_date_dmy_to_julian (tt.day, tt.month, tt.year);
	}
	item->seconds_begin = utl_time_hms_to_seconds (tt.hour, tt.minute, tt.second);

	tt = icaltime_from_string (icalproperty_get_value_as_string (dtime_end));
	item->seconds_end = utl_time_hms_to_seconds (tt.hour, tt.minute, tt.second);

	item->summary = g_strdup (icalproperty_get_value_as_string (summary));

	if (description != NULL) {
		item->description = g_strdup (icalproperty_get_value_as_string (description));
	} else {
		item->description = NULL;
	}

	file_entry->entries_list = g_slist_prepend (file_entry->entries_list, item);
}

/*------------------------------------------------------------------------------*/

gint
ics_parse_file (struct ics_file *file_entry, gchar *name, gchar *filename, gboolean desc_flag, 
                gboolean count_only, GUI *appGUI) {

FILE *ics_stream;
icalparser *ics_parser;
icalcomponent *ics_component;
icalcomponent *h, *l;
icalproperty *tb, *te, *p, *r;
gint n = 0;

	ics_parser = icalparser_new ();
	g_return_val_if_fail (ics_parser != NULL, n);

	ics_stream = fopen (filename, "r");

	if (ics_stream == NULL) {
		icalparser_free (ics_parser);
		return n;
	}

	icalparser_set_gen_data (ics_parser, ics_stream);
	ics_component = icalparser_parse (ics_parser, ics_read_stream);

	if (ics_component == NULL) {
		fclose (ics_stream);
		icalparser_free (ics_parser);
		return n;
	}

	if (icalcomponent_get_first_component (ics_component, ICAL_VEVENT_COMPONENT) != NULL) {

		for (h = icalcomponent_get_first_component (ics_component, ICAL_VEVENT_COMPONENT); h;
			 h = icalcomponent_get_next_component (ics_component, ICAL_VEVENT_COMPONENT)) {

			tb = icalcomponent_get_first_property (h, ICAL_DTSTART_PROPERTY);
            if (tb == NULL) continue;
			te = icalcomponent_get_first_property (h, ICAL_DTEND_PROPERTY);
            if (te == NULL) continue;
			p = icalcomponent_get_first_property (h, ICAL_SUMMARY_PROPERTY);
			if (p != NULL && count_only == FALSE) {
				r = icalcomponent_get_first_property (h, ICAL_DESCRIPTION_PROPERTY);
				ics_add_item (file_entry, tb, te, p, r);
			}
			n++;
		}

	} else if (icalcomponent_get_first_component (ics_component, ICAL_VCALENDAR_COMPONENT) != NULL) {

		for (l = icalcomponent_get_first_component (ics_component, ICAL_VCALENDAR_COMPONENT); l;
			 l = icalcomponent_get_next_component (ics_component, ICAL_VCALENDAR_COMPONENT)) {

			h = icalcomponent_get_first_component (l, ICAL_VEVENT_COMPONENT);
			tb = icalcomponent_get_first_property (h, ICAL_DTSTART_PROPERTY);
            if (tb == NULL) continue;
			te = icalcomponent_get_first_property (h, ICAL_DTEND_PROPERTY);
            if (te == NULL) continue;
			p = icalcomponent_get_first_property (h, ICAL_SUMMARY_PROPERTY);
			if (p != NULL && count_only == FALSE) {
				r = icalcomponent_get_first_property (h, ICAL_DESCRIPTION_PROPERTY);
				ics_add_item (file_entry, tb, te, p, r);
			}
			n++;
		}

	} else if (icalcomponent_get_first_component (ics_component, ICAL_VTODO_COMPONENT) != NULL) {

		for (l = icalcomponent_get_first_component (ics_component, ICAL_VTODO_COMPONENT); l;
			 l = icalcomponent_get_next_component (ics_component, ICAL_VTODO_COMPONENT)) {

			tb = icalcomponent_get_first_property (l, ICAL_DTSTART_PROPERTY);
            if (tb == NULL) continue;
			te = icalcomponent_get_first_property (l, ICAL_DTEND_PROPERTY);
            if (te == NULL) continue;
			p = icalcomponent_get_first_property (l, ICAL_SUMMARY_PROPERTY);
			if (p != NULL && count_only == FALSE) {
				r = icalcomponent_get_first_property (l, ICAL_DESCRIPTION_PROPERTY);
				ics_add_item (file_entry, tb, te, p, r);
			}
			n++;
		}
	}

	icalcomponent_free (ics_component);
	fclose (ics_stream);
	icalparser_free (ics_parser);

	return n;
}

/*------------------------------------------------------------------------------*/

void
ics_check_if_valid (GUI *appGUI) {

gint i;
gchar *ical_filename, *ical_name;
GtkTreeIter iter;
GdkPixbuf *image;
GtkWidget *helper;
const gchar *stock_id;
gboolean status, disable;

    helper = gtk_image_new ();
    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {

        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, 
                           ICAL_COLUMN_NAME, &ical_name, ICAL_COLUMN_FILENAME, &ical_filename, 
						   ICAL_COLUMN_DISABLED, &disable, -1);

		status = !g_access (ical_filename, F_OK | R_OK);

        if (status == TRUE) {
            if (ics_parse_file (NULL, ical_name, ical_filename, FALSE, TRUE, appGUI) == 0) {
                status = FALSE;
            }
        }

        stock_id = (status == TRUE) ? OSMO_STOCK_LIST_VALID : OSMO_STOCK_LIST_INVALID;
        image = gtk_widget_render_icon (helper, stock_id, GTK_ICON_SIZE_MENU, NULL);
        gtk_list_store_set(appGUI->opt->calendar_ical_files_store, &iter,
                           ICAL_COLUMN_VALID_ICON, image, ICAL_COLUMN_VALID_FLAG, status, 
						   ICAL_COLUMN_STATE, (status && !disable), -1);

        g_object_unref (image);
        g_free (ical_name);
        g_free (ical_filename);
    }

}

/*------------------------------------------------------------------------------*/

gboolean
ics_check_event (guint32 julian, GUI *appGUI)
{
	GtkTreeIter iter;
	GSList *node, *node2;
	struct ics_file *entry;
	struct ics_entry *item;
	gboolean valid_ics, year_flag, mark_flag, disabled;
	gboolean found;
	gchar *ical_filename;
	gint i;

	if (!g_date_valid_julian (julian)) return FALSE;

	found = FALSE;

	for (node = appGUI->cal->ics_files_list; node != NULL; node = node->next) {

		entry = node->data;

		i = 0;
		while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {

			gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter,
			                    ICAL_COLUMN_FILENAME, &ical_filename,
			                    ICAL_COLUMN_VALID_FLAG, &valid_ics,
			                    ICAL_COLUMN_DISABLED, &disabled,
			                    ICAL_COLUMN_MARK, &mark_flag,
			                    ICAL_COLUMN_FULL_DATE, &year_flag, -1);

			if (!strcmp (entry->filename, ical_filename)) {
				g_free (ical_filename);
				break;
			}

			g_free (ical_filename);
		}

		if (entry->entries_list != NULL && valid_ics == TRUE && disabled == FALSE && mark_flag == TRUE && year_flag == TRUE) {

			for (node2 = entry->entries_list; node2 != NULL; node2 = node2->next) {
				item = node2->data;
	
				if (item->julian == julian) {
						found = TRUE;
						break;
				}
			}
		}

		if (found == TRUE) break;
	}

	return found;
}

/*------------------------------------------------------------------------------*/

gint
sFunc (struct ics_entry *a, struct ics_entry *b)
{
  return (a->seconds_begin > b->seconds_begin);
}

void
calendar_display_ics (GDate *date, GUI *appGUI)
{
	GtkTreeIter iter;
	GSList *node, *node2;
    GSList *sorted_list;
	struct ics_file *entry;
	struct ics_entry *item, *sitem;
	gboolean valid_ics, desc_flag, year_flag, mark_flag, title_flag, disabled;
	gchar *ical_name, *ical_filename;
	gint i, day, month, year;
	gint jd, jm, jy;

	utl_date_get_dmy (date, &day, &month, &year);

	for (node = appGUI->cal->ics_files_list; node != NULL; node = node->next) {
		entry = node->data;
		i = 0;

		while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {

			gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter,
			                    ICAL_COLUMN_NAME, &ical_name,
			                    ICAL_COLUMN_FILENAME, &ical_filename,
			                    ICAL_COLUMN_VALID_FLAG, &valid_ics,
			                    ICAL_COLUMN_DISABLED, &disabled,
			                    ICAL_COLUMN_ENABLE_DESC, &desc_flag,
			                    ICAL_COLUMN_MARK, &mark_flag,
			                    ICAL_COLUMN_FULL_DATE, &year_flag, -1);

			if (!strcmp (entry->filename, ical_filename)) {
				g_free (ical_filename);
				break;
			}

			g_free (ical_name);
			g_free (ical_filename);
		}

		title_flag = FALSE;
		sorted_list = NULL;

		if (entry->entries_list != NULL && valid_ics == TRUE && disabled == FALSE) {

			for (node2 = entry->entries_list; node2 != NULL; node2 = node2->next) {
				item = node2->data;

				utl_date_julian_to_dmy (item->julian, &jd, &jm, &jy);

				if (day == jd && month == jm) {
					if (year_flag == FALSE || year == jy) {
						sitem = g_malloc (sizeof (struct ics_entry));
						if (sitem != NULL) {
							sitem->julian = item->julian;
							sitem->seconds_begin = item->seconds_begin;
							sitem->seconds_end = item->seconds_end;
							sitem->summary = item->summary;
							sitem->description = item->description;
							sorted_list = g_slist_insert_sorted (sorted_list, sitem, (GCompareFunc)sFunc);
						}

					}
				}
			}

			/* add to panel sorted items */

			if (sorted_list != NULL) {
				for (node2 = sorted_list; node2 != NULL; node2 = node2->next) {
					item = node2->data;
					if (title_flag == FALSE) {
						add_ics_entry_to_day_info_panel (ical_name, item, desc_flag, appGUI);
						title_flag = TRUE;
					} else {
						add_ics_entry_to_day_info_panel (NULL, item, desc_flag, appGUI);
					}
				}
			}

			g_slist_foreach (sorted_list, (GFunc) g_free, NULL);
			g_slist_free (sorted_list);
		}

		g_free (ical_name);
	}
}

/*------------------------------------------------------------------------------*/

void
ics_calendar_init (GUI *appGUI) {

struct ics_file *ics_file_entry;
gchar *ical_name, *ical_filename;
GtkTreeIter iter;
gint i;
gboolean flag, desc_flag, disabled;

    ics_initialize_timezone ();

    appGUI->cal->ics_files_list = NULL;

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {

        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, 
                           ICAL_COLUMN_NAME, &ical_name, ICAL_COLUMN_FILENAME, &ical_filename, 
                           ICAL_COLUMN_VALID_FLAG, &flag, ICAL_COLUMN_DISABLED, &disabled,
						   ICAL_COLUMN_ENABLE_DESC, &desc_flag, -1);

        if (flag == TRUE && disabled == FALSE) {

            ics_file_entry = g_malloc(sizeof(struct ics_file));

            if (ics_file_entry != NULL) {

                ics_file_entry->filename = g_strdup(ical_filename);
                ics_file_entry->entries_list = NULL;

                ics_parse_file (ics_file_entry, ical_name, ical_filename, desc_flag, FALSE, appGUI);

                appGUI->cal->ics_files_list = g_slist_append (appGUI->cal->ics_files_list, ics_file_entry);
            }
        }

        g_free (ical_name);
        g_free (ical_filename);
    }

}

/*------------------------------------------------------------------------------*/

void
ics_calendar_close (GUI *appGUI)
{
	GSList *node, *node2;
	struct ics_file *entry;
	struct ics_entry *item;

    for (node = appGUI->cal->ics_files_list; node != NULL; node = node->next) {
        entry = node->data;

        g_free (entry->filename);

        if (entry->entries_list != NULL) {

            for (node2 = entry->entries_list; node2 != NULL; node2 = node2->next) {
                item = node2->data;
                g_free (item->summary);
                g_free (item->description);
                g_free (item);
            }

			g_slist_free (entry->entries_list);
			entry->entries_list = NULL;
        }

        g_free (entry);
    }

	g_slist_free (appGUI->cal->ics_files_list);
	appGUI->cal->ics_files_list = NULL;
}

/*------------------------------------------------------------------------------*/

void
read_ical_entries (GUI *appGUI) {

xmlDocPtr doc;
xmlChar *key;
xmlNodePtr node, entry_node, main_node;
gboolean desc_flag = FALSE, mark_flag = FALSE, disabled = FALSE;
gchar item_name[BUFFER_SIZE], item_filename[BUFFER_SIZE];
GtkTreeIter iter;

    if (g_file_test (prefs_get_config_filename(ICAL_ENTRIES_FILENAME, appGUI), G_FILE_TEST_IS_REGULAR) == FALSE) 
        return;

    if((doc = xmlParseFile(prefs_get_config_filename(ICAL_ENTRIES_FILENAME, appGUI)))) {

        if(!(node = xmlDocGetRootElement(doc))) {
            xmlFreeDoc(doc);
            return;
        }

        if (xmlStrcmp(node->name, (const xmlChar *) ICAL_NAME)) {
            xmlFreeDoc(doc);
            return;
        }

        main_node = node->xmlChildrenNode;

        while (main_node != NULL) {

            if(!xmlStrcmp(main_node->name, (xmlChar *) "entry")) {

                entry_node = main_node->xmlChildrenNode;

                while (entry_node != NULL) {

                    if ((!xmlStrcmp(entry_node->name, (const xmlChar *) "name"))) {
                        key = xmlNodeListGetString(doc, entry_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            strncpy (item_name, (gchar *) key, BUFFER_SIZE-1);
                            xmlFree(key);
                        }
                    }
                    if ((!xmlStrcmp(entry_node->name, (const xmlChar *) "filename"))) {
                        key = xmlNodeListGetString(doc, entry_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            strncpy (item_filename, (gchar *) key, BUFFER_SIZE-1);
                            xmlFree(key);
                        }
                    }
                    if ((!xmlStrcmp(entry_node->name, (const xmlChar *) "disabled"))) {
                        key = xmlNodeListGetString(doc, entry_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            disabled = atoi((gchar *) key);
                            xmlFree(key);
                        }
                    }
                    if ((!xmlStrcmp(entry_node->name, (const xmlChar *) "description"))) {
                        key = xmlNodeListGetString(doc, entry_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            desc_flag = atoi((gchar *) key);
                            xmlFree(key);
                        }
                    }
                    if ((!xmlStrcmp(entry_node->name, (const xmlChar *) "mark"))) {
                        key = xmlNodeListGetString(doc, entry_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            mark_flag = atoi((gchar *) key);
                            xmlFree(key);
                        }
                    }
                    if ((!xmlStrcmp(entry_node->name, (const xmlChar *) "use_year"))) {
                        key = xmlNodeListGetString(doc, entry_node->xmlChildrenNode, 1);
                        if (key != NULL) {
                            gtk_list_store_append(appGUI->opt->calendar_ical_files_store, &iter);
                            gtk_list_store_set(appGUI->opt->calendar_ical_files_store, &iter, 
                                               ICAL_COLUMN_NAME, item_name, ICAL_COLUMN_FILENAME, item_filename, 
                                               ICAL_COLUMN_ENABLE_DESC, desc_flag, 
                                               ICAL_COLUMN_MARK, mark_flag, 
											   ICAL_COLUMN_DISABLED, disabled,
											   ICAL_COLUMN_STATE, !disabled,
                                               ICAL_COLUMN_FULL_DATE, atoi((gchar *) key), -1); 
                            xmlFree(key);
                        }
                    }

                    entry_node = entry_node->next;
                }
            }
            main_node = main_node->next;
        }

        xmlFreeDoc(doc);
    }

    ics_check_if_valid (appGUI);
    ics_calendar_init (appGUI);
}

/*------------------------------------------------------------------------------*/

void
write_ical_entries (GUI *appGUI) {

xmlDocPtr doc;
xmlNodePtr main_node, entry_node;
xmlAttrPtr attr;
GtkTreeIter iter;
gint i;
gchar *ical_name, *ical_filename;
gboolean desc_flag, mark_flag, year_flag, disabled;
gchar tmpbuf[BUFFER_SIZE];

    doc = xmlNewDoc((const xmlChar *) "1.0");
    attr = xmlNewDocProp (doc, (const xmlChar *) "encoding", (const xmlChar *) "utf-8");

    main_node = xmlNewNode(NULL, (const xmlChar *) ICAL_NAME);
    xmlDocSetRootElement(doc, main_node);

    i = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {

        gtk_tree_model_get (GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, 
                            ICAL_COLUMN_NAME, &ical_name, ICAL_COLUMN_FILENAME, &ical_filename, 
                            ICAL_COLUMN_ENABLE_DESC, &desc_flag, 
                            ICAL_COLUMN_MARK, &mark_flag, 
							ICAL_COLUMN_FULL_DATE, &year_flag, 
							ICAL_COLUMN_DISABLED, &disabled, -1);

        entry_node = xmlNewChild (main_node, NULL, (const xmlChar *) "entry", (xmlChar *) NULL);
        xmlNewChild (entry_node, NULL, (const xmlChar *) "name", (xmlChar *) ical_name);
        g_free (ical_name);
        xmlNewChild (entry_node, NULL, (const xmlChar *) "filename", (xmlChar *) ical_filename);
        g_free (ical_filename);
        sprintf (tmpbuf, "%d", disabled);
        xmlNewChild (entry_node, NULL, (const xmlChar *) "disabled", (xmlChar *) tmpbuf);
        sprintf (tmpbuf, "%d", desc_flag);
        xmlNewChild (entry_node, NULL, (const xmlChar *) "description", (xmlChar *) tmpbuf);
        sprintf (tmpbuf, "%d", mark_flag);
        xmlNewChild (entry_node, NULL, (const xmlChar *) "mark", (xmlChar *) tmpbuf);
        sprintf (tmpbuf, "%d", year_flag);
        xmlNewChild (entry_node, NULL, (const xmlChar *) "use_year", (xmlChar *) tmpbuf);
    }

	xmlSaveFormatFileEnc (prefs_get_config_filename(ICAL_ENTRIES_FILENAME, appGUI), doc, "utf-8", 1);
	xmlFreeProp (attr);
	xmlFreeDoc (doc);

	ics_calendar_close (appGUI);
}

/*------------------------------------------------------------------------------*/

void
ics_calendar_refresh (GUI *appGUI) {

    ics_calendar_close (appGUI);
    ics_calendar_init (appGUI);
    cal_set_day_info (appGUI);
}

/*------------------------------------------------------------------------------*/

void
ical_events_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

	gtk_window_get_size (GTK_WINDOW (appGUI->cal->events_window),
	                     &config.ib_window_size_x, &config.ib_window_size_y);
    gtk_widget_destroy (appGUI->cal->events_window);
}

/*------------------------------------------------------------------------------*/

void
button_ical_events_window_close_cb (GtkButton *button, gpointer user_data) {

    ical_events_window_close_cb (GTK_WIDGET(button), NULL, user_data);

}

/*------------------------------------------------------------------------------*/

gint 
ical_events_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {

    GUI *appGUI = (GUI *)data;

    switch(event->keyval) {

        case GDK_Escape:
			ical_events_window_close_cb (NULL, NULL, appGUI);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
ical_events_list_create (gint calendar, GUI *appGUI)
{
	struct ics_file *entry;
	struct ics_entry *item;
	GtkTreeIter iter;
	GSList *node;
	gboolean year_flag = FALSE, valid_flag, disabled;
	guint32 julian;
	gint j, n, cday, cmonth, cyear;
	gint jd, jm, jy;
	gchar tmpbuf[BUFFER_SIZE];

	gtk_list_store_clear (GTK_LIST_STORE (appGUI->cal->ical_events_list_store));
	utl_date_get_current_dmy (&cday, &cmonth, &cyear);

	/* get year flag */
	j = n = 0;
	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter, NULL, j++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter,
		                    ICAL_COLUMN_FULL_DATE, &year_flag, ICAL_COLUMN_VALID_FLAG, &valid_flag, 
							ICAL_COLUMN_DISABLED, &disabled, -1);
		if (valid_flag == FALSE || disabled == TRUE) continue;
		if (n == calendar) break;
		n++;
	}

	entry = g_slist_nth_data (appGUI->cal->ics_files_list, calendar);
	if (entry->entries_list == NULL) return;

	for (j = 0, node = entry->entries_list; node != NULL; node = node->next, j++) {
		item = g_slist_nth_data (entry->entries_list, j);
		gtk_list_store_append (appGUI->cal->ical_events_list_store, &iter);
		julian = item->julian;
		utl_date_julian_to_dmy (julian, &jd, &jm, &jy);

		if (year_flag == TRUE) {

			g_snprintf (tmpbuf, BUFFER_SIZE, "%s (%s)",
					   	julian_to_str (julian, config.date_format, config.override_locale_settings),
						utl_date_print_j (julian, DATE_DAY_OF_WEEK_NAME, config.override_locale_settings));

			gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter,
			                    I_COLUMN_DATE, tmpbuf, -1);

			if (item->seconds_begin != 0 && item->seconds_end !=0) {
				g_snprintf (tmpbuf, BUFFER_SIZE, "%s - %s", 
							utl_time_print_s (item->seconds_begin, TIME_HH_MM, config.override_locale_settings),
							utl_time_print_s (item->seconds_end, TIME_HH_MM, config.override_locale_settings));
				gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter,
									I_COLUMN_TIME, tmpbuf, -1);
			} else if (item->seconds_begin != 0) {
				g_snprintf (tmpbuf, BUFFER_SIZE, "%s", 
							utl_time_print_s (item->seconds_begin, TIME_HH_MM, config.override_locale_settings));
				gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter,
									I_COLUMN_TIME, tmpbuf, -1);
			} else {
				gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter,
									I_COLUMN_TIME, "-", -1);
			}

		} else {
			julian = utl_date_dmy_to_julian (jd, jm, LEAP_YEAR);
			gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter,
			                    I_COLUMN_DATE, julian_to_str (julian, DATE_NAME_DAY, config.override_locale_settings), 
								I_COLUMN_TIME, "-", -1);
		}

		gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter,
		                    I_COLUMN_DATE_JULIAN, julian,
							I_COLUMN_TIME_B_SECONDS, item->seconds_begin,
							I_COLUMN_TIME_E_SECONDS, item->seconds_end,
		                    I_COLUMN_SUMMARY, str_remove_backslash (item->summary),
		                    I_COLUMN_FONT_WEIGHT, PANGO_WEIGHT_NORMAL, -1);

		if (jd == cday && jm == cmonth && (year_flag == FALSE || jy == cyear))
			gtk_list_store_set (appGUI->cal->ical_events_list_store, &iter, I_COLUMN_FONT_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	}
}

/*------------------------------------------------------------------------------*/

void
calendar_update_items (gint calendar, GUI *appGUI)
{
	struct ics_file *entry;
	gchar tmpbuf[BUFFER_SIZE];
	gint i;

	entry = g_slist_nth_data (appGUI->cal->ics_files_list, calendar);
	g_return_if_fail (entry->entries_list != NULL);

	i = g_slist_length (entry->entries_list);

	g_snprintf (tmpbuf, BUFFER_SIZE, "<i>%4d %s</i>", i, ngettext ("event", "events", i));
	gtk_label_set_markup (GTK_LABEL (appGUI->cal->n_items_label), tmpbuf);
}

/*------------------------------------------------------------------------------*/

void
calendar_selected_cb (GtkComboBox *widget, gpointer user_data) {

gint n;

    GUI *appGUI = (GUI *)user_data;

    n = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    calendar_update_items (n, appGUI);
    ical_events_list_create (n, appGUI);

	/* sort trigger */
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *)appGUI->cal->ical_events_sort, 
                                          I_COLUMN_DATE_JULIAN, GTK_SORT_ASCENDING);

}

/*------------------------------------------------------------------------------*/

gint 
custom_ical_events_sort_function (GtkTreeModel *model, GtkTreeIter *iter_a, GtkTreeIter *iter_b, gpointer user_data) {

guint32 julian_a, julian_b, julian_s;
gint time_a, time_b, time_s;

    if(iter_a == NULL || iter_b == NULL) {
        return 0;
    }

    gtk_tree_model_get (model, iter_a, I_COLUMN_DATE_JULIAN, &julian_a, I_COLUMN_TIME_B_SECONDS, &time_a, -1);
    gtk_tree_model_get (model, iter_b, I_COLUMN_DATE_JULIAN, &julian_b, I_COLUMN_TIME_B_SECONDS, &time_b, -1);
 
	julian_s = julian_a - julian_b;
	time_s = time_a - time_b;

	if (julian_s == 0 && time_s != 0)
		return time_s;
	if (julian_s != 0)
		return julian_s;

	return 0;
}

/*------------------------------------------------------------------------------*/
void
ical_events_browser (GUI *appGUI) {

GtkWidget           *vbox1;
GtkWidget           *hseparator;
GtkWidget           *hbuttonbox;
GtkWidget           *close_button;
GtkTreeIter         iter;
GtkTreeViewColumn   *column;
GtkCellRenderer     *renderer;
GtkWidget           *scrolledwindow;
GtkWidget           *table;
GtkWidget           *label;
gchar tmpbuf[BUFFER_SIZE];
gchar *ical_name;
gboolean valid, disabled;
gint i, k;

    i = k = 0;
	while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {
        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, 
                           ICAL_COLUMN_VALID_FLAG, &valid, ICAL_COLUMN_DISABLED, &disabled, -1);
		if (valid == TRUE && disabled == FALSE) k++;
    }

    if (k < 1) {
        utl_gui_create_dialog (GTK_MESSAGE_INFO, _("No valid calendars defined"), GTK_WINDOW(appGUI->main_window));
        return;
    }

    appGUI->cal->events_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (appGUI->cal->events_window), _("iCalendar events"));
    gtk_window_set_position (GTK_WINDOW (appGUI->cal->events_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_default_size (GTK_WINDOW(appGUI->cal->events_window), config.ib_window_size_x, config.ib_window_size_y);
    gtk_window_set_modal (GTK_WINDOW (appGUI->cal->events_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->cal->events_window), "delete_event",
                      G_CALLBACK(ical_events_window_close_cb), appGUI);
    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cal->events_window), GTK_WINDOW(appGUI->main_window));
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->events_window), 8);
    g_signal_connect (G_OBJECT (appGUI->cal->events_window), "key_press_event",
                      G_CALLBACK (ical_events_key_press_cb), appGUI);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cal->events_window), vbox1);


    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 6);

    table = gtk_table_new (1, 4, FALSE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox1), table, FALSE, TRUE, 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

    sprintf(tmpbuf, "<b>%s:</b>", _("Calendar"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    appGUI->cal->ical_combobox = gtk_combo_box_new_text ();
    gtk_widget_show (appGUI->cal->ical_combobox);
    gtk_combo_box_set_focus_on_click (GTK_COMBO_BOX (appGUI->cal->ical_combobox), FALSE);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cal->ical_combobox, GTK_CAN_FOCUS);
    gtk_table_attach (GTK_TABLE (table), appGUI->cal->ical_combobox, 1, 2, 0, 1,
                     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                     (GtkAttachOptions) (GTK_FILL), 0, 0);
    g_signal_connect(appGUI->cal->ical_combobox, "changed", 
                     G_CALLBACK(calendar_selected_cb), appGUI);

    k = 0;

    while (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, NULL, k++)) {

        gtk_tree_model_get(GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, 
                           ICAL_COLUMN_NAME, &ical_name, ICAL_COLUMN_VALID_FLAG, &valid, 
						   ICAL_COLUMN_DISABLED, &disabled, -1);

		if (valid == TRUE && disabled == FALSE) {
	        gtk_combo_box_append_text (GTK_COMBO_BOX (appGUI->cal->ical_combobox), ical_name);
		}
        g_free (ical_name);
    }

    appGUI->cal->n_items_label = gtk_label_new ("");
    gtk_widget_show (appGUI->cal->n_items_label);
    gtk_widget_set_size_request (appGUI->cal->n_items_label, 100, -1);
    gtk_table_attach (GTK_TABLE (table), appGUI->cal->n_items_label, 3, 4, 0, 1,
                     (GtkAttachOptions) (GTK_FILL),
                     (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (appGUI->cal->n_items_label), TRUE);

    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator, FALSE, TRUE, 6);


    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow, TRUE, TRUE, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    appGUI->cal->ical_events_list_store = gtk_list_store_new(ICAL_EVENTS_NUM_COLUMNS, 
                                                             G_TYPE_STRING, G_TYPE_INT, 
                                                             G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT,
															 G_TYPE_STRING, G_TYPE_INT);

	appGUI->cal->ical_events_sort = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL(appGUI->cal->ical_events_list_store));

    appGUI->cal->ical_events_list = gtk_tree_view_new_with_model (GTK_TREE_MODEL(appGUI->cal->ical_events_sort));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(appGUI->cal->ical_events_list), FALSE);
    gtk_widget_show (appGUI->cal->ical_events_list);
    GTK_WIDGET_SET_FLAGS (appGUI->cal->ical_events_list, GTK_CAN_DEFAULT);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), appGUI->cal->ical_events_list);

    /* create columns */

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Date"), renderer, 
                                                      "text", I_COLUMN_DATE, 
                                                      "weight", I_COLUMN_FONT_WEIGHT,
                                                      NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Julian", renderer, 
                                                      "text", I_COLUMN_DATE_JULIAN, 
                                                      NULL);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);
    g_signal_emit_by_name(column, "clicked");

	renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Time"), renderer, 
                                                      "text", I_COLUMN_TIME, 
                                                      "weight", I_COLUMN_FONT_WEIGHT,
                                                      NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Begin seconds", renderer, 
                                                      "text", I_COLUMN_TIME_B_SECONDS, 
                                                      NULL);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("End seconds", renderer, 
                                                      "text", I_COLUMN_TIME_E_SECONDS, 
                                                      NULL);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    column = gtk_tree_view_column_new_with_attributes(_("Summary"), renderer, 
                                                      "text", I_COLUMN_SUMMARY, 
                                                      "weight", I_COLUMN_FONT_WEIGHT,
                                                      NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text", I_COLUMN_FONT_WEIGHT, NULL);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(appGUI->cal->ical_events_list), column);

	gtk_tree_sortable_set_sort_func ((GtkTreeSortable *)appGUI->cal->ical_events_sort, I_COLUMN_DATE_JULIAN, 
                                     (GtkTreeIterCompareFunc)custom_ical_events_sort_function, NULL, NULL);

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
    g_signal_connect(close_button, "clicked", G_CALLBACK(button_ical_events_window_close_cb), appGUI);
    gtk_container_add(GTK_CONTAINER(hbuttonbox), close_button);

    gtk_widget_show(appGUI->cal->events_window);
    gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->cal->ical_combobox), 0);
    gtk_widget_grab_focus (close_button);
}

/*------------------------------------------------------------------------------*/

void
ical_export_window_close_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data) {

    GUI *appGUI = (GUI *)user_data;

    config.ical_export_pane_pos = gtk_paned_get_position(GTK_PANED(appGUI->cal->ical_export_vpaned));

    gtk_widget_destroy (appGUI->cal->ical_export_window);
}

/*------------------------------------------------------------------------------*/

void
button_ical_export_window_close_cb (GtkButton *button, gpointer user_data) {

    ical_export_window_close_cb (GTK_WIDGET(button), NULL, user_data);
}

/*------------------------------------------------------------------------------*/

gint 
ical_export_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {

    switch(event->keyval) {

        case GDK_Escape:
            ical_export_window_close_cb (GTK_WIDGET(widget), NULL, user_data);
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
update_notes_marks (GuiCalendar *calendar, GUI *appGUI)
{
	guint32 julian;
	gint i, days;
	GDate *date = g_date_new ();

	gui_calendar_clear_marks (GUI_CALENDAR(calendar), DAY_NOTE_MARK);
	gui_calendar_get_gdate (GUI_CALENDAR (calendar), date);

	days = utl_date_get_days_in_month (date);
	g_date_set_day (date, 1);
	julian = g_date_get_julian (date);

	for (i = 0; i < days; i++)
		if (cal_check_note (julian + i, appGUI) == TRUE)
			gui_calendar_mark_day (GUI_CALENDAR (calendar), i + 1, DAY_NOTE_MARK);

	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void
use_date_period_cb (GtkToggleButton *togglebutton, gpointer user_data) {

gboolean state;

    GUI *appGUI = (GUI *)user_data;

    state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (togglebutton));

    gtk_widget_set_sensitive (appGUI->cal->export_end_period_calendar, state);

    if (state == TRUE) {
        update_notes_marks (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), appGUI);
    } else {
        gui_calendar_clear_marks (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), DAY_NOTE_MARK);
    }
}

/*------------------------------------------------------------------------------*/

void
select_file_cb (GtkWidget *widget, gpointer data) {

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

        if (g_str_has_suffix (f_filename, ".ics") == FALSE &&
            g_str_has_suffix (f_filename, ".ICS") == FALSE) {
            g_strlcat(f_filename, ".ics", PATH_MAX-1);
        }
        gtk_entry_set_text (GTK_ENTRY(appGUI->cal->output_file_entry), f_filename);
    }

    gtk_widget_destroy(dialog);
}

/*------------------------------------------------------------------------------*/

void
calendar_begin_end_day_selected_cb (GuiCalendar *calendar, gpointer user_data)
{
    GUI *appGUI = (GUI *) user_data;
    update_notes_marks (GUI_CALENDAR (calendar), appGUI);
}

/*------------------------------------------------------------------------------*/

gint
get_export_items (GUI *appGUI) {

GtkTreeIter iter;
gint i;

	i = 0;
	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->cal->export_events_list_store), &iter, NULL, i++));

    return i-1;
}

/*------------------------------------------------------------------------------*/

void
add_export_item_to_list (guint32 julian, GUI *appGUI)
{
	GtkTreeIter iter;
	gchar *text, *stripped;
	gint i;
	gchar tmpbuf[BUFFER_SIZE];

	if (cal_check_note (julian, appGUI) == TRUE) {
		text = cal_get_note (julian, appGUI);
		stripped = utl_gui_text_strip_tags (text);

		i = 0;
		do {
			tmpbuf[i] = stripped[i];
			i++;
		} while (stripped[i] != '\n' && i < BUFFER_SIZE);

		tmpbuf[i] = '\0';

		gtk_list_store_append (appGUI->cal->export_events_list_store, &iter);
		gtk_list_store_set (appGUI->cal->export_events_list_store, &iter,
		                    IE_COLUMN_DATE, julian_to_str (julian, DATE_FULL, config.override_locale_settings),
		                    IE_COLUMN_DATE_JULIAN, julian,
		                    IE_COLUMN_SUMMARY, tmpbuf,
		                    IE_COLUMN_DESCRIPTION, stripped, -1);

		g_free (stripped);
	}
}

/*------------------------------------------------------------------------------*/

void
button_add_date_cb (GtkButton *button, GUI *appGUI)
{
	guint32 julian, begin_julian, end_julian;
	GDate *date = g_date_new ();

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->cal->use_date_period_checkbutton)) == FALSE) {

        gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), date);
        add_export_item_to_list (g_date_get_julian (date), appGUI);

    } else {
        gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), date);
        begin_julian = g_date_get_julian (date);
        gui_calendar_get_gdate (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), date);
        end_julian = g_date_get_julian (date);

        if (begin_julian > end_julian) {
            julian = begin_julian;
            begin_julian = end_julian;
            end_julian = julian;
        }

        for (julian = begin_julian; julian <= end_julian; julian++) {
            add_export_item_to_list (julian, appGUI);
        }
    }

	g_date_free (date);
}

/*------------------------------------------------------------------------------*/

void 
summary_edited_cb (GtkCellRendererText *renderer, gchar *path, 
                   gchar *new_text, gpointer user_data) {

GtkTreeIter iter;
GtkTreeModel *model;
  
    GUI *appGUI = (GUI *)user_data;

    if (g_ascii_strcasecmp (new_text, "") != 0) {
        model = gtk_tree_view_get_model (GTK_TREE_VIEW(appGUI->cal->export_events_list_treeview));
        if (gtk_tree_model_get_iter_from_string (model, &iter, path)) {
            gtk_list_store_set(appGUI->cal->export_events_list_store, &iter, IE_COLUMN_SUMMARY, new_text, -1);
        }
    }
}

/*------------------------------------------------------------------------------*/

void
button_remove_date_cb (GtkButton *button, gpointer user_data) {

GtkTreeIter iter;
GtkTreePath *path;

    GUI *appGUI = (GUI *)user_data;

    gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->cal->export_events_list_treeview), &path, NULL);

    if (path != NULL) {
        gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->cal->export_events_list_store), &iter, path);
        gtk_list_store_remove (appGUI->cal->export_events_list_store, &iter);
    }
}

/*------------------------------------------------------------------------------*/

void
button_clear_list_cb (GtkButton *button, gpointer user_data) {

gint response;
gchar tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)user_data;

    if (!get_export_items (appGUI)) return;

    sprintf (tmpbuf, "%s\n\n%s", _("The list will be cleared and all entries will be lost."), _("Are you sure?"));

    response = utl_gui_create_dialog (GTK_MESSAGE_QUESTION, tmpbuf, GTK_WINDOW(appGUI->cal->ical_export_window));

    if (response == GTK_RESPONSE_YES) {   
        gtk_list_store_clear (appGUI->cal->export_events_list_store);
    }
}

/*------------------------------------------------------------------------------*/

void
export_list_item_selected (GtkTreeSelection *selection, gpointer user_data) {

GtkTreeIter iter;
GtkTextIter titer;
GtkTreeModel *model;
GtkTextBuffer *text_buffer;
gchar *text;

    GUI *appGUI = (GUI *)user_data;
    
    text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(appGUI->cal->export_day_note_textview));
    utl_gui_clear_text_buffer (text_buffer, &titer);

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, IE_COLUMN_DESCRIPTION, &text, -1);
        gtk_text_buffer_insert (text_buffer, &titer, text, -1);
        g_free (text);
    }
}

/*------------------------------------------------------------------------------*/

void
ical_export_cb (GtkButton *button, gpointer user_data) {

GtkTreeIter iter;
gchar *filename;
gint i;
guint32 julian_day;
gint day, month, year;
gchar *summary, *description;
icalcomponent *event, *calendar;
icalproperty *prop;
icalset *ics_file;
struct icaltimetype atime;
gchar tmpbuf[BUFFER_SIZE];

    GUI *appGUI = (GUI *)user_data;

    filename = g_strdup (gtk_entry_get_text(GTK_ENTRY(appGUI->cal->output_file_entry)));

    if (get_export_items(appGUI) == 0 || strlen(filename) == 0) {
        g_free (filename);
        return;
    }

    if (utl_gui_check_overwrite_file (filename, appGUI->cal->ical_export_window, appGUI) != 0) {
        return;
    } else {
        g_unlink (filename);
    }

    ics_file = icalset_new_file (filename); 

    if (ics_file == NULL) {
        ical_export_window_close_cb (NULL, NULL, user_data);
        g_free (filename);
        return;
    }

    i = 0;

    calendar = icalcomponent_new (ICAL_VCALENDAR_COMPONENT);

    prop = icalproperty_new_prodid ("//Clay//NONSGML Osmo PIM//EN");
    icalcomponent_add_property (calendar, prop);
    prop = icalproperty_new_version (VERSION);
    icalcomponent_add_property (calendar, prop);

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->cal->export_events_list_store), &iter, NULL, i++)) {

		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->cal->export_events_list_store), &iter,
		                    IE_COLUMN_DATE_JULIAN, &julian_day, 
                            IE_COLUMN_SUMMARY, &summary,
                            IE_COLUMN_DESCRIPTION, &description, -1);

        event = icalcomponent_new (ICAL_VEVENT_COMPONENT);

		utl_date_julian_to_dmy (julian_day, &day, &month, &year);
        atime.day = day;
        atime.month = month;
        atime.year = year;
        atime.is_date = 1;
		g_snprintf (tmpbuf, BUFFER_SIZE, "id%d%d%d%d@clayo.org", year, month, day, i);
        prop = icalproperty_new_uid (tmpbuf);
        icalcomponent_add_property (event, prop);
        icalproperty_free (prop);
        prop = icalproperty_new_summary (summary);
        icalcomponent_add_property (event, prop);
        icalproperty_free (prop);
        prop = icalproperty_new_description (description);
        icalcomponent_add_property (event, prop);
        icalproperty_free (prop);
        prop = icalproperty_new_dtstart (atime);
        icalcomponent_add_property (event, prop);
        icalproperty_free (prop);
        prop = icalproperty_new_dtend (atime);
        icalcomponent_add_property (event, prop);
        icalproperty_free (prop);

        icalcomponent_add_component (calendar, event);

        icalcomponent_free (event);
        g_free (summary);
        g_free (description);
    }

    icalset_add_component (ics_file, calendar);
    icalfileset_commit (ics_file);

    icalcomponent_free (calendar);
    g_free (filename);

    if (i >= 2) {
        sprintf(tmpbuf, "%s %d %s.\n", _("Done!"), i-1,
				ngettext ("event exported", "events exported", i-1));

        utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW(appGUI->cal->ical_export_window));
    }

    ical_export_window_close_cb (NULL, NULL, user_data);
}

/*------------------------------------------------------------------------------*/

void
ical_export (GUI *appGUI) {

GtkWidget *vbox1, *vbox2;
GtkWidget *hseparator;
GtkWidget *hbuttonbox;
GtkWidget *export_button;
GtkWidget *cancel_button;
GtkWidget *hbox1, *hbox2, *hbox3, *hbox4;
GtkWidget *frame;
GtkWidget *alignment;
GtkWidget *browse_dir_button;
GtkWidget *label;
GtkWidget *add_item_button;
GtkWidget *remove_item_button;
GtkWidget *clear_list_button;
GtkWidget *el_scrolledwindow;
GtkWidget *dn_scrolledwindow;
GtkTreeViewColumn *column;
GtkCellRenderer *renderer;
gchar tmpbuf[BUFFER_SIZE];

    appGUI->cal->ical_export_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (appGUI->cal->ical_export_window), _("iCalendar export"));
    gtk_window_set_position (GTK_WINDOW (appGUI->cal->ical_export_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_default_size (GTK_WINDOW(appGUI->cal->ical_export_window), 600, 650);
    gtk_window_set_modal (GTK_WINDOW (appGUI->cal->ical_export_window), TRUE);
    g_signal_connect (G_OBJECT (appGUI->cal->ical_export_window), "delete_event",
                      G_CALLBACK(ical_export_window_close_cb), appGUI);
    gtk_window_set_transient_for(GTK_WINDOW(appGUI->cal->ical_export_window), GTK_WINDOW(appGUI->main_window));
    gtk_container_set_border_width (GTK_CONTAINER (appGUI->cal->ical_export_window), 8);
    g_signal_connect (G_OBJECT (appGUI->cal->ical_export_window), "key_press_event",
                      G_CALLBACK (ical_export_key_press_cb), appGUI);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (appGUI->cal->ical_export_window), vbox1);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    sprintf(tmpbuf, "<b>%s:</b>", _("Day Selector"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    vbox2 = gtk_vbox_new (FALSE, 8);
    gtk_widget_show (vbox2);
    gtk_container_add (GTK_CONTAINER (alignment), vbox2);

    hbox1 = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 0);

    appGUI->cal->export_begin_period_calendar = gui_calendar_new ();
    gtk_widget_show (appGUI->cal->export_begin_period_calendar);
    gui_calendar_set_cursor_type (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), CURSOR_BLOCK);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), config.selection_color, config.selector_alpha, SELECTOR_COLOR);
    gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), config.today_marker_type, TODAY_MARKER);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
    gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), 
                                      (config.display_options & !GUI_CALENDAR_NO_MONTH_CHANGE) | 
                                      GUI_CALENDAR_SHOW_HEADING | GUI_CALENDAR_SHOW_DAY_NAMES | 
                                      (config.display_options & GUI_CALENDAR_WEEK_START_MONDAY));
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->export_begin_period_calendar, TRUE, TRUE, 0);
    update_notes_marks (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->export_begin_period_calendar), "day-selected", 
                      G_CALLBACK (calendar_begin_end_day_selected_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->export_begin_period_calendar), "day-selected-double-click", 
                      G_CALLBACK (button_add_date_cb), appGUI);

    appGUI->cal->export_end_period_calendar = gui_calendar_new ();
    gtk_widget_show (appGUI->cal->export_end_period_calendar);
    gui_calendar_set_cursor_type (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), CURSOR_BLOCK);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->export_begin_period_calendar), config.selection_color, config.selector_alpha, SELECTOR_COLOR);
    gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), config.today_marker_type, TODAY_MARKER);
    gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), config.mark_current_day_color, config.mark_current_day_alpha, TODAY_MARKER_COLOR);
    gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->export_end_period_calendar), 
                                      (config.display_options & !GUI_CALENDAR_NO_MONTH_CHANGE) | 
                                      GUI_CALENDAR_SHOW_HEADING | GUI_CALENDAR_SHOW_DAY_NAMES | 
                                      (config.display_options & GUI_CALENDAR_WEEK_START_MONDAY));
    gtk_box_pack_start (GTK_BOX (hbox1), appGUI->cal->export_end_period_calendar, TRUE, TRUE, 0);
    gtk_widget_set_sensitive (appGUI->cal->export_end_period_calendar, FALSE);
    g_signal_connect (G_OBJECT (appGUI->cal->export_end_period_calendar), "day-selected", 
                      G_CALLBACK (calendar_begin_end_day_selected_cb), appGUI);
    g_signal_connect (G_OBJECT (appGUI->cal->export_end_period_calendar), "day-selected-double-click", 
                      G_CALLBACK (button_add_date_cb), appGUI);

    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox2);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, TRUE, 0);

    appGUI->cal->use_date_period_checkbutton = gtk_check_button_new_with_mnemonic (_("Use date period"));
    gtk_widget_show (appGUI->cal->use_date_period_checkbutton);
    GTK_WIDGET_UNSET_FLAGS (appGUI->cal->use_date_period_checkbutton, GTK_CAN_FOCUS);
    g_signal_connect (G_OBJECT (appGUI->cal->use_date_period_checkbutton), "toggled",
                      G_CALLBACK (use_date_period_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox2), appGUI->cal->use_date_period_checkbutton, FALSE, FALSE, 0);

	add_item_button = utl_gui_create_button (GTK_STOCK_ADD, OSMO_STOCK_BUTTON_ADD, _("Add"));
    gtk_widget_show (add_item_button);
    GTK_WIDGET_UNSET_FLAGS (add_item_button, GTK_CAN_FOCUS);
    gtk_box_pack_end (GTK_BOX (hbox2), add_item_button, FALSE, FALSE, 0);
    g_signal_connect(add_item_button, "clicked", G_CALLBACK(button_add_date_cb), appGUI);

    appGUI->cal->ical_export_vpaned = gtk_vpaned_new ();
    gtk_widget_show (appGUI->cal->ical_export_vpaned);
    gtk_box_pack_start (GTK_BOX (vbox2), appGUI->cal->ical_export_vpaned, TRUE, TRUE, 0);
    gtk_paned_set_position (GTK_PANED (appGUI->cal->ical_export_vpaned), 0);
    gtk_paned_set_position(GTK_PANED(appGUI->cal->ical_export_vpaned), config.ical_export_pane_pos);

    el_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (el_scrolledwindow);
    gtk_paned_pack1 (GTK_PANED (appGUI->cal->ical_export_vpaned), el_scrolledwindow, TRUE, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (el_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (el_scrolledwindow), GTK_SHADOW_IN);

	appGUI->cal->export_events_list_store = gtk_list_store_new (ICAL_EXPORT_NUM_COLUMNS,
                                                                G_TYPE_STRING, G_TYPE_INT, 
                                                                G_TYPE_STRING, G_TYPE_STRING);

    appGUI->cal->export_events_list_treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(appGUI->cal->export_events_list_store));
    gtk_widget_show (appGUI->cal->export_events_list_treeview);
    gtk_container_add (GTK_CONTAINER (el_scrolledwindow), appGUI->cal->export_events_list_treeview);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (appGUI->cal->export_events_list_treeview), TRUE);

    appGUI->cal->export_events_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (appGUI->cal->export_events_list_treeview));
    g_signal_connect (G_OBJECT(appGUI->cal->export_events_list_selection), "changed",
                      G_CALLBACK(export_list_item_selected), appGUI);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Date"),
                              renderer,
                              "text", IE_COLUMN_DATE,
                              NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->cal->export_events_list_treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", IE_COLUMN_DATE_JULIAN,
                              NULL);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->cal->export_events_list_treeview), column);

    renderer = gtk_cell_renderer_text_new();
	g_object_set (G_OBJECT(renderer), "xpad", 2, NULL);
    g_object_set (G_OBJECT(renderer), "editable", TRUE, "editable-set", TRUE, NULL);
    g_object_set (G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (summary_edited_cb), appGUI);

    column = gtk_tree_view_column_new_with_attributes(_("Summary"),
                              renderer,
                              "text", IE_COLUMN_SUMMARY,
                              NULL);
    gtk_tree_view_column_set_visible (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->cal->export_events_list_treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(NULL,
                              renderer,
                              "text", IE_COLUMN_DESCRIPTION,
                              NULL);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(appGUI->cal->export_events_list_treeview), column);

    dn_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (dn_scrolledwindow);
    gtk_paned_pack2 (GTK_PANED (appGUI->cal->ical_export_vpaned), dn_scrolledwindow, TRUE, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dn_scrolledwindow), 
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (dn_scrolledwindow), GTK_SHADOW_IN);

    appGUI->cal->export_day_note_textview = gtk_text_view_new ();
    gtk_widget_show (appGUI->cal->export_day_note_textview);
    gtk_container_add (GTK_CONTAINER (dn_scrolledwindow), appGUI->cal->export_day_note_textview);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), FALSE);
    gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), FALSE);
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), 4);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), 4);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), 4);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (appGUI->cal->export_day_note_textview), 4);

    hbox4 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox4);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox4, FALSE, TRUE, 0);

    hbuttonbox = gtk_hbutton_box_new ();
    gtk_widget_show (hbuttonbox);
    gtk_box_pack_end (GTK_BOX (hbox4), hbuttonbox, FALSE, TRUE, 0);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	clear_list_button = utl_gui_create_button (GTK_STOCK_CLEAR, OSMO_STOCK_BUTTON_CLEAR, _("Clear"));
    gtk_widget_show (clear_list_button);
    GTK_WIDGET_UNSET_FLAGS (clear_list_button, GTK_CAN_FOCUS);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), clear_list_button);
    g_signal_connect(clear_list_button, "clicked", G_CALLBACK(button_clear_list_cb), appGUI);

	remove_item_button = utl_gui_create_button (GTK_STOCK_REMOVE, OSMO_STOCK_BUTTON_REMOVE, _("Remove"));
    gtk_widget_show (remove_item_button);
    GTK_WIDGET_UNSET_FLAGS (remove_item_button, GTK_CAN_FOCUS);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), remove_item_button);
    g_signal_connect(remove_item_button, "clicked", G_CALLBACK(button_remove_date_cb), appGUI);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    sprintf(tmpbuf, "<b>%s:</b>", _("Output filename"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    hbox3 = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox3);
    gtk_container_add (GTK_CONTAINER (alignment), hbox3);

    appGUI->cal->output_file_entry = gtk_entry_new ();
    gtk_widget_show (appGUI->cal->output_file_entry);
    gtk_box_pack_start (GTK_BOX (hbox3), appGUI->cal->output_file_entry, TRUE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS(appGUI->cal->output_file_entry, GTK_CAN_FOCUS);
    gtk_editable_set_editable (GTK_EDITABLE(appGUI->cal->output_file_entry), FALSE);

	browse_dir_button = utl_gui_create_button (GTK_STOCK_OPEN, OSMO_STOCK_BUTTON_OPEN, _("Browse"));
    gtk_widget_show (browse_dir_button);
    GTK_WIDGET_UNSET_FLAGS(browse_dir_button, GTK_CAN_FOCUS);
    g_signal_connect(browse_dir_button, "clicked", G_CALLBACK(select_file_cb), appGUI);
    gtk_box_pack_start (GTK_BOX (hbox3), browse_dir_button, FALSE, FALSE, 0);

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
    gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
    GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(button_ical_export_window_close_cb), appGUI);

    export_button = gtk_button_new_with_label (_("Export"));
    gtk_widget_show (export_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox), export_button);
    GTK_WIDGET_SET_FLAGS (export_button, GTK_CAN_DEFAULT);
    g_signal_connect(export_button, "clicked", G_CALLBACK(ical_export_cb), appGUI);

    gtk_widget_show(appGUI->cal->ical_export_window);
}

/*------------------------------------------------------------------------------*/

#endif  /* HAVE_LIBICAL */

