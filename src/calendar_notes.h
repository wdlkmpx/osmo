
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

#ifndef _CALENDAR_NOTES_H
#define _CALENDAR_NOTES_H

#include "gui.h"

#define CALENDAR_NOTES_NAME              "osmo_calendar_notes"
#define CALENDAR_DAY_CATEGORIES_NAME     "day_categories"
#define CALENDAR_NOTES_FILENAME          "calendar_notes.xml"

struct note {
    guint32 date;
    gchar *color;
    gchar *note;
};

enum {
    DN_FILTER_CURRENT_MONTH = 0,
    DN_FILTER_SELECTED_MONTH,
    DN_FILTER_CURRENT_YEAR,
    DN_FILTER_SELECTED_YEAR,
    DN_FILTER_SELECTED_MONTH_YEAR,
    DN_FILTER_ALL_NOTES
};

enum {     
    DN_COLUMN_DATE = 0,
    DN_COLUMN_DATE_JULIAN,
    DN_COLUMN_NOTE_LINE,
    DN_COLUMN_DONE,
    DAY_NOTES_NUM_COLUMNS
};

gboolean    cal_check_note              (guint32 julian, GUI *appGUI);
gboolean    cal_check_notes             (guint32 julian_start, guint32 julian_end, GUI *appGUI);
void        cal_notes_foreach           (guint32 julian_start, guint32 julian_end, gboolean (*cnfunc)(), GUI *appGUI);
gchar *     cal_get_note                (guint32 julian, GUI *appGUI);
gchar *     cal_get_note_color          (guint32 julian, GUI *appGUI);
void        cal_replace_note_color      (gchar *old_color, gchar *new_color, GUI *appGUI);
void        cal_add_note                (guint32 julian, gchar *color_str, gchar *text_note, GUI *appGUI);
void        cal_remove_note             (guint32 julian, GUI *appGUI);
void        cal_free_notes_list         (GUI *appGUI);
void        cal_read_notes              (GUI *appGUI);
void        cal_write_notes             (GUI *appGUI);
void        cal_notes_browser           (GUI *appGUI);
gchar *     cal_note_remove_empty_lines (gchar *note);

#endif /* _CALENDAR_NOTES_H */

