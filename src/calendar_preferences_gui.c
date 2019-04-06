/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Maka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Maka <silloz@users.sourceforge.net>
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

#include "calendar.h"
#include "calendar_ical.h"
#include "calendar_notes.h"
#include "calendar_preferences_gui.h"
#include "calendar_widget.h"
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "utils.h"
#include "utils_gui.h"

#ifdef HAVE_GTKSPELL
#include <gtkspell/gtkspell.h>
#endif /* HAVE_GTKSPELL */

static void calendar_options_cb (GtkToggleButton *togglebutton, GUI *appGUI);

/* ========================================================================== */

static void
calendar_cursor_settings_enable_disable (GUI *appGUI)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->enable_block_cursor_checkbutton))) {
		gtk_widget_hide (appGUI->opt->cft_hscale);
		gtk_widget_hide (appGUI->opt->cft_label_1);
		gtk_widget_hide (appGUI->opt->cft_label_2);
		gtk_widget_hide (appGUI->opt->cft_label_3);
	} else {
		gtk_widget_show (appGUI->opt->cft_hscale);
		gtk_widget_show (appGUI->opt->cft_label_1);
		gtk_widget_show (appGUI->opt->cft_label_2);
		gtk_widget_show (appGUI->opt->cft_label_3);
	}
}

/* ========================================================================== */

static void
day_marker_entry_changed_cb (GtkEntry *entry, GUI *appGUI)
{
	g_strlcpy (config.day_note_marker, gtk_entry_get_text (entry), MAXNAME);
	gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->calendar), config.day_note_marker);
	gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->calendar_prev), config.day_note_marker);
	gui_calendar_set_day_note_marker_symbol (GUI_CALENDAR (appGUI->cal->calendar_next), config.day_note_marker);
	utl_gui_change_bg_widget_state (appGUI->opt->day_marker_entry, NULL, appGUI);
}

/* ========================================================================== */

static gint
day_marker_key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	utl_gui_change_bg_widget_state (appGUI->opt->day_marker_entry, COLOR_BG_OK, appGUI);

	return FALSE;
}

/* ========================================================================== */

static void
event_marker_type_changed_cb (GtkComboBox *combobox, GUI *appGUI)
{
	config.event_marker_type = gtk_combo_box_get_active (combobox);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar), config.event_marker_type, EVENT_MARKER);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_prev), config.event_marker_type, EVENT_MARKER);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_next), config.event_marker_type, EVENT_MARKER);
}

/* ========================================================================== */

static void
today_marker_type_changed_cb (GtkComboBox *combobox, GUI *appGUI)
{
	config.today_marker_type = gtk_combo_box_get_active (combobox);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar), config.today_marker_type, EVENT_MARKER);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_prev), config.today_marker_type, EVENT_MARKER);
	gui_calendar_set_marker (GUI_CALENDAR (appGUI->cal->calendar_next), config.today_marker_type, EVENT_MARKER);
}

/* ========================================================================== */

static void
header_color_changed_cb (GtkColorButton *button, GUI *appGUI)
{
	GdkColor color;
	gchar str[MAXCOLORNAME];

	gtk_color_button_get_color (button, &color);
	g_sprintf (str, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_strlcpy (config.header_color, str, MAXCOLORNAME);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.header_color, 0, HEADER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.header_color, 0, HEADER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.header_color, 0, HEADER_COLOR);
}

/* ========================================================================== */

static void
weekend_color_changed_cb (GtkColorButton *button, GUI *appGUI)
{
	GdkColor color;
	gchar str[MAXCOLORNAME];

	gtk_color_button_get_color (button, &color);
	g_sprintf (str, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_strlcpy (config.weekend_color, str, MAXCOLORNAME);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.weekend_color, 0, WEEKEND_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.weekend_color, 0, WEEKEND_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.weekend_color, 0, WEEKEND_COLOR);
}

/* ========================================================================== */

static void
selection_color_changed_cb (GtkColorButton *button, GUI *appGUI)
{
	GdkColor color;
	gchar str[MAXCOLORNAME];

	gtk_color_button_get_color (button, &color);
	g_sprintf (str, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_strlcpy (config.selection_color, str, MAXCOLORNAME);
	config.selector_alpha = gtk_color_button_get_alpha (button);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.selection_color, config.selector_alpha, SELECTOR_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.selection_color, 0, SELECTOR_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.selection_color, 0, SELECTOR_COLOR);
}

/* ========================================================================== */

static void
mark_color_changed_cb (GtkColorButton *button, GUI *appGUI)
{
	GdkColor color;
	gchar str[MAXCOLORNAME];

	gtk_color_button_get_color (button, &color);
	g_sprintf (str, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_strlcpy (config.mark_color, str, MAXCOLORNAME);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.mark_color, 0, EVENT_MARKER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.mark_color, 0, EVENT_MARKER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.mark_color, 0, EVENT_MARKER_COLOR);
}

/* ========================================================================== */

static void
mark_current_day_color_changed_cb (GtkColorButton *button, GUI *appGUI)
{
	GdkColor color;
	gchar str[MAXCOLORNAME];

	gtk_color_button_get_color (button, &color);
	g_sprintf (str, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_strlcpy (config.mark_current_day_color, str, MAXCOLORNAME);
	config.mark_current_day_alpha = gtk_color_button_get_alpha (button);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.mark_current_day_color,
	                        config.mark_current_day_alpha, TODAY_MARKER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.mark_current_day_color,
	                        config.mark_current_day_alpha, TODAY_MARKER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.mark_current_day_color,
	                        config.mark_current_day_alpha, TODAY_MARKER_COLOR);
}

/* ========================================================================== */

static void
birthday_mark_color_changed_cb (GtkColorButton *button, GUI *appGUI)
{
	GdkColor color;
	gchar str[MAXCOLORNAME];

	gtk_color_button_get_color (button, &color);
	g_sprintf (str, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_strlcpy (config.birthday_mark_color, str, MAXCOLORNAME);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_prev), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
	gui_calendar_set_color (GUI_CALENDAR (appGUI->cal->calendar_next), config.birthday_mark_color, 0, BIRTHDAY_MARKER_COLOR);
}

/* ========================================================================== */

static void
cursor_thickness_changed_cb (GtkRange *range, GUI *appGUI)
{
    config.frame_cursor_thickness = (gint) gtk_range_get_value (range);
    gui_calendar_set_frame_cursor_thickness (GUI_CALENDAR (appGUI->cal->calendar), config.frame_cursor_thickness);
}

/* ========================================================================== */

static gboolean
date_header_format_entry_key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
    utl_gui_change_bg_widget_state (widget, COLOR_BG_OK, appGUI);
    return FALSE;
}

/* ========================================================================== */

static void
update_date_header (GUI *appGUI) 
{

	if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->date_header_format_entry))))
		g_strlcpy (config.date_header_format, 
				   gtk_entry_get_text (GTK_ENTRY (appGUI->opt->date_header_format_entry)), MAXNAME);

	gtk_label_set_text (GTK_LABEL (appGUI->cal->date_label), 
						utl_get_date_name_format (appGUI->cal->date, config.date_header_format));
}

/* ========================================================================== */

static void
date_header_format_entry_changed_cb (GtkEntry *entry, GUI *appGUI)
{
	utl_gui_change_bg_widget_state (appGUI->opt->date_header_format_entry, NULL, appGUI);
	update_date_header (appGUI);
}

/* ========================================================================== */

static void
dh_default_cb (GtkWidget *widget, GUI *appGUI)
{
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->date_header_format_entry), DEFAULT_DATE_HEADER_FORMAT);
	update_date_header (appGUI);
}

/* ========================================================================== */

static void
dh_help_cb (GtkWidget *widget, GUI *appGUI)
{
gchar tmpbuf[BUFFER_SIZE];

	g_snprintf (tmpbuf, BUFFER_SIZE, 
				"<span size='medium'><b>%s</b></span>:\n\n"
				"<i><tt>%%a</tt></i> : %s\n" "<i><tt>%%A</tt></i> : %s\n"
				"<i><tt>%%b</tt></i> : %s\n" "<i><tt>%%B</tt></i> : %s\n"
				"<i><tt>%%d</tt></i> : %s\n" "<i><tt>%%D</tt></i> : %s\n"
				"<i><tt>%%e</tt></i> : %s\n" "<i><tt>%%m</tt></i> : %s\n"
				"<i><tt>%%y</tt></i> : %s\n" "<i><tt>%%Y</tt></i> : %s\n",
				_("Syntax"), 
				_("abbreviated weekday name"), _("full weekday name"),
				_("abbreviated month name"), _("full month name"),
				_("day of the month"), _("MM/DD/YY"),
				_("day of the month without leading zeros"), _("month"),
				_("year without century"), _("year with century")
				);

	utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW (appGUI->opt->window));
}

/* ========================================================================== */

static void
create_appearance_section (GtkWidget *appearance_vbox, GUI *appGUI)
{
	GtkWidget *table, *label, *entry, *combobox, *color_button, *button;
	GtkWidget *entry_hbox, *colors_hbox, *b_hbox;
	GdkColor color;
	GtkObject *adj;
	gint i;

	static FONT_SEL sel1, sel2, sel3;

	sel1.appGUI = sel2.appGUI = sel3.appGUI = appGUI;
	sel1.save = sel2.save = sel3.save = TRUE;

	table = gtk_table_new (11, 7, FALSE);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	i = 0;

	label = utl_gui_create_label ("%s:", _("Date header format"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	appGUI->opt->date_header_format_entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->date_header_format_entry), config.date_header_format);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->date_header_format_entry, 1, 5, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->date_header_format_entry), "key_press_event", 
					  G_CALLBACK (date_header_format_entry_key_press_cb), appGUI);
	g_signal_connect (G_OBJECT (appGUI->opt->date_header_format_entry), "activate", 
					  G_CALLBACK (date_header_format_entry_changed_cb), appGUI);

	b_hbox = gtk_hbox_new (FALSE, 4);
	gtk_table_attach (GTK_TABLE (table), b_hbox, 5, 7, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_CLEAR, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_CLEAR, FALSE);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (button, _("Set default format"));
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (dh_default_cb), appGUI);
	gtk_box_pack_start (GTK_BOX (b_hbox), button, FALSE, FALSE, 0);

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_HELP, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_HELP, FALSE);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (button, _("Date format syntax"));
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (dh_help_cb), appGUI);
	gtk_box_pack_start (GTK_BOX (b_hbox), button, FALSE, FALSE, 0);

	i++;

	label = utl_gui_create_label ("%s:", _("Day note marker"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_set_size_request (label, 120, -1);

	entry_hbox = gtk_hbox_new (FALSE, 8);
	gtk_table_attach (GTK_TABLE (table), entry_hbox, 1, 3, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), 1);
	gtk_entry_set_text (GTK_ENTRY (entry), config.day_note_marker);
	gtk_widget_set_size_request (entry, 32, -1);
	gtk_box_pack_start (GTK_BOX (entry_hbox), entry, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (day_marker_entry_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (day_marker_key_press_cb), appGUI);
	appGUI->opt->day_marker_entry = entry;

	i++;

	label = utl_gui_create_label ("%s:", _("Event marker"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Circle"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Ellipse"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Wave"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.event_marker_type);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 3, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (event_marker_type_changed_cb), appGUI);
	appGUI->opt->event_marker_type_combobox = combobox;

	i++;

	label = utl_gui_create_label ("%s:", _("Current day marker"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Arrow"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Free-hand circle"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.today_marker_type);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 3, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (today_marker_type_changed_cb), appGUI);
	appGUI->opt->today_marker_type_combobox = combobox;

	i++;

	label = utl_gui_create_label ("%s:", _("Colors"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	colors_hbox = gtk_hbox_new (FALSE, 8);
	gtk_table_attach (GTK_TABLE (table), colors_hbox, 1, 5, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Header color"));
	gdk_color_parse (config.header_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (header_color_changed_cb), appGUI);
	appGUI->opt->header_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Weekend days color"));
	gdk_color_parse (config.weekend_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (weekend_color_changed_cb), appGUI);
	appGUI->opt->weekend_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Cursor color"));
	gdk_color_parse (config.selection_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON (color_button), TRUE);
	gtk_color_button_set_alpha (GTK_COLOR_BUTTON (color_button), config.selector_alpha);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (selection_color_changed_cb), appGUI);
	appGUI->opt->selection_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Event marker color"));
	gdk_color_parse (config.mark_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (mark_color_changed_cb), appGUI);
	appGUI->opt->mark_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Current day marker color"));
	gdk_color_parse (config.mark_current_day_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON (color_button), TRUE);
	gtk_color_button_set_alpha (GTK_COLOR_BUTTON (color_button), config.mark_current_day_alpha);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (mark_current_day_color_changed_cb), appGUI);
	appGUI->opt->mark_current_day_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Birthday marker color"));
	gdk_color_parse (config.birthday_mark_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (birthday_mark_color_changed_cb), appGUI);
	appGUI->opt->birthday_mark_color_picker = color_button;

	i++;
	label = utl_gui_create_label ("%s:", _("Date font"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	appGUI->opt->day_name_font_entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->day_name_font_entry), config.day_name_font);
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->day_name_font_entry, GTK_CAN_FOCUS);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->day_name_font_entry, 1, 6, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	sel1.config = config.day_name_font;
	sel1.entry = appGUI->opt->day_name_font_entry;
	sel1.font = &appGUI->cal->fd_day_name_font;
	sel1.widget = appGUI->cal->date_label;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 6, 7, i, i+1,
	                  (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (utl_gui_font_select_cb), &sel1);

	i++;
	label = utl_gui_create_label ("%s:", _("Calendar font"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	appGUI->opt->calendar_font_entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->calendar_font_entry), config.calendar_font);
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->calendar_font_entry, GTK_CAN_FOCUS);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->calendar_font_entry, 1, 6, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	sel2.config = config.calendar_font;
	sel2.entry = appGUI->opt->calendar_font_entry;
	sel2.font = &appGUI->cal->fd_cal_font;
	sel2.widget = appGUI->cal->calendar;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 6, 7, i, i+1,
	                  (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (utl_gui_font_select_cb), &sel2);

	i++;
	label = utl_gui_create_label ("%s:", _("Note font"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	appGUI->opt->notes_font_entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->notes_font_entry), config.notes_font);
	GTK_WIDGET_UNSET_FLAGS(appGUI->opt->notes_font_entry, GTK_CAN_FOCUS);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->notes_font_entry, 1, 6, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	sel3.config = config.notes_font;
	sel3.entry = appGUI->opt->notes_font_entry;
	sel3.font = &appGUI->cal->fd_notes_font;
	sel3.widget = appGUI->cal->calendar_note_textview;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (utl_gui_font_select_cb), &sel3);
	gtk_table_attach (GTK_TABLE (table), button, 6, 7, i, i+1,
	                  (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);

	i++;
	appGUI->opt->enable_block_cursor_checkbutton = gtk_check_button_new_with_mnemonic (_("Enable block cursor"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->opt->enable_block_cursor_checkbutton), !config.cursor_type);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->enable_block_cursor_checkbutton, 0, 6, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->enable_block_cursor_checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);

	i++;
	label = utl_gui_create_label ("%s:", _("Cursor thickness"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	appGUI->opt->cft_label_1 = label;

	label = utl_gui_create_label ("<u>%s</u>", _("Thin"));
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_table_attach (GTK_TABLE (table), label, 1, 5, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1.0);
	appGUI->opt->cft_label_2 = label;

	label = utl_gui_create_label ("<u>%s</u>", _("Thick"));
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_table_attach (GTK_TABLE (table), label, 1, 5, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
	appGUI->opt->cft_label_3 = label;

	i++;

	adj = gtk_adjustment_new (1, 1, 6, 1, 1, 1);
	appGUI->opt->cft_hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_scale_set_draw_value (GTK_SCALE (appGUI->opt->cft_hscale), FALSE);
	gtk_range_set_update_policy (GTK_RANGE (appGUI->opt->cft_hscale), GTK_UPDATE_DISCONTINUOUS);
	gtk_range_set_value (GTK_RANGE (appGUI->opt->cft_hscale), config.frame_cursor_thickness);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->cft_hscale, 1, 5, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->cft_hscale), "value-changed", G_CALLBACK (cursor_thickness_changed_cb), appGUI);
}

/* ========================================================================== */

static void
close_window (GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_destroy (window);
}

/* ========================================================================== */

static gint
color_edit_name_key_release_cb (GtkEntry *entry, GdkEventKey *event, GUI *appGUI)
{
	if (strlen (gtk_entry_get_text (entry)))
		gtk_widget_set_sensitive (appGUI->opt->color_edit_ok_button, TRUE);
	else
		gtk_widget_set_sensitive (appGUI->opt->color_edit_ok_button, FALSE);

	return FALSE;
}

/* ========================================================================== */

static void
color_edit_action_cb (GtkWidget *widget, GUI *appGUI)
{
	gchar *old_color, *new_color;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeIter p_iter;
	GtkTreePath *path;
	GdkColor color;
	GdkPixbuf *image;

	old_color = new_color = NULL;

	gtk_color_button_get_color (GTK_COLOR_BUTTON (appGUI->opt->color_edit_picker), &color);
	new_color = g_strdup_printf ("#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);

	gtk_tree_selection_get_selected (appGUI->opt->calendar_category_select, &model, &iter);
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &old_color, -1);

	if (old_color != NULL && new_color != NULL) {

		gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->calendar_category_treeview), &path, NULL);

		if (path != NULL) {
			gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &p_iter, path);
			gtk_list_store_remove (appGUI->opt->calendar_category_store, &p_iter);
			gtk_tree_path_free (path);
		}

		image = utl_gui_create_color_swatch (new_color);
		gtk_list_store_append (appGUI->opt->calendar_category_store, &iter);
		gtk_list_store_set (appGUI->opt->calendar_category_store, &iter, 0, image, 1, new_color,
		                    2, gtk_entry_get_text (GTK_ENTRY (appGUI->opt->color_edit_name_entry)), -1);
		g_object_unref (image);

		cal_replace_note_color (old_color, new_color, appGUI);
		cal_refresh_marks (appGUI);
		update_aux_calendars (appGUI);
	}

	g_free (old_color);
	g_free (new_color);

	close_window (NULL, appGUI->opt->color_edit_window);
}

/* ========================================================================== */

static gint
color_edit_key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	switch (event->keyval) {

		case GDK_Return:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->color_edit_name_entry))))
				color_edit_action_cb (NULL, appGUI);
			return TRUE;

		case GDK_Escape:
			close_window (NULL, appGUI->opt->color_edit_window);
			return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

static void
gui_color_edit_action (GtkTreeIter *iter, GtkTreeModel *model, GUI *appGUI)
{
	GtkWidget *vbox_top, *vbox, *hbox;
	GtkWidget *hseparator, *hbuttonbox;
	GtkWidget *cancel_button;
	GdkColor color;
	gchar *color_val, *color_name;

	GtkWidget *window;

	window = utl_gui_create_window (_("Edit category"), 350, -1, appGUI);
	g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (color_edit_key_press_cb), appGUI);
	appGUI->opt->color_edit_window = window;

	vbox_top = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox_top);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_top), hbox, TRUE, FALSE, 0);

	vbox = utl_gui_create_vbox_in_frame (hbox, _("Color"));

	appGUI->opt->color_edit_picker = gtk_color_button_new ();
	gtk_box_pack_start (GTK_BOX (vbox), appGUI->opt->color_edit_picker, FALSE, FALSE, 0);

	vbox = utl_gui_create_vbox_in_frame (hbox, _("Name"));

	appGUI->opt->color_edit_name_entry = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (vbox), appGUI->opt->color_edit_name_entry);
	g_signal_connect (G_OBJECT (appGUI->opt->color_edit_name_entry), "key_release_event",
	                  G_CALLBACK (color_edit_name_key_release_cb), appGUI);

	hseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), hseparator, FALSE, TRUE, 4);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), hbuttonbox, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
	g_signal_connect (G_OBJECT (cancel_button), "clicked", G_CALLBACK (close_window), appGUI->opt->color_edit_window);

	appGUI->opt->color_edit_ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->opt->color_edit_ok_button);
	g_signal_connect (G_OBJECT (appGUI->opt->color_edit_ok_button), "clicked", G_CALLBACK (color_edit_action_cb), appGUI);

	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 1, &color_val, 2, &color_name, -1);
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->color_edit_name_entry), color_name);
	gdk_color_parse (color_val, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (appGUI->opt->color_edit_picker), &color);
	g_free (color_val);
	g_free (color_name);

	gtk_widget_show_all (appGUI->opt->color_edit_window);
	gtk_widget_grab_focus (appGUI->opt->color_edit_name_entry);
}

/* ========================================================================== */

static void
calendar_category_add_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreeIter iter;
	GdkPixbuf *image;
	GdkColor color;
	const gchar *category_name;
	gchar category_color[MAXCOLORNAME];
	gchar *item_color, *item_name;
	gint i;

	category_name = gtk_entry_get_text (GTK_ENTRY (appGUI->opt->calendar_category_entry));
	if (!strlen (category_name)) return;

	gtk_color_button_get_color (GTK_COLOR_BUTTON (appGUI->opt->day_category_color_picker), &color);
	g_sprintf (category_color, "#%02X%02X%02X", color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);

	i = 0;
	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, 1, &item_color, 2, &item_name, -1);
		if (!strcmp (category_color, item_color)) {
			if (!strcmp (category_name, item_name)) {
				g_free (item_name);
				g_free (item_color);
				return;
			}
		}
		g_free (item_color);
		g_free (item_name);
	}

	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, NULL, 0);

	image = utl_gui_create_color_swatch (category_color);

	gtk_list_store_append (appGUI->opt->calendar_category_store, &iter);
	gtk_list_store_set (appGUI->opt->calendar_category_store, &iter, 0, image, 1, category_color, 2, category_name, -1);
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->calendar_category_entry), "");
	gtk_widget_set_sensitive (appGUI->opt->calendar_category_add_button, FALSE);
	g_object_unref (image);
}

/* ========================================================================== */

static void
calendar_category_edit_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean state;

	state = gtk_tree_selection_get_selected (appGUI->opt->calendar_category_select, &model, &iter);
	gtk_widget_set_sensitive (appGUI->opt->calendar_category_edit_button, state);
	gtk_widget_set_sensitive (appGUI->opt->calendar_category_remove_button, state);

	if (state)
        gui_color_edit_action (&iter, model, appGUI);
}

/* ========================================================================== */

static gint
calendar_category_entry_key_release_cb (GtkEntry *entry, GdkEventKey *event, GUI *appGUI)
{
	gboolean state = FALSE;

	if (strlen (gtk_entry_get_text (entry)))
		state = TRUE;

	gtk_widget_set_sensitive (appGUI->opt->calendar_category_add_button, state);

	if (event->keyval == GDK_Return) {
		if (state)
			calendar_category_add_cb (NULL, appGUI);
		return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

static gint
color_edit_list_dbclick_cb (GtkWidget *widget, GdkEventButton *event, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if ((event->type == GDK_2BUTTON_PRESS) && (event->button == 1)) {
		if (gtk_tree_selection_get_selected (appGUI->opt->calendar_category_select, &model, &iter)) {
			gui_color_edit_action (&iter, model, appGUI);
			return TRUE;
		}
	}

	return FALSE;
}

/* ========================================================================== */

static void
calendar_category_selected_cb (GtkTreeSelection *selection, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean state;

	state = gtk_tree_selection_get_selected (selection, &model, &iter);
	gtk_widget_set_sensitive (appGUI->opt->calendar_category_edit_button, state);
	gtk_widget_set_sensitive (appGUI->opt->calendar_category_remove_button, state);
}

/* ========================================================================== */

static void
calendar_category_remove_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->calendar_category_treeview), &path, NULL);
	if (path == NULL) return;

	gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, path);
	gtk_list_store_remove (appGUI->opt->calendar_category_store, &iter);
	gtk_tree_path_free (path);
}

/* ========================================================================== */

static void
create_day_categories_section (GtkWidget *day_categories_vbox, GUI *appGUI)
{
	GtkWidget *table, *scrolledwindow, *treeview, *button;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GdkColor color;

	table = gtk_table_new (4, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (day_categories_vbox), table, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	appGUI->opt->day_category_color_picker = gtk_color_button_new ();
	gdk_color_parse ("#a1aaaf", &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (appGUI->opt->day_category_color_picker), &color);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->day_category_color_picker, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	appGUI->opt->calendar_category_entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->calendar_category_entry, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->calendar_category_entry), "key_release_event",
	                  G_CALLBACK (calendar_category_entry_key_release_cb), appGUI);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 5, 0, 3,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (appGUI->opt->calendar_category_store));
	appGUI->opt->calendar_category_treeview = treeview;
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	gtk_container_set_border_width (GTK_CONTAINER (treeview), 4);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
	gtk_widget_set_size_request (treeview, -1, 80);
	g_signal_connect (G_OBJECT (treeview), "button_press_event", G_CALLBACK (color_edit_list_dbclick_cb), appGUI);

	appGUI->opt->calendar_category_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (appGUI->opt->calendar_category_select), "changed",
	                  G_CALLBACK (calendar_category_selected_cb), appGUI);

	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "pixbuf", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 1, NULL);
	gtk_tree_view_column_set_visible (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_ADD, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_ADD, FALSE);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_category_add_cb), appGUI);
	appGUI->opt->calendar_category_add_button = button;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_EDIT, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_EDIT, FALSE);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_category_edit_cb), appGUI);
	appGUI->opt->calendar_category_edit_button = button;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_REMOVE, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_REMOVE, FALSE);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_category_remove_cb), appGUI);
	appGUI->opt->calendar_category_remove_button = button;
}

/* ========================================================================== */

#ifdef HAVE_LIBICAL

static void
ical_description_toggled (GtkCellRendererToggle *cell, gchar *path_str, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter  iter;
	gboolean desc_status;

	model = GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store);
	path = gtk_tree_path_new_from_string (path_str);
	if (path == NULL) return;

	gtk_tree_model_get_iter (model, &iter, path);   /* get toggled iter */
	gtk_tree_model_get (model, &iter, ICAL_COLUMN_ENABLE_DESC, &desc_status, -1);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, ICAL_COLUMN_ENABLE_DESC, !desc_status, -1);
	ics_calendar_refresh (appGUI);

	gtk_tree_path_free (path);
}

/* ========================================================================== */

static void
ical_use_year_toggled (GtkCellRendererToggle *cell, gchar *path_str, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter  iter;
	gboolean fulldate_status;
	GtkTreeModel *model;

	model = GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store);
	path = gtk_tree_path_new_from_string (path_str);
	if (path == NULL) return;

	gtk_tree_model_get_iter (model, &iter, path);   /* get toggled iter */
	gtk_tree_model_get (model, &iter, ICAL_COLUMN_FULL_DATE, &fulldate_status, -1);
	fulldate_status = !fulldate_status;
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, ICAL_COLUMN_FULL_DATE, fulldate_status, -1);
	if (fulldate_status == FALSE)
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, ICAL_COLUMN_MARK, FALSE, -1);
	ics_calendar_refresh (appGUI);
	cal_refresh_marks (appGUI);
	gtk_tree_path_free (path);
}

/* ========================================================================== */

static void
ical_mark_toggled (GtkCellRendererToggle *cell, gchar *path_str, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter  iter;
	gboolean mark_status;
	GtkTreeModel *model;

	model = GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store);
	path = gtk_tree_path_new_from_string (path_str);
	if (path == NULL) return;

	gtk_tree_model_get_iter (model, &iter, path);   /* get toggled iter */
	gtk_tree_model_get (model, &iter, ICAL_COLUMN_MARK, &mark_status, -1);
	mark_status = !mark_status;
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, ICAL_COLUMN_MARK, mark_status, -1);
	if (mark_status == TRUE)
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, ICAL_COLUMN_FULL_DATE, TRUE, -1);
	ics_calendar_refresh (appGUI);
	cal_refresh_marks (appGUI);

	gtk_tree_path_free (path);
}

/* ========================================================================== */

static GtkWidget *
ical_file_browser (GUI *appGUI)
{
	GtkWidget *dialog;
	GtkFileFilter *filter_1, *filter_2;

	dialog = gtk_file_chooser_dialog_new (_("Select ICS file"),
	                                      GTK_WINDOW(appGUI->main_window),
	                                      GTK_FILE_CHOOSER_ACTION_OPEN,
	                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                      GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	filter_1 = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_1, "*");
	gtk_file_filter_set_name (GTK_FILE_FILTER (filter_1), _("All Files"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_1);

	filter_2 = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_2, "*.[iI][cC][sS]");
	gtk_file_filter_set_name (GTK_FILE_FILTER (filter_2), _("Calendar files (*.ics)"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter_2);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter_2);

	return dialog;
}

/* ========================================================================== */

static void
calendar_ical_files_add_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreeIter iter;
	const gchar *ical_name, *ical_filename;
	gchar *item_filename;
	gint i = 0;

	ical_name = gtk_entry_get_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_name_entry));
	ical_filename = gtk_entry_get_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_filename_entry));

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &iter, ICAL_COLUMN_FILENAME, &item_filename, -1);
		if (!strcmp (ical_filename, item_filename)) {
			g_free (item_filename);
			return;
		}
		g_free (item_filename);
	}

	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->calendar_category_store), &iter, NULL, 0);

	gtk_list_store_append (appGUI->opt->calendar_ical_files_store, &iter);
	gtk_list_store_set (appGUI->opt->calendar_ical_files_store, &iter,
	                    ICAL_COLUMN_NAME, ical_name,
	                    ICAL_COLUMN_FILENAME, ical_filename,
	                    ICAL_COLUMN_FULL_DATE, TRUE,
	                    ICAL_COLUMN_MARK, FALSE,
	                    ICAL_COLUMN_ENABLE_DESC, TRUE, -1);

	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_name_entry), "");
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_filename_entry), "");

	gtk_widget_set_sensitive (appGUI->opt->calendar_ical_files_add_button, FALSE);
	ics_check_if_valid (appGUI);
	ics_calendar_refresh (appGUI);
}

/* ========================================================================== */

static gint
calendar_ical_files_entry_key_release_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	gboolean state = FALSE;

	if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_name_entry))) &&
	    strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_filename_entry))))
	    state = TRUE;

	gtk_widget_set_sensitive (appGUI->opt->calendar_ical_files_add_button, state);

	if (event != NULL) {
		if (event->keyval == GDK_Return && state) {
			calendar_ical_files_add_cb (NULL, appGUI);
			return TRUE;
		}
	}

	return FALSE;
}

/* ========================================================================== */

static void
calendar_ical_files_browse_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkWidget *dialog;

	dialog = ical_file_browser (appGUI);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gtk_widget_hide (dialog);
		while (g_main_context_iteration (NULL, FALSE));

		gtk_entry_set_text (GTK_ENTRY (appGUI->opt->calendar_ical_files_filename_entry),
		                    gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));

		calendar_ical_files_entry_key_release_cb (NULL, NULL, appGUI);
	}

	gtk_widget_destroy (dialog);
}

/* ========================================================================== */

static void
ical_edit_filename_browse_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkWidget *dialog;

	dialog = ical_file_browser (appGUI);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gtk_widget_hide (dialog);
		while (g_main_context_iteration (NULL, FALSE));

		gtk_entry_set_text (GTK_ENTRY (appGUI->opt->ical_edit_filename_entry),
		                    gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));
	}

	gtk_widget_destroy (dialog);
}

/* ========================================================================== */

static void
ical_edit_action_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter, p_iter;
	gboolean desc_flag, mark_flag, year_flag;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->calendar_ical_files_treeview), &path, NULL);

	if (path != NULL) {
		gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &p_iter, path);
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store), &p_iter,
		                    ICAL_COLUMN_ENABLE_DESC, &desc_flag, 
                            ICAL_COLUMN_MARK, &mark_flag, 
							ICAL_COLUMN_FULL_DATE, &year_flag, -1);
		gtk_list_store_remove (appGUI->opt->calendar_ical_files_store, &p_iter);
		gtk_tree_path_free (path);
	}

	gtk_list_store_append (appGUI->opt->calendar_ical_files_store, &iter);
	gtk_list_store_set (appGUI->opt->calendar_ical_files_store, &iter,
	                    ICAL_COLUMN_NAME, gtk_entry_get_text (GTK_ENTRY (appGUI->opt->ical_edit_name_entry)),
	                    ICAL_COLUMN_FILENAME, gtk_entry_get_text (GTK_ENTRY (appGUI->opt->ical_edit_filename_entry)),
						ICAL_COLUMN_DISABLED, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ical_edit_disable_checkbutton)),
						ICAL_COLUMN_STATE, !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ical_edit_disable_checkbutton)),
	                    ICAL_COLUMN_ENABLE_DESC, desc_flag, 
                        ICAL_COLUMN_MARK, mark_flag, 
						ICAL_COLUMN_FULL_DATE, year_flag, -1);

	close_window (NULL, appGUI->opt->ical_edit_window);
	ics_check_if_valid (appGUI);
	ics_calendar_refresh (appGUI);
}

/* ========================================================================== */

static gint
ical_edit_key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	switch (event->keyval) {

		case GDK_Return:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->ical_edit_name_entry))) &&
			    strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->ical_edit_filename_entry))))
				ical_edit_action_cb (NULL, appGUI);
			return TRUE;

		case GDK_Escape:
			close_window (NULL, appGUI->opt->ical_edit_window);
			return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

static gint
ical_edit_name_key_release_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->ical_edit_name_entry))))
		gtk_widget_set_sensitive(appGUI->opt->ical_edit_ok_button, TRUE);
	else
		gtk_widget_set_sensitive(appGUI->opt->ical_edit_ok_button, FALSE);

	return FALSE;
}

/* ========================================================================== */

static void
disable_checkbutton_clicked_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	gboolean state = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (togglebutton));

	gtk_widget_set_sensitive (appGUI->opt->ical_edit_name_entry, state);
	gtk_widget_set_sensitive (appGUI->opt->ical_edit_filename_browse_button, state);
	gtk_widget_set_sensitive (appGUI->opt->ical_edit_filename_entry, state);
}

/* ========================================================================== */

static void
gui_ical_entry_edit_action (GtkTreeIter *iter, GtkTreeModel *model, GUI *appGUI)
{
	GtkWidget *window, *vbox_top, *hbox;
	GtkWidget *hseparator, *hbuttonbox;
	GtkWidget *cancel_button;
	gchar *ical_name, *ical_filename;
	gboolean ical_disabled;

	window = utl_gui_create_window (_("Modify ICAL parameters"), 450, -1, appGUI);
	appGUI->opt->ical_edit_window = window;
	g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (ical_edit_key_press_cb), appGUI);

	vbox_top = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox_top);

	hbox = utl_gui_create_hbox_in_frame (vbox_top, _("Filename"));

	appGUI->opt->ical_edit_filename_entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), appGUI->opt->ical_edit_filename_entry, TRUE, TRUE, 0);
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->ical_edit_filename_entry, GTK_CAN_FOCUS);
	gtk_editable_set_editable (GTK_EDITABLE (appGUI->opt->ical_edit_filename_entry), FALSE);

	if (config.default_stock_icons)
		appGUI->opt->ical_edit_filename_browse_button = utl_gui_stock_button (GTK_STOCK_DIRECTORY, FALSE);
	else
		appGUI->opt->ical_edit_filename_browse_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_OPEN, FALSE);

	gtk_box_pack_start (GTK_BOX (hbox), appGUI->opt->ical_edit_filename_browse_button, FALSE, TRUE, 0);
	g_signal_connect (appGUI->opt->ical_edit_filename_browse_button, "clicked", G_CALLBACK (ical_edit_filename_browse_cb), appGUI);

	hbox = utl_gui_create_hbox_in_frame (vbox_top, _("Name"));

	appGUI->opt->ical_edit_name_entry = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (hbox), appGUI->opt->ical_edit_name_entry);
	g_signal_connect (G_OBJECT (appGUI->opt->ical_edit_name_entry), "key_release_event",
	                  G_CALLBACK (ical_edit_name_key_release_cb), appGUI);

	hbox = utl_gui_create_hbox_in_frame (vbox_top, _("Options"));

	appGUI->opt->ical_edit_disable_checkbutton = gtk_check_button_new_with_mnemonic (_("Disabled"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->ical_edit_disable_checkbutton, GTK_CAN_FOCUS);
	gtk_container_add (GTK_CONTAINER (hbox), appGUI->opt->ical_edit_disable_checkbutton);
	g_signal_connect (G_OBJECT (appGUI->opt->ical_edit_disable_checkbutton), "toggled", 
					  G_CALLBACK (disable_checkbutton_clicked_cb), appGUI);

	hseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), hseparator, FALSE, TRUE, 4);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), hbuttonbox, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	cancel_button = utl_gui_create_button (GTK_STOCK_CANCEL, OSMO_STOCK_BUTTON_CANCEL, _("Cancel"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), cancel_button);
	g_signal_connect (G_OBJECT (cancel_button), "clicked", G_CALLBACK (close_window), (gpointer) window);

	appGUI->opt->ical_edit_ok_button = utl_gui_create_button (GTK_STOCK_OK, OSMO_STOCK_BUTTON_OK, _("OK"));
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->ical_edit_ok_button, GTK_CAN_FOCUS);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->opt->ical_edit_ok_button);
	g_signal_connect (G_OBJECT (appGUI->opt->ical_edit_ok_button), "clicked",
	                  G_CALLBACK (ical_edit_action_cb), appGUI);

	gtk_tree_model_get (GTK_TREE_MODEL (model), iter,
	                    ICAL_COLUMN_NAME, &ical_name,
	                    ICAL_COLUMN_FILENAME, &ical_filename, 
						ICAL_COLUMN_DISABLED, &ical_disabled, -1);
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->ical_edit_name_entry), ical_name);
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->ical_edit_filename_entry), ical_filename);
	gtk_editable_set_position (GTK_EDITABLE (appGUI->opt->ical_edit_filename_entry), -1);
	g_free (ical_name);
	g_free (ical_filename);

	gtk_widget_set_sensitive (appGUI->opt->ical_edit_name_entry, !ical_disabled);
	gtk_widget_set_sensitive (appGUI->opt->ical_edit_filename_browse_button, !ical_disabled);
	gtk_widget_set_sensitive (appGUI->opt->ical_edit_filename_entry, !ical_disabled);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->opt->ical_edit_disable_checkbutton), ical_disabled);

	gtk_widget_show_all (window);
}
/* ========================================================================== */

static void
calendar_ical_files_edit_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean state;

	state = gtk_tree_selection_get_selected (appGUI->opt->calendar_ical_files_select, &model, &iter);
	gtk_widget_set_sensitive (appGUI->opt->calendar_ical_files_edit_button, state);
	gtk_widget_set_sensitive (appGUI->opt->calendar_ical_files_remove_button, state);

	if (state)
		gui_ical_entry_edit_action (&iter, model, appGUI);
}

/* ========================================================================== */

static gint
ical_entry_edit_list_dbclick_cb (GtkWidget *widget, GdkEventButton *event, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if ((event->type == GDK_2BUTTON_PRESS) && (event->button == 1)) {
		if (gtk_tree_selection_get_selected (appGUI->opt->calendar_ical_files_select, &model, &iter)) {
			gui_ical_entry_edit_action (&iter, model, appGUI);
			return TRUE;
		}
	}

	return FALSE;
}

/* ========================================================================== */

static void
calendar_ical_files_remove_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->calendar_ical_files_treeview), &path, NULL);

	if (path != NULL) {
		gtk_tree_model_get_iter (GTK_TREE_MODEL(appGUI->opt->calendar_ical_files_store), &iter, path);
		gtk_list_store_remove (appGUI->opt->calendar_ical_files_store, &iter);
		gtk_tree_path_free (path);
	}

	ics_check_if_valid (appGUI);
	ics_calendar_refresh (appGUI);
}

/* ========================================================================== */

static void
calendar_ical_files_selected_cb (GtkTreeSelection *selection, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean state;

	state = gtk_tree_selection_get_selected (selection, &model, &iter);
	gtk_widget_set_sensitive (appGUI->opt->calendar_ical_files_edit_button, state);
	gtk_widget_set_sensitive (appGUI->opt->calendar_ical_files_remove_button, state);

	ics_check_if_valid (appGUI);
	ics_calendar_refresh (appGUI);
}

/* ========================================================================== */

static void
create_icalendar_files_section (GtkWidget *icalendar_files_vbox, GUI *appGUI)
{
	GtkWidget *table, *label, *scrolledwindow, *treeview, *button;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	table = gtk_table_new (4, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (icalendar_files_vbox), table, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	label = utl_gui_create_label ("%s:", _("Filename"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = utl_gui_create_label ("%s:", _("Name"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	appGUI->opt->calendar_ical_files_filename_entry = gtk_entry_new ();
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->calendar_ical_files_filename_entry, GTK_CAN_FOCUS);
	gtk_editable_set_editable (GTK_EDITABLE (appGUI->opt->calendar_ical_files_filename_entry), FALSE);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->calendar_ical_files_filename_entry, 1, 4, 3, 4,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_DIRECTORY, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_OPEN, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_ical_files_browse_cb), appGUI);
	appGUI->opt->calendar_ical_files_browse_button = button;

	appGUI->opt->calendar_ical_files_name_entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->calendar_ical_files_name_entry, 1, 2, 4, 5,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->calendar_ical_files_name_entry), "key_release_event",
	                  G_CALLBACK (calendar_ical_files_entry_key_release_cb), appGUI);

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_ADD, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_ADD, FALSE);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 2, 3, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_ical_files_add_cb), appGUI);
	appGUI->opt->calendar_ical_files_add_button = button;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_EDIT, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_EDIT, FALSE);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_ical_files_edit_cb), appGUI);
	appGUI->opt->calendar_ical_files_edit_button = button;

	if (config.default_stock_icons)
		button = utl_gui_stock_button (GTK_STOCK_REMOVE, FALSE);
	else
		button = utl_gui_stock_button (OSMO_STOCK_BUTTON_REMOVE, FALSE);
	gtk_widget_set_sensitive (button, FALSE);
	gtk_table_attach (GTK_TABLE (table), button, 4, 5, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (calendar_ical_files_remove_cb), appGUI);
	appGUI->opt->calendar_ical_files_remove_button = button;

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 5, 0, 3,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (appGUI->opt->calendar_ical_files_store));
	appGUI->opt->calendar_ical_files_treeview = treeview;
	g_signal_connect (G_OBJECT (treeview), "button_press_event", G_CALLBACK (ical_entry_edit_list_dbclick_cb), appGUI);
	appGUI->opt->calendar_ical_files_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (appGUI->opt->calendar_ical_files_select), "changed",
	                  G_CALLBACK (calendar_ical_files_selected_cb), appGUI);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	gtk_container_set_border_width (GTK_CONTAINER (treeview), 4);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
	gtk_widget_set_size_request (treeview, -1, 220);

	renderer = gtk_cell_renderer_pixbuf_new ();  /* icon */
	column = gtk_tree_view_column_new_with_attributes (_("Valid"), renderer, 
														"pixbuf", ICAL_COLUMN_VALID_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	renderer = gtk_cell_renderer_text_new ();    /* name */
	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer, 
														"text", ICAL_COLUMN_NAME, 
														"strikethrough", ICAL_COLUMN_DISABLED, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_expand (column, TRUE);

	renderer = gtk_cell_renderer_text_new ();    /* filename */
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", ICAL_COLUMN_FILENAME, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible (column, FALSE);

	renderer = gtk_cell_renderer_text_new ();    /* valid flag */
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", ICAL_COLUMN_VALID_FLAG, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible (column, FALSE);

	renderer = gtk_cell_renderer_toggle_new ();    /* enable description */
	column = gtk_tree_view_column_new_with_attributes (_("Description"), renderer, 
														"active", ICAL_COLUMN_ENABLE_DESC, 
														"activatable", ICAL_COLUMN_STATE,
														NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	g_signal_connect (renderer, "toggled", G_CALLBACK (ical_description_toggled), appGUI);

	renderer = gtk_cell_renderer_toggle_new ();    /* enable year */
	column = gtk_tree_view_column_new_with_attributes (_("Full date"), renderer, 
														"active", ICAL_COLUMN_FULL_DATE, 
														"activatable", ICAL_COLUMN_STATE,
														NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	g_signal_connect (renderer, "toggled", G_CALLBACK (ical_use_year_toggled), appGUI);

	renderer = gtk_cell_renderer_toggle_new();    /* mark event in calendar */
	column = gtk_tree_view_column_new_with_attributes (_("Mark"), renderer, 
														"active", ICAL_COLUMN_MARK, 
														"activatable", ICAL_COLUMN_STATE,
														NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	g_signal_connect (renderer, "toggled", G_CALLBACK (ical_mark_toggled), appGUI);

	renderer = gtk_cell_renderer_toggle_new();    /* disabled */
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "active", ICAL_COLUMN_DISABLED, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible (column, FALSE);

	renderer = gtk_cell_renderer_toggle_new();    /* inverse state */
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "active", ICAL_COLUMN_STATE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible (column, FALSE);
}

#endif /* HAVE_LIBICAL */

/* ========================================================================== */

static void
calendar_options_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
#ifdef HAVE_GTKSPELL
	GtkSpell *edSpell = NULL;
#endif /* HAVE_GTKSPELL */

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->show_day_names_checkbutton)))
		config.display_options |= GUI_CALENDAR_SHOW_DAY_NAMES;
	else
		config.display_options &= ~GUI_CALENDAR_SHOW_DAY_NAMES;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->no_month_change_checkbutton)))
		config.display_options |= GUI_CALENDAR_NO_MONTH_CHANGE;
	else
		config.display_options &= ~GUI_CALENDAR_NO_MONTH_CHANGE;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->show_week_numbers_checkbutton)))
		config.display_options |= GUI_CALENDAR_SHOW_WEEK_NUMBERS;
	else
		config.display_options &= ~GUI_CALENDAR_SHOW_WEEK_NUMBERS;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->week_start_monday_checkbutton)))
		config.display_options |= GUI_CALENDAR_WEEK_START_MONDAY;
	else
		config.display_options &= ~GUI_CALENDAR_WEEK_START_MONDAY;

	config.fy_simple_view = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->simple_view_in_fy_calendar_checkbutton));
	config.cursor_type = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->enable_block_cursor_checkbutton));

	config.enable_auxilary_calendars = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->enable_auxilary_calendars_checkbutton));
	config.strikethrough_past_notes = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->strikethrough_past_notes_checkbutton));
	config.ascending_sorting_in_day_notes_browser = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ascending_sorting_in_day_notes_checkbutton));

	if (!config.gui_layout) {
		if (config.enable_auxilary_calendars == TRUE) {
			gtk_widget_show (appGUI->cal->aux_cal_expander);
		} else {
			gtk_widget_hide (appGUI->cal->aux_cal_expander);
		}
	} else {
		if (config.enable_auxilary_calendars == TRUE) {
			gtk_widget_show (appGUI->cal->aux_calendars_table);
		} else {
			gtk_widget_hide (appGUI->cal->aux_calendars_table);
		}
	}

#ifdef HAVE_GTKSPELL
	config.day_note_spell_checker = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->spell_checker_in_day_notes_checkbutton));
	edSpell = gtkspell_get_from_text_view (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview));

	if (edSpell != NULL) {
		gtkspell_detach (edSpell);
	} else if (config.day_note_spell_checker == TRUE) {
		edSpell = gtkspell_new_attach (GTK_TEXT_VIEW (appGUI->cal->calendar_note_textview), NULL, NULL);
		if (config.override_locale_settings == TRUE) {
			gtkspell_set_language (edSpell, config.spell_lang, NULL);
		} else {
			gtkspell_set_language (edSpell, g_getenv("LANG"), NULL);
		}
	}
#endif /* HAVE_GTKSPELL */

	gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->calendar_prev),
	    (config.display_options & (GUI_CALENDAR_SHOW_DAY_NAMES | GUI_CALENDAR_WEEK_START_MONDAY)) | GUI_CALENDAR_NO_MONTH_CHANGE);
	gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->calendar_next),
	    (config.display_options & (GUI_CALENDAR_SHOW_DAY_NAMES | GUI_CALENDAR_WEEK_START_MONDAY)) | GUI_CALENDAR_NO_MONTH_CHANGE);

	if (togglebutton == GTK_TOGGLE_BUTTON (appGUI->opt->enable_block_cursor_checkbutton))
		calendar_cursor_settings_enable_disable (appGUI);

	gui_calendar_set_display_options (GUI_CALENDAR (appGUI->cal->calendar), config.display_options);
	gui_calendar_set_cursor_type (GUI_CALENDAR (appGUI->cal->calendar), config.cursor_type);
	g_signal_emit_by_name (G_OBJECT (appGUI->cal->calendar), "day-selected");
}

/* ========================================================================== */

static void
create_options_section (GtkWidget *calendar_opt_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton;
	gint i;

	table = gtk_table_new (8, 1, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_box_pack_start (GTK_BOX (calendar_opt_vbox), table, FALSE, FALSE, 0);

	i = 0;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Week start on Monday"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.display_options & GUI_CALENDAR_WEEK_START_MONDAY);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->week_start_monday_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Show day names"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.display_options & GUI_CALENDAR_SHOW_DAY_NAMES);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->show_day_names_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("No month change"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.display_options & GUI_CALENDAR_NO_MONTH_CHANGE);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->no_month_change_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Show week numbers"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.display_options & GUI_CALENDAR_SHOW_WEEK_NUMBERS);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->show_week_numbers_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Simple view in full-year calendar"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.fy_simple_view);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->simple_view_in_fy_calendar_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Enable auxilary calendars"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.enable_auxilary_calendars);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->enable_auxilary_calendars_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Strikethrough past day notes"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.strikethrough_past_notes);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->strikethrough_past_notes_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Ascending sorting in day notes browser"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.ascending_sorting_in_day_notes_browser);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->ascending_sorting_in_day_notes_checkbutton = checkbutton;

	i++;
#ifdef HAVE_GTKSPELL
	checkbutton = gtk_check_button_new_with_mnemonic (_("Enable spell checker in day notes"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.day_note_spell_checker);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (calendar_options_cb), appGUI);
	appGUI->opt->spell_checker_in_day_notes_checkbutton = checkbutton;
#endif /* HAVE_GTKSPELL */
}

/* ========================================================================== */

static void
day_info_panel_options_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	config.di_show_current_time = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_current_time_checkbutton));
	config.di_show_day_number = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_day_number_checkbutton));
	config.di_show_current_day_distance = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_current_day_distance_checkbutton));
	config.di_show_marked_days = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_marked_days_checkbutton));
	config.di_show_week_number = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_week_number_checkbutton));
	config.di_show_weekend_days = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_weekend_days_checkbutton));
	config.di_show_day_category = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_day_category_checkbutton));
	config.di_show_moon_phase = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_moon_phase_checkbutton));
	config.di_show_notes = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_notes_checkbutton));
	config.di_show_zodiac_sign = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->di_show_zodiac_sign_checkbutton));

	g_signal_emit_by_name (G_OBJECT (appGUI->cal->calendar), "day-selected");
}

/* ========================================================================== */

static void
create_day_info_panel_section (GtkWidget *day_info_panel_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton;
	gint i;

	table = gtk_table_new (9, 1, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_box_pack_start (GTK_BOX (day_info_panel_vbox), table, FALSE, FALSE, 0);

	i = 0;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Current time"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_current_time);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_current_time_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Day number"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_day_number);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_day_number_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Today distance"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_current_day_distance);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_current_day_distance_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Marked days"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_marked_days);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_marked_days_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Week number"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_week_number);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_week_number_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Weekend days"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_weekend_days);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_weekend_days_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Day category"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_day_category);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_day_category_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Moon phase"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_moon_phase);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_moon_phase_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Day notes"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_notes);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_notes_checkbutton = checkbutton;

	i++;
	checkbutton = gtk_check_button_new_with_mnemonic (_("Zodiac sign"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.di_show_zodiac_sign);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (day_info_panel_options_cb), appGUI);
	appGUI->opt->di_show_zodiac_sign_checkbutton = checkbutton;
}

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

GtkWidget *
cal_create_preferences_page (GtkWidget *notebook, GUI *appGUI)
{
	GtkWidget *vbox_top, *vbox_icon, *vbox, *hbox, *scrolledwindow;

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_top), BORDER_WIDTH);
	scrolledwindow = utl_gui_insert_in_scrolled_window (vbox_top, GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 2);
	vbox_icon = utl_gui_create_icon_with_label (OSMO_STOCK_CALENDAR, _("Calendar"));

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Appearance"));
	create_appearance_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Day categories"));
	create_day_categories_section (vbox, appGUI);

#ifdef HAVE_LIBICAL
	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("iCalendar files"));
	create_icalendar_files_section (vbox, appGUI);
#endif /* HAVE_LIBICAL */

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_top), hbox, FALSE, FALSE, 0);

	vbox = utl_gui_create_vbox_in_frame (hbox, _("Calendar"));
	create_options_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (hbox, _("Show in day info panel"));
	create_day_info_panel_section (vbox, appGUI);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, vbox_icon);
	gtk_widget_show_all (scrolledwindow);

	return scrolledwindow;
}

/* ========================================================================== */

