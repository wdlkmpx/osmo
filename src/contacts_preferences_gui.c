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

#include "contacts.h"
#include "contacts_preferences_gui.h"
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "utils_gui.h"

#ifdef CONTACTS_ENABLED

/* ========================================================================== */

static void
checkbutton_clicked_cb (GtkToggleButton *togglebutton, gint *option)
{
	*option = gtk_toggle_button_get_active (togglebutton);
}

/* ========================================================================== */

static void
contacts_photo_size_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	gint sizes[] = { PHOTO_SMALL, PHOTO_MEDIUM, PHOTO_LARGE };
	gint i;

	i = gtk_combo_box_get_active (widget);
	config.photo_width = sizes[i % 3];
	g_signal_emit_by_name (appGUI->cnt->contacts_list_selection, "changed");
}

/* ========================================================================== */

static void
contact_link_color_changed_cb (GtkColorButton *widget, GUI *appGUI)
{
	GdkColor color;

	gtk_color_button_get_color (widget, &color);
	g_snprintf (config.contact_link_color, MAXCOLORNAME, "#%02X%02X%02X",
	            color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_object_set (G_OBJECT (appGUI->gui_url_tag), "foreground-gdk", &color, NULL);
	g_signal_emit_by_name (G_OBJECT (appGUI->cnt->contacts_list_selection), "changed");
}

/* ========================================================================== */

static void
contact_tag_color_changed_cb (GtkColorButton *widget, GUI *appGUI)
{
	GdkColor color;

	gtk_color_button_get_color (widget, &color);
	g_snprintf (config.contact_tag_color, MAXCOLORNAME, "#%02X%02X%02X",
	            color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	g_signal_emit_by_name (appGUI->cnt->contacts_list_selection, "changed");
}

/* ========================================================================== */

static void
cn_font_size_changed_cb (GtkSpinButton *spinbutton, GUI *appGUI)
{
	config.contact_name_font_size = (gint) gtk_spin_button_get_value (spinbutton);
	g_signal_emit_by_name (G_OBJECT (appGUI->cnt->contacts_list_selection), "changed");
}

static void
ci_font_size_changed_cb (GtkSpinButton *spinbutton, GUI *appGUI)
{
	config.contact_item_font_size = (gint) gtk_spin_button_get_value (spinbutton);
	g_signal_emit_by_name (G_OBJECT (appGUI->cnt->contacts_list_selection), "changed");
}

/* ========================================================================== */

static void
create_appearance_section (GtkWidget *appearance_vbox, GUI *appGUI)
{
	GtkWidget *table, *colors_hbox, *fonts_hbox, *photo_hbox, *spinbutton;
	GtkWidget *label, *color_button, *combobox;
	GtkObject *adjustment;
	GdkColor color;
	gint i = 0;

	table = gtk_table_new (5, 5, FALSE);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);

	label = utl_gui_create_label ("%s:", _("Colors"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	colors_hbox = gtk_hbox_new (FALSE, 8);
	gtk_table_attach (GTK_TABLE (table), colors_hbox, 1, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Color of contact tags"));
	gdk_color_parse (config.contact_tag_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (contact_tag_color_changed_cb), appGUI);
	appGUI->opt->contact_tag_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Color of links"));
	gdk_color_parse (config.contact_link_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (contact_link_color_changed_cb), appGUI);
	appGUI->opt->contact_link_color_picker = color_button;

	i++;
	label = utl_gui_create_label ("%s:", _("Font size"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	fonts_hbox = gtk_hbox_new (FALSE, 8);
	gtk_table_attach (GTK_TABLE (table), fonts_hbox, 1, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	adjustment = gtk_adjustment_new (0, 8, 48, 1, 10, 0);
	spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (spinbutton, _("Name font size"));
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), config.contact_name_font_size);
	gtk_box_pack_start (GTK_BOX (fonts_hbox), spinbutton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (spinbutton), "value-changed", G_CALLBACK (cn_font_size_changed_cb), appGUI);
	appGUI->opt->cn_font_size_spinbutton = spinbutton;

	adjustment = gtk_adjustment_new (0, 8, 48, 1, 10, 0);
	spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (spinbutton, _("Item font size"));
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), config.contact_item_font_size);
	gtk_box_pack_start (GTK_BOX (fonts_hbox), spinbutton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (spinbutton), "value-changed", G_CALLBACK (ci_font_size_changed_cb), appGUI);
	appGUI->opt->ci_font_size_spinbutton = spinbutton;

	i++;
	label = utl_gui_create_label ("%s:", _("Photo size"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	photo_hbox = gtk_hbox_new (FALSE, 8);
	gtk_table_attach (GTK_TABLE (table), photo_hbox, 1, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (photo_hbox), combobox, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Small"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Medium"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Large"));
	appGUI->opt->contacts_photo_size_combobox = combobox;
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (contacts_photo_size_changed_cb), appGUI);

	if (config.photo_width == PHOTO_LARGE) {
		gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 2);
	} else if (config.photo_width == PHOTO_MEDIUM) {
		gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 1);
	} else {
		gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
	}
}

/* ========================================================================== */

static void
contacts_hide_group_column_changed_cb (GtkToggleButton *widget, GUI *appGUI)
{
	config.hide_group_column = gtk_toggle_button_get_active (widget);
	gtk_tree_view_column_set_visible (appGUI->cnt->contacts_columns[COLUMN_GROUP], !config.hide_group_column);
	set_contacts_columns_width (appGUI);
}

/* ========================================================================== */

static void
create_miscellaneous_section (GtkWidget *miscellaneous_vbox, GUI *appGUI)
{
	GtkWidget *checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Select and show first item after search"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.show_after_search);
	gtk_box_pack_start (GTK_BOX (miscellaneous_vbox), checkbutton, TRUE, TRUE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.show_after_search));
	appGUI->opt->contacts_select_first_entry_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Hide group column"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.hide_group_column);
	gtk_box_pack_start (GTK_BOX (miscellaneous_vbox), checkbutton, TRUE, TRUE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (contacts_hide_group_column_changed_cb), appGUI);
	appGUI->opt->contacts_hide_group_column_checkbutton = checkbutton;
}

/* ========================================================================== */

static void
contacts_group_remove_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->contacts_group_treeview), &path, NULL);

	if (path != NULL) {
		gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->opt->contacts_group_store), &iter, path);
		gtk_list_store_remove (appGUI->opt->contacts_group_store, &iter);
		gtk_tree_path_free (path);
	}
}

/* ========================================================================== */

static void
contacts_group_cell_edited_cb (GtkCellRendererText *renderer, gchar *path, gchar *new_text, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (g_ascii_strcasecmp (new_text, "") != 0) {
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (appGUI->opt->contacts_group_treeview));
		if (gtk_tree_model_get_iter_from_string (model, &iter, path))
			gtk_list_store_set (appGUI->opt->contacts_group_store, &iter, 0, new_text, -1);
	}
}

/* ========================================================================== */

static void
contacts_group_add_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreeIter iter;
	const gchar *category_name;
	gchar *item;
	gint i = 0;

	category_name = gtk_entry_get_text (GTK_ENTRY (appGUI->opt->contacts_group_entry));
	if (!strlen (category_name)) return;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->contacts_group_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->contacts_group_store), &iter, 0, &item, -1);
		if (!strcmp (category_name, item)) {
			g_free (item);
			return;
		}
		g_free (item);
	}

	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->contacts_group_store), &iter, NULL, 0);

	gtk_list_store_append (appGUI->opt->contacts_group_store, &iter);
	gtk_list_store_set (appGUI->opt->contacts_group_store, &iter, 0, category_name, -1);
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->contacts_group_entry), "");
	gtk_widget_set_sensitive (appGUI->opt->contacts_group_add_button, FALSE);
}

/* ========================================================================== */

static void
contacts_group_selected_cb (GtkTreeSelection *selection, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_widget_set_sensitive (appGUI->opt->contacts_group_remove_button, TRUE);
	else
		gtk_widget_set_sensitive (appGUI->opt->contacts_group_remove_button, FALSE);
}

/* ========================================================================== */

static gint
contacts_group_entry_key_release_cb (GtkEntry *widget, GdkEventKey *event, GUI *appGUI)
{
	gboolean state = FALSE;

	if (strlen (gtk_entry_get_text (widget)))
		state = TRUE;

	gtk_widget_set_sensitive (appGUI->opt->contacts_group_add_button, state);

	if (event->keyval == GDK_Return) {
		if (state) contacts_group_add_cb (NULL, appGUI);
		return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

static void
create_groups_section (GtkWidget *groups_vbox, GUI *appGUI)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *table, *scrolledwindow, *treeview;

	table = gtk_table_new (4, 3, FALSE);
	gtk_box_pack_start (GTK_BOX (groups_vbox), table, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	appGUI->opt->contacts_group_entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->contacts_group_entry, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->contacts_group_entry), "key_release_event",
	                  G_CALLBACK (contacts_group_entry_key_release_cb), appGUI);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 3, 0, 3,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (appGUI->opt->contacts_group_store));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	gtk_container_set_border_width (GTK_CONTAINER (treeview), 4);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
	gtk_widget_set_size_request (treeview, -1, 80);
	appGUI->opt->contacts_group_treeview = treeview;

	appGUI->opt->contacts_group_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (appGUI->opt->contacts_group_select), "changed", G_CALLBACK (contacts_group_selected_cb), appGUI);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (contacts_group_cell_edited_cb), appGUI);

	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	if (config.default_stock_icons) {
		appGUI->opt->contacts_group_add_button = utl_gui_stock_button (GTK_STOCK_ADD, FALSE);
	} else {
		appGUI->opt->contacts_group_add_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_ADD, FALSE);
	}
	gtk_widget_set_sensitive (appGUI->opt->contacts_group_add_button, FALSE);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->contacts_group_add_button, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (appGUI->opt->contacts_group_add_button, "clicked", G_CALLBACK (contacts_group_add_cb), appGUI);

	if (config.default_stock_icons) {
		appGUI->opt->contacts_group_remove_button = utl_gui_stock_button (GTK_STOCK_REMOVE, FALSE);
	} else {
		appGUI->opt->contacts_group_remove_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_REMOVE, FALSE);
	}
	gtk_widget_set_sensitive (appGUI->opt->contacts_group_remove_button, FALSE);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->contacts_group_remove_button, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (appGUI->opt->contacts_group_remove_button, "clicked", G_CALLBACK (contacts_group_remove_cb), appGUI);
}

/* ========================================================================== */

static void
contacts_sort_mode_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.contacts_sorting_mode = gtk_combo_box_get_active (widget);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->cnt->contacts_sort,
	                                      COLUMN_FIRST_NAME, config.contacts_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->cnt->contacts_sort,
	                                      COLUMN_LAST_NAME, config.contacts_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->cnt->contacts_sort,
	                                      COLUMN_GROUP, config.contacts_sorting_order);

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->cnt->contacts_filter));
}

/* ========================================================================== */

static void
contacts_sort_order_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.contacts_sorting_order = gtk_combo_box_get_active (widget);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->cnt->contacts_sort,
	                                      COLUMN_FIRST_NAME, config.contacts_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->cnt->contacts_sort,
	                                      COLUMN_LAST_NAME, config.contacts_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->cnt->contacts_sort,
	                                      COLUMN_GROUP, config.contacts_sorting_order);

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->cnt->contacts_filter));
}

/* ========================================================================== */

static void
create_sorting_section (GtkWidget *sorting_vbox, GUI *appGUI)
{
	GtkWidget *table, *label, *combobox;
	gchar *str;

	table = gtk_table_new (1, 5, FALSE);
	gtk_box_pack_start (GTK_BOX (sorting_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	label = utl_gui_create_label ("%s:", _("Order"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	label = utl_gui_create_label ("%s:", _("Mode"));
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Ascending"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Descending"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.contacts_sorting_order);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (contacts_sort_order_changed_cb), appGUI);
	appGUI->opt->contacts_sort_order_combobox = combobox;

	combobox = gtk_combo_box_new_text ();
	str = g_strdup_printf ("%s, %s, %s", _("Group"), _("First Name"), _("Last Name"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Group"), _("Last Name"), _("First Name"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Last Name"), _("First Name"), _("Group"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Last Name"), _("Group"), _("First Name"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("First Name"), _("Last Name"), _("Group"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("First Name"), _("Group"), _("Last Name"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.contacts_sorting_mode);
	gtk_table_attach (GTK_TABLE (table), combobox, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (contacts_sort_mode_changed_cb), appGUI);
	appGUI->opt->contacts_sort_mode_combobox = combobox;
}

/* ========================================================================== */

static void
create_visible_columns_section (GtkWidget *visible_columns_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton;

	table = gtk_table_new (1, 3, FALSE);
	gtk_box_pack_start (GTK_BOX (visible_columns_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Age"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.cnt_visible_age_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.cnt_visible_age_column));
	appGUI->opt->vc_age_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Birthday date"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.cnt_visible_birthday_date_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.cnt_visible_birthday_date_column));
	appGUI->opt->vc_birthday_date_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Zodiac sign"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.cnt_visible_zodiac_sign_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.cnt_visible_zodiac_sign_column));
	appGUI->opt->vc_zodiac_sign_checkbutton = checkbutton;
}

/* ========================================================================== */

GtkWidget *
cnt_create_preferences_page (GtkWidget *notebook, GUI *appGUI)
{
	GtkWidget *vbox_top, *vbox_icon, *vbox, *scrolledwindow;

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_top), BORDER_WIDTH);
	scrolledwindow = utl_gui_insert_in_scrolled_window (vbox_top, GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 2);
	vbox_icon = utl_gui_create_icon_with_label (OSMO_STOCK_CONTACTS, _("Contacts"));

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Appearance"));
	create_appearance_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Miscellaneous"));
	create_miscellaneous_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Groups"));
	create_groups_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Sorting"));
	create_sorting_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Visible columns in birthday browser"));
	create_visible_columns_section (vbox, appGUI);

	gtk_tree_view_column_set_visible (appGUI->cnt->contacts_columns[COLUMN_GROUP], !config.hide_group_column);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, vbox_icon);
	gtk_widget_show_all (scrolledwindow);

	return scrolledwindow;
}

/* ========================================================================== */

#endif /* CONTACTS_ENABLED */

