
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

#ifndef _CALENDAR_H
#define _CALENDAR_H

#include "gui.h"
#include "calendar_utils.h"

void        gui_create_calendar                     (GtkWidget *notebook, GUI *appGUI);
void        cal_jump_to_date                        (GDate *cdate, GUI *appGUI);
void        calendar_set_today                      (GUI *appGUI);
void        calendar_update_date                    (guint day, guint month, guint year, GUI *appGUI);
void        calendar_update_note                    (guint uday, guint umonth, guint uyear, gchar *color, GUI *appGUI);
void        calendar_clear_text_cb                  (GtkWidget *widget, gpointer data);
void        enable_disable_note_buttons             (GUI *appGUI);
gchar *     get_marker_symbol                       (gint idx);
gchar *     calendar_get_note_text                  (GUI *appGUI);
void        calendar_create_color_selector_window   (gboolean window_pos, GUI *appGUI);
void        calendar_create_popup_menu              (GtkWidget *menu, GUI *appGUI);
void        cal_set_day_info                        (GUI *appGUI);
void        mark_events                             (GtkWidget *calendar, guint month, guint year, GUI *appGUI);
void        cal_refresh_marks                       (GUI *appGUI);
void        update_clock                            (GUI *appGUI);
void        update_aux_calendars                    (GUI *appGUI);
void        calendar_mark_events                    (GtkWidget *calendar, guint32 julian_day, guint i, GUI *appGUI);

void        calendar_store_note                     (GUI *appGUI);

void        calendar_btn_prev_day                   (GUI *appGUI);
void        calendar_btn_next_day                   (GUI *appGUI);
void        calendar_btn_prev_week                  (GUI *appGUI);
void        calendar_btn_next_week                  (GUI *appGUI);
void        calendar_btn_prev_month                 (GUI *appGUI);
void        calendar_btn_next_month                 (GUI *appGUI);
void        calendar_btn_prev_year                  (GUI *appGUI);
void        calendar_btn_next_year                  (GUI *appGUI);

#endif /* _CALENDAR_H */


