/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GTK Calendar Widget
 * Copyright (C) 1998 Cesar Miquel and Shawn T. Amundson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Some minor modifications and adaptation to OSMO - a handy personal
 * manager by pasp@users.sourceforge.net
 */

#ifndef _CALENDAR_WIDGET_H
#define _CALENDAR_WIDGET_H

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

/* Not needed, retained for compatibility -Yosh */
#include <gtk/gtksignal.h>

G_BEGIN_DECLS

/* Spacing around day/week headers and main area, inside those windows */
#define CALENDAR_MARGIN      0
/* Spacing around day/week headers and main area, outside those windows */
#define INNER_BORDER         1
/* Separation between day headers and main area */
#define CALENDAR_YSEP        4
/* Separation between week headers and main area */
#define CALENDAR_XSEP        4

#define DAY_XSEP             0  /* not really good for small calendar */
#define DAY_YSEP             0  /* not really good for small calendar */

#define SCROLL_DELAY_FACTOR  5

/* Color usage */
#define HEADER_FG_COLOR(widget)      (& (widget)->style->fg[GTK_WIDGET_STATE (widget)])
#define HEADER_BG_COLOR(widget)      (& (widget)->style->bg[GTK_WIDGET_STATE (widget)])
#define SELECTED_BG_COLOR(widget)    (& (widget)->style->base[GTK_WIDGET_HAS_FOCUS (widget) ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE])
#define SELECTED_FG_COLOR(widget)    (& (widget)->style->text[GTK_WIDGET_HAS_FOCUS (widget) ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE])
#define NORMAL_DAY_COLOR(widget)     (& (widget)->style->fg[GTK_WIDGET_STATE (widget)])
#define PREV_MONTH_COLOR(widget)     (& (widget)->style->mid[GTK_WIDGET_STATE (widget)])
#define NEXT_MONTH_COLOR(widget)     (& (widget)->style->mid[GTK_WIDGET_STATE (widget)])
#define MARKED_COLOR(widget)         (& (widget)->style->fg[GTK_WIDGET_STATE (widget)])
#define BACKGROUND_COLOR(widget)     (& (widget)->style->base[GTK_WIDGET_STATE (widget)])
#define HIGHLIGHT_BACK_COLOR(widget) (& (widget)->style->mid[GTK_WIDGET_STATE (widget)])

#define GUI_TYPE_CALENDAR                  (gui_calendar_get_type ())
#define GUI_CALENDAR(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GUI_TYPE_CALENDAR, GuiCalendar))
#define GUI_CALENDAR_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GUI_TYPE_CALENDAR, GuiCalendarClass))
#define GUI_IS_CALENDAR(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GUI_TYPE_CALENDAR))
#define GUI_IS_CALENDAR_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GUI_TYPE_CALENDAR))
#define GUI_CALENDAR_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GUI_TYPE_CALENDAR, GuiCalendarClass))

typedef struct _GuiCalendar             GuiCalendar;
typedef struct _GuiCalendarClass       GuiCalendarClass;

typedef struct _GuiCalendarPrivate     GuiCalendarPrivate;

typedef enum {
    GUI_CALENDAR_SHOW_HEADING       = 1 << 0,
    GUI_CALENDAR_SHOW_DAY_NAMES     = 1 << 1,
    GUI_CALENDAR_NO_MONTH_CHANGE    = 1 << 2,
    GUI_CALENDAR_SHOW_WEEK_NUMBERS  = 1 << 3,
    GUI_CALENDAR_WEEK_START_MONDAY  = 1 << 4
} GuiCalendarDisplayOptions;

enum {
    ARROW_YEAR_LEFT,
    ARROW_YEAR_RIGHT,
    ARROW_MONTH_LEFT,
    ARROW_MONTH_RIGHT
};

enum {
    MONTH_PREV,
    MONTH_CURRENT,
    MONTH_NEXT
};

enum {
    MONTH_CHANGED_SIGNAL,
    DAY_SELECTED_SIGNAL,
    DAY_SELECTED_DOUBLE_CLICK_SIGNAL,
    PREV_MONTH_SIGNAL,
    NEXT_MONTH_SIGNAL,
    PREV_YEAR_SIGNAL,
    NEXT_YEAR_SIGNAL,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_YEAR,
    PROP_MONTH,
    PROP_DAY,
    PROP_SHOW_HEADING,
    PROP_SHOW_DAY_NAMES,
    PROP_NO_MONTH_CHANGE,
    PROP_SHOW_WEEK_NUMBERS,
    PROP_WEEK_START_MONDAY,
    PROP_LAST
};

enum {
    CURSOR_BLOCK = 0,
    CURSOR_FRAME
};

enum {
	EVENT_MARKER_CIRCLE = 0,
	EVENT_MARKER_ELLIPSE,
	EVENT_MARKER_WAVE
};

enum {
	TODAY_MARKER_ARROW = 0,
	TODAY_MARKER_FREEHAND_CIRCLE
};
		
enum {
	TODAY_MARKER =0,
	EVENT_MARKER
};

enum {
	DAY_NOTE_MARK =0,
	EVENT_MARK,
	BIRTHDAY_MARK
};

enum {
	HEADER_COLOR = 0,
	WEEKEND_COLOR,
	SELECTOR_COLOR,
	EVENT_MARKER_COLOR,
	TODAY_MARKER_COLOR,
	BIRTHDAY_MARKER_COLOR
};

struct _GuiCalendarPrivate {
    GdkWindow *header_win;
    GdkWindow *day_name_win;
    GdkWindow *main_win;
    GdkWindow *week_win;
    GdkWindow *arrow_win[4];

    guint header_h;
    guint day_name_h;
    guint main_h;

    guint arrow_state[4];
    guint arrow_width;
    guint max_month_width;
    guint max_year_width;

    guint day_width;
    guint week_width;

    guint min_day_width;
    guint max_day_char_width;
    guint max_day_char_ascent;
    guint max_day_char_descent;
    guint max_label_char_ascent;
    guint max_label_char_descent;
    guint max_week_char_width;

    /* flags */
	guint year_before : 1;
	guint need_timer  : 1;
	guint in_drag : 1;
	guint drag_highlight : 1;

    guint32 timer;
    gint click_child;

    gint week_start;

    gint drag_start_x;
    gint drag_start_y;
};

#define GUI_CALENDAR_GET_PRIVATE(widget)  (GUI_CALENDAR (widget)->priv)

struct _GuiCalendar {
    GtkWidget widget;

    GtkStyle  *header_style;
    GtkStyle  *label_style;

    gint month;
    gint year;
    gint selected_day;

    gint day_month[6][7];
    gint day[6][7];

	/* day notes */
    gint num_marked_dates;
    gint marked_date[31];
	/* events */
	gint event_num_marked_dates;
    gint event_marked_date[31];
	/* birthdays */
	gint birthday_num_marked_dates;
    gint birthday_marked_date[31];

    GuiCalendarDisplayOptions  display_flags;
    GdkColor marked_date_color[31];

    GdkColor header_color;
    GdkColor weekend_color;
    GdkColor selector_color;
    GdkColor event_marker_color;
    GdkColor today_marker_color;
    GdkColor birthday_marker_color;
	gint selector_alpha;
	gint today_marker_alpha;

    gunichar mark_sign;
    gint cursor_type;
    gint frame_cursor_thickness;
	gint event_marker_type;
	gint today_marker_type;

    gboolean enable_cursor;

    GdkGC *gc;          /* unused */
    GdkGC *xor_gc;      /* unused */

    gint focus_row;
    gint focus_col;

    gint highlight_row;
    gint highlight_col;

    GuiCalendarPrivate *priv;
    gchar grow_space [32];

    /* Padding for future expansion */
    void (*_gtk_reserved1) (void);
    void (*_gtk_reserved2) (void);
    void (*_gtk_reserved3) (void);
    void (*_gtk_reserved4) (void);
};

struct _GuiCalendarClass {
    GtkWidgetClass parent_class;

    /* Signal handlers */
    void (* month_changed)                  (GuiCalendar *calendar);
    void (* day_selected)                       (GuiCalendar *calendar);
    void (* day_selected_double_click)      (GuiCalendar *calendar);
    void (* prev_month)                     (GuiCalendar *calendar);
    void (* next_month)                     (GuiCalendar *calendar);
    void (* prev_year)                      (GuiCalendar *calendar);
    void (* next_year)                      (GuiCalendar *calendar);

};

GType      gui_calendar_get_type            (void) G_GNUC_CONST;
GtkWidget* gui_calendar_new                 (void);

gboolean   gui_calendar_select_month        (GuiCalendar *calendar, guint month, guint year);
void       gui_calendar_select_day          (GuiCalendar *calendar, guint day);

gboolean   gui_calendar_mark_day            (GuiCalendar *calendar, guint day, gint mark_type);
gboolean   gui_calendar_unmark_day          (GuiCalendar *calendar, guint day, gint mark_type);
void       gui_calendar_clear_marks         (GuiCalendar *calendar, gint mark_type);
gboolean   gui_calendar_set_day_color       (GuiCalendar *calendar, guint day, gchar *color_str);

void       gui_calendar_set_display_options (GuiCalendar *calendar, GuiCalendarDisplayOptions flags);
GuiCalendarDisplayOptions
		   gui_calendar_get_display_options (GuiCalendar *calendar);

void       gui_calendar_get_date            (GuiCalendar *calendar, guint *year, guint *month, guint *day);
void       gui_calendar_get_gdate           (GuiCalendar *calendar, GDate *cdate);

void       gui_calendar_set_marker                  (GuiCalendar *calendar, gint marker_type, gint marker);
void       gui_calendar_set_day_note_marker_symbol  (GuiCalendar *calendar, gchar *symbol);
void       gui_calendar_set_color                   (GuiCalendar *calendar, const gchar *color, 
											         gint alpha_value, gint color_type);

void       gui_calendar_enable_cursor              (GuiCalendar *calendar, gboolean state);
void       gui_calendar_set_cursor_type            (GuiCalendar *calendar, gint cursor_type);
void       gui_calendar_set_frame_cursor_thickness (GuiCalendar *calendar, gint thickness);

G_END_DECLS

#endif /* _CALENDAR_WIDGET_H */

