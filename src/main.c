
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

#include "gui.h"
#include "calendar.h"
#include "calendar_utils.h"
#include "utils.h"
#include "utils_date.h"
#include "utils_gui.h"
#include "i18n.h"
#include "check_events.h"
#include "options_prefs.h"

#ifdef BACKUP_SUPPORT
#include "backup.h"
#endif  /* BACKUP_SUPPORT */

#define RUN_FLAG_FILE   "lock"

/*------------------------------------------------------------------------------*/

int main(int argc, char **argv) {

GUI *appGUI = NULL;
CALENDAR *cal = NULL;

#ifdef TASKS_ENABLED
TASKS *tsk = NULL;
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
CONTACTS *cnt = NULL;
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
NOTES *nte = NULL;
#endif  /* NOTES_ENABLED */

OPTIONS *opt = NULL;
GtkWidget *info_dialog;
gchar tmpbuf[BUFFER_SIZE];
gint response = -1;
#ifndef WIN32
struct flock *s_lock = NULL;
#endif /* WIN32 */
int fhandle = 0;

GOptionContext *cmd_context;
gboolean cmd_calendar = FALSE;
gboolean cmd_check_events = FALSE;
gint cmd_check_ndays_events = 0;
gchar *cmd_cfg_path = NULL;
gboolean cmd_tiny_gui = FALSE;

GOptionEntry cmd_options[] = {
    { "calendar",   'c', 0, G_OPTION_ARG_NONE, &cmd_calendar, N_("Show small calendar window"), NULL  },
    { "check",      'e', 0, G_OPTION_ARG_NONE, &cmd_check_events, N_("Check for events since last run"), NULL  },
    { "days",       'd', 0, G_OPTION_ARG_INT, &cmd_check_ndays_events, N_("Number of days to check forward for events (default: 0)"), NULL  },
    { "config",     's', 0, G_OPTION_ARG_STRING, &cmd_cfg_path, N_("Set absolute path for settings and data files"), "PATH"  },
    { "tinygui",    't', 0, G_OPTION_ARG_NONE, &cmd_tiny_gui, N_("Modify GUI to use Osmo on low resolutions"), NULL  },
    { NULL }
};

    appGUI = g_new0 (GUI, 1);
    g_return_val_if_fail (appGUI != NULL, -1);

    cal = g_new0 (CALENDAR, 1);
    g_return_val_if_fail (cal != NULL, -1);

#ifdef TASKS_ENABLED
    tsk = g_new0 (TASKS, 1);
    g_return_val_if_fail (tsk != NULL, -1);
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    cnt = g_new0 (CONTACTS, 1);
    g_return_val_if_fail (cnt != NULL, -1);
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
    nte = g_new0 (NOTES, 1);
    g_return_val_if_fail (nte != NULL, -1);
#endif  /* NOTES_ENABLED */

    opt = g_new0 (OPTIONS, 1);
    g_return_val_if_fail (opt != NULL, -1);

    /* register modules */
    appGUI->cal = cal;

#ifdef TASKS_ENABLED
    appGUI->tsk = tsk;
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    appGUI->cnt = cnt;
#endif  /* CONTACTS_ENABLED */

#ifdef NOTES_ENABLED
    appGUI->nte = nte;
#endif  /* NOTES_ENABLED */

    appGUI->opt = opt;

    appGUI->run_date = utl_date_get_current_julian ();
    appGUI->run_time = utl_time_get_current_seconds ();
    appGUI->key_counter = 0;

    /* default values */
    appGUI->hovering_over_link = FALSE;
    appGUI->hand_cursor = NULL;
    appGUI->regular_cursor = NULL;
    appGUI->gui_url_tag = NULL;
    appGUI->trayicon_popup_menu = NULL;

    appGUI->calendar_only = FALSE;
	appGUI->cal->datecal_bio = FALSE;
    appGUI->check_events = FALSE;
    appGUI->tiny_gui = FALSE;
    appGUI->save_status = 0;
    appGUI->print_font_size = 10;
    g_sprintf (appGUI->version, "%02d%02d%02d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);

	appGUI->cal->last_selected_year = -1;
	appGUI->cal->date = utl_date_new_current ();
	appGUI->about_links_list = NULL;

#ifdef TASKS_ENABLED
    appGUI->tsk->next_id = 1;
    appGUI->tsk->tasks_panel_status = FALSE;
    appGUI->tsk->tasks_filter_disabled = FALSE;
    appGUI->tsk->tasks_list_store = NULL;
    appGUI->tsk->notifications = NULL;
#endif  /* TASKS_ENABLED */

#ifdef CONTACTS_ENABLED
    appGUI->cnt->contacts_uim_widget = NULL;
    appGUI->cnt->contacts_panel_status = FALSE;
    appGUI->cnt->contacts_filter_disabled = TRUE;
    appGUI->cnt->export_button = NULL;
    appGUI->cnt->output_file_entry = NULL;
	appGUI->cnt->write_flag = TRUE;
#endif  /* CONTACTS_ENABLED */

    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain (PACKAGE);

    if (argc == 2 && (!strncmp(argv[1], "--help", 6) || (argv[1][0] == '-' && argv[1][1] == '?'))) {
        fprintf(stderr, "\nOSMO v%s (%s)\n", VERSION, _("handy personal organizer"));
        fprintf(stderr, "Copyright (c) 2007-2008 Tomasz Maka <pasp@users.sourceforge.net>\n\n");
    }

    cmd_context = g_option_context_new(NULL);
    g_option_context_add_main_entries(cmd_context, cmd_options, TRANSLATION_DOMAIN);
    g_option_context_parse(cmd_context, &argc, &argv, NULL);
    g_option_context_free(cmd_context);

    appGUI->calendar_only = cmd_calendar;
    appGUI->check_events = cmd_check_events;
    appGUI->check_ndays_events = cmd_check_ndays_events;
    appGUI->config_path = cmd_cfg_path;
    appGUI->tiny_gui = cmd_tiny_gui;

    if (prefs_get_config_filename (CONFIG_FILENAME, appGUI) == NULL) {
        fprintf(stderr, "%s\n", _("ERROR: Cannot create config files"));
        goto finish;
    }

#if defined(BACKUP_SUPPORT) && defined(HAVE_LIBGRINGOTTS)
	backup_restore_run (appGUI);
#endif  /* BACKUP_SUPPORT & HAVE_LIBGRINGOTTS */

#ifndef WIN32
    s_lock = g_new0 (struct flock, 1);
	s_lock->l_type = F_WRLCK;
	s_lock->l_whence = SEEK_SET;
	s_lock->l_start = 0;
	s_lock->l_len = 0;
#endif /* WIN32 */

    prefs_read_config (appGUI);

  	g_thread_init (NULL);
    gtk_init (&argc, &argv);

	if (appGUI->calendar_only == FALSE) {

		close(creat(prefs_get_config_filename (RUN_FLAG_FILE, appGUI), S_IRUSR | S_IWUSR));     /* create lock file */

		fhandle = open(prefs_get_config_filename (RUN_FLAG_FILE, appGUI), O_RDWR);
#ifndef WIN32
		if (fhandle) {
			if (fcntl(fhandle, F_SETLK, s_lock) == -1) {
				close (fhandle);

				g_snprintf (tmpbuf, BUFFER_SIZE, "%s %s\n\n%s",
							_("Another copy of OSMO is already running."),
							_("Simultaneously use two or more copies of OSMO can be a cause of data loss."),
							_("Do you really want to continue?"));

				info_dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW_TOPLEVEL, GTK_DIALOG_MODAL,
													  GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, tmpbuf, NULL);

				gtk_window_set_title(GTK_WINDOW(info_dialog), _("Warning"));
				gtk_window_set_position(GTK_WINDOW(info_dialog), GTK_WIN_POS_CENTER);
				gtk_widget_show (info_dialog);

				response = gtk_dialog_run(GTK_DIALOG(info_dialog));
				gtk_widget_destroy(info_dialog);

				if (response == GTK_RESPONSE_NO || response == GTK_RESPONSE_DELETE_EVENT) {
					goto finish;
				}
			}
		}
#endif /* WIN32 */
	}


#ifdef HAVE_LIBNOTIFY
#ifdef TASKS_ENABLED
	notify_init("Osmo notification");
#endif  /* TASKS_ENABLED */
#endif  /* HAVE_LIBNOTIFY */

    if (appGUI->calendar_only != TRUE) {
        /* setup timer at 1000ms (1s) interval */
		g_timeout_add (1000, (GtkFunction) time_handler, appGUI);
    }

    config.run_counter++;

	g_snprintf (tmpbuf, BUFFER_SIZE, "Osmo %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	g_set_application_name (tmpbuf);
	gtk_window_set_default_icon_name ("osmo");

    if (gui_create_window (appGUI) == TRUE) {
		gdk_threads_enter ();
		gtk_main ();
		gdk_threads_leave ();
    }

    if (appGUI->calendar_only == FALSE) {
        config.lastrun_date = utl_date_get_current_julian ();
        config.lastrun_time = utl_time_get_current_seconds ();
    }

    if (appGUI->check_events == FALSE) {
        prefs_write_config (appGUI);
    }

    if (fhandle && appGUI->calendar_only == FALSE) {
 #ifndef WIN32
    	s_lock->l_type = F_UNLCK;
    	fcntl(fhandle, F_SETLK, s_lock);
 #endif /* WIN32 */
        close (fhandle);
    }

finish:
	g_date_free (appGUI->cal->date);
	utl_gui_url_remove_links (&appGUI->about_links_list, NULL);
	gdk_cursor_unref (appGUI->hand_cursor);
	gdk_cursor_unref (appGUI->regular_cursor);
	g_free (opt);

#ifdef NOTES_ENABLED
	g_free (nte);
#endif  /* NOTES_ENABLED */

#ifdef CONTACTS_ENABLED
	g_free (cnt);
#endif  /* CONTACTS_ENABLED */

#ifdef TASKS_ENABLED
	g_free (tsk);
#endif  /* TASKS_ENABLED */

	g_free (cal);
	g_free (appGUI);
#ifndef WIN32
	g_free (s_lock);
#endif /* WIN32 */
	return 0;
}

/*------------------------------------------------------------------------------*/

