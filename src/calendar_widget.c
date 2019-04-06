/* 
 * GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GTK Calendar Widget
 * Copyright (C) 1998 Cesar Miquel, Shawn T. Amundson and Mattias Groenlund
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
 *
 *
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 *
 * Modified for OSMO organizer by pasp@users.sourceforge.net 
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#include <glib.h>

#include <gtk/gtkdnd.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkprivate.h>
#include <gdk/gdkkeysyms.h>

#include "i18n.h"
#include "gui.h"
#include "calendar_widget.h"
#include "calendar_utils.h"
#include "utils.h"
#include "utils_date.h"

static void     gui_calendar_finalize           (GObject      *calendar);
static void     gui_calendar_destroy            (GtkObject    *calendar);
static void     gui_calendar_set_property       (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec);
static void     gui_calendar_get_property       (GObject      *object,
                                                 guint         prop_id,
                                                 GValue       *value,
                                                 GParamSpec   *pspec);
static void     gui_calendar_realize            (GtkWidget        *widget);
static void     gui_calendar_unrealize          (GtkWidget        *widget);
static void     gui_calendar_size_request       (GtkWidget        *widget,
                                                 GtkRequisition   *requisition);
static void     gui_calendar_size_allocate      (GtkWidget        *widget,
											     GtkAllocation    *allocation);
static gboolean gui_calendar_expose             (GtkWidget        *widget,
											     GdkEventExpose   *event);
static gboolean gui_calendar_button_press       (GtkWidget        *widget,
											     GdkEventButton   *event);
static gboolean gui_calendar_button_release     (GtkWidget        *widget,
											     GdkEventButton   *event);
static gboolean gui_calendar_motion_notify      (GtkWidget        *widget,
											     GdkEventMotion   *event);
static gboolean gui_calendar_enter_notify       (GtkWidget        *widget,
											     GdkEventCrossing *event);
static gboolean gui_calendar_leave_notify       (GtkWidget        *widget,
											     GdkEventCrossing *event);
static gboolean gui_calendar_scroll             (GtkWidget        *widget,
											     GdkEventScroll   *event);
static gboolean gui_calendar_key_press          (GtkWidget        *widget,
											     GdkEventKey      *event);
static gboolean gui_calendar_focus_out          (GtkWidget        *widget,
											     GdkEventFocus    *event);
static void     gui_calendar_grab_notify        (GtkWidget        *widget,
											     gboolean          was_grabbed);
static void     gui_calendar_state_changed      (GtkWidget        *widget,
											     GtkStateType      previous_state);
static void     gui_calendar_style_set          (GtkWidget        *widget,
											     GtkStyle         *previous_style);
static void     gui_calendar_drag_data_get      (GtkWidget        *widget,
											     GdkDragContext   *context,
											     GtkSelectionData *selection_data,
											     guint             info,
											     guint             time);
static void     gui_calendar_drag_data_received (GtkWidget        *widget,
												 GdkDragContext   *context,
												 gint              x,
												 gint              y,
												 GtkSelectionData *selection_data,
												 guint             info,
												 guint             time);
static gboolean gui_calendar_drag_motion        (GtkWidget        *widget,
												 GdkDragContext   *context,
												 gint              x,
												 gint              y,
												 guint             time);
static void     gui_calendar_drag_leave         (GtkWidget        *widget,
												 GdkDragContext   *context,
												 guint             time);
static gboolean gui_calendar_drag_drop          (GtkWidget        *widget,
												 GdkDragContext   *context,
												 gint              x,
												 gint              y,
												 guint             time);
static void     calendar_start_spinning         (GuiCalendar *calendar,
                                                 gint         click_child);
static void     calendar_stop_spinning          (GuiCalendar *calendar);
static void     calendar_invalidate_day         (GuiCalendar *widget,
												 gint       row,
												 gint       col);
static void     calendar_invalidate_day_num     (GuiCalendar *widget,
												 gint       day);
static void     calendar_invalidate_arrow       (GuiCalendar *widget,
												 guint      arrow);
static void     calendar_compute_days           (GuiCalendar *calendar);


static guint gui_calendar_signals[LAST_SIGNAL] = { 0 };
static char    *default_abbreviated_dayname[7];
static char    *default_monthname[12];

G_DEFINE_TYPE (GuiCalendar, gui_calendar, GTK_TYPE_WIDGET)


/*------------------------------------------------------------------------------*/

static void
gui_calendar_class_init (GuiCalendarClass *class) {

	GObjectClass   *gobject_class;
    GtkObjectClass   *object_class;
    GtkWidgetClass *widget_class;

    gobject_class = (GObjectClass*)  class;
    object_class = (GtkObjectClass*)  class;
    widget_class = (GtkWidgetClass*) class;

    gobject_class->set_property = gui_calendar_set_property;
    gobject_class->get_property = gui_calendar_get_property;
    gobject_class->finalize = gui_calendar_finalize;

    object_class->destroy = gui_calendar_destroy;

    widget_class->realize = gui_calendar_realize;
    widget_class->unrealize = gui_calendar_unrealize;
    widget_class->expose_event = gui_calendar_expose;
    widget_class->size_request = gui_calendar_size_request;
    widget_class->size_allocate = gui_calendar_size_allocate;
    widget_class->button_press_event = gui_calendar_button_press;
    widget_class->button_release_event = gui_calendar_button_release;
    widget_class->motion_notify_event = gui_calendar_motion_notify;
    widget_class->enter_notify_event = gui_calendar_enter_notify;
    widget_class->leave_notify_event = gui_calendar_leave_notify;
    widget_class->key_press_event = gui_calendar_key_press;
    widget_class->scroll_event = gui_calendar_scroll;
    widget_class->style_set = gui_calendar_style_set;
    widget_class->state_changed = gui_calendar_state_changed;
    widget_class->grab_notify = gui_calendar_grab_notify;
    widget_class->focus_out_event = gui_calendar_focus_out;

    widget_class->drag_data_get = gui_calendar_drag_data_get;
    widget_class->drag_motion = gui_calendar_drag_motion;
    widget_class->drag_leave = gui_calendar_drag_leave;
    widget_class->drag_drop = gui_calendar_drag_drop;
    widget_class->drag_data_received = gui_calendar_drag_data_received;

    g_object_class_install_property (gobject_class,
                                     PROP_YEAR,
                                     g_param_spec_int ("year",
                                                       "Year",
                                                       "The selected year",
                                                       0, G_MAXINT, 0,
                                                       GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_MONTH,
                                     g_param_spec_int ("month",
                                                       "Month",
                                                       "The selected month (as a number between 0 and 11)",
                                                       0, 11, 0,
                                                       GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_DAY,
                                     g_param_spec_int ("day",
                                                       "Day",
                                                       "The selected day (as a number between 1 and 31, or 0 to unselect the currently selected day)",
                                                       0, 31, 0,
                                                       GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_HEADING,
                                     g_param_spec_boolean ("show-heading",
                                                           "Show Heading",
                                                           "If TRUE, a heading is displayed", TRUE, GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_DAY_NAMES,
                                     g_param_spec_boolean ("show-day-names",
                                                           "Show Day Names",
                                                           "If TRUE, day names are displayed", TRUE, GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_NO_MONTH_CHANGE,
                                     g_param_spec_boolean ("no-month-change",
                                                           "No Month Change",
                                                           "If TRUE, the selected month cannot be changed",
                                                           FALSE,
                                                           GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_WEEK_NUMBERS,
                                     g_param_spec_boolean ("show-week-numbers",
                                                           "Show Week Numbers",
                                                           "If TRUE, week numbers are displayed",
                                                           FALSE,
                                                           GTK_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_WEEK_START_MONDAY,
                                     g_param_spec_boolean ("week-start-monday",
                                                           "Week Start Monday",
                                                           "If TRUE, week starts at monday",
                                                           FALSE,
                                                           GTK_PARAM_READWRITE));

    gui_calendar_signals[MONTH_CHANGED_SIGNAL] =
        g_signal_new ("month_changed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, month_changed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    gui_calendar_signals[DAY_SELECTED_SIGNAL] =
        g_signal_new ("day_selected",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, day_selected),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    gui_calendar_signals[DAY_SELECTED_DOUBLE_CLICK_SIGNAL] =
        g_signal_new ("day_selected_double_click",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, day_selected_double_click),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    gui_calendar_signals[PREV_MONTH_SIGNAL] =
        g_signal_new ("prev_month",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, prev_month),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    gui_calendar_signals[NEXT_MONTH_SIGNAL] =
        g_signal_new ("next_month",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, next_month),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    gui_calendar_signals[PREV_YEAR_SIGNAL] =
        g_signal_new ("prev_year",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, prev_year),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    gui_calendar_signals[NEXT_YEAR_SIGNAL] =
        g_signal_new ("next_year",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GuiCalendarClass, next_year),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    g_type_class_add_private (gobject_class, sizeof (GuiCalendarPrivate));
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_init (GuiCalendar *calendar) {

	GtkWidget *widget = GTK_WIDGET (calendar);
    time_t secs;
    struct tm *tm;
    gint i;
    char buffer[255];
    time_t tmp_time;
    GuiCalendarPrivate *priv;
    GdkColor tmpcolor;

    priv = calendar->priv = G_TYPE_INSTANCE_GET_PRIVATE (calendar,
                            GUI_TYPE_CALENDAR,
                            GuiCalendarPrivate);

    GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);

    if (!default_abbreviated_dayname[0])
        for (i=0; i<7; i++) {
            tmp_time= (i+3)*86400;
            strftime ( buffer, sizeof (buffer), "%a", gmtime (&tmp_time));
            default_abbreviated_dayname[i] = g_locale_to_utf8 (buffer, -1, NULL, NULL, NULL);
        }

    if (!default_monthname[0])
        for (i=0; i<12; i++) {
            tmp_time=i*2764800;
            strftime ( buffer, sizeof (buffer), "%B", gmtime (&tmp_time));
            default_monthname[i] = g_locale_to_utf8 (buffer, -1, NULL, NULL, NULL);
        }

    /* Set defaults */
    secs = time (NULL);
    tm = localtime (&secs);
    calendar->month = tm->tm_mon;
    calendar->year  = 1900 + tm->tm_year;

    for (i=0;i<31;i++) {
        calendar->marked_date[i] = FALSE;
        calendar->event_marked_date[i] = FALSE;
        calendar->birthday_marked_date[i] = FALSE;
    }
    calendar->num_marked_dates = 0;
    calendar->event_num_marked_dates = 0;
    calendar->birthday_num_marked_dates = 0;
    calendar->selected_day = tm->tm_mday;

    calendar->display_flags = ( GUI_CALENDAR_SHOW_HEADING |
                                GUI_CALENDAR_SHOW_DAY_NAMES );

    calendar->highlight_row = -1;
    calendar->highlight_col = -1;

    calendar->focus_row = -1;
    calendar->focus_col = -1;

    priv->max_year_width = 0;
    priv->max_month_width = 0;
    priv->max_day_char_width = 0;
    priv->max_week_char_width = 0;

    priv->max_day_char_ascent = 0;
    priv->max_day_char_descent = 0;
    priv->max_label_char_ascent = 0;
    priv->max_label_char_descent = 0;

    priv->arrow_width = 10;

    priv->need_timer = 0;
    priv->timer = 0;
    priv->click_child = -1;

    priv->in_drag = 0;
    priv->drag_highlight = 0;

    gtk_drag_dest_set (widget, 0, NULL, 0, GDK_ACTION_COPY);
    gtk_drag_dest_add_text_targets (widget);

    priv->year_before = 0;

    gdk_color_parse("#004048", &tmpcolor);
    calendar->header_color = tmpcolor;
    gdk_color_parse("#880000", &tmpcolor);
    calendar->weekend_color = tmpcolor;
    gdk_color_parse("#408080", &tmpcolor);
    calendar->selector_color = tmpcolor;

    calendar->mark_sign = '\'';
    calendar->frame_cursor_thickness = 2;
    calendar->enable_cursor = TRUE;

    priv->year_before = 0;  /* years to be displayed before months */
    priv->week_start  = 0;  /* sunday */

    calendar_compute_days (calendar);
}

/*------------------------------------------------------------------------------*/

static void
calendar_set_month_next (GuiCalendar *calendar) {
    gint month_len;

    g_return_if_fail (GTK_IS_WIDGET (calendar));

    if (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE)
        return;

    if (calendar->month == 11) {
        calendar->month = 0;
        calendar->year++;
    } else {
        calendar->month++;
    }

    calendar_compute_days (calendar);
    g_signal_emit (calendar,
                   gui_calendar_signals[NEXT_MONTH_SIGNAL],
                   0);
    g_signal_emit (calendar,
                   gui_calendar_signals[MONTH_CHANGED_SIGNAL],
                   0);

    month_len = utl_get_month_length(g_date_is_leap_year (calendar->year), calendar->month + 1);

    if (month_len < calendar->selected_day) {
        calendar->selected_day = 0;
        gui_calendar_select_day (calendar, month_len);
    } else {
        gui_calendar_select_day (calendar, calendar->selected_day);
    }

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

/*------------------------------------------------------------------------------*/

static void
calendar_set_year_prev (GuiCalendar *calendar) {
    gint month_len;

    g_return_if_fail (GTK_IS_WIDGET (calendar));

    calendar->year--;
    calendar_compute_days (calendar);
    g_signal_emit (calendar,
                   gui_calendar_signals[PREV_YEAR_SIGNAL],
                   0);
    g_signal_emit (calendar,
                   gui_calendar_signals[MONTH_CHANGED_SIGNAL],
                   0);

    month_len = utl_get_month_length(g_date_is_leap_year (calendar->year), calendar->month + 1);

    if (month_len < calendar->selected_day) {
        calendar->selected_day = 0;
        gui_calendar_select_day (calendar, month_len);
    } else {
        gui_calendar_select_day (calendar, calendar->selected_day);
    }

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

/*------------------------------------------------------------------------------*/

static void
calendar_set_year_next (GuiCalendar *calendar) {
    gint month_len;

    g_return_if_fail (GTK_IS_WIDGET (calendar));

    calendar->year++;
    calendar_compute_days (calendar);
    g_signal_emit (calendar,
                   gui_calendar_signals[NEXT_YEAR_SIGNAL],
                   0);
    g_signal_emit (calendar,
                   gui_calendar_signals[MONTH_CHANGED_SIGNAL],
                   0);

    month_len = utl_get_month_length(g_date_is_leap_year (calendar->year), calendar->month + 1);

    if (month_len < calendar->selected_day) {
        calendar->selected_day = 0;
        gui_calendar_select_day (calendar, month_len);
    } else {
        gui_calendar_select_day (calendar, calendar->selected_day);
    }

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

/*------------------------------------------------------------------------------*/

static void
calendar_compute_days (GuiCalendar *calendar) {

	GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (GTK_WIDGET (calendar));
    gint month;
    gint year;
    gint ndays_in_month;
    gint ndays_in_prev_month;
    gint first_day;
    gint row;
    gint col;
    gint day;

    g_return_if_fail (GUI_IS_CALENDAR (calendar));

    year = calendar->year;
    month = calendar->month + 1;

    ndays_in_month = utl_get_month_length(g_date_is_leap_year (year), month);

    first_day = utl_day_of_week (year, month, 1);
    first_day = (first_day + 7 - priv->week_start) % 7;

    /* Compute days of previous month */
    if (month > 1) {
        ndays_in_prev_month = utl_get_month_length(g_date_is_leap_year (year), month-1);
    } else {
        ndays_in_prev_month = utl_get_month_length(g_date_is_leap_year (year), 12);
    }

    day = ndays_in_prev_month - first_day + 1;

    row = 0;
    if (first_day > 0) {
        for (col = 0; col < first_day; col++) {
            calendar->day[row][col] = day;
            calendar->day_month[row][col] = MONTH_PREV;
            day++;
        }
    }

    /* Compute days of current month */
    col = first_day;
    for (day = 1; day <= ndays_in_month; day++) {
        calendar->day[row][col] = day;
        calendar->day_month[row][col] = MONTH_CURRENT;

        col++;
        if (col == 7) {
            row++;
            col = 0;
        }
    }

    /* Compute days of next month */
    day = 1;
    for (; row <= 5; row++) {
        for (; col <= 6; col++) {
            calendar->day[row][col] = day;
            calendar->day_month[row][col] = MONTH_NEXT;
            day++;
        }
        col = 0;
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_select_and_focus_day (GuiCalendar *calendar,
                               guint        day) {

	gint old_focus_row = calendar->focus_row;
    gint old_focus_col = calendar->focus_col;
    gint row;
    gint col;

    for (row = 0; row < 6; row ++)
        for (col = 0; col < 7; col++) {
            if (calendar->day_month[row][col] == MONTH_CURRENT
                    && calendar->day[row][col] == day) {
                calendar->focus_row = row;
                calendar->focus_col = col;
            }
        }

    if (old_focus_row != -1 && old_focus_col != -1) {
        calendar_invalidate_day (calendar, old_focus_row, old_focus_col);
    }

    gui_calendar_select_day (calendar, day);
}

/*------------------------------------------------------------------------------*/

static gint
calendar_row_height (GuiCalendar *calendar) {
    return (GUI_CALENDAR_GET_PRIVATE (calendar)->main_h - CALENDAR_MARGIN
            - ((calendar->display_flags & GUI_CALENDAR_SHOW_DAY_NAMES)
               ? CALENDAR_YSEP : CALENDAR_MARGIN)) / 6;
}

/*------------------------------------------------------------------------------*/
/* calendar_left_x_for_column: returns the x coordinate
 * for the left of the column */

static gint
calendar_left_x_for_column (GuiCalendar *calendar,
                            gint     column) {
    gint width;
    gint x_left;

    if (gtk_widget_get_direction (GTK_WIDGET (calendar)) == GTK_TEXT_DIR_RTL)
        column = 6 - column;

    width = GUI_CALENDAR_GET_PRIVATE (calendar)->day_width;

    if (calendar->display_flags & GUI_CALENDAR_SHOW_WEEK_NUMBERS) {
        x_left = CALENDAR_XSEP + (width + DAY_XSEP) * column;
    } else {
        x_left = CALENDAR_MARGIN + (width + DAY_XSEP) * column;
    }

    return x_left;
}

/*------------------------------------------------------------------------------*/
/* column_from_x: returns the column 0-6 that the
 * x pixel of the xwindow is in */

static gint
calendar_column_from_x (GuiCalendar *calendar,
                        gint         event_x) {
    gint c, column;
    gint x_left, x_right;

    column = -1;

    for (c = 0; c < 7; c++) {
        x_left = calendar_left_x_for_column (calendar, c);
        x_right = x_left + GUI_CALENDAR_GET_PRIVATE (calendar)->day_width;

        if (event_x >= x_left && event_x < x_right) {
            column = c;
            break;
        }
    }

    return column;
}

/*------------------------------------------------------------------------------*/
/* calendar_top_y_for_row: returns the y coordinate
 * for the top of the row */

static gint
calendar_top_y_for_row (GuiCalendar *calendar,
                        gint         row) {

    return (GUI_CALENDAR_GET_PRIVATE (calendar)->main_h
            - (CALENDAR_MARGIN + (6 - row)
               * calendar_row_height (calendar)));
}

/*------------------------------------------------------------------------------*/
/* row_from_y: returns the row 0-5 that the
 * y pixel of the xwindow is in */

static gint
calendar_row_from_y (GuiCalendar *calendar,
                     gint     event_y) {
    gint r, row;
    gint height;
    gint y_top, y_bottom;

    height = calendar_row_height (calendar);
    row = -1;

    for (r = 0; r < 6; r++) {
        y_top = calendar_top_y_for_row (calendar, r);
        y_bottom = y_top + height;

        if (event_y >= y_top && event_y < y_bottom) {
            row = r;
            break;
        }
    }

    return row;
}

/*------------------------------------------------------------------------------*/

static void
calendar_arrow_rectangle (GuiCalendar  *calendar,
                          guint         arrow,
                          GdkRectangle *rect) {

	GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    gboolean year_left;

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR) {
        year_left = priv->year_before;
    } else {
        year_left = !priv->year_before;
    }

    rect->y = 3;
    rect->width = priv->arrow_width;
    rect->height = priv->header_h - 7;

    switch (arrow) {
    case ARROW_MONTH_LEFT:
        if (year_left) {
            rect->x = (widget->allocation.width - 2 * widget->style->xthickness
                       - (3 + 2*priv->arrow_width
                          + priv->max_month_width));
        } else {
            rect->x = 3;
        }
        break;
    case ARROW_MONTH_RIGHT:
        if (year_left) {
            rect->x = (widget->allocation.width - 2 * widget->style->xthickness
                       - 3 - priv->arrow_width);
        } else {
            rect->x = (priv->arrow_width
                       + priv->max_month_width);
        }
        break;
    case ARROW_YEAR_LEFT:
        if (year_left) {
            rect->x = 3;
        } else {
            rect->x = (widget->allocation.width - 2 * widget->style->xthickness
                       - (3 + 2*priv->arrow_width
                          + priv->max_year_width));
        }
        break;
    case ARROW_YEAR_RIGHT:
        if (year_left) {
            rect->x = (priv->arrow_width
                       + priv->max_year_width);
        } else {
            rect->x = (widget->allocation.width - 2 * widget->style->xthickness
                       - 3 - priv->arrow_width);
        }
        break;
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_day_rectangle (GuiCalendar  *calendar,
                        gint          row,
                        gint          col,
                        GdkRectangle *rect) {

	GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);

    rect->x = calendar_left_x_for_column (calendar, col);
    rect->y = calendar_top_y_for_row (calendar, row);
    rect->height = calendar_row_height (calendar);
    rect->width = priv->day_width;
}

/*------------------------------------------------------------------------------*/

static void
calendar_set_month_prev (GuiCalendar *calendar) {
    gint month_len;

    if (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE)
        return;

    if (calendar->month == 0) {
        calendar->month = 11;
        calendar->year--;
    } else {
        calendar->month--;
    }

    month_len = utl_get_month_length(g_date_is_leap_year (calendar->year), calendar->month + 1);

    calendar_compute_days (calendar);

    g_signal_emit (calendar,
                   gui_calendar_signals[PREV_MONTH_SIGNAL],
                   0);
    g_signal_emit (calendar,
                   gui_calendar_signals[MONTH_CHANGED_SIGNAL],
                   0);

    if (month_len < calendar->selected_day) {
        calendar->selected_day = 0;
        gui_calendar_select_day (calendar, month_len);
    } else {
        if (calendar->selected_day < 0)
            calendar->selected_day = calendar->selected_day + 1 + utl_get_month_length(g_date_is_leap_year (calendar->year), calendar->month + 1);
        gui_calendar_select_day (calendar, calendar->selected_day);
    }

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_finalize (GObject *object) {
    (* G_OBJECT_CLASS (gui_calendar_parent_class)->finalize) (object);
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_destroy (GtkObject *object) {
    calendar_stop_spinning (GUI_CALENDAR (object));

    GTK_OBJECT_CLASS (gui_calendar_parent_class)->destroy (object);
}

/*------------------------------------------------------------------------------*/

static void
calendar_set_display_option (GuiCalendar              *calendar,
                             GuiCalendarDisplayOptions flag,
                             gboolean                  setting) {
    GuiCalendarDisplayOptions flags;
    if (setting) {
        flags = calendar->display_flags | flag;
    } else {
        flags = calendar->display_flags & ~flag;
    }
    gui_calendar_set_display_options (calendar, flags);
}

/*------------------------------------------------------------------------------*/

static gboolean
calendar_get_display_option (GuiCalendar              *calendar,
                             GuiCalendarDisplayOptions flag) {
    return (calendar->display_flags & flag) != 0;
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec) {
    GuiCalendar *calendar;

    calendar = GUI_CALENDAR (object);

    switch (prop_id) {
    case PROP_YEAR:
        gui_calendar_select_month (calendar,
                                   calendar->month,
                                   g_value_get_int (value));
        break;
    case PROP_MONTH:
        gui_calendar_select_month (calendar,
                                   g_value_get_int (value),
                                   calendar->year);
        break;
    case PROP_DAY:
        gui_calendar_select_day (calendar,
                                 g_value_get_int (value));
        break;
    case PROP_SHOW_HEADING:
        calendar_set_display_option (calendar,
                                     GUI_CALENDAR_SHOW_HEADING,
                                     g_value_get_boolean (value));
        break;
    case PROP_SHOW_DAY_NAMES:
        calendar_set_display_option (calendar,
                                     GUI_CALENDAR_SHOW_DAY_NAMES,
                                     g_value_get_boolean (value));
        break;
    case PROP_NO_MONTH_CHANGE:
        calendar_set_display_option (calendar,
                                     GUI_CALENDAR_NO_MONTH_CHANGE,
                                     g_value_get_boolean (value));
        break;
    case PROP_SHOW_WEEK_NUMBERS:
        calendar_set_display_option (calendar,
                                     GUI_CALENDAR_SHOW_WEEK_NUMBERS,
                                     g_value_get_boolean (value));
        break;
    case PROP_WEEK_START_MONDAY:
        calendar_set_display_option (calendar,
                                     GUI_CALENDAR_WEEK_START_MONDAY,
                                     g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_get_property (GObject      *object,
                           guint         prop_id,
                           GValue       *value,
                           GParamSpec   *pspec) {
    GuiCalendar *calendar;

    calendar = GUI_CALENDAR (object);

    switch (prop_id) {
    case PROP_YEAR:
        g_value_set_int (value, calendar->year);
        break;
    case PROP_MONTH:
        g_value_set_int (value, calendar->month);
        break;
    case PROP_DAY:
        g_value_set_int (value, calendar->selected_day);
        break;
    case PROP_SHOW_HEADING:
        g_value_set_boolean (value, calendar_get_display_option (calendar,
                             GUI_CALENDAR_SHOW_HEADING));
        break;
    case PROP_SHOW_DAY_NAMES:
        g_value_set_boolean (value, calendar_get_display_option (calendar,
                             GUI_CALENDAR_SHOW_DAY_NAMES));
        break;
    case PROP_NO_MONTH_CHANGE:
        g_value_set_boolean (value, calendar_get_display_option (calendar,
                             GUI_CALENDAR_NO_MONTH_CHANGE));
        break;
    case PROP_SHOW_WEEK_NUMBERS:
        g_value_set_boolean (value, calendar_get_display_option (calendar,
                             GUI_CALENDAR_SHOW_WEEK_NUMBERS));
        break;
    case PROP_WEEK_START_MONDAY:
        g_value_set_boolean (value, calendar_get_display_option (calendar,
                             GUI_CALENDAR_WEEK_START_MONDAY));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_realize_arrows (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    GdkWindowAttr attributes;
    gint attributes_mask;
    gint i;

    /* arrow windows */
    if (! (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE)
            && (calendar->display_flags & GUI_CALENDAR_SHOW_HEADING)) {
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);
        attributes.event_mask = (gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK
                                 | GDK_BUTTON_PRESS_MASK    | GDK_BUTTON_RELEASE_MASK
                                 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
        for (i = 0; i < 4; i++) {
            GdkRectangle rect;
            calendar_arrow_rectangle (calendar, i, &rect);

            attributes.x = rect.x;
            attributes.y = rect.y;
            attributes.width = rect.width;
            attributes.height = rect.height;
            priv->arrow_win[i] = gdk_window_new (priv->header_win,
                                                 &attributes,
                                                 attributes_mask);
            if (GTK_WIDGET_IS_SENSITIVE (widget))
                priv->arrow_state[i] = GTK_STATE_NORMAL;
            else
                priv->arrow_state[i] = GTK_STATE_INSENSITIVE;
            gdk_window_set_background (priv->arrow_win[i],
                                       HEADER_BG_COLOR (GTK_WIDGET (calendar)));
            gdk_window_show (priv->arrow_win[i]);
            gdk_window_set_user_data (priv->arrow_win[i], widget);
        }
    } else {
        for (i = 0; i < 4; i++)
            priv->arrow_win[i] = NULL;
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_realize_header (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    GdkWindowAttr attributes;
    gint attributes_mask;

    /* header window */
    if (calendar->display_flags & GUI_CALENDAR_SHOW_HEADING) {
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);
        attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
        attributes.x = widget->style->xthickness;
        attributes.y = widget->style->ythickness;
        attributes.width = widget->allocation.width - 2 * attributes.x;
        attributes.height = priv->header_h - 2 * attributes.y;
        priv->header_win = gdk_window_new (widget->window,
                                           &attributes, attributes_mask);

        gdk_window_set_background (priv->header_win,
                                   HEADER_BG_COLOR (GTK_WIDGET (calendar)));
        gdk_window_show (priv->header_win);
        gdk_window_set_user_data (priv->header_win, widget);

    } else {
        priv->header_win = NULL;
    }
    calendar_realize_arrows (calendar);
}

/*------------------------------------------------------------------------------*/

static void
calendar_realize_day_names (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    GdkWindowAttr attributes;
    gint attributes_mask;

    /* day names window */
    if ( calendar->display_flags & GUI_CALENDAR_SHOW_DAY_NAMES) {
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);
        attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
        attributes.x = (widget->style->xthickness + INNER_BORDER);
        attributes.y = priv->header_h + (widget->style->ythickness
                                         + INNER_BORDER);
        attributes.width = (widget->allocation.width
                            - (widget->style->xthickness + INNER_BORDER)
                            * 2);
        attributes.height = priv->day_name_h;
        priv->day_name_win = gdk_window_new (widget->window,
                                             &attributes,
                                             attributes_mask);
        gdk_window_set_background (priv->day_name_win,
                                   BACKGROUND_COLOR ( GTK_WIDGET ( calendar)));
        gdk_window_show (priv->day_name_win);
        gdk_window_set_user_data (priv->day_name_win, widget);
    } else {
        priv->day_name_win = NULL;
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_realize_week_numbers (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    GdkWindowAttr attributes;
    gint attributes_mask;

    /* week number window */
    if (calendar->display_flags & GUI_CALENDAR_SHOW_WEEK_NUMBERS) {
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);
        attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
        attributes.x = widget->style->xthickness + INNER_BORDER;
        attributes.y = (priv->header_h + priv->day_name_h
                        + (widget->style->ythickness + INNER_BORDER));
        attributes.width = priv->week_width;
        attributes.height = priv->main_h;
        priv->week_win = gdk_window_new (widget->window,
                                         &attributes, attributes_mask);
        gdk_window_set_background (priv->week_win,
                                   BACKGROUND_COLOR (GTK_WIDGET (calendar)));
        gdk_window_show (priv->week_win);
        gdk_window_set_user_data (priv->week_win, widget);
    } else {
        priv->week_win = NULL;
    }
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_realize (GtkWidget *widget) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    GdkWindowAttr attributes;
    gint attributes_mask;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask =  (gtk_widget_get_events (widget)
                              | GDK_EXPOSURE_MASK |GDK_KEY_PRESS_MASK | GDK_SCROLL_MASK);
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    widget->window = gdk_window_new (widget->parent->window,
                                     &attributes, attributes_mask);

    widget->style = gtk_style_attach (widget->style, widget->window);

    /* header window */
    calendar_realize_header (calendar);
    /* day names window */
    calendar_realize_day_names (calendar);
    /* week number window */
    calendar_realize_week_numbers (calendar);
    /* main window */
    attributes.event_mask =  (gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK
                              | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                              | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);

    attributes.x = priv->week_width + (widget->style->ythickness + INNER_BORDER);
    attributes.y = (priv->header_h + priv->day_name_h
                    + (widget->style->ythickness + INNER_BORDER));
    attributes.width = (widget->allocation.width - attributes.x
                        - (widget->style->xthickness + INNER_BORDER));
    attributes.height = priv->main_h;
    priv->main_win = gdk_window_new (widget->window,
                                     &attributes, attributes_mask);
    gdk_window_set_background (priv->main_win,
                               BACKGROUND_COLOR ( GTK_WIDGET ( calendar)));
    gdk_window_show (priv->main_win);
    gdk_window_set_user_data (priv->main_win, widget);
    gdk_window_set_background (widget->window, BACKGROUND_COLOR (widget));
    gdk_window_show (widget->window);
    gdk_window_set_user_data (widget->window, widget);
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_unrealize (GtkWidget *widget) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    gint i;

    if (priv->header_win) {
        for (i = 0; i < 4; i++) {
            if (priv->arrow_win[i]) {
                gdk_window_set_user_data (priv->arrow_win[i], NULL);
                gdk_window_destroy (priv->arrow_win[i]);
                priv->arrow_win[i] = NULL;
            }
        }
        gdk_window_set_user_data (priv->header_win, NULL);
        gdk_window_destroy (priv->header_win);
        priv->header_win = NULL;
    }

    if (priv->week_win) {
        gdk_window_set_user_data (priv->week_win, NULL);
        gdk_window_destroy (priv->week_win);
        priv->week_win = NULL;
    }

    if (priv->main_win) {
        gdk_window_set_user_data (priv->main_win, NULL);
        gdk_window_destroy (priv->main_win);
        priv->main_win = NULL;
    }
    if (priv->day_name_win) {
        gdk_window_set_user_data (priv->day_name_win, NULL);
        gdk_window_destroy (priv->day_name_win);
        priv->day_name_win = NULL;
    }

    if (GTK_WIDGET_CLASS (gui_calendar_parent_class)->unrealize)
        (* GTK_WIDGET_CLASS (gui_calendar_parent_class)->unrealize) (widget);
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_size_request (GtkWidget      *widget,
                           GtkRequisition *requisition) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    PangoLayout *layout;
    PangoRectangle logical_rect;

    gint height;
    gint i;
    gint calendar_margin = CALENDAR_MARGIN;
    gint header_width, main_width;
    gint max_header_height = 0;
    gint focus_width;
    gint focus_padding;

    gtk_widget_style_get (GTK_WIDGET (widget),
                          "focus-line-width", &focus_width,
                          "focus-padding", &focus_padding,
                          NULL);

    layout = gtk_widget_create_pango_layout (widget, NULL);

    /* calculate the requisition width for the widget */

    /* header width */

    if (calendar->display_flags & GUI_CALENDAR_SHOW_HEADING) {
        priv->max_month_width = 0;
        for (i = 0; i < 12; i++) {
            pango_layout_set_text (layout, default_monthname[i], -1);
            pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
            priv->max_month_width = MAX (priv->max_month_width,
                                         logical_rect.width + 8);
            max_header_height = MAX (max_header_height, logical_rect.height);
        }

        priv->max_year_width = 0;
        pango_layout_set_text (layout, "2000", -1); /* the widest year text */
        pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
        priv->max_year_width = MAX (priv->max_year_width,
                                    logical_rect.width + 8);
        max_header_height = MAX (max_header_height, logical_rect.height);
    } else {
        priv->max_month_width = 0;
        priv->max_year_width = 0;
    }

    if (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE)
        header_width = (priv->max_month_width
                        + priv->max_year_width
                        + 3 * 3);
    else
        header_width = (priv->max_month_width
                        + priv->max_year_width
                        + 4 * priv->arrow_width + 3 * 3);

    /* mainwindow labels width */

    priv->max_day_char_width = 0;
    priv->max_day_char_ascent = 0;
    priv->max_day_char_descent = 0;
    priv->min_day_width = 0;

    for (i = 0; i < 9; i++) {
        gchar buffer[32];
 
        g_snprintf (buffer, sizeof (buffer), "%d", i * 11);     /* %d - calendar day digits */
        pango_layout_set_text (layout, buffer, -1);
        pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
        priv->min_day_width = MAX (priv->min_day_width,
                                   logical_rect.width);

        priv->max_day_char_ascent = MAX (priv->max_day_char_ascent,
                                         PANGO_ASCENT (logical_rect));
        priv->max_day_char_descent = MAX (priv->max_day_char_descent,
                                          PANGO_DESCENT (logical_rect));
    }
    /* We add one to max_day_char_width to be able to make the marked day "bold" */
    priv->max_day_char_width = priv->min_day_width / 2 + 1;

    priv->max_label_char_ascent = 0;
    priv->max_label_char_descent = 0;
    if (calendar->display_flags & GUI_CALENDAR_SHOW_DAY_NAMES)
        for (i = 0; i < 7; i++) {
            pango_layout_set_text (layout, default_abbreviated_dayname[i], -1);
            pango_layout_line_get_pixel_extents (pango_layout_get_lines (layout)->data, NULL, &logical_rect);

            priv->min_day_width = MAX (priv->min_day_width, logical_rect.width);
            priv->max_label_char_ascent = MAX (priv->max_label_char_ascent,
                                               PANGO_ASCENT (logical_rect));
            priv->max_label_char_descent = MAX (priv->max_label_char_descent,
                                                PANGO_DESCENT (logical_rect));
        }

    priv->max_week_char_width = 0;
    if (calendar->display_flags & GUI_CALENDAR_SHOW_WEEK_NUMBERS)
        for (i = 0; i < 9; i++) {
            gchar buffer[32];
            g_snprintf (buffer, sizeof (buffer), "%d", i * 11);     /* %d - calendar week digits */
            pango_layout_set_text (layout, buffer, -1);
            pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
            priv->max_week_char_width = MAX (priv->max_week_char_width,
                                             logical_rect.width / 2);
        }

    main_width = (7 * (priv->min_day_width + (focus_padding + focus_width) * 2) + (DAY_XSEP * 6) + CALENDAR_MARGIN * 2
                  + (priv->max_week_char_width
                     ? priv->max_week_char_width * 2 + (focus_padding + focus_width) * 2 + CALENDAR_XSEP * 2
                     : 0));


    requisition->width = MAX (header_width, main_width + INNER_BORDER * 2) + widget->style->xthickness * 2;

    /* calculate the requisition height for the widget */

    if (calendar->display_flags & GUI_CALENDAR_SHOW_HEADING) {
        priv->header_h = (max_header_height + CALENDAR_YSEP * 2);
    } else {
        priv->header_h = 0;
    }

    if (calendar->display_flags & GUI_CALENDAR_SHOW_DAY_NAMES) {
        priv->day_name_h = (priv->max_label_char_ascent
                            + priv->max_label_char_descent
                            + 2 * (focus_padding + focus_width) + calendar_margin);
        calendar_margin = CALENDAR_YSEP;
    } else {
        priv->day_name_h = 0;
    }

    priv->main_h = (CALENDAR_MARGIN + calendar_margin
                    + 6 * (priv->max_day_char_ascent
                           + priv->max_day_char_descent
                           + 2 * (focus_padding + focus_width))
                    + DAY_YSEP * 5);

    height = (priv->header_h + priv->day_name_h
              + priv->main_h);

    requisition->height = height + (widget->style->ythickness + INNER_BORDER) * 2;

    g_object_unref (layout);
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    gint xthickness = widget->style->xthickness;
    gint ythickness = widget->style->xthickness;
    guint i;

    widget->allocation = *allocation;

    if (calendar->display_flags & GUI_CALENDAR_SHOW_WEEK_NUMBERS) {
        priv->day_width = (priv->min_day_width
                           * ((allocation->width - (xthickness + INNER_BORDER) * 2
                               - (CALENDAR_MARGIN * 2) -  (DAY_XSEP * 6) - CALENDAR_XSEP * 2))
                           / (7 * priv->min_day_width + priv->max_week_char_width * 2));
        priv->week_width = ((allocation->width - (xthickness + INNER_BORDER) * 2
                             - (CALENDAR_MARGIN * 2) - (DAY_XSEP * 6) - CALENDAR_XSEP * 2 )
                            - priv->day_width * 7 + CALENDAR_MARGIN + CALENDAR_XSEP);
    } else {
        priv->day_width = (allocation->width
                           - (xthickness + INNER_BORDER) * 2
                           - (CALENDAR_MARGIN * 2)
                           - (DAY_XSEP * 6))/7;
        priv->week_width = 0;
    }

    if (GTK_WIDGET_REALIZED (widget)) {
        gdk_window_move_resize (widget->window,
                                allocation->x, allocation->y,
                                allocation->width, allocation->height);
        if (priv->header_win)
            gdk_window_move_resize (priv->header_win,
                                    xthickness, ythickness,
                                    allocation->width - 2 * xthickness, priv->header_h);

        for (i = 0 ; i < 4 ; i++) {
            if (priv->arrow_win[i]) {
                GdkRectangle rect;
                calendar_arrow_rectangle (calendar, i, &rect);

                gdk_window_move_resize (priv->arrow_win[i],
                                        rect.x, rect.y, rect.width, rect.height);
            }
        }

        if (priv->day_name_win)
            gdk_window_move_resize (priv->day_name_win,
                                    xthickness + INNER_BORDER,
                                    priv->header_h + (widget->style->ythickness + INNER_BORDER),
                                    allocation->width - (xthickness + INNER_BORDER) * 2,
                                    priv->day_name_h);
        if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR) {
            if (priv->week_win)
                gdk_window_move_resize (priv->week_win,
                                        (xthickness + INNER_BORDER),
                                        priv->header_h + priv->day_name_h
                                        + (widget->style->ythickness + INNER_BORDER),
                                        priv->week_width,
                                        priv->main_h);
            gdk_window_move_resize (priv->main_win,
                                    priv->week_width + (xthickness + INNER_BORDER),
                                    priv->header_h + priv->day_name_h
                                    + (widget->style->ythickness + INNER_BORDER),
                                    allocation->width
                                    - priv->week_width
                                    - (xthickness + INNER_BORDER) * 2,
                                    priv->main_h);
        } else {
            gdk_window_move_resize (priv->main_win,
                                    (xthickness + INNER_BORDER),
                                    priv->header_h + priv->day_name_h
                                    + (widget->style->ythickness + INNER_BORDER),
                                    allocation->width
                                    - priv->week_width
                                    - (xthickness + INNER_BORDER) * 2,
                                    priv->main_h);
            if (priv->week_win)
                gdk_window_move_resize (priv->week_win,
                                        allocation->width
                                        - priv->week_width
                                        - (xthickness + INNER_BORDER),
                                        priv->header_h + priv->day_name_h
                                        + (widget->style->ythickness + INNER_BORDER),
                                        priv->week_width,
                                        priv->main_h);
        }
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_paint_header (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    cairo_t *cr;
    char buffer[255];
    int x, y;
    gint header_width;
    gint max_month_width;
    gint max_year_width;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gboolean year_left;
    time_t tmp_time;
    struct tm *tm;
    gchar *str;

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR) {
        year_left = priv->year_before;
	} else {
        year_left = !priv->year_before;
	}

    cr = gdk_cairo_create (priv->header_win);

    header_width = widget->allocation.width - 2 * widget->style->xthickness;

    max_month_width = priv->max_month_width;
    max_year_width = priv->max_year_width;

    gtk_paint_shadow (widget->style, priv->header_win,
                      GTK_STATE_NORMAL, GTK_SHADOW_OUT,
                      NULL, widget, "calendar",
                      0, 0, header_width, priv->header_h);

    tmp_time = 1;  /* Jan 1 1970, 00:00:01 UTC */
    tm = gmtime (&tmp_time);
    tm->tm_year = calendar->year - 1900;
    strftime (buffer, sizeof (buffer), "%Y", tm);       /* year format */
    str = g_locale_to_utf8 (buffer, -1, NULL, NULL, NULL);
    layout = gtk_widget_create_pango_layout (widget, str);
    g_free (str);

    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    /* draw title */
    y = (priv->header_h - logical_rect.height) / 2;

    /* draw year and its arrows */

    if (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE) {
        if (year_left) {
            x = 3 + (max_year_width - logical_rect.width)/2;
        } else {
            x = header_width - (3 + max_year_width
                                - (max_year_width - logical_rect.width)/2);
        }
    } else {
        if (year_left) {
            x = 3 + priv->arrow_width + (max_year_width - logical_rect.width)/2;
        } else {
            x = header_width - (3 + priv->arrow_width + max_year_width
                                - (max_year_width - logical_rect.width)/2);
        }
    }

    gdk_cairo_set_source_color (cr, HEADER_FG_COLOR (GTK_WIDGET (calendar)));
    cairo_move_to (cr, x, y);
    pango_cairo_show_layout (cr, layout);

    /* draw month */
    g_snprintf (buffer, sizeof (buffer), "%s", default_monthname[calendar->month]);
    pango_layout_set_text (layout, buffer, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    if (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE) {
        if (year_left) {
            x = header_width - (3 + max_month_width
                                - (max_month_width - logical_rect.width)/2);
        } else {
            x = 3 + (max_month_width - logical_rect.width) / 2;
        }
    } else {
        if (year_left) {
            x = header_width - (3 + priv->arrow_width + max_month_width
                                - (max_month_width - logical_rect.width)/2);
        } else {
            x = 3 + priv->arrow_width + (max_month_width - logical_rect.width)/2;
        }
    }

    cairo_move_to (cr, x, y);
    pango_cairo_show_layout (cr, layout);

    g_object_unref (layout);
    cairo_destroy (cr);
}

/*------------------------------------------------------------------------------*/

static void
calendar_paint_day_names (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    cairo_t *cr;
    char buffer[255];
    int day,i;
    int day_width, cal_width;
    int day_wid_sep;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gint focus_padding;
    gint focus_width;

    cr = gdk_cairo_create (priv->day_name_win);

    gtk_widget_style_get (GTK_WIDGET (widget),
                          "focus-line-width", &focus_width,
                          "focus-padding", &focus_padding,
                          NULL);

    day_width = priv->day_width;
    cal_width = widget->allocation.width;
    day_wid_sep = day_width + DAY_XSEP;

    /* draw rectangles as inverted background for the labels */

    gdk_cairo_set_source_color (cr, &calendar->header_color);
    cairo_rectangle (cr,
                     CALENDAR_MARGIN, CALENDAR_MARGIN,
                     cal_width-CALENDAR_MARGIN * 2,
                     priv->day_name_h - CALENDAR_MARGIN);
    cairo_fill (cr);

    if (calendar->display_flags & GUI_CALENDAR_SHOW_WEEK_NUMBERS) {
        cairo_rectangle (cr,
                         CALENDAR_MARGIN,
                         priv->day_name_h - CALENDAR_YSEP,
                         priv->week_width - CALENDAR_YSEP - CALENDAR_MARGIN,
                         CALENDAR_YSEP);
        cairo_fill (cr);
    }

    /* write the labels */

    layout = gtk_widget_create_pango_layout (widget, NULL);

    gdk_cairo_set_source_color (cr, SELECTED_FG_COLOR (widget));

    for (i = 0; i < 7; i++) {

        if (gtk_widget_get_direction (GTK_WIDGET (calendar)) == GTK_TEXT_DIR_RTL) {
            day = 6 - i;
        } else {
            day = i;
        }

        day = (day + priv->week_start) % 7;
        g_snprintf (buffer, sizeof (buffer), "%s", default_abbreviated_dayname[day]);

        pango_layout_set_text (layout, buffer, -1);
        pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

        cairo_move_to (cr,
                       (CALENDAR_MARGIN +
                        + (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR ?
                           (priv->week_width + (priv->week_width ? CALENDAR_XSEP : 0))
                                   : 0)
                                + day_wid_sep * i
                                + (day_width - logical_rect.width)/2),
                               CALENDAR_MARGIN + focus_width + focus_padding + logical_rect.y);
        pango_cairo_show_layout (cr, layout);
    }

    g_object_unref (layout);
    cairo_destroy (cr);
}

/*------------------------------------------------------------------------------*/

static void
calendar_paint_week_numbers (GuiCalendar *calendar) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    cairo_t *cr;
    guint row, week = 0, year;
    gint x_loc;
    char buffer[32];
    gint y_loc, day_height;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gint focus_padding;
    gint focus_width;

    cr = gdk_cairo_create (priv->week_win);

    gtk_widget_style_get (GTK_WIDGET (widget),
                          "focus-line-width", &focus_width,
                          "focus-padding", &focus_padding,
                          NULL);

    /* draw a rectangle as inverted background for the labels */

    gdk_cairo_set_source_color (cr, &calendar->header_color);
    if (priv->day_name_win) {
        cairo_rectangle (cr,
                         CALENDAR_MARGIN,
                         0,
                         priv->week_width - CALENDAR_MARGIN,
                         priv->main_h - CALENDAR_MARGIN);
    } else {
        cairo_rectangle (cr,
                         CALENDAR_MARGIN,
                         CALENDAR_MARGIN,
                         priv->week_width - CALENDAR_MARGIN,
                         priv->main_h - 2 * CALENDAR_MARGIN);
    }
    cairo_fill (cr);

    /* write the labels */

    layout = gtk_widget_create_pango_layout (widget, NULL);

    gdk_cairo_set_source_color (cr, SELECTED_FG_COLOR (widget));
    day_height = calendar_row_height (calendar);
    for (row = 0; row < 6; row++) {
        gboolean result;

        year = calendar->year;
        if (calendar->day[row][6] < 15 && row > 3 && calendar->month == 11)
            year++;

        result = utl_week_of_year (&week, &year,
                               ((calendar->day[row][6] < 15 && row > 3 ? 1 : 0)
                                + calendar->month) % 12 + 1, calendar->day[row][6]);
        g_return_if_fail (result);

        g_snprintf (buffer, sizeof (buffer), "%d", week);   /* %d - calendar week digits, %Id - for localized digits */
        pango_layout_set_text (layout, buffer, -1);
        pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

        y_loc = calendar_top_y_for_row (calendar, row) + (day_height - logical_rect.height) / 2;

        x_loc = (priv->week_width
                 - logical_rect.width
                 - CALENDAR_XSEP - focus_padding - focus_width);

        cairo_move_to (cr, x_loc, y_loc);
        pango_cairo_show_layout (cr, layout);
    }

    g_object_unref (layout);
    cairo_destroy (cr);
}

/*------------------------------------------------------------------------------*/

static void
calendar_invalidate_day_num (GuiCalendar *calendar,
                             gint         day) {
    gint r, c, row, col;

    row = -1;
    col = -1;
    for (r = 0; r < 6; r++)
        for (c = 0; c < 7; c++)
            if (calendar->day_month[r][c] == MONTH_CURRENT && calendar->day[r][c] == day) {
                row = r;
                col = c;
            }

    g_return_if_fail (row != -1);
    g_return_if_fail (col != -1);

    calendar_invalidate_day (calendar, row, col);
}

/*------------------------------------------------------------------------------*/

static void
calendar_invalidate_day (GuiCalendar *calendar,
                         gint         row,
                         gint         col) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);

    if (priv->main_win) {
        GdkRectangle day_rect;

        calendar_day_rectangle (calendar, row, col, &day_rect);
        gdk_window_invalidate_rect (priv->main_win, &day_rect, FALSE);
    }
}

/*------------------------------------------------------------------------------*/

static void
cal_draw_rounded_rectangle (cairo_t *cr, GdkRectangle *rect, GdkColor *color, gint alpha, gint stroke)
{
	utl_cairo_set_color (cr, color, alpha);
	utl_draw_rounded_rectangle (cr, rect->x, rect->y, rect->width, rect->height, rect->height * 0.2, 1);
	utl_cairo_draw (cr, stroke);
}

/*------------------------------------------------------------------------------*/

static void
cal_draw_left_arrow (cairo_t *cr, GdkRectangle *rect, GdkColor *color, gint alpha)
{
	utl_cairo_set_color (cr, color, alpha);
	utl_draw_left_arrow (cr, rect->x + rect->width * 0.8, rect->y + rect->height * 0.5,
	                         rect->width * 0.13, rect->height * 0.55, 0.65);
	cairo_fill (cr);
}

/*------------------------------------------------------------------------------*/

static void
cal_draw_ellipse (cairo_t *cr, GdkRectangle *rect, GdkColor *color)
{
	gdk_cairo_set_source_color (cr, color);
	cairo_save (cr);
	cairo_scale (cr, 2, 1);
	cairo_arc (cr, (rect->x + (rect->width * 0.5)) * 0.5, rect->y + rect->height * 0.5,
	           rect->height * 0.5 - 3.0, 0.0, 2.0 * M_PI);
	cairo_fill (cr);
	cairo_restore (cr);
}

/*------------------------------------------------------------------------------*/

static void
cal_draw_wave (cairo_t *cr, GdkRectangle *rect, GdkColor *color)
{
	gdk_cairo_set_source_color (cr, color);
	cairo_set_line_width (cr, rect->height * 0.7);
	cairo_move_to (cr, rect->x + rect->width * 0.2, rect->y + rect->height * 0.5);
	cairo_curve_to (cr, rect->x + rect->width * 0.5, rect->y + rect->height * 0.3,
	                    rect->x + rect->width * 0.5, rect->y + rect->height * 0.7,
	                    rect->x + rect->width * 0.8, rect->y + rect->height * 0.5);
	cairo_stroke (cr);
}

/*------------------------------------------------------------------------------*/

static void
cal_draw_circle (cairo_t *cr, GdkRectangle *rect, GdkColor *color)
{
	gdk_cairo_set_source_color (cr, color);
	cairo_arc (cr, rect->x + rect->width * 0.5, rect->y + rect->height * 0.5, rect->height * 0.5 - 3.0, 0.0, 2.0 * M_PI);
	cairo_fill (cr);
}

/*------------------------------------------------------------------------------*/

static void
cal_draw_freehand_circle (cairo_t *cr, GdkRectangle *rect, GdkColor *color, guint alpha)
{
	gdouble dtop, dbottom, dleft, dright;
	gdouble dwidth, dheight;

	dheight = rect->height;
	dwidth = dheight * 2.0;
	dtop = rect->y;
	dbottom = rect->y + dheight;
	dleft = rect->x + (rect->width - dwidth) / 2.0;
	dright = dleft + dwidth;

	utl_cairo_set_color (cr, color, alpha);
	cairo_move_to (cr, dleft + dwidth * 0.60, dtop);
	cairo_curve_to (cr, dleft + dwidth * 0.05, dtop, dleft, dbottom, dleft + dwidth * 0.50, dbottom);
	cairo_curve_to (cr, dright, dbottom,
						dright - dwidth * 0.05, dtop - dheight * 0.15,
						dleft + dwidth * 0.40, dtop + dheight * 0.20);
	cairo_line_to (cr, dleft + dwidth * 0.40, dtop + dheight * 0.25);
	cairo_curve_to (cr, dright - dwidth * 0.15, dtop - dheight * 0.05,
						dright - dwidth * 0.05, dbottom - dheight * 0.15,
						dleft + dwidth * 0.50, dbottom - dheight * 0.15);
	cairo_curve_to (cr, dleft + dwidth * 0.05, dbottom - dheight * 0.15,
						dleft + dwidth * 0.20, dtop,
						dleft + dwidth * 0.60, dtop + dheight * 0.05);
	cairo_close_path (cr);
	cairo_fill (cr);
}

/*------------------------------------------------------------------------------*/

static void
cal_cairo_draw_cursor (cairo_t *cr, GdkRectangle *rect, gint thickness)
{
	gint bias_x, bias_y, bias;

	bias = thickness % 2;
	bias_x = thickness >> 1;
	bias_y = bias_x + bias;

	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

	cairo_move_to  (cr, rect->x+bias, rect->y+bias_y-bias);
	cairo_line_to (cr, rect->x+rect->width-bias_x, rect->y+bias_y-bias);
	cairo_line_to (cr, rect->x+rect->width-bias_x, rect->y+rect->height-bias_y);
	cairo_line_to (cr, rect->x+bias_x+bias, rect->y+rect->height-bias_y);
	cairo_line_to (cr, rect->x+bias_x+bias, rect->y+bias_y-bias_y*bias);
	cairo_set_line_width (cr, thickness);
	cairo_stroke (cr);

	cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
}

/*------------------------------------------------------------------------------*/

static void
calendar_paint_day (GuiCalendar *calendar,
                    gint         row,
                    gint         col) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    cairo_t *cr;
    GdkColor *text_color;
    gchar buffer[32];
    gint day;
    gint x_loc, y_loc;
    GdkRectangle day_rect;
    time_t secs;
    struct tm *tm;

    PangoLayout *layout;
    PangoRectangle logical_rect;

    g_return_if_fail (row < 6);
    g_return_if_fail (col < 7);

    cr = gdk_cairo_create (priv->main_win);

    day = calendar->day[row][col];

    calendar_day_rectangle (calendar, row, col, &day_rect);

    if (calendar->mark_sign) {
        if (calendar->marked_date[day-1] && calendar->day_month[row][col] == MONTH_CURRENT) {
            g_snprintf (buffer, sizeof (buffer), "%lc%d", calendar->mark_sign, day);
        } else {
            g_snprintf (buffer, sizeof (buffer), "%d", day);
        }
    } else {
        g_snprintf (buffer, sizeof (buffer), "%d", day);
    }
    text_color = NORMAL_DAY_COLOR (widget);

    if (g_utf8_validate(buffer, -1, NULL) == FALSE) {
        g_snprintf (buffer, sizeof (buffer), "%d", day);
    }

    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = day_rect.x + day_rect.width / 2 + priv->max_day_char_width;
    x_loc -= logical_rect.width;
    y_loc = day_rect.y + (day_rect.height - logical_rect.height) / 2;

	/* mark day note */
	if (calendar->marked_date[day-1] && calendar->day_month[row][col] == MONTH_CURRENT) {
		cal_draw_rounded_rectangle (cr, &day_rect, &calendar->marked_date_color[day-1], OPAQUE, 0);
	}

	/* mark event */
	if (calendar->event_marked_date[day-1] && calendar->day_month[row][col] == MONTH_CURRENT) {

		if (calendar->event_marker_type == EVENT_MARKER_ELLIPSE) {
			cal_draw_ellipse (cr, &day_rect, &calendar->event_marker_color);
		} else if (calendar->event_marker_type == EVENT_MARKER_WAVE) {
			cal_draw_wave (cr, &day_rect, &calendar->event_marker_color);
		} else {
			cal_draw_circle (cr, &day_rect, &calendar->event_marker_color);
		}

	}

	/* mark birthday */
	if (calendar->birthday_marked_date[day-1] && calendar->day_month[row][col] == MONTH_CURRENT) {
		gdk_cairo_set_source_color (cr, &calendar->birthday_marker_color);
		utl_draw_rounded_rectangle (cr, day_rect.x + day_rect.width - day_rect.height * 0.35, 
									day_rect.y + (day_rect.height - day_rect.height * 0.7) * 0.5, 
									day_rect.height * 0.25, day_rect.height * 0.7, 3.5, 1.0);
		utl_cairo_draw (cr, 0);
	}

    /* mark current day */
    secs = time (NULL);
    tm = localtime (&secs);

	if (calendar->month == tm->tm_mon && calendar->year == 1900 + tm->tm_year
	    && calendar->day[row][col] == tm->tm_mday && calendar->day_month[row][col] == MONTH_CURRENT) {

		if (calendar->today_marker_type == TODAY_MARKER_FREEHAND_CIRCLE) {
			cal_draw_freehand_circle (cr, &day_rect, &(calendar->today_marker_color), calendar->today_marker_alpha);
		} else {
			cal_draw_left_arrow (cr, &day_rect, &(calendar->today_marker_color), calendar->today_marker_alpha);
		}

	}

    if (calendar->day_month[row][col] == MONTH_PREV) {
        text_color = PREV_MONTH_COLOR (widget);
    } else if (calendar->day_month[row][col] == MONTH_NEXT) {
        text_color =  NEXT_MONTH_COLOR (widget);
    } else {

		if (calendar->selected_day == day && calendar->enable_cursor == TRUE) {
			utl_cairo_set_color (cr, &calendar->selector_color, calendar->selector_alpha);

			if (calendar->cursor_type == CURSOR_FRAME) {
				cal_cairo_draw_cursor (cr, &day_rect, calendar->frame_cursor_thickness);
			} else {
				gdk_cairo_rectangle (cr, &day_rect);
				cairo_fill (cr);
			}

		}

        text_color = NORMAL_DAY_COLOR (widget);

        /* mark saturday and sunday */
        if (((col == 5 || col == 6) && (priv->week_start == 1)) || ((!col || col == 6) && !priv->week_start)) {
            if (calendar->day_month[row][col] != MONTH_NEXT && calendar->day_month[row][col] != MONTH_PREV) {
                text_color = &calendar->weekend_color;
            }
        }

        if (calendar->enable_cursor == TRUE && calendar->cursor_type == CURSOR_BLOCK
            && calendar->selected_day == day && calendar->selector_alpha == OPAQUE) {
            text_color = SELECTED_FG_COLOR (widget);
        }
    }

    gdk_cairo_set_source_color (cr, text_color);
    cairo_move_to (cr, x_loc, y_loc);
    pango_cairo_show_layout (cr, layout);

    if (calendar->marked_date[day-1] && calendar->day_month[row][col] == MONTH_CURRENT) {
        gdk_cairo_set_source_color (cr, text_color);
        cairo_move_to (cr, x_loc - 1, y_loc);
        pango_cairo_show_layout (cr, layout);
    }


    if (GTK_WIDGET_HAS_FOCUS (calendar) && calendar->focus_row == row && calendar->focus_col == col) {

        GtkStateType state;

        if (calendar->selected_day == day) {
            state = GTK_WIDGET_HAS_FOCUS (widget) ? GTK_STATE_SELECTED : GTK_STATE_ACTIVE;
        } else {
            state = GTK_STATE_NORMAL;
        }

        gtk_paint_focus (widget->style,
                         priv->main_win,
                         state,
                         NULL, widget, "calendar-day",
                         day_rect.x,     day_rect.y,
                         day_rect.width, day_rect.height);
    }

    g_object_unref (layout);
    cairo_destroy (cr);
}

/*------------------------------------------------------------------------------*/

static void
calendar_paint_main (GuiCalendar *calendar) {
    gint row, col;

    for (col = 0; col < 7; col++)
        for (row = 0; row < 6; row++)
            calendar_paint_day (calendar, row, col);
}

/*------------------------------------------------------------------------------*/

static void
calendar_invalidate_arrow (GuiCalendar *calendar,
                           guint        arrow) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    GdkWindow *window;

    window = priv->arrow_win[arrow];
    if (window)
        gdk_window_invalidate_rect (window, NULL, FALSE);
}

/*------------------------------------------------------------------------------*/

static void
calendar_paint_arrow (GuiCalendar *calendar,
                      guint        arrow) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    GdkWindow *window;

    window = priv->arrow_win[arrow];
    if (window) {
        cairo_t *cr = gdk_cairo_create (window);
        gint width, height;
        gint state;

        state = priv->arrow_state[arrow];

        gdk_cairo_set_source_color (cr, &widget->style->bg[state]);
        cairo_paint (cr);
        cairo_destroy (cr);

        gdk_drawable_get_size (window, &width, &height);
        if (arrow == ARROW_MONTH_LEFT || arrow == ARROW_YEAR_LEFT)
            gtk_paint_arrow (widget->style, window, state,
                             GTK_SHADOW_OUT, NULL, widget, "calendar",
                             GTK_ARROW_LEFT, TRUE,
                             width/2 - 3, height/2 - 4, 8, 8);
        else
            gtk_paint_arrow (widget->style, window, state,
                             GTK_SHADOW_OUT, NULL, widget, "calendar",
                             GTK_ARROW_RIGHT, TRUE,
                             width/2 - 4, height/2 - 4, 8, 8);
    }
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_expose (GtkWidget      *widget,
                     GdkEventExpose *event) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    int i;

    if (GTK_WIDGET_DRAWABLE (widget)) {

        if (calendar->display_flags & GUI_CALENDAR_WEEK_START_MONDAY) {
            priv->week_start  = 1;  /* monday */
        } else {
            priv->week_start  = 0;  /* sunday */
        }

        calendar_compute_days (calendar);

        if (event->window == priv->main_win)
            calendar_paint_main (calendar);

        if (event->window == priv->header_win)
            calendar_paint_header (calendar);

        for (i = 0; i < 4; i++)
            if (event->window == priv->arrow_win[i])
                calendar_paint_arrow (calendar, i);

        if (event->window == priv->day_name_win)
            calendar_paint_day_names (calendar);

        if (event->window == priv->week_win)
            calendar_paint_week_numbers (calendar);
        if (event->window == widget->window) {
            gtk_paint_shadow (widget->style, widget->window, GTK_WIDGET_STATE (widget),
                              GTK_SHADOW_IN, NULL, widget, "calendar",
                              0, 0, widget->allocation.width, widget->allocation.height);
        }
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

static void
calendar_arrow_action (GuiCalendar *calendar,
                       guint        arrow) {
    switch (arrow) {
    case ARROW_YEAR_LEFT:
        calendar_set_year_prev (calendar);
        break;
    case ARROW_YEAR_RIGHT:
        calendar_set_year_next (calendar);
        break;
    case ARROW_MONTH_LEFT:
        calendar_set_month_prev (calendar);
        break;
    case ARROW_MONTH_RIGHT:
        calendar_set_month_next (calendar);
        break;
    default:
        ;
        /* do nothing */
    }
}

/*------------------------------------------------------------------------------*/

static gboolean
calendar_timer (gpointer data) {

	GuiCalendar *calendar = data;
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    gboolean retval = FALSE;

    GDK_THREADS_ENTER ();

    if (priv->timer) {
        calendar_arrow_action (calendar, priv->click_child);

        if (priv->need_timer) {
            GtkSettings *settings;
            guint        timeout;

            settings = gtk_widget_get_settings (GTK_WIDGET (calendar));
            g_object_get (settings, "gtk-timeout-repeat", &timeout, NULL);

            priv->need_timer = FALSE;
            priv->timer = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                              timeout * SCROLL_DELAY_FACTOR,
                                              (GSourceFunc) calendar_timer,
                                              (gpointer) calendar, NULL);
        } else
            retval = TRUE;
    }

    GDK_THREADS_LEAVE ();

    return retval;
}

/*------------------------------------------------------------------------------*/

static void
calendar_start_spinning (GuiCalendar *calendar,
                         gint         click_child) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);

    priv->click_child = click_child;

    if (!priv->timer) {
        GtkSettings *settings;
        guint        timeout;

        settings = gtk_widget_get_settings (GTK_WIDGET (calendar));
        g_object_get (settings, "gtk-timeout-initial", &timeout, NULL);

        priv->need_timer = TRUE;
        priv->timer = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                          timeout,
                                          (GSourceFunc) calendar_timer,
                                          (gpointer) calendar, NULL);
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_stop_spinning (GuiCalendar *calendar) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);

    if (priv->timer) {
        g_source_remove (priv->timer);
        priv->timer = 0;
        priv->need_timer = FALSE;
    }
}

/*------------------------------------------------------------------------------*/

static void
calendar_main_button_press (GuiCalendar    *calendar,
                            GdkEventButton *event) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    gint x, y;
    gint row, col;
    gint day_month;
    gint day;

    x = (gint) (event->x);
    y = (gint) (event->y);

    row = calendar_row_from_y (calendar, y);
    col = calendar_column_from_x (calendar, x);

    /* If row or column isn't found, just return. */
    if (row == -1 || col == -1)
        return;

    day_month = calendar->day_month[row][col];

    if (event->type == GDK_BUTTON_PRESS) {
        if (event->button == 1) {
            day = calendar->day[row][col];

            if (day_month == MONTH_PREV)
                calendar_set_month_prev (calendar);
            else if (day_month == MONTH_NEXT)
                calendar_set_month_next (calendar);

            if (!GTK_WIDGET_HAS_FOCUS (widget))
                gtk_widget_grab_focus (widget);

            if (event->button == 1) {
                priv->in_drag = 0;                /* TODO: drag enable */
                priv->drag_start_x = x;
                priv->drag_start_y = y;
            }

            if (calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE) {
                if (day_month != MONTH_PREV && day_month != MONTH_NEXT) {
                    calendar_select_and_focus_day (calendar, day);
                }
            } else {
                calendar_select_and_focus_day (calendar, day);
            }
       }
    } else if (event->type == GDK_2BUTTON_PRESS) {
        priv->in_drag = 0;
        if (day_month == MONTH_CURRENT && event->button == 1)
            g_signal_emit (calendar,
                           gui_calendar_signals[DAY_SELECTED_DOUBLE_CLICK_SIGNAL],
                           0);
    }
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_button_press (GtkWidget      *widget,
                           GdkEventButton *event) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    gint arrow = -1;

    if (event->window == priv->main_win)
        calendar_main_button_press (calendar, event);

    if (!GTK_WIDGET_HAS_FOCUS (widget))
        gtk_widget_grab_focus (widget);

    for (arrow = ARROW_YEAR_LEFT; arrow <= ARROW_MONTH_RIGHT; arrow++) {
        if (event->window == priv->arrow_win[arrow]) {

            /* only call the action on single click, not double */
            if (event->type == GDK_BUTTON_PRESS) {
                if (event->button == 1)
                    calendar_start_spinning (calendar, arrow);

                calendar_arrow_action (calendar, arrow);
            }

            return TRUE;
        }
    }

    if (event->type == GDK_BUTTON_PRESS) {
        if (event->button == 1)
            return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_button_release (GtkWidget    *widget,
                             GdkEventButton *event) {

	GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);

    if (event->button == 1) {
        calendar_stop_spinning (calendar);

        if (priv->in_drag)
            priv->in_drag = 0;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_motion_notify (GtkWidget      *widget,
                            GdkEventMotion *event) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    gint event_x, event_y;
    gint row, col;
    gint old_row, old_col;

    event_x = (gint) (event->x);
    event_y = (gint) (event->y);

    if (event->window == priv->main_win) {

        if (priv->in_drag) {
            if (gtk_drag_check_threshold (widget,
                                          priv->drag_start_x, priv->drag_start_y,
                                          event->x, event->y)) {
                GdkDragContext *context;
                GtkTargetList *target_list = gtk_target_list_new (NULL, 0);
                gtk_target_list_add_text_targets (target_list, 0);
                context = gtk_drag_begin (widget, target_list, GDK_ACTION_COPY,
                                          1, (GdkEvent *)event);
                priv->in_drag = 0;

                gtk_target_list_unref (target_list);
                gtk_drag_set_icon_default (context);
            }
        } else {
            row = calendar_row_from_y (calendar, event_y);
            col = calendar_column_from_x (calendar, event_x);

            if (row != calendar->highlight_row || calendar->highlight_col != col) {
                old_row = calendar->highlight_row;
                old_col = calendar->highlight_col;
                if (old_row > -1 && old_col > -1) {
                    calendar->highlight_row = -1;
                    calendar->highlight_col = -1;
                    calendar_invalidate_day (calendar, old_row, old_col);
                }

                calendar->highlight_row = row;
                calendar->highlight_col = col;

                if (row > -1 && col > -1)
                    calendar_invalidate_day (calendar, row, col);
            }
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_enter_notify (GtkWidget        *widget,
                           GdkEventCrossing *event) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);

    if (event->window == priv->arrow_win[ARROW_MONTH_LEFT]) {
        priv->arrow_state[ARROW_MONTH_LEFT] = GTK_STATE_PRELIGHT;
        calendar_invalidate_arrow (calendar, ARROW_MONTH_LEFT);
    }

    if (event->window == priv->arrow_win[ARROW_MONTH_RIGHT]) {
        priv->arrow_state[ARROW_MONTH_RIGHT] = GTK_STATE_PRELIGHT;
        calendar_invalidate_arrow (calendar, ARROW_MONTH_RIGHT);
    }

    if (event->window == priv->arrow_win[ARROW_YEAR_LEFT]) {
        priv->arrow_state[ARROW_YEAR_LEFT] = GTK_STATE_PRELIGHT;
        calendar_invalidate_arrow (calendar, ARROW_YEAR_LEFT);
    }

    if (event->window == priv->arrow_win[ARROW_YEAR_RIGHT]) {
        priv->arrow_state[ARROW_YEAR_RIGHT] = GTK_STATE_PRELIGHT;
        calendar_invalidate_arrow (calendar, ARROW_YEAR_RIGHT);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_leave_notify (GtkWidget        *widget,
                           GdkEventCrossing *event) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    gint row;
    gint col;

    if (event->window == priv->main_win) {
        row = calendar->highlight_row;
        col = calendar->highlight_col;
        calendar->highlight_row = -1;
        calendar->highlight_col = -1;
        if (row > -1 && col > -1)
            calendar_invalidate_day (calendar, row, col);
    }

    if (event->window == priv->arrow_win[ARROW_MONTH_LEFT]) {
        priv->arrow_state[ARROW_MONTH_LEFT] = GTK_STATE_NORMAL;
        calendar_invalidate_arrow (calendar, ARROW_MONTH_LEFT);
    }

    if (event->window == priv->arrow_win[ARROW_MONTH_RIGHT]) {
        priv->arrow_state[ARROW_MONTH_RIGHT] = GTK_STATE_NORMAL;
        calendar_invalidate_arrow (calendar, ARROW_MONTH_RIGHT);
    }

    if (event->window == priv->arrow_win[ARROW_YEAR_LEFT]) {
        priv->arrow_state[ARROW_YEAR_LEFT] = GTK_STATE_NORMAL;
        calendar_invalidate_arrow (calendar, ARROW_YEAR_LEFT);
    }

    if (event->window == priv->arrow_win[ARROW_YEAR_RIGHT]) {
        priv->arrow_state[ARROW_YEAR_RIGHT] = GTK_STATE_NORMAL;
        calendar_invalidate_arrow (calendar, ARROW_YEAR_RIGHT);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_scroll (GtkWidget      *widget,
                     GdkEventScroll *event) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);

    if (event->direction == GDK_SCROLL_UP) {
        if (!GTK_WIDGET_HAS_FOCUS (widget))
            gtk_widget_grab_focus (widget);
        calendar_set_month_prev (calendar);
    } else if (event->direction == GDK_SCROLL_DOWN) {
        if (!GTK_WIDGET_HAS_FOCUS (widget))
            gtk_widget_grab_focus (widget);
        calendar_set_month_next (calendar);
    } else
        return FALSE;

    return TRUE;
}

/*------------------------------------------------------------------------------*/

static void
move_focus (GuiCalendar *calendar,
            gint         direction) {
    GtkTextDirection text_dir = gtk_widget_get_direction (GTK_WIDGET (calendar));

    if (calendar->enable_cursor == FALSE) {
        return;
    }

    if ((text_dir == GTK_TEXT_DIR_LTR && direction == -1) ||
            (text_dir == GTK_TEXT_DIR_RTL && direction == 1)) {
        if (calendar->focus_col > 0)
            calendar->focus_col--;
        else if (calendar->focus_row > 0) {
            calendar->focus_col = 6;
            calendar->focus_row--;
        }

        if (calendar->focus_col < 0)
            calendar->focus_col = 6;
        if (calendar->focus_row < 0)
            calendar->focus_row = 5;
    } else {
        if (calendar->focus_col < 6)
            calendar->focus_col++;
        else if (calendar->focus_row < 5) {
            calendar->focus_col = 0;
            calendar->focus_row++;
        }

        if (calendar->focus_col < 0)
            calendar->focus_col = 0;
        if (calendar->focus_row < 0)
            calendar->focus_row = 0;
    }
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_key_press (GtkWidget   *widget,
                        GdkEventKey *event) {
    GuiCalendar *calendar;
    gint return_val;
    gint old_focus_row;
    gint old_focus_col;
    gint row, col, day;

    calendar = GUI_CALENDAR (widget);
    return_val = FALSE;

    if (calendar->enable_cursor == FALSE) {
        return return_val;
    }

    old_focus_row = calendar->focus_row;
    old_focus_col = calendar->focus_col;

    switch (event->keyval) {
    case GDK_KP_Left:
    case GDK_Left:
        return_val = TRUE;
        if (event->state & GDK_CONTROL_MASK)
            calendar_set_month_prev (calendar);
        else {
            move_focus (calendar, -1);
            calendar_invalidate_day (calendar, old_focus_row, old_focus_col);
            calendar_invalidate_day (calendar, calendar->focus_row,
                                     calendar->focus_col);
        }
        break;
    case GDK_KP_Right:
    case GDK_Right:
        return_val = TRUE;
        if (event->state & GDK_CONTROL_MASK)
            calendar_set_month_next (calendar);
        else {
            move_focus (calendar, 1);
            calendar_invalidate_day (calendar, old_focus_row, old_focus_col);
            calendar_invalidate_day (calendar, calendar->focus_row,
                                     calendar->focus_col);
        }
        break;
    case GDK_KP_Up:
    case GDK_Up:
        return_val = TRUE;
        if (event->state & GDK_CONTROL_MASK)
            calendar_set_year_prev (calendar);
        else {
            if (calendar->focus_row > 0)
                calendar->focus_row--;
            if (calendar->focus_row < 0)
                calendar->focus_row = 5;
            if (calendar->focus_col < 0)
                calendar->focus_col = 6;
            calendar_invalidate_day (calendar, old_focus_row, old_focus_col);
            calendar_invalidate_day (calendar, calendar->focus_row,
                                     calendar->focus_col);
        }
        break;
    case GDK_KP_Down:
    case GDK_Down:
        return_val = TRUE;
        if (event->state & GDK_CONTROL_MASK)
            calendar_set_year_next (calendar);
        else {
            if (calendar->focus_row < 5)
                calendar->focus_row++;
            if (calendar->focus_col < 0)
                calendar->focus_col = 0;
            calendar_invalidate_day (calendar, old_focus_row, old_focus_col);
            calendar_invalidate_day (calendar, calendar->focus_row,
                                     calendar->focus_col);
        }
        break;
    case GDK_KP_Space:
    case GDK_space:
        row = calendar->focus_row;
        col = calendar->focus_col;

        if (row > -1 && col > -1) {
            return_val = TRUE;

            day = calendar->day[row][col];
            if (calendar->day_month[row][col] == MONTH_PREV)
                calendar_set_month_prev (calendar);
            else if (calendar->day_month[row][col] == MONTH_NEXT)
                calendar_set_month_next (calendar);

            calendar_select_and_focus_day (calendar, day);
        }
    }

    return return_val;
}

/*------------------------------------------------------------------------------*/

static void
calendar_set_background (GtkWidget *widget) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    gint i;

    if (GTK_WIDGET_REALIZED (widget)) {
        for (i = 0; i < 4; i++) {
            if (priv->arrow_win[i])
                gdk_window_set_background (priv->arrow_win[i],
                                           HEADER_BG_COLOR (widget));
        }
        if (priv->header_win)
            gdk_window_set_background (priv->header_win,
                                       HEADER_BG_COLOR (widget));
        if (priv->day_name_win)
            gdk_window_set_background (priv->day_name_win,
                                       BACKGROUND_COLOR (widget));
        if (priv->week_win)
            gdk_window_set_background (priv->week_win,
                                       BACKGROUND_COLOR (widget));
        if (priv->main_win)
            gdk_window_set_background (priv->main_win,
                                       BACKGROUND_COLOR (widget));
        if (widget->window)
            gdk_window_set_background (widget->window,
                                       BACKGROUND_COLOR (widget));
    }
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_style_set (GtkWidget *widget,
                        GtkStyle  *previous_style) {
    if (previous_style && GTK_WIDGET_REALIZED (widget))
        calendar_set_background (widget);
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_state_changed (GtkWidget      *widget,
                            GtkStateType    previous_state) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    int i;

    if (!GTK_WIDGET_IS_SENSITIVE (widget)) {
        priv->in_drag = 0;
        calendar_stop_spinning (calendar);
    }

    for (i = 0; i < 4; i++)
        if (GTK_WIDGET_IS_SENSITIVE (widget))
            priv->arrow_state[i] = GTK_STATE_NORMAL;
        else
            priv->arrow_state[i] = GTK_STATE_INSENSITIVE;

    calendar_set_background (widget);
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_grab_notify (GtkWidget *widget,
                          gboolean   was_grabbed) {
    if (!was_grabbed)
        calendar_stop_spinning (GUI_CALENDAR (widget));
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_focus_out (GtkWidget     *widget,
                        GdkEventFocus *event) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);

    gtk_widget_queue_draw (widget);

    calendar_stop_spinning (GUI_CALENDAR (widget));

    priv->in_drag = 0;

    return FALSE;
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_drag_data_get (GtkWidget        *widget,
                            GdkDragContext   *context,
                            GtkSelectionData *selection_data,
                            guint             info,
                            guint             time) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    GDate *date;
    gchar str[128];
    gsize len;

    date = g_date_new_dmy (calendar->selected_day, calendar->month + 1, calendar->year);
    len = g_date_strftime (str, 127, "%x", date);
    gtk_selection_data_set_text (selection_data, str, len);

    g_date_free (date);
}

/*------------------------------------------------------------------------------*/
/* Get/set whether drag_motion requested the drag data and
 * drag_data_received should thus not actually insert the data,
 * since the data doesn't result from a drop.
 */

static void
set_status_pending (GdkDragContext *context,
                    GdkDragAction   suggested_action) {
    g_object_set_data (G_OBJECT (context),
                       "gtk-calendar-status-pending",
                       GINT_TO_POINTER (suggested_action));
}

/*------------------------------------------------------------------------------*/

static GdkDragAction
get_status_pending (GdkDragContext *context) {
    return GPOINTER_TO_INT (g_object_get_data (G_OBJECT (context),
                            "gtk-calendar-status-pending"));
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_drag_leave (GtkWidget      *widget,
                         GdkDragContext *context,
                         guint           time) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);

    priv->drag_highlight = 0;
    gtk_drag_unhighlight (widget);

}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_drag_motion (GtkWidget      *widget,
                          GdkDragContext *context,
                          gint            x,
                          gint            y,
                          guint           time) {
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (widget);
    GdkAtom target;

    if (!priv->drag_highlight) {
        priv->drag_highlight = 1;
        gtk_drag_highlight (widget);
    }

    target = gtk_drag_dest_find_target (widget, context, NULL);
    if (target == GDK_NONE || context->suggested_action == 0)
        gdk_drag_status (context, 0, time);
    else {
        set_status_pending (context, context->suggested_action);
        gtk_drag_get_data (widget, context, target, time);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

static gboolean
gui_calendar_drag_drop (GtkWidget      *widget,
                        GdkDragContext *context,
                        gint            x,
                        gint            y,
                        guint           time) {
    GdkAtom target;

    target = gtk_drag_dest_find_target (widget, context, NULL);
    if (target != GDK_NONE) {
        gtk_drag_get_data (widget, context,
                           target,
                           time);
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

static void
gui_calendar_drag_data_received (GtkWidget        *widget,
                                 GdkDragContext   *context,
                                 gint              x,
                                 gint              y,
                                 GtkSelectionData *selection_data,
                                 guint             info,
                                 guint             time) {
    GuiCalendar *calendar = GUI_CALENDAR (widget);
    guint day, month, year;
    gchar *str;
    GDate *date;
    GdkDragAction suggested_action;

    suggested_action = get_status_pending (context);

    if (suggested_action) {
        set_status_pending (context, 0);

        /* We are getting this data due to a request in drag_motion,
         * rather than due to a request in drag_drop, so we are just
         * supposed to call drag_status, not actually paste in the
         * data.
         */
        str = (gchar *) gtk_selection_data_get_text (selection_data);
        if (str) {
            date = g_date_new ();
            g_date_set_parse (date, str);
            if (!g_date_valid (date))
                suggested_action = 0;
            g_date_free (date);
            g_free (str);
        } else
            suggested_action = 0;

        gdk_drag_status (context, suggested_action, time);

        return;
    }

    date = g_date_new ();
    str = (gchar *) gtk_selection_data_get_text (selection_data);
    if (str) {
        g_date_set_parse (date, str);
        g_free (str);
    }

    if (!g_date_valid (date)) {
        g_warning ("Received invalid date data\n");
        g_date_free (date);
        gtk_drag_finish (context, FALSE, FALSE, time);
        return;
    }

    day = g_date_get_day (date);
    month = g_date_get_month (date);
    year = g_date_get_year (date);
    g_date_free (date);

    gtk_drag_finish (context, TRUE, FALSE, time);


    g_object_freeze_notify (G_OBJECT (calendar));
    if (!(calendar->display_flags & GUI_CALENDAR_NO_MONTH_CHANGE)
            && (calendar->display_flags & GUI_CALENDAR_SHOW_HEADING))
        gui_calendar_select_month (calendar, month - 1, year);
    gui_calendar_select_day (calendar, day);
    g_object_thaw_notify (G_OBJECT (calendar));
}

/*------------------------------------------------------------------------------*/
/* Public API */
/*------------------------------------------------------------------------------*/

GtkWidget*
gui_calendar_new (void) {
    return g_object_new (GUI_TYPE_CALENDAR, NULL);
}

/*------------------------------------------------------------------------------*/

GuiCalendarDisplayOptions
gui_calendar_get_display_options (GuiCalendar         *calendar) {
    g_return_val_if_fail (GUI_IS_CALENDAR (calendar), 0);

    return calendar->display_flags;
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_set_display_options (GuiCalendar          *calendar,
                                  GuiCalendarDisplayOptions flags) {
    GtkWidget *widget = GTK_WIDGET (calendar);
    GuiCalendarPrivate *priv = GUI_CALENDAR_GET_PRIVATE (calendar);
    gint resize = 0;
    gint i;
    GuiCalendarDisplayOptions old_flags;

    g_return_if_fail (GUI_IS_CALENDAR (calendar));

    old_flags = calendar->display_flags;

    if (GTK_WIDGET_REALIZED (widget)) {
        if ((flags ^ calendar->display_flags) & GUI_CALENDAR_NO_MONTH_CHANGE) {
            resize ++;
            if (! (flags & GUI_CALENDAR_NO_MONTH_CHANGE)
                    && (priv->header_win)) {
                calendar->display_flags &= ~GUI_CALENDAR_NO_MONTH_CHANGE;
                calendar_realize_arrows (calendar);
            } else {
                for (i = 0; i < 4; i++) {
                    if (priv->arrow_win[i]) {
                        gdk_window_set_user_data (priv->arrow_win[i],
                                                  NULL);
                        gdk_window_destroy (priv->arrow_win[i]);
                        priv->arrow_win[i] = NULL;
                    }
                }
            }
        }

        if ((flags ^ calendar->display_flags) & GUI_CALENDAR_SHOW_HEADING) {
            resize++;

            if (flags & GUI_CALENDAR_SHOW_HEADING) {
                calendar->display_flags |= GUI_CALENDAR_SHOW_HEADING;
                calendar_realize_header (calendar);
            } else {
                for (i = 0; i < 4; i++) {
                    if (priv->arrow_win[i]) {
                        gdk_window_set_user_data (priv->arrow_win[i],
                                                  NULL);
                        gdk_window_destroy (priv->arrow_win[i]);
                        priv->arrow_win[i] = NULL;
                    }
                }
                gdk_window_set_user_data (priv->header_win, NULL);
                gdk_window_destroy (priv->header_win);
                priv->header_win = NULL;
            }
        }


        if ((flags ^ calendar->display_flags) & GUI_CALENDAR_SHOW_DAY_NAMES) {
            resize++;

            if (flags & GUI_CALENDAR_SHOW_DAY_NAMES) {
                calendar->display_flags |= GUI_CALENDAR_SHOW_DAY_NAMES;
                calendar_realize_day_names (calendar);
            } else {
                gdk_window_set_user_data (priv->day_name_win, NULL);
                gdk_window_destroy (priv->day_name_win);
                priv->day_name_win = NULL;
            }
        }

        if ((flags ^ calendar->display_flags) & GUI_CALENDAR_SHOW_WEEK_NUMBERS) {
            resize++;

            if (flags & GUI_CALENDAR_SHOW_WEEK_NUMBERS) {
                calendar->display_flags |= GUI_CALENDAR_SHOW_WEEK_NUMBERS;
                calendar_realize_week_numbers (calendar);
            } else {
                gdk_window_set_user_data (priv->week_win, NULL);
                gdk_window_destroy (priv->week_win);
                priv->week_win = NULL;
            }
        }

        calendar->display_flags = flags;
        if (resize)
            gtk_widget_queue_resize (GTK_WIDGET (calendar));

    } else
        calendar->display_flags = flags;

    g_object_freeze_notify (G_OBJECT (calendar));

	if ((old_flags ^ calendar->display_flags) & GUI_CALENDAR_SHOW_HEADING)
        g_object_notify (G_OBJECT (calendar), "show-heading");

	if ((old_flags ^ calendar->display_flags) & GUI_CALENDAR_SHOW_DAY_NAMES)
        g_object_notify (G_OBJECT (calendar), "show-day-names");

	if ((old_flags ^ calendar->display_flags) & GUI_CALENDAR_NO_MONTH_CHANGE)
        g_object_notify (G_OBJECT (calendar), "no-month-change");

	if ((old_flags ^ calendar->display_flags) & GUI_CALENDAR_SHOW_WEEK_NUMBERS)
        g_object_notify (G_OBJECT (calendar), "show-week-numbers");

	if ((old_flags ^ calendar->display_flags) & GUI_CALENDAR_WEEK_START_MONDAY)
        g_object_notify (G_OBJECT (calendar), "week-start-monday");

	g_object_thaw_notify (G_OBJECT (calendar));
}

/*------------------------------------------------------------------------------*/

gboolean
gui_calendar_select_month (GuiCalendar *calendar,
                           guint    month,
                           guint    year) {
    g_return_val_if_fail (GUI_IS_CALENDAR (calendar), FALSE);
    g_return_val_if_fail (month <= 11, FALSE);

    calendar->month = month;
    calendar->year  = year;

    calendar_compute_days (calendar);

    gtk_widget_queue_draw (GTK_WIDGET (calendar));

    g_object_freeze_notify (G_OBJECT (calendar));
    g_object_notify (G_OBJECT (calendar), "month");
    g_object_notify (G_OBJECT (calendar), "year");
    g_object_thaw_notify (G_OBJECT (calendar));

    g_signal_emit (calendar,
                   gui_calendar_signals[MONTH_CHANGED_SIGNAL],
                   0);
    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_select_day (GuiCalendar *calendar,
                         guint        day) {
    g_return_if_fail (GUI_IS_CALENDAR (calendar));
    g_return_if_fail (day <= 31);

    /* deselect the old day */
    if (calendar->selected_day > 0) {
        gint selected_day;

        selected_day = calendar->selected_day;
        calendar->selected_day = 0;
        if (GTK_WIDGET_DRAWABLE (GTK_WIDGET (calendar)))
            calendar_invalidate_day_num (calendar, selected_day);
    }

    calendar->selected_day = day;

    /* select the new day */
    if (day != 0) {
        if (GTK_WIDGET_DRAWABLE (GTK_WIDGET (calendar)))
            calendar_invalidate_day_num (calendar, day);
    }

    g_object_notify (G_OBJECT (calendar), "day");

    g_signal_emit (calendar,
                   gui_calendar_signals[DAY_SELECTED_SIGNAL],
                   0);
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_clear_marks (GuiCalendar *calendar, gint mark_type) {

guint day;

	g_return_if_fail (GUI_IS_CALENDAR (calendar));

	switch (mark_type) {

		case DAY_NOTE_MARK:
			for (day = 0; day < 31; day++) {
				calendar->marked_date[day] = FALSE;
			}

			calendar->num_marked_dates = 0;
			break;

		case EVENT_MARK:
			for (day = 0; day < 31; day++) {
				calendar->event_marked_date[day] = FALSE;
			}

			calendar->event_num_marked_dates = 0;
			break;

		case BIRTHDAY_MARK:
			for (day = 0; day < 31; day++) {
				calendar->birthday_marked_date[day] = FALSE;
			}

			calendar->birthday_num_marked_dates = 0;
			break;
	}

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

/*------------------------------------------------------------------------------*/

gboolean
gui_calendar_mark_day (GuiCalendar *calendar, guint day, gint mark_type) {

	g_return_val_if_fail (GUI_IS_CALENDAR (calendar), FALSE);

	switch (mark_type) {

		case DAY_NOTE_MARK:
			if (day >= 1 && day <= 31 && calendar->marked_date[day-1] == FALSE) {
				calendar->marked_date[day - 1] = TRUE;
				GtkWidget *widget = GTK_WIDGET (calendar);
				calendar->marked_date_color[day - 1] = (widget)->style->base[GTK_WIDGET_STATE (widget)];
				calendar->num_marked_dates++;
				calendar_invalidate_day_num (calendar, day);
			}
			break;

		case EVENT_MARK:
			if (day >= 1 && day <= 31 && calendar->event_marked_date[day-1] == FALSE) {
				calendar->event_marked_date[day - 1] = TRUE;
				calendar->event_num_marked_dates++;
				calendar_invalidate_day_num (calendar, day);
			}
			break;

		case BIRTHDAY_MARK:
			if (day >= 1 && day <= 31 && calendar->birthday_marked_date[day-1] == FALSE) {
				calendar->birthday_marked_date[day - 1] = TRUE;
				calendar->birthday_num_marked_dates++;
				calendar_invalidate_day_num (calendar, day);
			}
			break;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------*/

gboolean
gui_calendar_unmark_day (GuiCalendar *calendar, guint day, gint mark_type) {

	g_return_val_if_fail (GUI_IS_CALENDAR (calendar), FALSE);

	switch (mark_type) {

		case DAY_NOTE_MARK:
			if (day >= 1 && day <= 31 && calendar->marked_date[day-1] == TRUE) {
				calendar->marked_date[day - 1] = FALSE;
				GtkWidget *widget = GTK_WIDGET (calendar);
				calendar->marked_date_color[day - 1] = (widget)->style->base[GTK_WIDGET_STATE (widget)];
				calendar->num_marked_dates--;
				calendar_invalidate_day_num (calendar, day);
			}
			break;

		case EVENT_MARK:
			if (day >= 1 && day <= 31 && calendar->event_marked_date[day-1] == TRUE) {
				calendar->event_marked_date[day - 1] = FALSE;
				calendar->event_num_marked_dates--;
				calendar_invalidate_day_num (calendar, day);
			}
			break;

		case BIRTHDAY_MARK:
			if (day >= 1 && day <= 31 && calendar->birthday_marked_date[day-1] == TRUE) {
				calendar->birthday_marked_date[day - 1] = FALSE;
				calendar->birthday_num_marked_dates--;
				calendar_invalidate_day_num (calendar, day);
			}
			break;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------*/

gboolean
gui_calendar_set_day_color (GuiCalendar *calendar,
                            guint        day,
                            gchar       *color_str) {

	g_return_val_if_fail (GUI_IS_CALENDAR (calendar), FALSE);

    if (day >= 1 && day <= 31 && calendar->marked_date[day-1] == FALSE) {
        calendar->marked_date[day - 1] = TRUE;
        GtkWidget *widget = GTK_WIDGET (calendar);
        calendar->marked_date_color[day - 1] = (widget)->style->base[GTK_WIDGET_STATE (widget)];
        if (color_str != NULL) {
            gdk_color_parse(color_str, &calendar->marked_date_color[day - 1]);
        } 
        calendar->num_marked_dates++;
        calendar_invalidate_day_num (calendar, day);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_get_date (GuiCalendar *calendar,
                       guint       *year,
                       guint       *month,
                       guint       *day) {
    g_return_if_fail (GUI_IS_CALENDAR (calendar));

    if (year)
        *year = calendar->year;

    if (month)
        *month = calendar->month;

    if (day)
        *day = calendar->selected_day;
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_get_gdate (GuiCalendar *calendar, GDate *cdate)
{
    g_return_if_fail (GUI_IS_CALENDAR (calendar));
    g_date_set_dmy (cdate, calendar->selected_day, calendar->month + 1, calendar->year);
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_set_color (GuiCalendar *calendar, const gchar *color, gint alpha_value, gint color_type) {

	g_return_if_fail (GUI_IS_CALENDAR (calendar));

	switch (color_type) {

		case HEADER_COLOR:
            gdk_color_parse(color, &calendar->header_color);
			break;
		case WEEKEND_COLOR:
            gdk_color_parse(color, &calendar->weekend_color);
			break;
		case SELECTOR_COLOR:
            gdk_color_parse(color, &calendar->selector_color);
            calendar->selector_alpha = alpha_value;
			break;
		case EVENT_MARKER_COLOR:
            gdk_color_parse(color, &calendar->event_marker_color);
			break;
		case TODAY_MARKER_COLOR:
            gdk_color_parse(color, &calendar->today_marker_color);
            calendar->today_marker_alpha = alpha_value;
			break;		
		case BIRTHDAY_MARKER_COLOR:
            gdk_color_parse(color, &calendar->birthday_marker_color);
			break;		
	}
}

/*------------------------------------------------------------------------------*/

void       
gui_calendar_set_marker (GuiCalendar *calendar, gint marker_type, gint marker) {

	g_return_if_fail (GUI_IS_CALENDAR (calendar));

	switch (marker) {

		case TODAY_MARKER:
            calendar->today_marker_type = marker_type;
			break;
		case EVENT_MARKER:
            calendar->event_marker_type = marker_type;
			break;
	}
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_set_day_note_marker_symbol (GuiCalendar *calendar, gchar *symbol) {
    g_return_if_fail (GUI_IS_CALENDAR (calendar));
    calendar->mark_sign = g_utf8_get_char (symbol);
}

/*------------------------------------------------------------------------------*/

void       
gui_calendar_enable_cursor (GuiCalendar *calendar, gboolean state) {
    g_return_if_fail (GUI_IS_CALENDAR (calendar));
    calendar->enable_cursor = state;
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_set_cursor_type (GuiCalendar *calendar, gint cursor_type) {
    g_return_if_fail (GUI_IS_CALENDAR (calendar));
    calendar->cursor_type = cursor_type;
}

/*------------------------------------------------------------------------------*/

void
gui_calendar_set_frame_cursor_thickness (GuiCalendar *calendar, gint thickness) {
    g_return_if_fail (GUI_IS_CALENDAR (calendar));
    g_return_if_fail (thickness > 0 && thickness < 6);
    calendar->frame_cursor_thickness = thickness;
}

/*------------------------------------------------------------------------------*/

