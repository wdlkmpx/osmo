
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

#ifndef _OPTIONS_PREFS_H
#define _OPTIONS_PREFS_H

#include "gui.h"

#define CONFIG_DIRNAME      ".osmo"
#define CONFIG_FILENAME     "config.xml"
#define CONFIG_NAME         "osmo_config"

#define MAXNAME             1024
#define MAXFONTNAME         512
#define MAXCOLORNAME        128
#define MAXHELPERCMD        1024
#define MAXCONTACTFIELDS    512
#define MAXADDRESS          64

#define DEFAULT_DATE_HEADER_FORMAT  "%e %B %Y"      /* example: 1 January 2008 */

struct osmo_prefs
{
    /* OSMO */
    gint        window_x;
    gint        window_y;
    gint        window_size_x;
    gint        window_size_y;
    gint        enable_tooltips;
    gint        latest_tab;
    gint        tabs_position;
    gint        remember_latest_tab;
    gint        save_data_after_modification;
    gint        default_stock_icons;
    gint        disable_underline_links;
    gint        rules_hint;
    gint        date_format;
    gint        time_format;
    gint        enable_systray;
    gint        start_minimised_in_systray;
    gint        blink_on_events;
    gint        ignore_day_note_events;
    gint        run_counter;
    gint        lastrun_date;
    gint        lastrun_time;
    gint        hide_calendar;
    gint        hide_tasks;
    gint        hide_contacts;
    gint        hide_notes;
	gint        override_locale_settings;
	gint        gui_layout;
	gint        sound_alarm_repeat;
    gchar       link_color[MAXCOLORNAME];
    gchar       spell_lang[MAXNAME];
    gchar       web_browser[MAXHELPERCMD];
    gchar       email_client[MAXHELPERCMD];
    gchar       sound_player[MAXHELPERCMD];

    /* CALENDAR */
    gint        fy_window_size_x;
    gint        fy_window_size_y;
    gint        fy_simple_view;
    gint        fy_alternative_view;
    gint        cb_window_size_x;
    gint        cb_window_size_y;
    gint        ib_window_size_x;
    gint        ib_window_size_y;
    gint        display_options;
    gint        day_notes_visible;
    gint        timeline_start;
    gint        timeline_end;
    gint        timeline_step;
    gint        di_show_current_time;
    gint        di_show_day_number;
    gint        di_show_current_day_distance;
    gint        di_show_marked_days;
    gint        di_show_week_number;
    gint        di_show_weekend_days;
    gint        di_show_day_category;
    gint        di_show_moon_phase;
    gint        di_show_notes;
    gint        di_show_zodiac_sign;
    gint        cursor_type;
    gint        frame_cursor_thickness;
    gint        enable_auxilary_calendars;
    gint        enable_day_mark;
    gint        strikethrough_past_notes;
    gint        ascending_sorting_in_day_notes_browser;
    gint        auxilary_calendars_state;
    gint        day_note_spell_checker;
    gchar       day_note_marker[MAXNAME];
    gchar       date_header_format[MAXNAME];
    gint        event_marker_type;
    gint        today_marker_type;
    gint        day_notes_browser_filter;
    gint        ical_export_pane_pos;
    gchar       header_color[MAXCOLORNAME];
    gchar       weekend_color[MAXCOLORNAME];
    gchar       selection_color[MAXCOLORNAME];
    gint        selector_alpha;
    gchar       mark_color[MAXCOLORNAME];
    gchar       mark_current_day_color[MAXCOLORNAME];
    gint        mark_current_day_alpha;
    gchar       birthday_mark_color[MAXCOLORNAME];
    gchar       day_name_font[MAXFONTNAME];
    gchar       calendar_font[MAXFONTNAME];
    gchar       notes_font[MAXFONTNAME];
	gchar       cal_print_month_name_font[MAXFONTNAME];
	gchar       cal_print_day_name_font[MAXFONTNAME];
	gchar       cal_print_day_num_font[MAXFONTNAME];
	gchar       cal_print_event_font[MAXFONTNAME];
	gint        cal_print_event_length;
	gint        cal_print_padding;
	gint        cal_print_page_orientation;
	gint        cal_print_tasks;
	gint        cal_print_birthdays;
	gint        cal_print_namedays;
	gint        cal_print_day_notes;
	gint        cal_print_ical;

    /* TASKS */
    gint        tasks_high_in_bold;
    gint        hide_completed;
    gint        delete_completed;
    gint        add_edit;
    gint        remember_category_in_tasks;
    gint        current_category_in_tasks;
    gint        tasks_pane_pos;
    gint        tasks_sorting_order;
    gint        tasks_sorting_mode;
    gint        tsk_visible_due_date_column;
    gint        tsk_visible_type_column;
    gint        tsk_visible_priority_column;
    gint        tsk_visible_category_column;
    gint        tasks_addedit_win_x;
    gint        tasks_addedit_win_y;
    gint        tasks_addedit_win_w;
    gint        tasks_addedit_win_h;
    gint        postpone_time;
	gint        tasks_column_idx_0;
	gint        tasks_column_idx_0_width;
	gint        tasks_column_idx_1;
	gint        tasks_column_idx_1_width;
	gint        tasks_column_idx_2;
	gint        tasks_column_idx_2_width;
	gint        tasks_column_idx_3;
	gint        tasks_column_idx_3_width;
	gint        tasks_column_idx_4;
	gint        tasks_column_idx_4_width;
	gint        tasks_column_idx_5;
	gint        tasks_column_idx_5_width;
    gchar       due_today_color[MAXCOLORNAME];
    gchar       due_7days_color[MAXCOLORNAME];
    gchar       past_due_color[MAXCOLORNAME];
    gchar       task_info_font[MAXFONTNAME];
    gchar       global_notification_command[MAXHELPERCMD];

    /* CONTACTS */
    gint        find_mode;
    gint        show_after_search;
    gint        hide_group_column;
    gint        contacts_pane_pos;
    gint        photo_width;
    gint        cnt_visible_age_column;
    gint        cnt_visible_birthday_date_column;
    gint        cnt_visible_zodiac_sign_column;
    gint        contacts_sorting_order;
    gint        contacts_sorting_mode;
    gint        contacts_addedit_win_x;
    gint        contacts_addedit_win_y;
    gint        contacts_addedit_win_w;
    gint        contacts_addedit_win_h;
    gint        contacts_export_win_x;
    gint        contacts_export_win_y;
    gint        contacts_export_win_w;
    gint        contacts_export_win_h;
    gint        contacts_import_sel_win_x;
    gint        contacts_import_sel_win_y;
    gint        contacts_import_win_x;
    gint        contacts_import_win_y;
    gint        contacts_import_win_w;
    gint        contacts_import_win_h;
    gint        contacts_birthdays_win_w;
    gint        contacts_birthdays_win_h;
	gint        contacts_column_idx_0;
	gint        contacts_column_idx_0_width;
	gint        contacts_column_idx_1;
	gint        contacts_column_idx_1_width;
	gint        contacts_column_idx_2;
	gint        contacts_column_idx_2_width;
    gchar       contact_tag_color[MAXCOLORNAME];
    gchar       contact_link_color[MAXCOLORNAME];
    gint        contact_name_font_size;
    gint        contact_item_font_size;
    gint        import_type;
    gint        import_interface_type;
    gint        import_bluetooth_channel;
    gint        import_usb_interface;
    gint        import_binary_xml;
    gchar       import_bluetooth_address[MAXADDRESS];
    gint        export_format;
    gchar       export_fields[MAXCONTACTFIELDS];

    /* NOTES */
    gint        notes_enc_algorithm;
    gint        notes_enc_hashing;
    gint        notes_comp_algorithm;
    gint        notes_comp_ratio;
    gint        notes_sorting_order;
    gint        notes_sorting_mode;
    gint        nte_visible_type_column;
    gint        nte_visible_category_column;
    gint        nte_visible_last_changes_column;
    gint        nte_visible_created_column;
	gint        notes_column_idx_0;
	gint        notes_column_idx_0_width;
	gint        notes_column_idx_1;
	gint        notes_column_idx_1_width;
	gint        notes_column_idx_2;
	gint        notes_column_idx_2_width;
	gint        notes_column_idx_3;
	gint        notes_column_idx_3_width;
	gint        notes_column_idx_4;
	gint        notes_column_idx_4_width;
    gint        remember_category_in_notes;
    gint        current_category_in_notes;
    gint        use_system_date_in_notes;
    gchar       text_separator;
    gchar       notes_editor_font[MAXFONTNAME];
};

extern  struct osmo_prefs     config;

gchar*  prefs_get_config_dir        (GUI *appGUI);
gchar*  prefs_get_config_filename   (gchar *config_filename, GUI *appGUI);
void    prefs_read_config           (GUI *appGUI);
void    prefs_write_config          (GUI *appGUI);

#endif /* _OPTIONS_PREFS_H */

