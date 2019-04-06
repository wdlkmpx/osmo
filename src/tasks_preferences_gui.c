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
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "tasks.h"
#include "tasks_preferences_gui.h"
#include "utils_gui.h"

#ifdef TASKS_ENABLED

/* ========================================================================== */

static void
checkbutton_clicked_cb (GtkToggleButton *togglebutton, gint *option)
{
	*option = gtk_toggle_button_get_active (togglebutton);
}

/* ========================================================================== */

static void
due_today_color_changed_cb (GtkColorButton *color_button, GUI *appGUI)
{
	GdkColor color;

	gtk_color_button_get_color (color_button, &color);
	g_snprintf (config.due_today_color, MAXCOLORNAME, "#%02X%02X%02X",
	            color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	refresh_tasks (appGUI);
}

/* ========================================================================== */

static void
due_7days_color_changed_cb (GtkColorButton *color_button, GUI *appGUI)
{
	GdkColor color;

	gtk_color_button_get_color (color_button, &color);
	g_snprintf (config.due_7days_color, MAXCOLORNAME, "#%02X%02X%02X",
	            color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	refresh_tasks (appGUI);
}

/* ========================================================================== */

static void
past_due_color_changed_cb (GtkColorButton *color_button, GUI *appGUI)
{
	GdkColor color;

	gtk_color_button_get_color (color_button, &color);
	g_snprintf (config.past_due_color, MAXCOLORNAME, "#%02X%02X%02X",
	            color.red * 256 / 65536, color.green * 256 / 65536, color.blue * 256 / 65536);
	refresh_tasks (appGUI);
}

/* ========================================================================== */

static void
bold_items_cb (GtkToggleButton *widget, GUI *appGUI)
{
	config.tasks_high_in_bold = gtk_toggle_button_get_active (widget);
	apply_task_attributes (appGUI);
}

/* ========================================================================== */

static void
ti_font_select_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkWidget *font_selector;
	gchar *font_name;
	gint response;

	font_selector = gtk_font_selection_dialog_new (_("Select a font..."));
	gtk_window_set_modal (GTK_WINDOW (font_selector), TRUE);
	gtk_window_set_position (GTK_WINDOW (font_selector), GTK_WIN_POS_MOUSE);
	gtk_window_set_transient_for (GTK_WINDOW (font_selector), GTK_WINDOW (appGUI->main_window));
	gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG (font_selector), config.task_info_font);
	gtk_widget_show (font_selector);
	response = gtk_dialog_run (GTK_DIALOG (font_selector));

	if (response == GTK_RESPONSE_OK) {
		font_name = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG (font_selector));
		g_strlcpy (config.task_info_font, font_name, MAXFONTNAME);
		gtk_entry_set_text (GTK_ENTRY (appGUI->opt->ti_font_entry), font_name);
		g_free (font_name);
		g_object_set (G_OBJECT (appGUI->tsk->font_tag_object), "font", (gchar *) config.task_info_font, NULL);
	}

	gtk_widget_destroy (font_selector);
}

/* ========================================================================== */

static void
create_appearance_section (GtkWidget *appearance_vbox, GUI *appGUI)
{
	GtkWidget *table, *colors_hbox;
	GtkWidget *checkbutton, *label, *color_button, *font_button;
	GdkColor color;

	table = gtk_table_new (4, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);

	colors_hbox = gtk_hbox_new (FALSE, 8);
	gtk_table_attach (GTK_TABLE (table), colors_hbox, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Color of items that are due today"));
	gdk_color_parse (config.due_today_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (due_today_color_changed_cb), appGUI);
	appGUI->opt->due_today_color_picker = color_button;

	color_button = gtk_color_button_new ();
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (due_7days_color_changed_cb), appGUI);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Color of items that are due in the next 7 days"));
	gdk_color_parse (config.due_7days_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	appGUI->opt->due_7days_color_picker = color_button;

	color_button = gtk_color_button_new ();
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (color_button, _("Color of items that are past due"));
	gdk_color_parse(config.past_due_color, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &color);
	gtk_box_pack_start (GTK_BOX (colors_hbox), color_button, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (color_button), "color-set", G_CALLBACK (past_due_color_changed_cb), appGUI);
	appGUI->opt->past_due_color_picker = color_button;

	appGUI->opt->ti_font_entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->ti_font_entry), config.task_info_font);
	GTK_WIDGET_UNSET_FLAGS (appGUI->opt->ti_font_entry, GTK_CAN_FOCUS);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->ti_font_entry, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	if (config.default_stock_icons)
		font_button = utl_gui_stock_button (GTK_STOCK_SELECT_FONT, FALSE);
	else
		font_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_SELECT_FONT, FALSE);
	gtk_table_attach (GTK_TABLE (table), font_button, 2, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (font_button), "clicked", G_CALLBACK (ti_font_select_cb), appGUI);

	label = utl_gui_create_label ("%s:", _("Task info font"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = utl_gui_create_label ("%s:", _("Colors"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Show in bold tasks with high priority"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.tasks_high_in_bold);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (bold_items_cb), appGUI);
	appGUI->opt->ct_bold_items_checkbutton = checkbutton;
}

/* ========================================================================== */

static void
visible_columns_changed_cb (GtkToggleButton *widget, GUI *appGUI)
{
	config.tsk_visible_due_date_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->vc_due_date_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_DUE_DATE], config.tsk_visible_due_date_column);
	config.tsk_visible_type_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->vc_type_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_TYPE], config.tsk_visible_type_column);
	config.tsk_visible_category_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->vc_category_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_CATEGORY], config.tsk_visible_category_column);
	config.tsk_visible_priority_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->vc_priority_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->tsk->tasks_columns[TA_COLUMN_PRIORITY], config.tsk_visible_priority_column);

	set_tasks_columns_width (appGUI);
}

/* ========================================================================== */

static void
create_visible_columns_section (GtkWidget *visible_columns_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton;

	table = gtk_table_new (1, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (visible_columns_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Due date"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.tsk_visible_due_date_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (visible_columns_changed_cb), appGUI);
	appGUI->opt->vc_due_date_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Type"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.tsk_visible_type_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (visible_columns_changed_cb), appGUI);
	appGUI->opt->vc_type_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Priority"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.tsk_visible_priority_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (visible_columns_changed_cb), appGUI);
	appGUI->opt->vc_priority_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Category"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.tsk_visible_category_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (visible_columns_changed_cb), appGUI);
	appGUI->opt->vc_category_checkbutton = checkbutton;
}

/* ========================================================================== */

static void
tasks_category_remove_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->tasks_category_treeview), &path, NULL);

	if (path != NULL) {
		gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->opt->tasks_category_store), &iter, path);
		gtk_list_store_remove (appGUI->opt->tasks_category_store, &iter);
		gtk_tree_path_free (path);
		refresh_tasks (appGUI);
	}
}

/* ========================================================================== */

static void
tasks_category_add_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreeIter iter;
	const gchar *category_name;
	gchar *item;
	gint i;

	category_name = gtk_entry_get_text (GTK_ENTRY (appGUI->opt->tasks_category_entry));
	if (!strlen (category_name)) return;

	i = 0;
	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->tasks_category_store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (appGUI->opt->tasks_category_store), &iter, TC_COLUMN_NAME, &item, -1);
		if (!strcmp (category_name, item)) {
			g_free (item);
			return;
		}
		g_free (item);
	}

	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (appGUI->opt->tasks_category_store), &iter, NULL, 0);

	gtk_list_store_append (appGUI->opt->tasks_category_store, &iter);
	gtk_list_store_set (appGUI->opt->tasks_category_store, &iter,
	                    TC_COLUMN_NAME, category_name,
	                    TC_COLUMN_CALENDAR, TRUE,
	                    TC_COLUMN_TASKS, TRUE, -1);

	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->tasks_category_entry), "");
	gtk_widget_set_sensitive (appGUI->opt->tasks_category_add_button, FALSE);

	utl_gui_create_category_combobox (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), appGUI->opt->tasks_category_store, FALSE);
	gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->tsk->cf_combobox), 0);

	apply_task_attributes (appGUI);
	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter));
	update_tasks_number (appGUI);
}

/* ========================================================================== */

static void
tsk_show_in_tasks_list_toggled (GtkCellRendererToggle *cell, gchar *path_str, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean done_status;

	model = GTK_TREE_MODEL (appGUI->opt->tasks_category_store);
	path = gtk_tree_path_new_from_string (path_str);

	if (path != NULL) {
		gtk_tree_model_get_iter (model, &iter, path);   /* get toggled iter */
		gtk_tree_model_get (model, &iter, TC_COLUMN_TASKS, &done_status, -1);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, TC_COLUMN_TASKS, !done_status, -1);
		refresh_tasks (appGUI);
		gtk_tree_path_free (path);
	}
}

/* ========================================================================== */

static void
tsk_show_in_calendar_toggled (GtkCellRendererToggle *cell, gchar *path_str, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean done_status;

	model = GTK_TREE_MODEL (appGUI->opt->tasks_category_store);
	path = gtk_tree_path_new_from_string (path_str);

	if (path != NULL) {
		gtk_tree_model_get_iter (model, &iter, path);   /* get toggled iter */
		gtk_tree_model_get (model, &iter, TC_COLUMN_CALENDAR, &done_status, -1);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, TC_COLUMN_CALENDAR, !done_status, -1);
		cal_set_day_info (appGUI);
		cal_refresh_marks (appGUI);
		gtk_tree_path_free (path);
	}
}

/* ========================================================================== */

static void
tasks_category_cell_edited_cb (GtkCellRendererText *renderer, gchar *path, gchar *new_text, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (g_ascii_strcasecmp (new_text, "") != 0) {
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (appGUI->opt->tasks_category_treeview));
		if (gtk_tree_model_get_iter_from_string (model, &iter, path)) {
			gtk_list_store_set (appGUI->opt->tasks_category_store, &iter, TC_COLUMN_NAME, new_text, -1);
		}
	}
}

/* ========================================================================== */

static void
tasks_category_selected_cb (GtkTreeSelection *selection, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_widget_set_sensitive (appGUI->opt->tasks_category_remove_button, TRUE);
	else
		gtk_widget_set_sensitive (appGUI->opt->tasks_category_remove_button, FALSE);
}

/* ========================================================================== */

static gint
tasks_category_entry_key_release_cb (GtkEntry *widget, GdkEventKey *event, GUI *appGUI)
{
	gboolean state = FALSE;

	if (strlen (gtk_entry_get_text (widget)))
		state = TRUE;

	gtk_widget_set_sensitive (appGUI->opt->tasks_category_add_button, state);

	if (event->keyval == GDK_Return) {
		if (state) tasks_category_add_cb (NULL, appGUI);
		return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

static void
create_categories_section (GtkWidget *categories_vbox, GUI *appGUI)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *table, *scrolledwindow, *treeview;

	table = gtk_table_new (4, 3, FALSE);
	gtk_box_pack_start (GTK_BOX (categories_vbox), table, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	appGUI->opt->tasks_category_entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->tasks_category_entry, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (appGUI->opt->tasks_category_entry), "key_release_event",
	                  G_CALLBACK (tasks_category_entry_key_release_cb), appGUI);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 3, 0, 3,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (appGUI->opt->tasks_category_store));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	gtk_container_set_border_width (GTK_CONTAINER (treeview), 4);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
	gtk_widget_set_size_request (treeview, -1, 120);
	appGUI->opt->tasks_category_treeview = treeview;

	appGUI->opt->tasks_category_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (appGUI->opt->tasks_category_select), "changed", G_CALLBACK (tasks_category_selected_cb), appGUI);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (tasks_category_cell_edited_cb), appGUI);

	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer, "text", TC_COLUMN_NAME, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_expand (column, TRUE);

	renderer = gtk_cell_renderer_toggle_new ();    /* Show in calendar */
	column = gtk_tree_view_column_new_with_attributes(_("Calendar"), renderer, "active", TC_COLUMN_CALENDAR, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	g_signal_connect (renderer, "toggled", G_CALLBACK (tsk_show_in_calendar_toggled), appGUI);

	renderer = gtk_cell_renderer_toggle_new ();    /* Show in tasks list */
	column = gtk_tree_view_column_new_with_attributes (_("Tasks"), renderer, "active", TC_COLUMN_TASKS, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	g_signal_connect (renderer, "toggled", G_CALLBACK (tsk_show_in_tasks_list_toggled), appGUI);

	if (config.default_stock_icons)
		appGUI->opt->tasks_category_add_button = utl_gui_stock_button (GTK_STOCK_ADD, FALSE);
	else
		appGUI->opt->tasks_category_add_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_ADD, FALSE);
	gtk_widget_set_sensitive (appGUI->opt->tasks_category_add_button, FALSE);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->tasks_category_add_button, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (appGUI->opt->tasks_category_add_button, "clicked", G_CALLBACK (tasks_category_add_cb), appGUI);

	if (config.default_stock_icons)
		appGUI->opt->tasks_category_remove_button = utl_gui_stock_button (GTK_STOCK_REMOVE, FALSE);
	else
		appGUI->opt->tasks_category_remove_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_REMOVE, FALSE);
	gtk_widget_set_sensitive (appGUI->opt->tasks_category_remove_button, FALSE);
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->tasks_category_remove_button, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (appGUI->opt->tasks_category_remove_button, "clicked", G_CALLBACK (tasks_category_remove_cb), appGUI);
}

/* ========================================================================== */

static void
tasks_sort_mode_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.tasks_sorting_mode = gtk_combo_box_get_active (widget);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->tsk->tasks_sort,
	                                      TA_COLUMN_DUE_DATE, config.tasks_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->tsk->tasks_sort,
	                                      TA_COLUMN_PRIORITY, config.tasks_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->tsk->tasks_sort,
	                                      TA_COLUMN_DONE, config.tasks_sorting_order);

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter));
}

/* ========================================================================== */

static void
tasks_sort_order_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.tasks_sorting_order = gtk_combo_box_get_active (widget);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->tsk->tasks_sort,
	                                      TA_COLUMN_DUE_DATE, config.tasks_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->tsk->tasks_sort,
	                                      TA_COLUMN_PRIORITY, config.tasks_sorting_order);
	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->tsk->tasks_sort,
	                                      TA_COLUMN_DONE, config.tasks_sorting_order);

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter));
}

/* ========================================================================== */

static void
create_sorting_section (GtkWidget *sorting_vbox, GUI *appGUI)
{
	GtkWidget *table, *combobox, *label;
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
	gtk_combo_box_set_active (GTK_COMBO_BOX(combobox), config.tasks_sorting_order);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (tasks_sort_order_changed_cb), appGUI);
	appGUI->opt->tasks_sort_order_combobox = combobox;

	combobox = gtk_combo_box_new_text ();
	str = g_strdup_printf ("%s, %s, %s", _("Done"), _("Due date"), _("Priority"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Done"), _("Priority"), _("Due date"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Priority"), _("Due date"), _("Done"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Priority"), _("Done"), _("Due date"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Due date"), _("Priority"), _("Done"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Due date"), _("Done"), _("Priority"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.tasks_sorting_mode);
	gtk_table_attach (GTK_TABLE (table), combobox, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (tasks_sort_mode_changed_cb), appGUI);
	appGUI->opt->tasks_sort_mode_combobox = combobox;
}

/* ========================================================================== */

static void
delete_items_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ct_delete_items_checkbutton)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->opt->ct_hide_items_checkbutton), FALSE);
}

/* ========================================================================== */

static void
hide_items_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ct_hide_items_checkbutton)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->opt->ct_delete_items_checkbutton), FALSE);
}

/* ========================================================================== */

static void
hide_delete_changed_cb (GtkToggleButton *widget, GUI *appGUI)
{
	config.hide_completed = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ct_hide_items_checkbutton));
	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter));
	config.delete_completed = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->ct_delete_items_checkbutton));
}

/* ========================================================================== */

static void
create_tasks_options_section (GtkWidget *tasks_opt_vbox, GUI *appGUI)
{
	GtkWidget *checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Hide completed tasks"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.hide_completed);
	gtk_box_pack_start (GTK_BOX (tasks_opt_vbox), checkbutton, FALSE, FALSE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_delete_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_items_cb), appGUI);
	appGUI->opt->ct_hide_items_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Delete completed tasks without confirmation"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.delete_completed);
	gtk_box_pack_start (GTK_BOX (tasks_opt_vbox), checkbutton, FALSE, FALSE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_delete_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (delete_items_cb), appGUI);
	appGUI->opt->ct_delete_items_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Add new task when double clicked on tasks list"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.add_edit);
	gtk_box_pack_start (GTK_BOX (tasks_opt_vbox), checkbutton, FALSE, FALSE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.add_edit));
	appGUI->opt->ct_add_item_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Remember the last selected category"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.remember_category_in_tasks);
	gtk_box_pack_start (GTK_BOX (tasks_opt_vbox), checkbutton, FALSE, FALSE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb),
	                  &(config.remember_category_in_tasks));
	appGUI->opt->ct_remember_category_checkbutton = checkbutton;
}

/* ========================================================================== */

static gint
global_notification_entry_key_release_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	utl_gui_update_command_status (widget, appGUI->opt->global_notification_valid_image, appGUI);
	g_strlcpy (config.global_notification_command, gtk_entry_get_text (GTK_ENTRY (widget)), MAXHELPERCMD);

	return FALSE;
}

/* ========================================================================== */

static void
sound_alarm_repeat_changed_cb (GtkSpinButton *spinbutton, GUI *appGUI)
{
	config.sound_alarm_repeat = (gint) gtk_spin_button_get_value (spinbutton);
}

/* ========================================================================== */

static void
postpone_time_changed_cb (GtkSpinButton *spinbutton, GUI *appGUI)
{
	config.postpone_time = (gint) gtk_spin_button_get_value (spinbutton);
}

/* ========================================================================== */

static void
create_reminder_options_section (GtkWidget *reminder_opt_vbox, GUI *appGUI)
{
	GtkWidget *table, *label, *spinbutton, *entry, *valid_hbox;
	GtkObject *adjustment;
	gchar *str;
	gint i = 0;

	table = gtk_table_new (3, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (reminder_opt_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);

	label = utl_gui_create_label ("%s:", _("Postpone time"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	adjustment = gtk_adjustment_new (0, 0, 1440, 1, 10, 0);
	spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), config.postpone_time);
	gtk_table_attach (GTK_TABLE (table), spinbutton, 1, 2, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (spinbutton), "value-changed", G_CALLBACK (postpone_time_changed_cb), appGUI);
	appGUI->opt->postpone_time_spinbutton = spinbutton;

	str = g_strdup_printf ("%s (%s)", _("minutes"), _("0 for disable"));
	label = gtk_label_new (str);
	g_free (str);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	i++;
	label = utl_gui_create_label ("%s:", _("Repeat sound alarm"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	adjustment = gtk_adjustment_new (0, 0, 100, 1, 10, 0);
	spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), config.sound_alarm_repeat);
	gtk_table_attach (GTK_TABLE (table), spinbutton, 1, 2, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (spinbutton), "value-changed", G_CALLBACK (sound_alarm_repeat_changed_cb), appGUI);
	appGUI->opt->sound_alarm_repeat_spinbutton = spinbutton;

	str = g_strdup_printf ("%s (%s)", _("times"), _("0 for disable"));
	label = gtk_label_new (str);
	g_free (str);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	i++;
	label = utl_gui_create_label ("%s:", _("Global notification command"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	valid_hbox = gtk_hbox_new (FALSE, 2);
	gtk_table_attach (GTK_TABLE (table), valid_hbox, 1, 3, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (GTK_FILL), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (entry), config.global_notification_command);
	gtk_box_pack_start (GTK_BOX (valid_hbox), entry, TRUE, TRUE, 0);
	g_signal_connect (G_OBJECT (entry), "key_release_event",
	                  G_CALLBACK (global_notification_entry_key_release_cb), appGUI);
	appGUI->opt->global_notification_cmd_entry = entry;

	appGUI->opt->global_notification_valid_image = gtk_image_new ();
	gtk_box_pack_start (GTK_BOX (valid_hbox), appGUI->opt->global_notification_valid_image, FALSE, FALSE, 0);

	utl_gui_update_command_status (entry, appGUI->opt->global_notification_valid_image, appGUI);
}

/* ========================================================================== */

GtkWidget *
tsk_create_preferences_page (GtkWidget *notebook, GUI *appGUI)
{
	GtkWidget *vbox_top, *vbox_icon, *vbox, *scrolledwindow;

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_top), BORDER_WIDTH);
	scrolledwindow = utl_gui_insert_in_scrolled_window (vbox_top, GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 2);
	vbox_icon = utl_gui_create_icon_with_label (OSMO_STOCK_TASKS, _("Tasks"));

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Appearance"));
	create_appearance_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Visible columns"));
	create_visible_columns_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Categories"));
	create_categories_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Sorting"));
	create_sorting_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Tasks options"));
	create_tasks_options_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Reminder options"));
	create_reminder_options_section (vbox, appGUI);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, vbox_icon);
	gtk_widget_show_all (scrolledwindow);

	return scrolledwindow;
}

/* ========================================================================== */

#endif /* TASKS_ENABLED */

