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

#include "about.h"
#include "gui_logo.h"
#include "i18n.h"
#include "options_prefs.h"
#include "stock_icons.h"
#include "utils_gui.h"

static void     display_license         (GUI *appGUI);
static void     display_help            (GUI *appGUI);
static void     add_credits_section     (const gchar *section_title, OsmoCreditEntry *credits,
                                         guint32 n_credits, GUI *appGUI);
static void     display_about           (GUI *appGUI);
static void     about_switch_buttons    (gboolean left, GUI *appGUI);
static void     button_released_cb      (GtkButton *button, GUI *appGUI);
static void     gui_create_about        (GtkWidget *vbox_top, GUI *appGUI);

static OsmoCreditEntry credits_programming[] = {
	{ NULL, "Tomasz Mąka", "pasp@users.sourceforge.net" },
	{ NULL, "Piotr Mąka", "silloz@users.sourceforge.net" }
};

static OsmoCreditEntry credits_graphics[] = {
	{ NULL, "Maja Kocoń", "http://pinky-babble.org" }
};

static OsmoCreditEntry credits_contributors[] = {
	{ NULL, "Markus Dahms", "mad@automagically.de" },
	{ NULL, "Nacho Alonso González", "nacho.alonso.gonzalez@gmail.com" }
};

static OsmoCreditEntry credits_translators[] = {
	{ "bg",     "Borislav Totev", "btotev@hotmail.com" },
	{ "ca",     "Roger Adell", "roger.adell@gmail.com" },
	{ "cs",     "Jaroslav Lichtblau", "dragonlord@seznam.cz" },
	{ "da",     "Joakim Seeberg", "joak@users.sourceforge.net" },
	{ "de",     "Markus Dahms", "mad@automagically.de" },               /* de */
	{ NULL,     "Mario Blättermann", "mariobl@gnome.org" },             /* de */
	{ "el",     "Konstantinos Tsakaloglou", "tsakf@yahoo.com" },
	{ "en_GB",  "Steve Cook (Yorvyk)", "yorvik.ubunto@googlemail.com" },
	{ "es",     "Nacho Alonso González", "nacho.alonso.gonzalez@gmail.com" },
	{ "fi",     "Toivo Miettinen", "toivo.miettinen@panuma.fi" },
	{ "fr",     "Jean-Jacques Moulinier", "postmaster@moulinier.net" }, /* fr */
	{ NULL,     "Rémi Roupsard", "remi.roupsard@gmail.com"},            /* fr */
	{ NULL,     "Lylliann Essandre", "lylambda@gmail.com"},             /* fr */
	{ "hu",     "Peter Polonkai", "polesz@nedudu.hu" },
	{ "it",     "Bautz", "bautz@email.com" },                           /* it */
	{ NULL,     "Calogero Bonasia", "kbonasia@gmail.com" },             /* it */
	{ "ja",     "Norihiro Yoneda", "aoba@avis.ne.jp" },
	{ "lt",     "Vaidotas Kazla", "joshas@gmail.com" },
	{ "nl",     "Tiger!P", "tigerp@tigerp.net" },
	{ "pl",     "Piotr Mąka", "silloz@users.sourceforge.net" },
	{ "pt",     "Bruno Miguel", "brunoalexandremiguel@gmail.com" },
	{ "ru",     "Vyacheslav A. Trishkin", "dedovsk@mail.ru" },          /* ru */
	{ NULL,     "Alexander Vozhennikov", "vodka_ploho@mail.ru" },       /* ru */
	{ NULL,     "Sergey Panasenko", "nitay@users.sourceforge.net" },    /* ru */
	{ "sv",     "Niklas Grahn", "terra.unknown@yahoo.com" },
	{ "tr",     "Hasan Yılmaz", "hasanyilmaz@users.sourceforge.net" },
	{ "uk",     "Sergey Panasenko", "nitay@users.sourceforge.net" },
	{ "zh_CN",  "Sasaqqdan", "sasaqqdan@gmail.com" },                   /* zh_cn */
    { NULL,     "Darcsis", "darcsis@gmail.com" }                        /* zh_cn */
};

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

		case GDK_Page_Down:
			about_switch_buttons (FALSE, appGUI);
			return TRUE;

		case GDK_Page_Up:
			about_switch_buttons (TRUE, appGUI);
			return TRUE;

	}

	return FALSE;
}

/* ========================================================================== */

GtkWidget *
opt_create_about_window (GUI *appGUI)
{
	GtkWidget *window, *vbox_top;
	GtkWidget *hbuttonbox, *button_close;

	window = utl_gui_create_window (_("About"), 510, 600, appGUI);
	gtk_widget_set_events (window, GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (key_press), appGUI);

	vbox_top = gtk_vbox_new (FALSE, VBOX_SPACING);
	gtk_container_add (GTK_CONTAINER (window), vbox_top);

	gui_create_about (vbox_top, appGUI);

	/* Close button */
	hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox_top), hbuttonbox, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), HBOX_SPACING);

	button_close = utl_gui_create_button (GTK_STOCK_CLOSE, OSMO_STOCK_BUTTON_CLOSE, _("Close"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), button_close);
	GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
	g_signal_connect (G_OBJECT (button_close), "clicked", G_CALLBACK (close_window), window);
	gtk_widget_grab_focus (button_close);

	gtk_widget_show_all (vbox_top);

	return window;
}

/* ========================================================================== */

static void
display_license (GUI *appGUI)
{
	const gchar license_text[] = {
		"\nThis program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version."
		"\n\n"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		"GNU General Public License for more details."
		"\n\n"
		"You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA."
	};

	utl_gui_clear_text_buffer (appGUI->about_entry_buffer, &appGUI->about_entry_iter);
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, license_text, -1);
	gtk_widget_grab_focus (GTK_WIDGET (appGUI->scrolled_window));
}

/* ========================================================================== */

static void
display_help (GUI *appGUI)
{
	const gchar *general_keys_text[] = {
		N_("Ctrl+PageUp"),            N_("switch to previous tab"),
		N_("Ctrl+PageDn"),            N_("switch to next tab"),
		"Alt+1...6, F1-F4",           N_("switch to selected page"),
		"F5",                         N_("show options window"),
		"F6",                         N_("show about window"),
		N_("PageUp/PageDn"),          N_("switch page in options and about tab"),
		"Ctrl+q",                     N_("exit")
	};

	const gchar *calendar_keys_text[] = {
		N_("Space"),                  N_("select current date"),
		N_("Ctrl+Space"),             N_("toggle personal data visibility"),
		N_("Arrows"),                 N_("change day"),
		N_("Ctrl+Up/Down"),           N_("scroll the contents in the day info panel"),
		N_("PageUp/PageDn"),          N_("change month"),
		N_("Home/End"),               N_("change year"),
		"a",                          N_("toggle calendars for the previous and next month"),
		"b",                          N_("day notes browser"),
		"c",                          N_("assign background color to day note"),
		"d",                          N_("date calculator"),
		"f",                          N_("show full-year calendar"),
		"g",                          N_("jump to date"),
		N_("Delete"),                 N_("remove day note")
	};

	const gchar *calendar_editor_keys_text[] = {
		N_("Alt+Arrows"),             N_("change day"),
		N_("Esc"),                    N_("close editor"),
		"Ctrl+b",                     N_("toggle bold"),
		"Ctrl+i",                     N_("toggle italic"),
		"Ctrl+u",                     N_("toggle underline"),
		"Ctrl+t",                     N_("toggle strikethrough"),
		"Ctrl+m",                     N_("toggle highlight")
	};

	const gchar *calendar_fy_keys_text[] = {
		N_("Arrows Up/Down"),         N_("change year"),
		"F1",                         N_("toggle alternative view"),
		"F2",                         N_("year info"),
		"F3",                         N_("set current year"),
		N_("Esc"),                    N_("close full-year calendar")
	};

#ifdef TASKS_ENABLED
	const gchar *tasks_keys_text[] = {
		N_("Alt+a, Insert"),          N_("add task"),
		N_("Alt+e, Ctrl+Enter"),      N_("edit task"),
		N_("Alt+r, Delete"),          N_("remove task"),
		"Ctrl+h",                     N_("toggle hidden tasks"),
		"Ctrl+l",                     N_("activate search field"),
		N_("Left, Right"),            N_("change category filter"),
		N_("Esc"),                    N_("close task info panel")
	};
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
	const gchar *contacts_keys_text[] = {
		N_("Insert"),                 N_("add contact"),
		N_("Ctrl+Enter"),             N_("edit contact"),
		N_("Delete"),                 N_("remove contact"),
		"Ctrl+l",                     N_("activate search field"),
		N_("Ctrl+Up/Down"),           N_("change search mode"),
		N_("Esc"),                    N_("close contact details panel")
	};
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
	const gchar *notes_selector_keys_text[] = {
		N_("Enter"),                  N_("open note"),
		N_("Insert"),                 N_("add note"),
		N_("Delete"),                 N_("remove note"),
		N_("Ctrl+Enter"),             N_("edit note name and category"),
		"Ctrl+l",                     N_("activate search field"),
		N_("Left, Right"),            N_("change category filter")
	};

const gchar *notes_editor_keys_text[] = {
		"Ctrl+w",                     N_("close note editor"),
		"Ctrl+s",                     N_("save note"),
		"Ctrl+f",                     N_("find text"),
		"Ctrl+b",                     N_("toggle bold"),
		"Ctrl+i",                     N_("toggle italic"),
		"Ctrl+u",                     N_("toggle underline"),
		"Ctrl+t",                     N_("toggle strikethrough"),
		"Ctrl+m",                     N_("toggle highlight"),
		"Ctrl+n",                     N_("clear selection attributes")
	};
#endif  /* NOTES_ENABLED */

	gchar *str;
	gint i;

	utl_gui_clear_text_buffer (appGUI->about_entry_buffer, &appGUI->about_entry_iter);

	str = g_strdup_printf ("\n%s\n\n", _("OSMO was designed keeping in mind the user convenience, so there are many key shortcuts. Here is the full list:"));
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

	/* ============================== */

	str = g_strdup_printf ("* %s\n", _("General"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (general_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (general_keys_text[i*2]), gettext (general_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);

	/* ============================== */

	str = g_strdup_printf ("* %s\n", _("Calendar"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (calendar_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (calendar_keys_text[i*2]), gettext (calendar_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}

	str = g_strdup_printf ("\t%s:\n", _("Note editor"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (calendar_editor_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (calendar_editor_keys_text[i*2]), gettext (calendar_editor_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}

	str = g_strdup_printf ("\t%s:\n", _("Full-year calendar"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (calendar_fy_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (calendar_fy_keys_text[i*2]), gettext (calendar_fy_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}

	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);

	/* ============================== */

#ifdef TASKS_ENABLED
	str = g_strdup_printf ("* %s\n", _("Tasks"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (tasks_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (tasks_keys_text[i*2]), gettext (tasks_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);
#endif  /* TASKS_ENABLED */

	/* ============================== */

#ifdef CONTACTS_ENABLED
	str = g_strdup_printf ("* %s\n", _("Contacts"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (contacts_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (contacts_keys_text[i*2]), gettext (contacts_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);
#endif  /* CONTACTS_ENABLED */

	/* ============================== */

#ifdef NOTES_ENABLED
	str = g_strdup_printf ("* %s\n", _("Notes"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	str = g_strdup_printf ("\t%s:\n", _("Selector"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (notes_selector_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (notes_selector_keys_text[i*2]), gettext (notes_selector_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}

	str = g_strdup_printf ("\t%s:\n", _("Editor"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	for (i = 0; i < G_N_ELEMENTS (notes_editor_keys_text) / 2; i++) {
		str = g_strdup_printf ("\t<%s> - %s\n", gettext (notes_editor_keys_text[i*2]), gettext (notes_editor_keys_text[i*2+1]));
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
		g_free (str);
	}
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);
#endif  /* NOTES_ENABLED */

	gtk_widget_grab_focus (GTK_WIDGET (appGUI->scrolled_window));
}

/* ========================================================================== */

static void
add_credits_section (const gchar *section_title, OsmoCreditEntry *credits, guint32 n_credits, GUI *appGUI)
{
	gint32 i;
	gchar *s;

	s = g_strdup_printf ("\n %s:\n", section_title);
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, s, -1, "bold", NULL);
	g_free (s);

	for (i = 0; i < n_credits; i++) {

		if (credits[i].tag) {
			if(strlen(credits[i].tag) > 2) {
				s = g_strdup_printf ("  [%s]", credits[i].tag);
			} else {
				s = g_strdup_printf ("  [%s]   ", credits[i].tag);
			}
		} else {
			s = g_strdup ("         ");
		}

		gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
												  &appGUI->about_entry_iter, s, -1, "fixed", NULL);
		g_free (s);

		s = g_strdup_printf ("  %s <", credits[i].name);
		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, s, -1);
		g_free (s);

		utl_gui_url_insert_link (&appGUI->about_links_list,
							     &appGUI->about_link_index, appGUI->about_textview,
							     &appGUI->about_entry_iter, config.link_color, NULL, credits[i].email, FALSE, appGUI);

		gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, ">\n", -1);
	}
}

/* ========================================================================== */

static void
display_about (GUI *appGUI)
{
	gchar *str;

	utl_gui_url_remove_links (&appGUI->about_links_list, &appGUI->about_link_index);
	utl_gui_clear_text_buffer (appGUI->about_entry_buffer, &appGUI->about_entry_iter);

	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);
	str = g_strdup_printf ("%s\n", _("A handy personal organizer"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "big", "center", NULL);
	g_free (str);
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);

	utl_gui_url_insert_link (&appGUI->about_links_list, &appGUI->about_link_index,
	                         appGUI->about_textview, &appGUI->about_entry_iter, config.link_color,
	                         NULL, OSMO_WEBSITE, TRUE, appGUI);
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);

	str = g_strdup_printf ("\n(%s %s, %s)\n\n", _("compiled on"), __DATE__, __TIME__);
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "center", "italic", NULL);
	g_free (str);

	add_credits_section (_("Programming"), credits_programming, G_N_ELEMENTS (credits_programming), appGUI);
	add_credits_section (_("Graphics"), credits_graphics, G_N_ELEMENTS (credits_graphics), appGUI);
	add_credits_section (_("Contributors"), credits_contributors, G_N_ELEMENTS (credits_contributors), appGUI);
	add_credits_section (_("Translators"), credits_translators, G_N_ELEMENTS (credits_translators), appGUI);

	/* Links */

	str = g_strdup_printf ("\n %s:\n", _("Mailing lists"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\t      ", -1);
	utl_gui_url_insert_link (&appGUI->about_links_list, &appGUI->about_link_index,
	                         appGUI->about_textview, &appGUI->about_entry_iter, config.link_color,
	                         NULL, MAILING_LIST, FALSE, appGUI);
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);

	str = g_strdup_printf ("\n %s:\n", _("Bug tracker"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\t      ", -1);
	utl_gui_url_insert_link (&appGUI->about_links_list, &appGUI->about_link_index,
	                         appGUI->about_textview, &appGUI->about_entry_iter, config.link_color,
	                         NULL, BUG_TRACKER, FALSE, appGUI);
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);

	str = g_strdup_printf ("\n %s:\n", _("Feature requests"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\t      ", -1);
	utl_gui_url_insert_link (&appGUI->about_links_list, &appGUI->about_link_index,
	                         appGUI->about_textview, &appGUI->about_entry_iter, config.link_color,
	                         NULL, FEATURE_REQUESTS, FALSE, appGUI);
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, "\n", -1);


	/* Available modules */

	str = g_strdup_printf ("\n %s:\n", _("Available modules"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

	str = g_strdup_printf ("     [+]\t%s\n", _("Calendar"));
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef TASKS_ENABLED
	str = g_strdup_printf ("     [+]\t%s\n", _("Tasks"));
#else
	str = g_strdup_printf ("     [-]\t%s\n", _("Tasks"));
#endif  /* TASKS_ENABLED */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef CONTACTS_ENABLED
	str = g_strdup_printf ("     [+]\t%s\n", _("Contacts"));
#else
	str = g_strdup_printf ("     [-]\t%s\n", _("Contacts"));
#endif  /* CONTACTS_ENABLED */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef NOTES_ENABLED
	str = g_strdup_printf ("     [+]\t%s\n", _("Notes"));
#else
	str = g_strdup_printf ("     [-]\t%s\n", _("Notes"));
#endif  /* NOTES_ENABLED */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);


	/* Compiled-in features */

	str = g_strdup_printf ("\n %s:\n", _("Compiled-in features"));
	gtk_text_buffer_insert_with_tags_by_name (appGUI->about_entry_buffer,
	                                          &appGUI->about_entry_iter, str, -1, "bold", NULL);
	g_free (str);

#ifdef HAVE_LIBICAL
	str = g_strdup_printf ("     [+]\t%s (libical)\n", _("iCalendar support"));
#else
	str = g_strdup_printf ("     [-]\t%s (libical)\n", _("iCalendar support"));
#endif  /* HAVE_LIBICAL */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef HAVE_LIBGRINGOTTS
	str = g_strdup_printf ("     [+]\t%s (libgringotts)\n", _("Encrypted notes support"));
#else
	str = g_strdup_printf ("     [-]\t%s (libgringotts)\n", _("Encrypted notes support"));
#endif  /* HAVE_LIBGRINGOTTS */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef HAVE_LIBGTKHTML
	str = g_strdup_printf ("     [+]\t%s (libgtkhtml)\n", _("HTML view required for Contacts module"));
#else
	str = g_strdup_printf ("     [-]\t%s (libgtkhtml)\n", _("HTML view required for Contacts module"));
#endif  /* HAVE_LIBGTKHTML */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef HAVE_LIBSYNCML
	str = g_strdup_printf ("     [+]\t%s (libsyncml)\n", _("SyncML enabled device support"));
#else
	str = g_strdup_printf ("     [-]\t%s (libsyncml)\n", _("SyncML enabled device support"));
#endif  /* HAVE_LIBSYNCML */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#if defined(BACKUP_SUPPORT) && defined(HAVE_LIBGRINGOTTS)
	str = g_strdup_printf ("     [+]\t%s (libtar + libgringotts)\n", _("Backup support"));
#else
	str = g_strdup_printf ("     [-]\t%s (libtar + libgringotts)\n", _("Backup support"));
#endif  /* BACKUP_SUPPORT && HAVE_LIBGRINGOTTS */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef PRINTING_SUPPORT
	str = g_strdup_printf ("     [+]\t%s\n", _("Printing support"));
#else
	str = g_strdup_printf ("     [-]\t%s\n", _("Printing support"));
#endif  /* PRINTING_SUPPORT */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

#ifdef HAVE_GTKSPELL
	str = g_strdup_printf ("     [+]\t%s (gtkspell)\n", _("Spell checker support"));
#else
	str = g_strdup_printf ("     [-]\t%s (gtkspell)\n", _("Spell checker support"));
#endif  /* HAVE_GTKSPELL */
	gtk_text_buffer_insert (appGUI->about_entry_buffer, &appGUI->about_entry_iter, str, -1);
	g_free (str);

	gtk_widget_grab_focus (GTK_WIDGET (appGUI->scrolled_window));
}

/* ========================================================================== */

static void
about_switch_buttons (gboolean left, GUI *appGUI)
{
	if (left == TRUE && appGUI->about_counter > 0) {
		--appGUI->about_counter;
	} else if (left == FALSE && appGUI->about_counter < 2) {
		appGUI->about_counter++;
	}

	if (appGUI->about_counter == 0){
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->about_radiobutton), TRUE);
		g_signal_emit_by_name (G_OBJECT (appGUI->about_radiobutton), "released");
	} else if (appGUI->about_counter == 1) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->help_radiobutton), TRUE);
		g_signal_emit_by_name (G_OBJECT (appGUI->help_radiobutton), "released");
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->license_radiobutton), TRUE);
		g_signal_emit_by_name (G_OBJECT (appGUI->license_radiobutton), "released");
	}
}

/* ========================================================================== */

static void
button_released_cb (GtkButton *button, GUI *appGUI)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->about_radiobutton)) == TRUE) {

		display_about (appGUI);
		appGUI->about_counter = 0;

	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->help_radiobutton)) == TRUE) {

		display_help (appGUI);
		appGUI->about_counter = 1;

	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (appGUI->license_radiobutton)) == TRUE) {

		display_license (appGUI);
		appGUI->about_counter = 2;

	}
}

/* ========================================================================== */

void
gui_create_about (GtkWidget *vbox_top, GUI *appGUI)
{
	GtkWidget *hbox = NULL, *vbox = NULL;
	GtkWidget *label;
	GtkWidget *hseparator;
	GtkWidget *logo_area;
	GdkPixbuf *logo, *logo_scaled = NULL;
	GtkWidget *viewport;
	GtkWidget *hbuttonbox = NULL;
	GSList    *radiobutton_group = NULL;
	char *str;

	appGUI->about_counter = 0;
	appGUI->about_vbox = GTK_BOX (vbox_top);

	logo_area = gtk_image_new ();
	logo = gdk_pixbuf_new_from_inline (-1, osmo_logo, FALSE, NULL);
	gtk_widget_show (logo_area);
	gtk_box_pack_start (GTK_BOX (vbox_top), logo_area, FALSE, TRUE, 0);

	if (appGUI->tiny_gui == TRUE) {
		logo_scaled = gdk_pixbuf_scale_simple (logo,
		                                       gdk_pixbuf_get_width (logo) / 2,
		                                       gdk_pixbuf_get_height (logo) / 2,
		                                       GDK_INTERP_HYPER);
	}

#ifndef REV
	str = g_strdup_printf ("%s %s", _("version"), VERSION);
#else
	str = g_strdup_printf ("%s %d", _("SVN revision"), REV);
#endif

	label = gtk_label_new (str);
	g_free (str);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (vbox_top), label, FALSE, TRUE, 4);

	if (logo_scaled != NULL) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (logo_area), logo_scaled);
		g_object_unref (logo_scaled);
	} else {
		gtk_image_set_from_pixbuf (GTK_IMAGE (logo_area), logo);
		g_object_unref (logo);
	}


	/*--------------------------------------------------------------------------*/

	vbox = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (vbox_top), vbox, FALSE, TRUE, 0);

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, TRUE, 0);

	if (appGUI->tiny_gui == TRUE) {
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_widget_show (hbox);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	} else {
		hbuttonbox = gtk_hbutton_box_new ();
		gtk_widget_show (hbuttonbox);
		gtk_box_pack_start (GTK_BOX (vbox), hbuttonbox, FALSE, TRUE, 0);
		gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	}

	if (appGUI->tiny_gui == TRUE) {
		appGUI->about_radiobutton = utl_gui_stock_label_radio_button (NULL, OSMO_STOCK_ABOUT, GTK_ICON_SIZE_LARGE_TOOLBAR);
	} else {
		appGUI->about_radiobutton = utl_gui_stock_label_radio_button (_("About"), OSMO_STOCK_ABOUT, GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->about_radiobutton);
	}
	gtk_widget_show (appGUI->about_radiobutton);

	gtk_button_set_relief (GTK_BUTTON (appGUI->about_radiobutton), GTK_RELIEF_NONE);
	GTK_WIDGET_UNSET_FLAGS (appGUI->about_radiobutton, GTK_CAN_FOCUS);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (appGUI->about_radiobutton), radiobutton_group);
	radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (appGUI->about_radiobutton));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (appGUI->about_radiobutton), TRUE);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (appGUI->about_radiobutton), FALSE);

	g_signal_connect (G_OBJECT (appGUI->about_radiobutton), "released",
	                  G_CALLBACK (button_released_cb), appGUI);

	if (appGUI->tiny_gui == TRUE) {
		appGUI->help_radiobutton = utl_gui_stock_label_radio_button (NULL, OSMO_STOCK_HELP, GTK_ICON_SIZE_LARGE_TOOLBAR);
	} else {
		appGUI->help_radiobutton = utl_gui_stock_label_radio_button (_("Key shortcuts"), OSMO_STOCK_HELP, GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->help_radiobutton);
	}
	gtk_widget_show (appGUI->help_radiobutton);
	gtk_button_set_relief (GTK_BUTTON (appGUI->help_radiobutton), GTK_RELIEF_NONE);
	GTK_WIDGET_UNSET_FLAGS (appGUI->help_radiobutton, GTK_CAN_FOCUS);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (appGUI->help_radiobutton), radiobutton_group);
	radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (appGUI->help_radiobutton));
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (appGUI->help_radiobutton), FALSE);

	g_signal_connect (G_OBJECT (appGUI->help_radiobutton), "released",
	                  G_CALLBACK (button_released_cb), appGUI);

	if (appGUI->tiny_gui == TRUE) {
		appGUI->license_radiobutton = utl_gui_stock_label_radio_button (NULL, OSMO_STOCK_LICENSE, GTK_ICON_SIZE_LARGE_TOOLBAR);
	} else {
		appGUI->license_radiobutton = utl_gui_stock_label_radio_button (_("License"), OSMO_STOCK_LICENSE, GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_container_add (GTK_CONTAINER (hbuttonbox), appGUI->license_radiobutton);
	}
	gtk_widget_show (appGUI->license_radiobutton);
	gtk_button_set_relief (GTK_BUTTON (appGUI->license_radiobutton), GTK_RELIEF_NONE);
	GTK_WIDGET_UNSET_FLAGS (appGUI->license_radiobutton, GTK_CAN_FOCUS);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (appGUI->license_radiobutton), radiobutton_group);
	radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (appGUI->license_radiobutton));
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (appGUI->license_radiobutton), FALSE);

	g_signal_connect (G_OBJECT (appGUI->license_radiobutton), "released",
	                  G_CALLBACK (button_released_cb), appGUI);

	if (appGUI->tiny_gui == TRUE) {
		gtk_box_pack_end (GTK_BOX (hbox), appGUI->license_radiobutton, FALSE, TRUE, 0);
		gtk_box_pack_end (GTK_BOX (hbox), appGUI->help_radiobutton, FALSE, TRUE, 0);
		gtk_box_pack_end (GTK_BOX (hbox), appGUI->about_radiobutton, FALSE, TRUE, 0);
	}

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, TRUE, 0);

	/*--------------------------------------------------------------------------*/

	appGUI->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox_top), appGUI->scrolled_window, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (appGUI->scrolled_window),
	                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (appGUI->scrolled_window);

	viewport = gtk_viewport_new (NULL, NULL);
	gtk_widget_show (viewport);
	gtk_container_set_border_width (GTK_CONTAINER (viewport), 0);
	gtk_container_add (GTK_CONTAINER (appGUI->scrolled_window), viewport);

	appGUI->about_entry_buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_get_iter_at_offset (appGUI->about_entry_buffer, &appGUI->about_entry_iter, 0);
	gtk_text_buffer_create_tag (appGUI->about_entry_buffer, "fixed", "family", "monospace", NULL);
	gtk_text_buffer_create_tag (appGUI->about_entry_buffer, "bold", "weight", PANGO_WEIGHT_ULTRABOLD, NULL);
	gtk_text_buffer_create_tag (appGUI->about_entry_buffer, "big", "size", 16 * PANGO_SCALE, NULL);
	gtk_text_buffer_create_tag (appGUI->about_entry_buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag (appGUI->about_entry_buffer, "center", "justification", GTK_JUSTIFY_CENTER, NULL);
	gtk_text_buffer_get_iter_at_offset (appGUI->about_entry_buffer, &appGUI->about_entry_iter, 0);

	appGUI->about_textview = gtk_text_view_new_with_buffer (appGUI->about_entry_buffer);
	gtk_container_set_border_width (GTK_CONTAINER (appGUI->about_textview), 1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (appGUI->about_textview), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (appGUI->about_textview), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (appGUI->about_textview), GTK_WRAP_WORD);
	gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (appGUI->about_textview), 2);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (appGUI->about_textview), 6);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (appGUI->about_textview), 6);
	gtk_widget_show (appGUI->about_textview);
	gtk_container_add (GTK_CONTAINER (viewport), appGUI->about_textview);

	utl_gui_url_setup (&appGUI->about_links_list, &appGUI->about_link_index, appGUI->about_textview, appGUI);

	display_about (appGUI);
}

/* ========================================================================== */

