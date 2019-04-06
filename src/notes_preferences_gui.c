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

#include "i18n.h"
#include "notes.h"
#include "notes_preferences_gui.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "utils_gui.h"

#ifdef HAVE_LIBGRINGOTTS
#include <libgringotts.h>

/* ========================================================================== */
/* FIXME */
gint
get_enc_algorithm_value (void)
{
	gint algorithms_table [8] = {
		GRG_AES, GRG_SERPENT, GRG_TWOFISH, GRG_CAST_256,
		GRG_SAFERPLUS, GRG_LOKI97, GRG_3DES, GRG_RIJNDAEL_256
	};

	return algorithms_table[config.notes_enc_algorithm % 8];
}

/* ========================================================================== */

gint
get_enc_hashing_value (void)
{
	gint hashing_table [2] = {
	    GRG_SHA1, GRG_RIPEMD_160
	};

	return hashing_table[config.notes_enc_hashing % 2];
}

/* ========================================================================== */

gint
get_comp_algorithm_value (void)
{
	gint algorithm_table [2] = {
		GRG_ZLIB, GRG_BZIP
	};

	return algorithm_table[config.notes_comp_algorithm % 2];
}

/* ========================================================================== */

gint
get_comp_ratio_value (void)
{
	gint ratio_table [4] = {
	    GRG_LVL_NONE, GRG_LVL_FAST, GRG_LVL_GOOD, GRG_LVL_BEST
	};

	return ratio_table[config.notes_comp_ratio % 4];
}

/* ========================================================================== */

#endif /* HAVE_LIBGRINGOTTS */

#ifdef NOTES_ENABLED

/* ========================================================================== */

static void
combobox_clicked_cb (GtkComboBox *combobox, gint *option)
{
	*option = gtk_combo_box_get_active (combobox);
}

/* ========================================================================== */

static void
checkbutton_clicked_cb (GtkToggleButton *togglebutton, gint *option)
{
	*option = gtk_toggle_button_get_active (togglebutton);
}

/* ========================================================================== */

static void
notes_comp_ratio_cb (GtkComboBox *combobox, gint *option)
{
	*option = gtk_combo_box_get_active (combobox) +1;
}

/* ========================================================================== */

#ifdef HAVE_LIBGRINGOTTS

static void
create_encryption_section (GtkWidget *encryption_vbox, GUI *appGUI)
{
	GtkWidget *table, *combobox, *label;

	table = gtk_table_new (1, 5, FALSE);
	gtk_box_pack_start (GTK_BOX (encryption_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	label = utl_gui_create_label ("%s:", _("Algorithm"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	label = utl_gui_create_label ("%s:", _("Hashing"));
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "AES");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "Serpent");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "Twofish");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "CAST 256");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "Safer+");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "Loki 97");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "Triple DES");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "Rijndael");
	gtk_combo_box_set_active (GTK_COMBO_BOX(combobox), config.notes_enc_algorithm);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (combobox_clicked_cb), &(config.notes_enc_algorithm));
	appGUI->opt->notes_enc_algorithm_combobox = combobox;

	combobox = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "SHA-1");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "RIPEMD-160");
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.notes_enc_hashing);
	gtk_table_attach (GTK_TABLE (table), combobox, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (combobox_clicked_cb), &(config.notes_enc_algorithm));
	appGUI->opt->notes_enc_hashing_combobox = combobox;

	label = utl_gui_create_label ("%s:", _("Compression"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	label = utl_gui_create_label ("%s:", _("Ratio"));
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "ZLib");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "BZip2");
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.notes_comp_algorithm);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (combobox_clicked_cb), &(config.notes_comp_algorithm));
	appGUI->opt->notes_comp_algorithm_combobox = combobox;

	combobox = gtk_combo_box_new_text ();
	gtk_table_attach (GTK_TABLE (table), combobox, 3, 4, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Fast"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Good"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Best"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.notes_comp_ratio - 1);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (notes_comp_ratio_cb), &(config.notes_comp_ratio));
	appGUI->opt->notes_comp_ratio_combobox = combobox;
}

#endif /* HAVE_LIBGRINGOTTS */

/* ========================================================================== */

static void
notes_visible_columns_changed_cb (GtkToggleButton *widget, GUI *appGUI)
{
	config.nte_visible_type_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->nvc_type_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_TYPE], config.nte_visible_type_column);

	config.nte_visible_category_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->nvc_category_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_CATEGORY], config.nte_visible_category_column);

	config.nte_visible_last_changes_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->nvc_last_changes_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_LAST_CHANGES_DATE], config.nte_visible_last_changes_column);

	config.nte_visible_created_column = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->nvc_created_checkbutton));
	gtk_tree_view_column_set_visible (appGUI->nte->notes_columns[N_COLUMN_CREATE_DATE], config.nte_visible_created_column);

	set_note_columns_width (appGUI);
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

	checkbutton = gtk_check_button_new_with_mnemonic (_("Type"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.nte_visible_type_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (notes_visible_columns_changed_cb), appGUI);
	appGUI->opt->nvc_type_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Category"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.nte_visible_category_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (notes_visible_columns_changed_cb), appGUI);
	appGUI->opt->nvc_category_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Last changes"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.nte_visible_last_changes_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (notes_visible_columns_changed_cb), appGUI);
	appGUI->opt->nvc_last_changes_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Created"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.nte_visible_created_column);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (notes_visible_columns_changed_cb), appGUI);
	appGUI->opt->nvc_created_checkbutton = checkbutton;
}

/* ========================================================================== */

static void
notes_category_selected_cb (GtkTreeSelection *selection, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_widget_set_sensitive (appGUI->opt->notes_category_remove_button, TRUE);
	else
		gtk_widget_set_sensitive (appGUI->opt->notes_category_remove_button, FALSE);
}

/* ========================================================================== */

static void
notes_category_add_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkListStore *store = appGUI->opt->notes_category_store;
	GtkTreeIter iter;
	const gchar *category_name;
	gchar *item;
	gint i;

	category_name = gtk_entry_get_text (GTK_ENTRY (appGUI->opt->notes_category_entry));
	if (!strlen (category_name)) return;

	i = 0;

	while (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store), &iter, NULL, i++)) {
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &item, -1);
		if (!strcmp (category_name, item)) {
			g_free (item);
			return;
		}
		g_free (item);
	}

	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store), &iter, NULL, 0);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, category_name, -1);
	gtk_entry_set_text (GTK_ENTRY (appGUI->opt->notes_category_entry), "");
	gtk_widget_set_sensitive (appGUI->opt->notes_category_add_button, FALSE);

	utl_gui_create_category_combobox (GTK_COMBO_BOX (appGUI->nte->cf_combobox), store, FALSE);
	gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->nte->cf_combobox), 0);
}

/* ========================================================================== */

static gint
notes_category_entry_key_release_cb (GtkEntry *entry, GdkEventKey *event, GUI *appGUI)
{
	gboolean state = FALSE;

	if (strlen (gtk_entry_get_text (entry)))
		state = TRUE;

	gtk_widget_set_sensitive (appGUI->opt->notes_category_add_button, state);

	if (event->keyval == GDK_Return) {
		if (state) notes_category_add_cb (NULL, appGUI);
		return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

static void
notes_category_cell_edited_cb (GtkCellRendererText *renderer, gchar *path, gchar *new_text, GUI *appGUI)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (g_ascii_strcasecmp (new_text, "") == 0) return;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (appGUI->opt->notes_category_treeview));
	if (gtk_tree_model_get_iter_from_string (model, &iter, path))
		gtk_list_store_set (appGUI->opt->notes_category_store, &iter, 0, new_text, -1);
}

/* ========================================================================== */

static void
notes_category_remove_cb (GtkWidget *widget, GUI *appGUI)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkListStore *store = appGUI->opt->notes_category_store;

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (appGUI->opt->notes_category_treeview), &path, NULL);
	if (path == NULL) return;

	gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
	gtk_list_store_remove (store, &iter);
	gtk_tree_path_free (path);

	utl_gui_create_category_combobox (GTK_COMBO_BOX (appGUI->nte->cf_combobox), store, FALSE);
	gtk_combo_box_set_active (GTK_COMBO_BOX (appGUI->nte->cf_combobox), 0);
}

/* ========================================================================== */

static void
create_categories_section (GtkWidget *categories_vbox, GUI *appGUI)
{
	GtkWidget *table, *entry, *treeview, *scrolledwindow;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	table = gtk_table_new (4, 3, FALSE);
	gtk_box_pack_start (GTK_BOX (categories_vbox), table, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), entry, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (entry), "key_release_event", G_CALLBACK (notes_category_entry_key_release_cb), appGUI);
	appGUI->opt->notes_category_entry = entry;

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_table_attach_defaults (GTK_TABLE (table), scrolledwindow, 0, 3, 0, 3);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (appGUI->opt->notes_category_store));
	appGUI->opt->notes_category_treeview = treeview;
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	gtk_container_set_border_width (GTK_CONTAINER (treeview), 4);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);
	gtk_widget_set_size_request (treeview, -1, 80);

	appGUI->opt->notes_category_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (appGUI->opt->notes_category_select), "changed", G_CALLBACK (notes_category_selected_cb), appGUI);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (notes_category_cell_edited_cb), appGUI);

	column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	if (config.default_stock_icons) {
		appGUI->opt->notes_category_add_button = utl_gui_stock_button (GTK_STOCK_ADD, FALSE);
	} else {
		appGUI->opt->notes_category_add_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_ADD, FALSE);
	}
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->notes_category_add_button, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_sensitive (appGUI->opt->notes_category_add_button, FALSE);
	g_signal_connect (appGUI->opt->notes_category_add_button, "clicked", G_CALLBACK (notes_category_add_cb), appGUI);

	if (config.default_stock_icons) {
		appGUI->opt->notes_category_remove_button = utl_gui_stock_button (GTK_STOCK_REMOVE, FALSE);
	} else {
		appGUI->opt->notes_category_remove_button = utl_gui_stock_button (OSMO_STOCK_BUTTON_REMOVE, FALSE);
	}
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->notes_category_remove_button, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_sensitive (appGUI->opt->notes_category_remove_button, FALSE);
	g_signal_connect (appGUI->opt->notes_category_remove_button, "clicked", G_CALLBACK (notes_category_remove_cb), appGUI);
}

/* ========================================================================== */

static void
notes_sort_order_changed_cb (GtkComboBox *combobox, GUI *appGUI)
{
	config.notes_sorting_order = gtk_combo_box_get_active (combobox);

	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->nte->notes_sort,
	                                      N_COLUMN_NAME, config.notes_sorting_order);

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->nte->notes_filter));
}

/* ========================================================================== */

static void
notes_sort_mode_changed_cb (GtkComboBox *combobox, GUI *appGUI)
{
	config.notes_sorting_mode = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));

	gtk_tree_sortable_set_sort_column_id ((GtkTreeSortable *) appGUI->nte->notes_sort,
	                                      N_COLUMN_NAME, config.notes_sorting_order);

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (appGUI->nte->notes_filter));
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
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.notes_sorting_order);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (notes_sort_order_changed_cb), appGUI);
	appGUI->opt->notes_sort_order_combobox = combobox;

	combobox = gtk_combo_box_new_text ();
	str = g_strdup_printf ("%s, %s, %s", _("Name"), _("Last changes"), _("Category"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Name"), _("Category"), _("Last changes"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Category"), _("Last changes"), _("Name"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Category"), _("Name"), _("Last changes"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Last changes"), _("Category"), _("Name"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("%s, %s, %s", _("Last changes"), _("Name"), _("Category"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.notes_sorting_mode);
	gtk_table_attach (GTK_TABLE (table), combobox, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (notes_sort_mode_changed_cb), appGUI);
	appGUI->opt->notes_sort_mode_combobox = combobox;
}

/* ========================================================================== */

static void
create_options_section (GtkWidget *notes_opt_vbox, GUI *appGUI)
{
	GtkWidget *checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Remember the last selected category"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.remember_category_in_notes);
	gtk_box_pack_start (GTK_BOX (notes_opt_vbox), checkbutton, FALSE, FALSE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.remember_category_in_notes));
	appGUI->opt->cn_remember_category_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Use system format for date and time"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.use_system_date_in_notes);
	gtk_box_pack_start (GTK_BOX (notes_opt_vbox), checkbutton, FALSE, FALSE, 4);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.use_system_date_in_notes));
	appGUI->opt->cn_use_system_date_checkbutton = checkbutton;
}

/* ========================================================================== */

GtkWidget *
nte_create_preferences_page (GtkWidget *notebook, GUI *appGUI)
{
	GtkWidget *vbox_top, *vbox_icon, *vbox, *scrolledwindow;

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_top), BORDER_WIDTH);
	scrolledwindow = utl_gui_insert_in_scrolled_window (vbox_top, GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 2);
	vbox_icon = utl_gui_create_icon_with_label (OSMO_STOCK_NOTES, _("Notes"));

#ifdef HAVE_LIBGRINGOTTS
	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Encryption"));
	create_encryption_section (vbox, appGUI);
#endif /* HAVE_LIBGRINGOTTS */

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Visible columns"));
	create_visible_columns_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Categories"));
	create_categories_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Sorting"));
	create_sorting_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Notes options"));
	create_options_section (vbox, appGUI);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, vbox_icon);
	gtk_widget_show_all (scrolledwindow);

	return scrolledwindow;
}

/* ========================================================================== */

#endif /* NOTES_ENABLED */

