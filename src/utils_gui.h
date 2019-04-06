
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Mąka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Mąka <silloz@users.sourceforge.net>
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

#ifndef _UTILS_GUI_H
#define _UTILS_GUI_H

#include "gui.h"

typedef struct {
	GUI *appGUI;
	gchar *config;
	GtkWidget *entry;
	PangoFontDescription **font;
	GtkWidget *widget;
	gboolean save;
} FONT_SEL;

enum {
    SW_MOVE_UP = 0,
    SW_MOVE_DOWN
};

#define     VBOX_SPACING              8
#define     HBOX_SPACING              8
#define     ALIGNMENT_PADDING_TOP     4
#define     ALIGNMENT_PADDING_RIGHT   4
#define     ALIGNMENT_PADDING_BOTTOM  4
#define     ALIGNMENT_PADDING_LEFT   12
#define     CHECKBOX_PADDING          0
#define     WINDOW_BORDER             8
#define     BORDER_WIDTH              8

#define     COLOR_BG_OK     "#fff77a"
#define     COLOR_BG_FAIL   "#ff7770"

GtkWidget * utl_gui_create_label                    (const gchar *format, const gchar *name);
GtkWidget * utl_gui_create_window                   (const gchar *name, gint width, gint height, GUI *appGUI);
GtkWidget * utl_gui_create_button                   (const gchar *gtk_name, const gchar *osmo_name, gchar *label);
GtkWidget * utl_gui_create_frame                    (GtkWidget *container, const gchar *name);
GtkWidget * utl_gui_create_vbox_in_frame            (GtkWidget *container, const gchar *name);
GtkWidget * utl_gui_create_hbox_in_frame            (GtkWidget *container, const gchar *name);
GtkWidget * utl_gui_create_table_in_frame           (GtkWidget *container, const gchar *name, guint rows, guint columns);
GtkWidget * utl_gui_create_icon_with_label          (const gchar *stock_id, const gchar *label_str);
GtkWidget * utl_gui_insert_in_scrolled_window       (GtkWidget *widget, GtkShadowType type);

void        utl_gui_font_select_cb                  (GtkWidget *widget, gpointer user_data);
gint        utl_gui_get_sw_vscrollbar_width         (GtkWidget *scrolled_window);
void        utl_gui_sw_vscrollbar_move_position     (GtkWidget *scrolled_window, gint direction);
gint        utl_gui_get_column_position             (GtkTreeViewColumn *column, GtkTreeView *treeview,
                                                     gint M, GUI *appGUI);

gint        utl_gui_get_combobox_items              (GtkComboBox *combo_box);
void        utl_gui_create_category_combobox        (GtkComboBox *combo_box, GtkListStore *store, gboolean none);

gchar*      utl_gui_text_buffer_get_text_with_tags  (GtkTextBuffer *buffer);
void        utl_gui_text_buffer_set_text_with_tags  (GtkTextBuffer *buffer, const gchar *text, gboolean clear);
void        utl_gui_text_buffer_toggle_tags         (GtkTextBuffer *buffer, const gchar *tag_name);
void        utl_gui_text_buffer_remove_tags         (GtkTextBuffer *buffer);
gchar *     utl_gui_text_strip_tags                 (gchar *text);
void        utl_gui_clear_text_buffer               (GtkTextBuffer *buffer, GtkTextIter *iter);

void        utl_gui_change_bg_widget_state          (GtkWidget *widget, gchar *color_str, GUI *appGUI);
GdkPixbuf*  utl_gui_create_color_swatch             (gchar *color);

void        utl_gui_fill_iconlabel                  (GtkWidget *dialog, gchar *stock_icon, gchar *message);
void        utl_gui_update_command_status           (GtkWidget *entry_widget, GtkWidget *icon_widget, GUI *appGUI);

gint        utl_gui_create_dialog                   (gint dialog_type, gchar *message, GtkWindow *parent);
gint        utl_gui_check_overwrite_file            (gchar *filename, GtkWidget *window, GUI *appGUI);

gint        utl_gui_list_store_get_text_index       (GtkListStore *store, gchar *text);

GtkWidget*  utl_gui_stock_button                    (const gchar *bstock, gboolean toggle);
GtkWidget*  utl_gui_stock_name_button               (const gchar *bstock, gchar *blabel);
GtkWidget*  utl_gui_stock_label_radio_button        (gchar *label, const gchar *stock, GtkIconSize size);
GtkWidget*  utl_gui_image_label_radio_button        (gchar *label, const guint8 *pix);

void        utl_gui_url_initialize                  (GUI *appGUI);
void        utl_gui_url_setup                       (GSList **links_list, gint *link_index, GtkWidget *textview,
                                                     GUI *appGUI);
void        utl_gui_url_insert_link                 (GSList **links_list, gint *link_index, GtkWidget *textview,
                                                     GtkTextIter *iter, gchar *color, gchar *font, gchar *text,
                                                     gboolean center, GUI *appGUI);
void        utl_gui_url_remove_links                (GSList **links_list, gint *link_index);

#endif /* _UTILS_GUI_H */

