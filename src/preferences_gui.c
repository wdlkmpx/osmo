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

#include "backup.h"
#include "calendar_preferences_gui.h"
#include "contacts_preferences_gui.h"
#include "i18n.h"
#include "notes.h"
#include "notes_preferences_gui.h"
#include "options_prefs.h"
#include "preferences_gui.h"
#include "stock_icons.h"
#include "tasks.h"
#include "tasks_preferences_gui.h"
#include "utils_gui.h"

/* ========================================================================== */

static void
checkbutton_clicked_cb (GtkToggleButton *togglebutton, gint *option)
{
	*option = gtk_toggle_button_get_active (togglebutton);
}

/* ========================================================================== */

static void
layout_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.gui_layout = gtk_combo_box_get_active (widget);

	utl_gui_create_dialog (GTK_MESSAGE_INFO, _("Osmo has to be restarted to take effect."),
	                       GTK_WINDOW (appGUI->opt->window));
}

/* ========================================================================== */

static void
tabs_position_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	GtkWidget *label;
	gint angle;

	config.tabs_position = gtk_combo_box_get_active (widget);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (appGUI->notebook), config.tabs_position);

	if (config.tabs_position == GTK_POS_LEFT) angle = 90;
	else if (config.tabs_position == GTK_POS_RIGHT) angle = -90;
	else angle = 0;

	label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (appGUI->notebook), GTK_WIDGET (appGUI->cal->vbox));
	gtk_label_set_angle (GTK_LABEL (label), angle);

#ifdef TASKS_ENABLED
	label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (appGUI->notebook), GTK_WIDGET (appGUI->tsk->vbox));
	gtk_label_set_angle (GTK_LABEL (label), angle);
#endif /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
	label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (appGUI->notebook), GTK_WIDGET (appGUI->cnt->vbox));
	gtk_label_set_angle (GTK_LABEL (label), angle);
#endif /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
	label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (appGUI->notebook), GTK_WIDGET (appGUI->nte->vbox));
	gtk_label_set_angle (GTK_LABEL (label), angle);
#endif /* NOTES_ENABLED */
}

/* ========================================================================== */

static void
disable_underline_in_links_changed_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	config.disable_underline_links = gtk_toggle_button_get_active (togglebutton);

#ifdef CONTACTS_ENABLED
	g_signal_emit_by_name (G_OBJECT (appGUI->cnt->contacts_list_selection), "changed");
#endif /* CONTACTS_ENABLED */
}

/* ========================================================================== */

static void
enable_rules_hint_changed_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	config.rules_hint = gtk_toggle_button_get_active (togglebutton);

#ifdef CONTACTS_ENABLED
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (appGUI->cnt->contacts_list), config.rules_hint);
#endif /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (appGUI->nte->notes_list), config.rules_hint);
#endif /* NOTES_ENABLED */

#ifdef TASKS_ENABLED
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (appGUI->tsk->tasks_list), config.rules_hint);
#endif /* TASKS_ENABLED */
}

/* ========================================================================== */

static void
create_appearance_section (GtkWidget *appearance_vbox, GUI *appGUI)
{
	GtkWidget *table, *label, *combobox, *checkbutton;

	table = gtk_table_new (1, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), table, FALSE, TRUE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 16);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);

	label = utl_gui_create_label ("%s:", _("Layout"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Vertical"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Horizontal"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.gui_layout);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (layout_changed_cb), appGUI);

	label = utl_gui_create_label ("%s:", _("Tabs position"));
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_table_attach (GTK_TABLE (table), combobox, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Left"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Right"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Top"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("Bottom"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.tabs_position);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (tabs_position_changed_cb), appGUI);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Use default stock icons (needs restart)"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.default_stock_icons);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), checkbutton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.default_stock_icons));

	checkbutton = gtk_check_button_new_with_mnemonic (_("Disable underline in links"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.disable_underline_links);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), checkbutton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (disable_underline_in_links_changed_cb), appGUI);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Draw rows in alternating colors"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.rules_hint);
	gtk_box_pack_start (GTK_BOX (appearance_vbox), checkbutton, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (enable_rules_hint_changed_cb), appGUI);
}

/* ========================================================================== */

#if defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED) || defined(NOTES_ENABLED)

static void
hide_module_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	gboolean cal, tsk, nte, cnt;

	cal = tsk = nte = cnt = TRUE;

	cal = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->hide_calendar_checkbutton));

#ifdef TASKS_ENABLED
	tsk = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->hide_tasks_checkbutton));
#endif /* TASKS_ENABLED */

#ifdef NOTES_ENABLED
	nte = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->hide_notes_checkbutton));
#endif /* NOTES_ENABLED */

#ifdef CONTACTS_ENABLED
	cnt = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->opt->hide_contacts_checkbutton));
#endif /* CONTACTS_ENABLED */

	if (cal && tsk && nte && cnt) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("At least one module must be visible."),
		                       GTK_WINDOW (appGUI->opt->window));
		cal = FALSE;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->opt->hide_calendar_checkbutton), FALSE);
	}

	if (cal) {
		config.hide_calendar = TRUE;
		gtk_widget_hide (GTK_WIDGET (appGUI->cal->vbox));
		gtk_widget_hide (appGUI->opt->calendar);
		gtk_widget_hide (appGUI->trayicon_menu_calendar_item);
	} else {
		config.hide_calendar = FALSE;
		gtk_widget_show (GTK_WIDGET (appGUI->cal->vbox));
		gtk_widget_show (appGUI->opt->calendar);
		gtk_widget_show (appGUI->trayicon_menu_calendar_item);
	}

#ifdef TASKS_ENABLED
	if (tsk) {
		config.hide_tasks = TRUE;
		gtk_widget_hide(GTK_WIDGET (appGUI->tsk->vbox));
		gtk_widget_hide (appGUI->opt->tasks);
		gtk_widget_hide (appGUI->trayicon_menu_tasks_item);
	} else {
		config.hide_tasks = FALSE;
		gtk_widget_show (GTK_WIDGET (appGUI->tsk->vbox));
		gtk_widget_show (appGUI->opt->tasks);
		gtk_widget_show (appGUI->trayicon_menu_tasks_item);
	}
#endif /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
	if (cnt) {
		config.hide_contacts = TRUE;
		gtk_widget_hide (GTK_WIDGET (appGUI->cnt->vbox));
		gtk_widget_hide (appGUI->opt->contacts);
		gtk_widget_hide (appGUI->trayicon_menu_contacts_item);
	} else {
		config.hide_contacts = FALSE;
		gtk_widget_show (GTK_WIDGET (appGUI->cnt->vbox));
		gtk_widget_show (appGUI->opt->contacts);
		gtk_widget_show (appGUI->trayicon_menu_contacts_item);
	}
#endif /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
	if (nte) {
		config.hide_notes = TRUE;
		gtk_widget_hide (GTK_WIDGET (appGUI->nte->vbox));
		gtk_widget_hide (appGUI->opt->notes);
		gtk_widget_hide (appGUI->trayicon_menu_notes_item);
	} else {
		config.hide_notes = FALSE;
		gtk_widget_show (GTK_WIDGET (appGUI->nte->vbox));
		gtk_widget_show (appGUI->opt->notes);
		gtk_widget_show (appGUI->trayicon_menu_notes_item);
	}
#endif /* NOTES_ENABLED */
}

/* ========================================================================== */

static void
create_hide_section (GtkWidget *hide_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton;

	table = gtk_table_new (1, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (hide_vbox), table, TRUE, TRUE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 16);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Calendar"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.hide_calendar);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_module_cb), appGUI);
	appGUI->opt->hide_calendar_checkbutton = checkbutton;

#ifdef TASKS_ENABLED
	checkbutton = gtk_check_button_new_with_mnemonic (_("Tasks"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.hide_tasks);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_module_cb), appGUI);
	appGUI->opt->hide_tasks_checkbutton = checkbutton;
#endif /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
	checkbutton = gtk_check_button_new_with_mnemonic (_("Contacts"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.hide_contacts);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_module_cb), appGUI);
	appGUI->opt->hide_contacts_checkbutton = checkbutton;
#endif /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
	checkbutton = gtk_check_button_new_with_mnemonic (_("Notes"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.hide_notes);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (hide_module_cb), appGUI);
	appGUI->opt->hide_notes_checkbutton = checkbutton;
#endif /* NOTES_ENABLED */

}

#endif /* defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED) || defined(NOTES_ENABLED) */

/* ========================================================================== */

static void
override_locale_changed_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	config.override_locale_settings = gtk_toggle_button_get_active (togglebutton);

	gtk_widget_set_sensitive (appGUI->opt->override_locale_label_1, config.override_locale_settings);
	gtk_widget_set_sensitive (appGUI->opt->override_locale_label_2, config.override_locale_settings);
	gtk_widget_set_sensitive (appGUI->opt->date_format_combobox, config.override_locale_settings);
	gtk_widget_set_sensitive (appGUI->opt->time_format_combobox, config.override_locale_settings);

#ifdef HAVE_GTKSPELL
	gtk_widget_set_sensitive (appGUI->opt->override_locale_label_3, config.override_locale_settings);
	gtk_widget_set_sensitive (appGUI->opt->entry_spell_lang, config.override_locale_settings);
#endif  /* HAVE_GTKSPELL */

#ifdef TASKS_ENABLED
	refresh_tasks (appGUI);
#endif  /* TASKS_ENABLED */

#ifdef NOTES_ENABLED
	refresh_notes (appGUI);
#endif  /* NOTES_ENABLED */
}

/* ========================================================================== */

static void
date_format_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.date_format = gtk_combo_box_get_active (widget);

#ifdef TASKS_ENABLED
	refresh_tasks (appGUI);
#endif  /* TASKS_ENABLED */
}

/* ========================================================================== */

static void
time_format_changed_cb (GtkComboBox *widget, GUI *appGUI)
{
	config.time_format = gtk_combo_box_get_active (widget);

#ifdef TASKS_ENABLED
	refresh_tasks (appGUI);
#endif  /* TASKS_ENABLED */
}

/* ========================================================================== */

#ifdef HAVE_GTKSPELL

static void
spell_checker_entry_changed_cb (GtkEntry *entry, GUI *appGUI)
{
	if (strlen (gtk_entry_get_text (entry)))
		g_strlcpy (config.spell_lang, gtk_entry_get_text (entry), MAXNAME);

	utl_gui_change_bg_widget_state (GTK_WIDGET (entry), NULL, appGUI);
}

/* ========================================================================== */

static gint
spell_checker_key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	utl_gui_change_bg_widget_state (widget, COLOR_BG_OK, appGUI);
	return FALSE;
}

#endif  /* HAVE_GTKSPELL */

/* ========================================================================== */

static void
create_general_section (GtkWidget *general_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton, *combobox;
	gchar *str;
	gint i = 0;

	table = gtk_table_new (6, 4, FALSE);
	gtk_box_pack_start (GTK_BOX (general_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 16);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Override locale settings"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.override_locale_settings);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (override_locale_changed_cb), appGUI);
	appGUI->opt->override_locale_checkbutton = checkbutton;

	i++;

	appGUI->opt->override_locale_label_1 = utl_gui_create_label ("%s:", _("Date format"));
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->override_locale_label_1, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("DD-MM-YYYY"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("MM-DD-YYYY"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("YYYY-MM-DD"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("YYYY-DD-MM"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.date_format);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (date_format_changed_cb), appGUI);
	appGUI->opt->date_format_combobox = combobox;

	i++;

	appGUI->opt->override_locale_label_2 = utl_gui_create_label ("%s:", _("Time format"));
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->override_locale_label_2, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	combobox = gtk_combo_box_new_text ();
	str = g_strdup_printf ("24 %s", _("hours"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	str = g_strdup_printf ("12 %s", _("hours"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), str);
	g_free (str);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), config.time_format);
	gtk_table_attach (GTK_TABLE (table), combobox, 1, 2, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (time_format_changed_cb), appGUI);
	appGUI->opt->time_format_combobox = combobox;

	i++;

#ifdef HAVE_GTKSPELL

	GtkWidget *entry;

	appGUI->opt->override_locale_label_3 = utl_gui_create_label ("%s:", _("Spell checker language"));
	gtk_table_attach (GTK_TABLE (table), appGUI->opt->override_locale_label_3, 0, 1, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), MAXNAME);
    gtk_entry_set_text (GTK_ENTRY (entry), config.spell_lang);
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (spell_checker_entry_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (spell_checker_key_press_cb), appGUI);
	appGUI->opt->entry_spell_lang = entry;

	i++;

#endif /* HAVE_GTKSPELL */

	checkbutton = gtk_check_button_new_with_mnemonic (_("Enable tooltips"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.enable_tooltips);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.enable_tooltips));

	i++;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Remember last selected page"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.remember_latest_tab);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.remember_latest_tab));

	i++;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Save data after every modification"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.save_data_after_modification);
	gtk_table_attach (GTK_TABLE (table), checkbutton, 0, 4, i, i+1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.save_data_after_modification));

}

/* ========================================================================== */

static void
helper_entry_changed_cb (GtkEntry *entry, GUI *appGUI)
{
	if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->entry_web_browser))))
		g_strlcpy (config.web_browser, gtk_entry_get_text (GTK_ENTRY (appGUI->opt->entry_web_browser)), MAXHELPERCMD);

	if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->entry_email_client))))
		g_strlcpy (config.email_client, gtk_entry_get_text (GTK_ENTRY (appGUI->opt->entry_email_client)), MAXHELPERCMD);

	if (strlen (gtk_entry_get_text (GTK_ENTRY (appGUI->opt->entry_sound_player))))
		g_strlcpy (config.sound_player, gtk_entry_get_text (GTK_ENTRY (appGUI->opt->entry_sound_player)), MAXHELPERCMD);

	utl_gui_change_bg_widget_state (appGUI->opt->entry_web_browser, NULL, appGUI);
	utl_gui_change_bg_widget_state (appGUI->opt->entry_email_client, NULL, appGUI);
	utl_gui_change_bg_widget_state (appGUI->opt->entry_sound_player, NULL, appGUI);
}

/* ========================================================================== */

static gint 
helper_key_press_cb (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
    utl_gui_change_bg_widget_state (widget, COLOR_BG_OK, appGUI);
    return FALSE;
}

/* ========================================================================== */

void
create_helpers_section (GtkWidget *helpers_vbox, GUI *appGUI)
{
	GtkWidget *table, *label, *entry;

	table = gtk_table_new (3, 2, FALSE);
	gtk_box_pack_start (GTK_BOX (helpers_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	label = utl_gui_create_label ("%s:", _("Web browser"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	label = utl_gui_create_label ("%s:", _("E-mail client"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = utl_gui_create_label ("%s:", _("Sound player"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), MAXHELPERCMD);
	gtk_entry_set_text (GTK_ENTRY (entry), config.web_browser);
	if (config.enable_tooltips) {
		gtk_widget_set_tooltip_text (entry, _("The %s pattern will be replaced with web address"));
	}
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (helper_entry_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (helper_key_press_cb), appGUI);
	appGUI->opt->entry_web_browser = entry;

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), MAXHELPERCMD);
	gtk_entry_set_text (GTK_ENTRY (entry), config.email_client);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (entry, _("The %s pattern will be replaced with e-mail address"));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (helper_entry_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (helper_key_press_cb), appGUI);
	appGUI->opt->entry_email_client = entry;

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), MAXHELPERCMD);
	gtk_entry_set_text (GTK_ENTRY (entry), config.sound_player);
	if (config.enable_tooltips)
		gtk_widget_set_tooltip_text (entry, _("The %s pattern will be replaced with sound filename"));
	gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 2, 3,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (helper_entry_changed_cb), appGUI);
	g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (helper_key_press_cb), appGUI);
	appGUI->opt->entry_sound_player = entry;
}

/* ========================================================================== */

#if defined(BACKUP_SUPPORT) && defined(HAVE_LIBGRINGOTTS)

static void
button_create_backup_cb (GtkWidget *widget, GUI *appGUI)
{
	backup_create (appGUI);
}

/* ========================================================================== */

static void
button_restore_backup_cb (GtkWidget *widget, GUI *appGUI)
{
	backup_restore (appGUI);
}

/* ========================================================================== */

static void
create_backup_section (GtkWidget *backup_vbox, GUI *appGUI)
{
	GtkWidget *hbuttonbox, *create_button, *restore_button;

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_container_add (GTK_CONTAINER (backup_vbox), hbuttonbox);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 4);

	create_button = gtk_button_new_with_label (_("Create"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), create_button);
	g_signal_connect (create_button, "clicked", G_CALLBACK (button_create_backup_cb), appGUI);

	restore_button = gtk_button_new_with_label (_("Restore"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), restore_button);
	g_signal_connect (restore_button, "clicked", G_CALLBACK (button_restore_backup_cb), appGUI);

}

#endif /* BACKUP_SUPPORT && HAVE_LIBGRINGOTTS */

/* ========================================================================== */

static void
enable_systray_changed_cb (GtkToggleButton *togglebutton, GUI *appGUI)
{
	if (gtk_toggle_button_get_active (togglebutton)) {
		config.enable_systray = 1;
		gtk_widget_set_sensitive (appGUI->opt->start_minimised_checkbutton, TRUE);
		gtk_widget_set_sensitive (appGUI->opt->blink_on_events_checkbutton, TRUE);
		gtk_widget_set_sensitive (appGUI->opt->ignore_day_note_events_checkbutton, TRUE);
		gtk_status_icon_set_visible (appGUI->osmo_trayicon, TRUE);
	} else {
		config.enable_systray = 0;
		gtk_widget_set_sensitive (appGUI->opt->start_minimised_checkbutton, FALSE);
		gtk_widget_set_sensitive (appGUI->opt->blink_on_events_checkbutton, FALSE);
		gtk_widget_set_sensitive (appGUI->opt->ignore_day_note_events_checkbutton, FALSE);
		gtk_status_icon_set_visible (appGUI->osmo_trayicon, FALSE);
	}
}

/* ========================================================================== */

void
create_system_tray_section (GtkWidget *system_tray_vbox, GUI *appGUI)
{
	GtkWidget *table, *checkbutton;

	table = gtk_table_new (3, 3, FALSE);
	gtk_widget_show (table);
	gtk_box_pack_start (GTK_BOX (system_tray_vbox), table, FALSE, FALSE, 0);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);

	checkbutton = gtk_check_button_new_with_mnemonic (_("Enable system tray"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.enable_systray);
	gtk_table_attach_defaults (GTK_TABLE (table), checkbutton, 0, 1, 0, 1);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (enable_systray_changed_cb), appGUI);
	appGUI->opt->enable_systray_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Start minimised"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.start_minimised_in_systray);
	gtk_widget_set_sensitive (checkbutton, config.enable_systray);
	gtk_table_attach_defaults (GTK_TABLE (table), checkbutton, 1, 2, 0, 1);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.start_minimised_in_systray));
	appGUI->opt->start_minimised_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Blink on events"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.blink_on_events);
	gtk_widget_set_sensitive (checkbutton, config.enable_systray);
	gtk_table_attach_defaults (GTK_TABLE (table), checkbutton, 2, 3, 0, 1);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.blink_on_events));
	appGUI->opt->blink_on_events_checkbutton = checkbutton;

	checkbutton = gtk_check_button_new_with_mnemonic (_("Ignore day note events"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.ignore_day_note_events);
	gtk_widget_set_sensitive (checkbutton, config.enable_systray);
	gtk_table_attach_defaults (GTK_TABLE (table), checkbutton, 0, 3, 1, 2);
	g_signal_connect (G_OBJECT (checkbutton), "toggled", G_CALLBACK (checkbutton_clicked_cb), &(config.ignore_day_note_events));
	appGUI->opt->ignore_day_note_events_checkbutton = checkbutton;
}

/* ========================================================================== */

static GtkWidget *
create_general_page (GtkWidget *notebook, GUI *appGUI)
{
	GtkWidget *vbox_top, *vbox_icon, *vbox, *scrolledwindow;

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_top), BORDER_WIDTH);
	scrolledwindow = utl_gui_insert_in_scrolled_window (vbox_top, GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 2);
	vbox_icon = utl_gui_create_icon_with_label (OSMO_STOCK_PREFERENCES, _("General"));

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Appearance"));
	create_appearance_section (vbox, appGUI);

#if defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED) || defined(NOTES_ENABLED)
	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Hide"));
	create_hide_section (vbox, appGUI);
#endif /* defined(TASKS_ENABLED) || defined(CONTACTS_ENABLED) || defined(NOTES_ENABLED) */

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("General"));
	create_general_section (vbox, appGUI);

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Helpers"));
	create_helpers_section (vbox, appGUI);

#if defined(BACKUP_SUPPORT) && defined(HAVE_LIBGRINGOTTS)
	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("Backup"));
	create_backup_section (vbox, appGUI);
#endif /* BACKUP_SUPPORT && HAVE_LIBGRINGOTTS */

	vbox = utl_gui_create_vbox_in_frame (vbox_top, _("System tray"));
	create_system_tray_section (vbox, appGUI);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, vbox_icon);
	gtk_widget_show_all (scrolledwindow);

	return scrolledwindow;
}

/* ========================================================================== */

static void
close_window (GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_destroy (window);
}

/* ========================================================================== */

static gint
key_press (GtkWidget *widget, GdkEventKey *event, GUI *appGUI)
{
	switch (event->keyval) {
		case GDK_Escape:
			close_window (NULL, widget);
			return TRUE;
	}

	return FALSE;
}

/* ========================================================================== */

GtkWidget *
opt_create_preferences_window (GUI *appGUI)
{
	GtkWidget *window, *vbox_top;
	GtkWidget *notebook;
	GtkWidget *hbuttonbox, *close_button;

	window = utl_gui_create_window (_("Preferences"), 650, 600, appGUI);
	g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (key_press), appGUI);

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_add (GTK_CONTAINER (window), vbox_top);

	appGUI->opt->notebook = notebook = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), notebook, TRUE, TRUE, 0);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
	gtk_widget_show (notebook);

	appGUI->opt->general = create_general_page (notebook, appGUI);
	appGUI->opt->calendar = cal_create_preferences_page (notebook, appGUI);
	if (config.hide_calendar)
		gtk_widget_hide (appGUI->opt->calendar);

#ifdef TASKS_ENABLED
	appGUI->opt->tasks = tsk_create_preferences_page (notebook, appGUI);
	if (config.hide_tasks)
		gtk_widget_hide (appGUI->opt->tasks);
#endif /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
	appGUI->opt->contacts = cnt_create_preferences_page (notebook, appGUI);
	if (config.hide_contacts)
		gtk_widget_hide (appGUI->opt->contacts);
#endif /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
	appGUI->opt->notes = nte_create_preferences_page (notebook, appGUI);
	if (config.hide_notes)
		gtk_widget_hide (appGUI->opt->notes);
#endif /* NOTES_ENABLED */

	/* Close button */
	hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), hbuttonbox, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), HBOX_SPACING);

	close_button = utl_gui_create_button (GTK_STOCK_CLOSE, OSMO_STOCK_BUTTON_CLOSE, _("Close"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), close_button);
	GTK_WIDGET_SET_FLAGS (close_button, GTK_CAN_DEFAULT);
	g_signal_connect (G_OBJECT (close_button), "clicked", G_CALLBACK (close_window), window);
	gtk_widget_grab_focus (close_button);
	gtk_widget_show_all (hbuttonbox);

	gtk_widget_show (vbox_top);

	return window;
}

/* ========================================================================== */

