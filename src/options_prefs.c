
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

#include "options_prefs.h"
#include "i18n.h"
#include "utils.h"
#include "utils_date.h"
#include "contacts.h"
#include "contacts_import.h"
#include "contacts_export.h"
#include "calendar.h"
#include "calendar_widget.h"
#include "calendar_notes.h"
#include "calendar_utils.h"

/*------------------------------------------------------------------------------*/

gchar* 
prefs_get_config_dir (GUI *appGUI) {

static gchar dirname[PATH_MAX];

    if (appGUI->config_path == NULL) {
#if defined(CONFIG_PATH) && defined(CONFIG_DIR)
        g_snprintf (dirname, PATH_MAX, "%s%c%s", CONFIG_PATH, G_DIR_SEPARATOR, CONFIG_DIR);
#elif defined(CONFIG_DIR)
        g_snprintf (dirname, PATH_MAX, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, CONFIG_DIR);
#elif defined(CONFIG_PATH)
        g_snprintf (dirname, PATH_MAX, "%s%c%s", CONFIG_PATH, G_DIR_SEPARATOR, CONFIG_DIRNAME);
#else
        g_snprintf (dirname, PATH_MAX, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, CONFIG_DIRNAME);
#endif
    } else {
        g_strlcpy (dirname, appGUI->config_path, PATH_MAX);
    }

	return dirname;
}

/*------------------------------------------------------------------------------*/

gchar* 
prefs_get_config_filename (gchar *config_filename, GUI *appGUI) {

static gchar filename[PATH_MAX];
gchar *dirname = NULL;
struct stat cfg;

	dirname = g_strdup (prefs_get_config_dir(appGUI));
	g_return_val_if_fail (dirname != NULL, NULL);

    if(g_stat (dirname, &cfg) < 0)
        g_mkdir (dirname, S_IRUSR | S_IWUSR | S_IXUSR);

    if (g_access (dirname, R_OK | W_OK) == -1) {
        return NULL;
    }

    g_snprintf (filename, PATH_MAX, "%s%c%s", dirname, G_DIR_SEPARATOR, config_filename);
	g_free (dirname);

    return filename;
}

/*------------------------------------------------------------------------------*/

void 
prefs_set_default_values (void) {

gint i;

    /* general */
    config.window_x = 0;
    config.window_y = 0;
    config.window_size_x = 540;
    config.window_size_y = 600;
    config.enable_tooltips = TRUE;
    config.latest_tab = 0;      /* calendar */
    config.tabs_position = GTK_POS_TOP;
    config.remember_latest_tab = TRUE;
    config.save_data_after_modification = TRUE;
    config.default_stock_icons = FALSE;
    config.disable_underline_links = FALSE;
    config.rules_hint = FALSE;
    config.date_format = DATE_YYYY_MM_DD;
    config.time_format = TIME_24;
    config.enable_systray = TRUE;
    config.start_minimised_in_systray = FALSE;
    config.blink_on_events = FALSE;
    config.ignore_day_note_events = FALSE;
    config.run_counter = 0;
    config.lastrun_date = utl_date_get_current_julian ();
    config.lastrun_time = utl_time_get_current_seconds ();
    config.hide_calendar = FALSE;
    config.hide_tasks = FALSE;
    config.hide_contacts = FALSE;
    config.hide_notes = FALSE;
    config.override_locale_settings = FALSE;
    config.gui_layout = 0;      /* 0 - vertical, 1 - horizontal */
    config.sound_alarm_repeat = 1;
	g_strlcpy (config.link_color, "blue", MAXCOLORNAME);
    g_strlcpy (config.spell_lang, g_getenv("LANG"), MAXNAME);
	g_strlcpy (config.web_browser, "xdg-open %s", MAXHELPERCMD);
	g_strlcpy (config.email_client, "xdg-email %s", MAXHELPERCMD);
	g_strlcpy (config.sound_player, "play %s", MAXHELPERCMD);

    /* calendar */
    config.fy_window_size_x = 750;
    config.fy_window_size_y = 550;
    config.fy_simple_view = TRUE;
    config.fy_alternative_view = FALSE;
    config.cb_window_size_x = 570;
    config.cb_window_size_y = 680;
    config.ib_window_size_x = 550;
    config.ib_window_size_y = 650;
    config.display_options = GUI_CALENDAR_SHOW_DAY_NAMES | GUI_CALENDAR_NO_MONTH_CHANGE | GUI_CALENDAR_WEEK_START_MONDAY;
    config.day_notes_visible = FALSE;
    config.timeline_start = 8*60;
    config.timeline_end = 15*60;
    config.timeline_step = 60;
    config.di_show_current_time = TRUE;
    config.di_show_day_number = TRUE;
    config.di_show_current_day_distance = TRUE;
    config.di_show_marked_days = TRUE;
    config.di_show_week_number = TRUE;
    config.di_show_weekend_days = TRUE;
    config.di_show_day_category = TRUE;
    config.di_show_moon_phase = TRUE;
    config.di_show_notes = TRUE;
    config.di_show_zodiac_sign = FALSE;
    config.cursor_type = CURSOR_FRAME;
    config.frame_cursor_thickness = 2;
    config.enable_auxilary_calendars = TRUE;
    config.enable_day_mark = TRUE;
    config.strikethrough_past_notes = FALSE;
    config.ascending_sorting_in_day_notes_browser = FALSE;
    config.auxilary_calendars_state = FALSE;
    config.day_note_spell_checker = FALSE;
    g_strlcpy (config.day_note_marker, "\'", MAXNAME);
    g_strlcpy (config.date_header_format, DEFAULT_DATE_HEADER_FORMAT, MAXNAME);
    config.event_marker_type = EVENT_MARKER_CIRCLE;
    config.today_marker_type = TODAY_MARKER_FREEHAND_CIRCLE;
    config.day_notes_browser_filter = DN_FILTER_CURRENT_MONTH;
    config.ical_export_pane_pos = 180;
	g_strlcpy (config.header_color, "#808080", MAXCOLORNAME);
	g_strlcpy (config.weekend_color, "#880000", MAXCOLORNAME);
	g_strlcpy (config.selection_color, "#526565", MAXCOLORNAME);
	g_strlcpy (config.mark_color, "#dddddd", MAXCOLORNAME);
	g_strlcpy (config.mark_current_day_color, "#34A434", MAXCOLORNAME);
	g_strlcpy (config.birthday_mark_color, "#FFA500", MAXCOLORNAME);
    config.mark_current_day_alpha = 32768;
    config.selector_alpha = 32768;
	g_strlcpy (config.day_name_font, "Sans Bold 18", MAXFONTNAME);
	g_strlcpy (config.calendar_font, "Sans 16", MAXFONTNAME);
	g_strlcpy (config.notes_font, "Sans 10", MAXFONTNAME);
	g_strlcpy (config.cal_print_month_name_font, "Sans Bold 36", MAXFONTNAME);
	g_strlcpy (config.cal_print_day_name_font, "Sans Bold 9", MAXFONTNAME);
	g_strlcpy (config.cal_print_day_num_font, "Sans Bold 14", MAXFONTNAME);
	g_strlcpy (config.cal_print_event_font, "Sans 6", MAXFONTNAME);
	config.cal_print_event_length = 256;
	config.cal_print_padding = 4;
	config.cal_print_page_orientation = PORTRAIT;
	config.cal_print_tasks = TRUE;
	config.cal_print_birthdays = TRUE;
	config.cal_print_namedays = TRUE;
	config.cal_print_day_notes = TRUE;
	config.cal_print_ical = TRUE;

    /* tasks */
    config.tasks_high_in_bold = TRUE;
    config.hide_completed = FALSE;
    config.delete_completed = FALSE;
    config.add_edit = FALSE;
    config.remember_category_in_tasks = FALSE;
    config.current_category_in_tasks = 0;
    config.tasks_pane_pos = 275;
    config.tasks_sorting_order = GTK_SORT_ASCENDING;
    config.tasks_sorting_mode = 0;
    config.tsk_visible_due_date_column = TRUE;
    config.tsk_visible_type_column = TRUE;
    config.tsk_visible_priority_column = FALSE;
    config.tsk_visible_category_column = FALSE;
    config.tasks_addedit_win_w = 370;
    config.tasks_addedit_win_h = 440;
    config.tasks_addedit_win_x = config.window_x+config.window_size_x/2-config.tasks_addedit_win_w/2;
    config.tasks_addedit_win_y = config.window_y+20;
    config.postpone_time = 10;
	config.tasks_column_idx_0 = 0;
	config.tasks_column_idx_0_width = 50;
	config.tasks_column_idx_1 = 1;
	config.tasks_column_idx_1_width = 50;
	config.tasks_column_idx_2 = 2;
	config.tasks_column_idx_2_width = 100;
	config.tasks_column_idx_3 = 3;
	config.tasks_column_idx_3_width = 100;
	config.tasks_column_idx_4 = 4;
	config.tasks_column_idx_4_width = 100;
	config.tasks_column_idx_5 = 5;
	config.tasks_column_idx_5_width = 100;
	g_strlcpy (config.due_today_color, "#00981E", MAXCOLORNAME);
	g_strlcpy (config.due_7days_color, "#0047B7", MAXCOLORNAME);
	g_strlcpy (config.past_due_color, "#CB362C", MAXCOLORNAME);
	g_strlcpy (config.task_info_font, "Sans 12", MAXFONTNAME);
	config.global_notification_command[0] = '\0';

    /* contacts */
    config.find_mode = CONTACTS_FF_FIRST_NAME;
    config.show_after_search = TRUE;
    config.hide_group_column = FALSE;
    config.contacts_pane_pos = 275;
    config.photo_width = 80;
    config.cnt_visible_age_column = TRUE;
    config.cnt_visible_birthday_date_column = TRUE;
    config.cnt_visible_zodiac_sign_column = TRUE;
    config.contacts_sorting_order = GTK_SORT_ASCENDING;
    config.contacts_sorting_mode = 0;
    config.contacts_addedit_win_w = 500;
    config.contacts_addedit_win_h = 600;
    config.contacts_addedit_win_x = config.window_x+config.window_size_x/2-config.contacts_addedit_win_w/2;
    config.contacts_addedit_win_y = config.window_y+20;
    config.contacts_export_win_w = 500;
    config.contacts_export_win_h = 400;
    config.contacts_export_win_x = config.window_x+config.window_size_x/2-config.contacts_export_win_w/2;
    config.contacts_export_win_y = config.window_y+20;
    config.contacts_import_win_w = 650;
    config.contacts_import_win_h = 450;
    config.contacts_import_win_x = config.window_x+config.window_size_x/2-config.contacts_import_win_w/2;
    config.contacts_import_win_y = config.window_y+20;
    config.contacts_import_sel_win_x = config.window_x+60;
    config.contacts_import_sel_win_y = config.window_y+20;
    config.contacts_birthdays_win_w = 650;
    config.contacts_birthdays_win_h = 700;
	config.contacts_column_idx_0 = 0;
	config.contacts_column_idx_0_width = 150;
	config.contacts_column_idx_1 = 1;
	config.contacts_column_idx_1_width = 150;
	config.contacts_column_idx_2 = 2;
	config.contacts_column_idx_2_width = 150;
	config.import_type = IMPORT_TYPE_FILE;
    config.import_interface_type = 0;
    config.import_bluetooth_channel = 1;
    config.import_usb_interface = 0;
    config.import_binary_xml = 0;
	config.contact_name_font_size = 16;
	config.contact_item_font_size = 10;
	g_strlcpy (config.import_bluetooth_address, "00:00:00:00:00:00", MAXADDRESS);
	g_strlcpy (config.contact_tag_color, "#228B22", MAXCOLORNAME);
	g_strlcpy (config.contact_link_color, "blue", MAXCOLORNAME);

    config.export_format = EXPORT_TO_CSV;

    for(i=0; i < CONTACTS_NUM_COLUMNS; i++) {
        config.export_fields[i] = '-';
    }
    config.export_fields[0] = '\0';
    config.export_fields[COLUMN_GROUP] = '+';
    config.export_fields[COLUMN_FIRST_NAME] = '+';
    config.export_fields[COLUMN_LAST_NAME] = '+';
    config.export_fields[COLUMN_NICK_NAME] = '+';

    /* notes */
    config.notes_enc_algorithm = 1;         /* Serpent */
    config.notes_enc_hashing = 1;           /* RIPEMD-160 */
    config.notes_comp_algorithm = 0;        /* ZLib */
    config.notes_comp_ratio = 3;            /* BEST */
    config.notes_sorting_order = GTK_SORT_ASCENDING;
    config.notes_sorting_mode = 0;
    config.nte_visible_type_column = TRUE;
    config.nte_visible_category_column = TRUE;
    config.nte_visible_last_changes_column = TRUE;
    config.nte_visible_created_column = TRUE;
	config.notes_column_idx_0 = 0;
	config.notes_column_idx_0_width = 50;
	config.notes_column_idx_1 = 1;
	config.notes_column_idx_1_width = 200;
	config.notes_column_idx_2 = 2;
	config.notes_column_idx_2_width = 80;
	config.notes_column_idx_3 = 3;
	config.notes_column_idx_3_width = 150;
	config.notes_column_idx_4 = 4;
	config.notes_column_idx_4_width = 150;
    config.remember_category_in_notes = FALSE;
    config.current_category_in_notes = 0;
    config.use_system_date_in_notes = FALSE;
    config.text_separator = '=';
	g_strlcpy (config.notes_editor_font, "Sans 10", MAXFONTNAME);
}

/*------------------------------------------------------------------------------*/

void 
prefs_read_config (GUI *appGUI)
{
gboolean cfg_file;
xmlDocPtr doc;
xmlNodePtr node, general_node, calendar_node, tasks_node, contacts_node, notes_node;

	cfg_file = g_file_test (prefs_get_config_filename (CONFIG_FILENAME, appGUI), G_FILE_TEST_IS_REGULAR);

	if (cfg_file == TRUE) {
		doc = xmlParseFile (prefs_get_config_filename (CONFIG_FILENAME, appGUI));
		if (doc == NULL) return;

		node = xmlDocGetRootElement (doc);
		if (node == NULL) {
			xmlFreeDoc (doc);
			return;
		}

		if (xmlStrcmp (node->name, (const xmlChar *) CONFIG_NAME)) {
			xmlFreeDoc (doc);
			return;
		}

		prefs_set_default_values ();
		node = node->xmlChildrenNode;

		while (node != NULL) {

			/*---------------------------------------------------------------------------------------*/
			/* general */

			if ((!xmlStrcmp (node->name, (const xmlChar *) "general"))) {
				general_node = node->xmlChildrenNode;

				while (general_node != NULL) {
					utl_xml_get_int ("window_x", &config.window_x, general_node);
					utl_xml_get_int ("window_y", &config.window_y, general_node);
					utl_xml_get_int ("window_size_x", &config.window_size_x, general_node);
					utl_xml_get_int ("window_size_y", &config.window_size_y, general_node);
					utl_xml_get_int ("enable_tooltips", &config.enable_tooltips, general_node);
					utl_xml_get_int ("latest_tab", &config.latest_tab, general_node);
					utl_xml_get_int ("tabs_position", &config.tabs_position, general_node);
					utl_xml_get_int ("remember_latest_tab", &config.remember_latest_tab, general_node);
					utl_xml_get_int ("save_data_after_modification", &config.save_data_after_modification, general_node);
					utl_xml_get_int ("default_stock_icons", &config.default_stock_icons, general_node);
					utl_xml_get_int ("disable_underline_links", &config.disable_underline_links, general_node);
					utl_xml_get_int ("rules_hint", &config.rules_hint, general_node);
					utl_xml_get_int ("date_format", &config.date_format, general_node);
					utl_xml_get_int ("time_format", &config.time_format, general_node);
					utl_xml_get_int ("enable_systray", &config.enable_systray, general_node);
					utl_xml_get_int ("start_minimised_in_systray", &config.start_minimised_in_systray, general_node);
					utl_xml_get_int ("blink_on_events", &config.blink_on_events, general_node);
					utl_xml_get_int ("ignore_day_note_events", &config.ignore_day_note_events, general_node);
					utl_xml_get_int ("run_counter", &config.run_counter, general_node);
					utl_xml_get_int ("lastrun_date", &config.lastrun_date, general_node);
					utl_xml_get_int ("lastrun_time", &config.lastrun_time, general_node);
					utl_xml_get_int ("hide_calendar", &config.hide_calendar, general_node);
					utl_xml_get_int ("hide_tasks", &config.hide_tasks, general_node);
					utl_xml_get_int ("hide_contacts", &config.hide_contacts, general_node);
					utl_xml_get_int ("hide_notes", &config.hide_notes, general_node);
					utl_xml_get_int ("override_locale_settings", &config.override_locale_settings, general_node);
					utl_xml_get_int ("gui_layout", &config.gui_layout, general_node);
					utl_xml_get_int ("sound_alarm_repeat", &config.sound_alarm_repeat, general_node);
					utl_xml_get_strn ("spell_lang", config.spell_lang, MAXNAME, general_node);
					utl_xml_get_strn ("web_browser", config.web_browser, MAXHELPERCMD, general_node);
					utl_xml_get_strn ("email_client", config.email_client, MAXHELPERCMD, general_node);
					utl_xml_get_strn ("sound_player", config.sound_player, MAXHELPERCMD, general_node);

					general_node = general_node->next;
				}

			}

			/*---------------------------------------------------------------------------------------*/
			/* calendar */

			if ((!xmlStrcmp (node->name, (const xmlChar *) "calendar"))) {
				calendar_node = node->xmlChildrenNode;

				while (calendar_node != NULL) {
					utl_xml_get_int ("fy_window_size_x", &config.fy_window_size_x, calendar_node);
					utl_xml_get_int ("fy_window_size_y", &config.fy_window_size_y, calendar_node);
					utl_xml_get_int ("fy_simple_view", &config.fy_simple_view, calendar_node);
					utl_xml_get_int ("fy_alternative_view", &config.fy_alternative_view, calendar_node);
					utl_xml_get_int ("cb_window_size_x", &config.cb_window_size_x, calendar_node);
					utl_xml_get_int ("cb_window_size_y", &config.cb_window_size_y, calendar_node);
					utl_xml_get_int ("ib_window_size_x", &config.ib_window_size_x, calendar_node);
					utl_xml_get_int ("ib_window_size_y", &config.ib_window_size_y, calendar_node);
					utl_xml_get_int ("display_options", &config.display_options, calendar_node);
					utl_xml_get_int ("day_notes_visible", &config.day_notes_visible, calendar_node);
					utl_xml_get_int ("timeline_start", &config.timeline_start, calendar_node);
					utl_xml_get_int ("timeline_end", &config.timeline_end, calendar_node);
					utl_xml_get_int ("timeline_step", &config.timeline_step, calendar_node);
					utl_xml_get_int ("di_show_current_time", &config.di_show_current_time, calendar_node);
					utl_xml_get_int ("di_show_day_number", &config.di_show_day_number, calendar_node);
					utl_xml_get_int ("di_show_current_day_distance", &config.di_show_current_day_distance, calendar_node);
					utl_xml_get_int ("di_show_marked_days", &config.di_show_marked_days, calendar_node);
					utl_xml_get_int ("di_show_week_number", &config.di_show_week_number, calendar_node);
					utl_xml_get_int ("di_show_weekend_days", &config.di_show_weekend_days, calendar_node);
					utl_xml_get_int ("di_show_day_category", &config.di_show_day_category, calendar_node);
					utl_xml_get_int ("di_show_moon_phase", &config.di_show_moon_phase, calendar_node);
					utl_xml_get_int ("di_show_notes", &config.di_show_notes, calendar_node);
					utl_xml_get_int ("di_show_zodiac_sign", &config.di_show_zodiac_sign, calendar_node);
					utl_xml_get_int ("cursor_type", &config.cursor_type, calendar_node);
					utl_xml_get_int ("frame_cursor_thickness", &config.frame_cursor_thickness, calendar_node);
					utl_xml_get_int ("enable_auxilary_calendars", &config.enable_auxilary_calendars, calendar_node);
					utl_xml_get_int ("enable_day_mark", &config.enable_day_mark, calendar_node);
					utl_xml_get_int ("strikethrough_past_notes", &config.strikethrough_past_notes, calendar_node);
					utl_xml_get_int ("ascending_sorting_in_day_notes_browser", &config.ascending_sorting_in_day_notes_browser, calendar_node);
					utl_xml_get_int ("auxilary_calendars_state", &config.auxilary_calendars_state, calendar_node);
					utl_xml_get_int ("day_note_spell_checker", &config.day_note_spell_checker, calendar_node);
					utl_xml_get_strn ("day_note_marker", config.day_note_marker, MAXNAME, calendar_node);
					utl_xml_get_strn ("date_header_format", config.date_header_format, MAXNAME, calendar_node);
					utl_xml_get_int ("event_marker_type", &config.event_marker_type, calendar_node);
					utl_xml_get_int ("today_marker_type", &config.today_marker_type, calendar_node);
					utl_xml_get_int ("day_notes_browser_filter", &config.day_notes_browser_filter, calendar_node);
					utl_xml_get_int ("ical_export_pane_pos", &config.ical_export_pane_pos, calendar_node);
					utl_xml_get_strn ("header_color", config.header_color, MAXCOLORNAME, calendar_node);
					utl_xml_get_strn ("weekend_color", config.weekend_color, MAXCOLORNAME, calendar_node);
					utl_xml_get_strn ("selection_color", config.selection_color, MAXCOLORNAME, calendar_node);
					utl_xml_get_strn ("mark_color", config.mark_color, MAXCOLORNAME, calendar_node);
					utl_xml_get_strn ("mark_current_day_color", config.mark_current_day_color, MAXCOLORNAME, calendar_node);
					utl_xml_get_int ("mark_current_day_alpha", &config.mark_current_day_alpha, calendar_node);
					utl_xml_get_strn ("birthday_mark_color", config.birthday_mark_color, MAXCOLORNAME, calendar_node);
					utl_xml_get_int ("selector_alpha", &config.selector_alpha, calendar_node);
					utl_xml_get_strn ("day_name_font", config.day_name_font, MAXFONTNAME, calendar_node);
					utl_xml_get_strn ("calendar_font", config.calendar_font, MAXFONTNAME, calendar_node);
					utl_xml_get_strn ("notes_font", config.notes_font, MAXFONTNAME, calendar_node);
					utl_xml_get_strn ("cal_print_month_name_font", config.cal_print_month_name_font, MAXFONTNAME, calendar_node);
					utl_xml_get_strn ("cal_print_day_name_font", config.cal_print_day_name_font, MAXFONTNAME, calendar_node);
					utl_xml_get_strn ("cal_print_day_num_font", config.cal_print_day_num_font, MAXFONTNAME, calendar_node);
					utl_xml_get_strn ("cal_print_event_font", config.cal_print_event_font, MAXFONTNAME, calendar_node);
					utl_xml_get_int ("cal_print_event_length", &config.cal_print_event_length, calendar_node);
					utl_xml_get_int ("cal_print_padding", &config.cal_print_padding, calendar_node);
					utl_xml_get_int ("cal_print_page_orientation", &config.cal_print_page_orientation, calendar_node);
					utl_xml_get_int ("cal_print_tasks", &config.cal_print_tasks, calendar_node);
					utl_xml_get_int ("cal_print_birthdays", &config.cal_print_birthdays, calendar_node);
					utl_xml_get_int ("cal_print_namedays", &config.cal_print_namedays, calendar_node);
					utl_xml_get_int ("cal_print_day_notes", &config.cal_print_day_notes, calendar_node);
					utl_xml_get_int ("cal_print_ical", &config.cal_print_ical, calendar_node);

					calendar_node = calendar_node->next;
				}
			}

			/*---------------------------------------------------------------------------------------*/
			/* tasks */

			if ((!xmlStrcmp (node->name, (const xmlChar *) "tasks"))) {
				tasks_node = node->xmlChildrenNode;

				while (tasks_node != NULL) {
					utl_xml_get_int ("high_priority_in_bold", &config.tasks_high_in_bold, tasks_node);
					utl_xml_get_int ("hide_completed", &config.hide_completed, tasks_node);
					utl_xml_get_int ("delete_completed", &config.delete_completed, tasks_node);
					utl_xml_get_int ("add_edit", &config.add_edit, tasks_node);
					utl_xml_get_int ("remember_category", &config.remember_category_in_tasks, tasks_node);
					utl_xml_get_int ("current_category", &config.current_category_in_tasks, tasks_node);
					utl_xml_get_int ("pane_pos", &config.tasks_pane_pos, tasks_node);
					utl_xml_get_int ("tasks_sorting_order", &config.tasks_sorting_order, tasks_node);
					utl_xml_get_int ("tasks_sorting_mode", &config.tasks_sorting_mode, tasks_node);
					utl_xml_get_int ("visible_due_date_column", &config.tsk_visible_due_date_column, tasks_node);
					utl_xml_get_int ("visible_type_column", &config.tsk_visible_type_column, tasks_node);
					utl_xml_get_int ("visible_priority_column", &config.tsk_visible_priority_column, tasks_node);
					utl_xml_get_int ("visible_category_column", &config.tsk_visible_category_column, tasks_node);
					utl_xml_get_int ("tasks_addedit_win_x", &config.tasks_addedit_win_x, tasks_node);
					utl_xml_get_int ("tasks_addedit_win_y", &config.tasks_addedit_win_y, tasks_node);
					utl_xml_get_int ("tasks_addedit_win_w", &config.tasks_addedit_win_w, tasks_node);
					utl_xml_get_int ("tasks_addedit_win_h", &config.tasks_addedit_win_h, tasks_node);
					utl_xml_get_int ("postpone_time", &config.postpone_time, tasks_node);
					utl_xml_get_int ("column_idx_0", &config.tasks_column_idx_0, tasks_node);
					utl_xml_get_int ("column_idx_0_width", &config.tasks_column_idx_0_width, tasks_node);
					utl_xml_get_int ("column_idx_1", &config.tasks_column_idx_1, tasks_node);
					utl_xml_get_int ("column_idx_1_width", &config.tasks_column_idx_1_width, tasks_node);
					utl_xml_get_int ("column_idx_2", &config.tasks_column_idx_2, tasks_node);
					utl_xml_get_int ("column_idx_2_width", &config.tasks_column_idx_2_width, tasks_node);
					utl_xml_get_int ("column_idx_3", &config.tasks_column_idx_3, tasks_node);
					utl_xml_get_int ("column_idx_3_width", &config.tasks_column_idx_3_width, tasks_node);
					utl_xml_get_int ("column_idx_4", &config.tasks_column_idx_4, tasks_node);
					utl_xml_get_int ("column_idx_4_width", &config.tasks_column_idx_4_width, tasks_node);
					utl_xml_get_int ("column_idx_5", &config.tasks_column_idx_5, tasks_node);
					utl_xml_get_int ("column_idx_5_width", &config.tasks_column_idx_5_width, tasks_node);
					utl_xml_get_strn ("due_today_color", config.due_today_color, MAXCOLORNAME, tasks_node);
					utl_xml_get_strn ("due_7days_color", config.due_7days_color, MAXCOLORNAME, tasks_node);
					utl_xml_get_strn ("past_due_color", config.past_due_color, MAXCOLORNAME, tasks_node);
					utl_xml_get_strn ("task_info_font", config.task_info_font, MAXFONTNAME, tasks_node);
					utl_xml_get_strn ("global_notification_command", config.global_notification_command, MAXHELPERCMD, tasks_node);

					tasks_node = tasks_node->next;
				}
			}

			/*---------------------------------------------------------------------------------------*/
			/* contacts */

			if ((!xmlStrcmp (node->name, (const xmlChar *) "contacts"))) {
				contacts_node = node->xmlChildrenNode;

				while (contacts_node != NULL) {
					utl_xml_get_int ("find_mode", &config.find_mode, contacts_node);
					utl_xml_get_int ("show_after_search", &config.show_after_search, contacts_node);
					utl_xml_get_int ("hide_group_column", &config.hide_group_column, contacts_node);
					utl_xml_get_int ("pane_pos", &config.contacts_pane_pos, contacts_node);
					utl_xml_get_int ("photo_width", &config.photo_width, contacts_node);
					utl_xml_get_int ("visible_age_column", &config.cnt_visible_age_column, contacts_node);
					utl_xml_get_int ("visible_birthday_date_column", &config.cnt_visible_birthday_date_column, contacts_node);
					utl_xml_get_int ("visible_zodiac_sign_column", &config.cnt_visible_zodiac_sign_column, contacts_node);
					utl_xml_get_strn ("contact_tag_color", config.contact_tag_color, MAXCOLORNAME, contacts_node);
					utl_xml_get_strn ("contact_link_color", config.contact_link_color, MAXCOLORNAME, contacts_node);
					utl_xml_get_int ("contact_name_font_size", &config.contact_name_font_size, contacts_node);
					utl_xml_get_int ("contact_item_font_size", &config.contact_item_font_size, contacts_node);
					utl_xml_get_int ("export_format", &config.export_format, contacts_node);
					utl_xml_get_strn ("export_fields", config.export_fields, MAXCONTACTFIELDS, contacts_node);
					utl_xml_get_int ("contacts_sorting_order", &config.contacts_sorting_order, contacts_node);
					utl_xml_get_int ("contacts_sorting_mode", &config.contacts_sorting_mode, contacts_node);
					utl_xml_get_int ("contacts_addedit_win_x", &config.contacts_addedit_win_x, contacts_node);
					utl_xml_get_int ("contacts_addedit_win_y", &config.contacts_addedit_win_y, contacts_node);
					utl_xml_get_int ("contacts_addedit_win_w", &config.contacts_addedit_win_w, contacts_node);
					utl_xml_get_int ("contacts_addedit_win_h", &config.contacts_addedit_win_h, contacts_node);
					utl_xml_get_int ("contacts_export_win_x", &config.contacts_export_win_x, contacts_node);
					utl_xml_get_int ("contacts_export_win_y", &config.contacts_export_win_y, contacts_node);
					utl_xml_get_int ("contacts_export_win_w", &config.contacts_export_win_w, contacts_node);
					utl_xml_get_int ("contacts_export_win_h", &config.contacts_export_win_h, contacts_node);
					utl_xml_get_int ("contacts_import_sel_win_x", &config.contacts_import_sel_win_x, contacts_node);
					utl_xml_get_int ("contacts_import_sel_win_y", &config.contacts_import_sel_win_y, contacts_node);
					utl_xml_get_int ("contacts_import_win_x", &config.contacts_import_win_x, contacts_node);
					utl_xml_get_int ("contacts_import_win_y", &config.contacts_import_win_y, contacts_node);
					utl_xml_get_int ("contacts_import_win_w", &config.contacts_import_win_w, contacts_node);
					utl_xml_get_int ("contacts_import_win_h", &config.contacts_import_win_h, contacts_node);
					utl_xml_get_int ("contacts_birthdays_win_w", &config.contacts_birthdays_win_w, contacts_node);
					utl_xml_get_int ("contacts_birthdays_win_h", &config.contacts_birthdays_win_h, contacts_node);
					utl_xml_get_int ("column_idx_0", &config.contacts_column_idx_0, contacts_node);
					utl_xml_get_int ("column_idx_0_width", &config.contacts_column_idx_0_width, contacts_node);
					utl_xml_get_int ("column_idx_1", &config.contacts_column_idx_1, contacts_node);
					utl_xml_get_int ("column_idx_1_width", &config.contacts_column_idx_1_width, contacts_node);
					utl_xml_get_int ("column_idx_2", &config.contacts_column_idx_2, contacts_node);
					utl_xml_get_int ("column_idx_2_width", &config.contacts_column_idx_2_width, contacts_node);
					utl_xml_get_int ("import_type", &config.import_type, contacts_node);
					utl_xml_get_int ("import_interface_type", &config.import_interface_type, contacts_node);
					utl_xml_get_int ("import_bluetooth_channel", &config.import_bluetooth_channel, contacts_node);
					utl_xml_get_int ("import_usb_interface", &config.import_usb_interface, contacts_node);
					utl_xml_get_int ("import_binary_xml", &config.import_binary_xml, contacts_node);
					utl_xml_get_strn ("import_bluetooth_address", config.import_bluetooth_address, MAXADDRESS, contacts_node);

					contacts_node = contacts_node->next;
				}
			}

			/*---------------------------------------------------------------------------------------*/
			/* notes */

			if ((!xmlStrcmp (node->name, (const xmlChar *) "notes"))) {
				notes_node = node->xmlChildrenNode;

				while (notes_node != NULL) {
					utl_xml_get_int ("enc_algorithm", &config.notes_enc_algorithm, notes_node);
					utl_xml_get_int ("enc_hashing", &config.notes_enc_hashing, notes_node);
					utl_xml_get_int ("comp_algorithm", &config.notes_comp_algorithm, notes_node);
					utl_xml_get_int ("comp_ratio", &config.notes_comp_ratio, notes_node);
					utl_xml_get_int ("sorting_order", &config.notes_sorting_order, notes_node);
					utl_xml_get_int ("sorting_mode", &config.notes_sorting_mode, notes_node);
					utl_xml_get_int ("visible_type_column", &config.nte_visible_type_column, notes_node);
					utl_xml_get_int ("visible_category_column", &config.nte_visible_category_column, notes_node);
					utl_xml_get_int ("visible_last_changes_column", &config.nte_visible_last_changes_column, notes_node);
					utl_xml_get_int ("visible_created_column", &config.nte_visible_created_column, notes_node);
					utl_xml_get_int ("column_idx_0", &config.notes_column_idx_0, notes_node);
					utl_xml_get_int ("column_idx_0_width", &config.notes_column_idx_0_width, notes_node);
					utl_xml_get_int ("column_idx_1", &config.notes_column_idx_1, notes_node);
					utl_xml_get_int ("column_idx_1_width", &config.notes_column_idx_1_width, notes_node);
					utl_xml_get_int ("column_idx_2", &config.notes_column_idx_2, notes_node);
					utl_xml_get_int ("column_idx_2_width", &config.notes_column_idx_2_width, notes_node);
					utl_xml_get_int ("column_idx_3", &config.notes_column_idx_3, notes_node);
					utl_xml_get_int ("column_idx_3_width", &config.notes_column_idx_3_width, notes_node);
					utl_xml_get_int ("column_idx_4", &config.notes_column_idx_4, notes_node);
					utl_xml_get_int ("column_idx_4_width", &config.notes_column_idx_4_width, notes_node);
					utl_xml_get_int ("remember_category", &config.remember_category_in_notes, notes_node);
					utl_xml_get_int ("current_category", &config.current_category_in_notes, notes_node);
					utl_xml_get_int ("use_system_date", &config.use_system_date_in_notes, notes_node);
					utl_xml_get_char ("text_separator", &config.text_separator, notes_node);
					utl_xml_get_strn ("editor_font", config.notes_editor_font, MAXFONTNAME, notes_node);

					notes_node = notes_node->next;
				}
				
				if (config.notes_comp_ratio == 0 /* GRG_LVL_NONE */) {      /* ignore 'None' option */
					config.notes_comp_ratio = 1 /* GRG_LVL_FAST */;
				}
			}

			/*---------------------------------------------------------------------------------------*/

			node = node->next;
		}

		xmlFreeDoc (doc);

	} else {
		prefs_set_default_values ();
		prefs_write_config (appGUI);
	}

}

/*------------------------------------------------------------------------------*/

void
prefs_write_config (GUI *appGUI)
{
xmlDocPtr doc;
xmlNodePtr node, general_node, calendar_node, tasks_node, contacts_node, notes_node;

	doc = xmlNewDoc ((const xmlChar *) "1.0");
	node = xmlNewNode (NULL, (const xmlChar *) CONFIG_NAME);
	xmlDocSetRootElement (doc, node);

	/*---------------------------------------------------------------------------------------*/
	/* general */

	general_node = xmlNewNode (NULL, (const xmlChar *) "general");
	xmlAddChild (node, general_node);

	utl_xml_put_int ("window_x", config.window_x, general_node);
	utl_xml_put_int ("window_y", config.window_y, general_node);
	utl_xml_put_int ("window_size_x", config.window_size_x, general_node);
	utl_xml_put_int ("window_size_y", config.window_size_y, general_node);
	utl_xml_put_int ("enable_tooltips", config.enable_tooltips, general_node);
	utl_xml_put_int ("latest_tab", config.latest_tab, general_node);
	utl_xml_put_int ("tabs_position", config.tabs_position, general_node);
	utl_xml_put_int ("remember_latest_tab", config.remember_latest_tab, general_node);
	utl_xml_put_int ("save_data_after_modification", config.save_data_after_modification, general_node);
	utl_xml_put_int ("default_stock_icons", config.default_stock_icons, general_node);
	utl_xml_put_int ("disable_underline_links", config.disable_underline_links, general_node);
	utl_xml_put_int ("rules_hint", config.rules_hint, general_node);
	utl_xml_put_int ("date_format", config.date_format, general_node);
	utl_xml_put_int ("time_format", config.time_format, general_node);
	utl_xml_put_int ("enable_systray", config.enable_systray, general_node);
	utl_xml_put_int ("start_minimised_in_systray", config.start_minimised_in_systray, general_node);
	utl_xml_put_int ("blink_on_events", config.blink_on_events, general_node);
	utl_xml_put_int ("ignore_day_note_events", config.ignore_day_note_events, general_node);
	utl_xml_put_int ("run_counter", config.run_counter, general_node);
	utl_xml_put_int ("lastrun_date", config.lastrun_date, general_node);
	utl_xml_put_int ("lastrun_time", config.lastrun_time, general_node);
	utl_xml_put_int ("hide_calendar", config.hide_calendar, general_node);
	utl_xml_put_int ("hide_tasks", config.hide_tasks, general_node);
	utl_xml_put_int ("hide_contacts", config.hide_contacts, general_node);
	utl_xml_put_int ("hide_notes", config.hide_notes, general_node);
	utl_xml_put_int ("override_locale_settings", config.override_locale_settings, general_node);
	utl_xml_put_int ("gui_layout", config.gui_layout, general_node);
	utl_xml_put_int ("sound_alarm_repeat", config.sound_alarm_repeat, general_node);
	utl_xml_put_strn ("spell_lang", config.spell_lang, MAXNAME, general_node, doc);
	utl_xml_put_strn ("web_browser", config.web_browser, MAXHELPERCMD, general_node, doc);
	utl_xml_put_strn ("email_client", config.email_client, MAXHELPERCMD, general_node, doc);
	utl_xml_put_strn ("sound_player", config.sound_player, MAXHELPERCMD, general_node, doc);

	/*---------------------------------------------------------------------------------------*/
	/* calendar */

	calendar_node = xmlNewNode (NULL, (const xmlChar *) "calendar");
	xmlAddChild (node, calendar_node);

	utl_xml_put_int ("fy_window_size_x", config.fy_window_size_x, calendar_node);
	utl_xml_put_int ("fy_window_size_y", config.fy_window_size_y, calendar_node);
	utl_xml_put_int ("fy_simple_view", config.fy_simple_view, calendar_node);
	utl_xml_put_int ("fy_alternative_view", config.fy_alternative_view, calendar_node);
	utl_xml_put_int ("cb_window_size_x", config.cb_window_size_x, calendar_node);
	utl_xml_put_int ("cb_window_size_y", config.cb_window_size_y, calendar_node);
	utl_xml_put_int ("ib_window_size_x", config.ib_window_size_x, calendar_node);
	utl_xml_put_int ("ib_window_size_y", config.ib_window_size_y, calendar_node);
	utl_xml_put_int ("display_options", config.display_options, calendar_node);
	utl_xml_put_int ("day_notes_visible", config.day_notes_visible, calendar_node);
	utl_xml_put_int ("timeline_start", config.timeline_start, calendar_node);
	utl_xml_put_int ("timeline_end", config.timeline_end, calendar_node);
	utl_xml_put_int ("timeline_step", config.timeline_step, calendar_node);
	utl_xml_put_int ("di_show_current_time", config.di_show_current_time, calendar_node);
	utl_xml_put_int ("di_show_day_number", config.di_show_day_number, calendar_node);
	utl_xml_put_int ("di_show_current_day_distance", config.di_show_current_day_distance, calendar_node);
	utl_xml_put_int ("di_show_marked_days", config.di_show_marked_days, calendar_node);
	utl_xml_put_int ("di_show_week_number", config.di_show_week_number, calendar_node);
	utl_xml_put_int ("di_show_weekend_days", config.di_show_weekend_days, calendar_node);
	utl_xml_put_int ("di_show_day_category", config.di_show_day_category, calendar_node);
	utl_xml_put_int ("di_show_moon_phase", config.di_show_moon_phase, calendar_node);
	utl_xml_put_int ("di_show_notes", config.di_show_notes, calendar_node);
	utl_xml_put_int ("di_show_zodiac_sign", config.di_show_zodiac_sign, calendar_node);
	utl_xml_put_int ("cursor_type", config.cursor_type, calendar_node);
	utl_xml_put_int ("frame_cursor_thickness", config.frame_cursor_thickness, calendar_node);
	utl_xml_put_int ("enable_auxilary_calendars", config.enable_auxilary_calendars, calendar_node);
	utl_xml_put_int ("enable_day_mark", config.enable_day_mark, calendar_node);
	utl_xml_put_int ("strikethrough_past_notes", config.strikethrough_past_notes, calendar_node);
	utl_xml_put_int ("ascending_sorting_in_day_notes_browser", config.ascending_sorting_in_day_notes_browser, calendar_node);
	utl_xml_put_int ("auxilary_calendars_state", config.auxilary_calendars_state, calendar_node);
	utl_xml_put_int ("day_note_spell_checker", config.day_note_spell_checker, calendar_node);
	utl_xml_put_strn ("day_note_marker", config.day_note_marker, MAXNAME, calendar_node, doc);
	utl_xml_put_strn ("date_header_format", config.date_header_format, MAXNAME, calendar_node, doc);
	utl_xml_put_int ("event_marker_type", config.event_marker_type, calendar_node);
	utl_xml_put_int ("today_marker_type", config.today_marker_type, calendar_node);
	utl_xml_put_int ("day_notes_browser_filter", config.day_notes_browser_filter, calendar_node);
	utl_xml_put_int ("ical_export_pane_pos", config.ical_export_pane_pos, calendar_node);
	utl_xml_put_strn ("header_color", config.header_color, MAXCOLORNAME, calendar_node, doc);
	utl_xml_put_strn ("weekend_color", config.weekend_color, MAXCOLORNAME, calendar_node, doc);
	utl_xml_put_strn ("selection_color", config.selection_color, MAXCOLORNAME, calendar_node, doc);
	utl_xml_put_strn ("mark_color", config.mark_color, MAXCOLORNAME, calendar_node, doc);
	utl_xml_put_strn ("mark_current_day_color", config.mark_current_day_color, MAXCOLORNAME, calendar_node, doc);
	utl_xml_put_int ("mark_current_day_alpha", config.mark_current_day_alpha, calendar_node);
	utl_xml_put_strn ("birthday_mark_color", config.birthday_mark_color, MAXCOLORNAME, calendar_node, doc);
	utl_xml_put_int ("selector_alpha", config.selector_alpha, calendar_node);
	utl_xml_put_strn ("day_name_font", config.day_name_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_strn ("calendar_font", config.calendar_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_strn ("notes_font", config.notes_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_strn ("cal_print_month_name_font", config.cal_print_month_name_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_strn ("cal_print_day_name_font", config.cal_print_day_name_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_strn ("cal_print_day_num_font", config.cal_print_day_num_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_strn ("cal_print_event_font", config.cal_print_event_font, MAXFONTNAME, calendar_node, doc);
	utl_xml_put_int ("cal_print_event_length", config.cal_print_event_length, calendar_node);
	utl_xml_put_int ("cal_print_padding", config.cal_print_padding, calendar_node);
	utl_xml_put_int ("cal_print_page_orientation", config.cal_print_page_orientation, calendar_node);
	utl_xml_put_int ("cal_print_tasks", config.cal_print_tasks, calendar_node);
	utl_xml_put_int ("cal_print_birthdays", config.cal_print_birthdays, calendar_node);
	utl_xml_put_int ("cal_print_namedays", config.cal_print_namedays, calendar_node);
	utl_xml_put_int ("cal_print_day_notes", config.cal_print_day_notes, calendar_node);
	utl_xml_put_int ("cal_print_ical", config.cal_print_ical, calendar_node);

	/*---------------------------------------------------------------------------------------*/
	/* tasks */

	tasks_node = xmlNewNode (NULL, (const xmlChar *) "tasks");
	xmlAddChild (node, tasks_node);

	utl_xml_put_int ("high_priority_in_bold", config.tasks_high_in_bold, tasks_node);
	utl_xml_put_int ("hide_completed", config.hide_completed, tasks_node);
	utl_xml_put_int ("delete_completed", config.delete_completed, tasks_node);
	utl_xml_put_int ("add_edit", config.add_edit, tasks_node);
	utl_xml_put_int ("remember_category", config.remember_category_in_tasks, tasks_node);
	utl_xml_put_int ("current_category", config.current_category_in_tasks, tasks_node);
	utl_xml_put_int ("pane_pos", config.tasks_pane_pos, tasks_node);
	utl_xml_put_int ("tasks_sorting_order", config.tasks_sorting_order, tasks_node);
	utl_xml_put_int ("tasks_sorting_mode", config.tasks_sorting_mode, tasks_node);
	utl_xml_put_int ("visible_due_date_column", config.tsk_visible_due_date_column, tasks_node);
	utl_xml_put_int ("visible_type_column", config.tsk_visible_type_column, tasks_node);
	utl_xml_put_int ("visible_priority_column", config.tsk_visible_priority_column, tasks_node);
	utl_xml_put_int ("visible_category_column", config.tsk_visible_category_column, tasks_node);
	utl_xml_put_int ("tasks_addedit_win_x", config.tasks_addedit_win_x, tasks_node);
	utl_xml_put_int ("tasks_addedit_win_y", config.tasks_addedit_win_y, tasks_node);
	utl_xml_put_int ("tasks_addedit_win_w", config.tasks_addedit_win_w, tasks_node);
	utl_xml_put_int ("tasks_addedit_win_h", config.tasks_addedit_win_h, tasks_node);
	utl_xml_put_int ("postpone_time", config.postpone_time, tasks_node);
	utl_xml_put_int ("column_idx_0", config.tasks_column_idx_0, tasks_node);
	utl_xml_put_int ("column_idx_0_width", config.tasks_column_idx_0_width, tasks_node);
	utl_xml_put_int ("column_idx_1", config.tasks_column_idx_1, tasks_node);
	utl_xml_put_int ("column_idx_1_width", config.tasks_column_idx_1_width, tasks_node);
	utl_xml_put_int ("column_idx_2", config.tasks_column_idx_2, tasks_node);
	utl_xml_put_int ("column_idx_2_width", config.tasks_column_idx_2_width, tasks_node);
	utl_xml_put_int ("column_idx_3", config.tasks_column_idx_3, tasks_node);
	utl_xml_put_int ("column_idx_3_width", config.tasks_column_idx_3_width, tasks_node);
	utl_xml_put_int ("column_idx_4", config.tasks_column_idx_4, tasks_node);
	utl_xml_put_int ("column_idx_4_width", config.tasks_column_idx_4_width, tasks_node);
	utl_xml_put_int ("column_idx_5", config.tasks_column_idx_5, tasks_node);
	utl_xml_put_int ("column_idx_5_width", config.tasks_column_idx_5_width, tasks_node);
	utl_xml_put_strn ("due_today_color", config.due_today_color, MAXCOLORNAME, tasks_node, doc);
	utl_xml_put_strn ("due_7days_color", config.due_7days_color, MAXCOLORNAME, tasks_node, doc);
	utl_xml_put_strn ("past_due_color", config.past_due_color, MAXCOLORNAME, tasks_node, doc);
	utl_xml_put_strn ("task_info_font", config.task_info_font, MAXFONTNAME, tasks_node, doc);
	utl_xml_put_strn ("global_notification_command", config.global_notification_command, MAXHELPERCMD, tasks_node, doc);

	/*---------------------------------------------------------------------------------------*/
	/* contacts */

	contacts_node = xmlNewNode (NULL, (const xmlChar *) "contacts");
	xmlAddChild (node, contacts_node);

	utl_xml_put_int ("find_mode", config.find_mode, contacts_node);
	utl_xml_put_int ("show_after_search", config.show_after_search, contacts_node);
	utl_xml_put_int ("hide_group_column", config.hide_group_column, contacts_node);
	utl_xml_put_int ("pane_pos", config.contacts_pane_pos, contacts_node);
	utl_xml_put_int ("photo_width", config.photo_width, contacts_node);
	utl_xml_put_int ("visible_age_column", config.cnt_visible_age_column, contacts_node);
	utl_xml_put_int ("visible_birthday_date_column", config.cnt_visible_birthday_date_column, contacts_node);
	utl_xml_put_int ("visible_zodiac_sign_column", config.cnt_visible_zodiac_sign_column, contacts_node);
	utl_xml_put_strn ("contact_tag_color", config.contact_tag_color, MAXCOLORNAME, contacts_node, doc);
	utl_xml_put_strn ("contact_link_color", config.contact_link_color, MAXCOLORNAME, contacts_node, doc);
	utl_xml_put_int ("contact_name_font_size", config.contact_name_font_size, contacts_node);
	utl_xml_put_int ("contact_item_font_size", config.contact_item_font_size, contacts_node);
	utl_xml_put_int ("export_format", config.export_format, contacts_node);
	utl_xml_put_strn ("export_fields", config.export_fields, MAXCONTACTFIELDS, contacts_node, doc);
	utl_xml_put_int ("contacts_sorting_order", config.contacts_sorting_order, contacts_node);
	utl_xml_put_int ("contacts_sorting_mode", config.contacts_sorting_mode, contacts_node);
	utl_xml_put_int ("contacts_addedit_win_x", config.contacts_addedit_win_x, contacts_node);
	utl_xml_put_int ("contacts_addedit_win_y", config.contacts_addedit_win_y, contacts_node);
	utl_xml_put_int ("contacts_addedit_win_w", config.contacts_addedit_win_w, contacts_node);
	utl_xml_put_int ("contacts_addedit_win_h", config.contacts_addedit_win_h, contacts_node);
	utl_xml_put_int ("contacts_export_win_x", config.contacts_export_win_x, contacts_node);
	utl_xml_put_int ("contacts_export_win_y", config.contacts_export_win_y, contacts_node);
	utl_xml_put_int ("contacts_export_win_w", config.contacts_export_win_w, contacts_node);
	utl_xml_put_int ("contacts_export_win_h", config.contacts_export_win_h, contacts_node);
	utl_xml_put_int ("contacts_import_sel_win_x", config.contacts_import_sel_win_x, contacts_node);
	utl_xml_put_int ("contacts_import_sel_win_y", config.contacts_import_sel_win_y, contacts_node);
	utl_xml_put_int ("contacts_import_win_x", config.contacts_import_win_x, contacts_node);
	utl_xml_put_int ("contacts_import_win_y", config.contacts_import_win_y, contacts_node);
	utl_xml_put_int ("contacts_import_win_w", config.contacts_import_win_w, contacts_node);
	utl_xml_put_int ("contacts_import_win_h", config.contacts_import_win_h, contacts_node);
	utl_xml_put_int ("contacts_birthdays_win_w", config.contacts_birthdays_win_w, contacts_node);
	utl_xml_put_int ("contacts_birthdays_win_h", config.contacts_birthdays_win_h, contacts_node);
	utl_xml_put_int ("column_idx_0", config.contacts_column_idx_0, contacts_node);
	utl_xml_put_int ("column_idx_0_width", config.contacts_column_idx_0_width, contacts_node);
	utl_xml_put_int ("column_idx_1", config.contacts_column_idx_1, contacts_node);
	utl_xml_put_int ("column_idx_1_width", config.contacts_column_idx_1_width, contacts_node);
	utl_xml_put_int ("column_idx_2", config.contacts_column_idx_2, contacts_node);
	utl_xml_put_int ("column_idx_2_width", config.contacts_column_idx_2_width, contacts_node);
	utl_xml_put_int ("import_type", config.import_type, contacts_node);
	utl_xml_put_int ("import_interface_type", config.import_interface_type, contacts_node);
	utl_xml_put_int ("import_bluetooth_channel", config.import_bluetooth_channel, contacts_node);
	utl_xml_put_int ("import_usb_interface", config.import_usb_interface, contacts_node);
	utl_xml_put_int ("import_binary_xml", config.import_binary_xml, contacts_node);
	utl_xml_put_strn ("import_bluetooth_address", config.import_bluetooth_address, MAXADDRESS, contacts_node, doc);

	/*---------------------------------------------------------------------------------------*/
	/* notes */

	notes_node = xmlNewNode (NULL, (const xmlChar *) "notes");
	xmlAddChild (node, notes_node);

	utl_xml_put_int ("enc_algorithm", config.notes_enc_algorithm, notes_node);
	utl_xml_put_int ("enc_hashing", config.notes_enc_hashing, notes_node);
	utl_xml_put_int ("comp_algorithm", config.notes_comp_algorithm, notes_node);
	utl_xml_put_int ("comp_ratio", config.notes_comp_ratio, notes_node);
	utl_xml_put_int ("sorting_order", config.notes_sorting_order, notes_node);
	utl_xml_put_int ("sorting_mode", config.notes_sorting_mode, notes_node);
	utl_xml_put_int ("visible_type_column", config.nte_visible_type_column, notes_node);
	utl_xml_put_int ("visible_category_column", config.nte_visible_category_column, notes_node);
	utl_xml_put_int ("visible_last_changes_column", config.nte_visible_last_changes_column, notes_node);
	utl_xml_put_int ("visible_created_column", config.nte_visible_created_column, notes_node);
	utl_xml_put_int ("column_idx_0", config.notes_column_idx_0, notes_node);
	utl_xml_put_int ("column_idx_0_width", config.notes_column_idx_0_width, notes_node);
	utl_xml_put_int ("column_idx_1", config.notes_column_idx_1, notes_node);
	utl_xml_put_int ("column_idx_1_width", config.notes_column_idx_1_width, notes_node);
	utl_xml_put_int ("column_idx_2", config.notes_column_idx_2, notes_node);
	utl_xml_put_int ("column_idx_2_width", config.notes_column_idx_2_width, notes_node);
	utl_xml_put_int ("column_idx_3", config.notes_column_idx_3, notes_node);
	utl_xml_put_int ("column_idx_3_width", config.notes_column_idx_3_width, notes_node);
	utl_xml_put_int ("column_idx_4", config.notes_column_idx_4, notes_node);
	utl_xml_put_int ("column_idx_4_width", config.notes_column_idx_4_width, notes_node);
	utl_xml_put_int ("remember_category", config.remember_category_in_notes, notes_node);
	utl_xml_put_int ("current_category", config.current_category_in_notes, notes_node);
	utl_xml_put_int ("use_system_date", config.use_system_date_in_notes, notes_node);
	utl_xml_put_char ("text_separator", config.text_separator, notes_node, doc);
	utl_xml_put_strn ("editor_font", config.notes_editor_font, MAXFONTNAME, notes_node, doc);

	/*---------------------------------------------------------------------------------------*/

	xmlSaveFormatFile (prefs_get_config_filename (CONFIG_FILENAME, appGUI), doc, 1);
	xmlFreeDoc (doc);
}

/*------------------------------------------------------------------------------*/


