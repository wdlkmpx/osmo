
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


#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/mman.h>
#endif /* WIN32 */

#include "i18n.h"
#include "backup.h"
#include "gui.h"
#include "notes_preferences_gui.h"
#include "utils.h"
#include "utils_gui.h"
#include "options_prefs.h"

#if defined(BACKUP_SUPPORT) && defined(HAVE_LIBGRINGOTTS)

#include <libgringotts.h>
#include <libtar.h>

/*------------------------------------------------------------------------------*/

void
add_all_files_to_list (gchar *directory, gboolean subdir, GUI *appGUI) {

GDir *dir_path = NULL;
const gchar *item_name = NULL;
gchar *full_filename = NULL, *fitem = NULL, *sdir = NULL;
gchar separator[2] = { G_DIR_SEPARATOR, '\0' };

	dir_path = g_dir_open (directory, 0, NULL);
	g_return_if_fail (dir_path != NULL);

    while ((item_name = g_dir_read_name (dir_path)) != NULL) {

		full_filename = g_strconcat (directory, separator, item_name, NULL);

		if (g_utf8_collate (item_name, BACKUP_TEMPLATE) == 0) {     /* don't include backup file! */
			g_free (full_filename);
			continue;
		}

		if (g_file_test (full_filename, G_FILE_TEST_IS_SYMLINK)) {  /* ignore symlinks */
			g_free (full_filename);
			continue;
		}

		if (g_file_test (full_filename, G_FILE_TEST_IS_DIR)) {
			add_all_files_to_list (full_filename, TRUE, appGUI);
			g_free (full_filename);
			continue;
		}

		if (subdir) {
			sdir = g_path_get_basename (directory);
			fitem = g_strconcat (sdir, separator, item_name, NULL);
			g_free (sdir);
		} else {
			fitem = g_strdup (item_name);
		}
		g_free (full_filename);

        appGUI->file_list = g_slist_append (appGUI->file_list, fitem);
	}

	g_dir_close (dir_path);
}

/*------------------------------------------------------------------------------*/

void
add_files_to_list (gchar *directory, gchar *extension, GUI *appGUI) {

GDir *dir_path = NULL;
const gchar *item_name = NULL;
gchar *full_filename = NULL, *fitem = NULL;
gchar separator[2] = { G_DIR_SEPARATOR, '\0' };

	dir_path = g_dir_open (directory, 0, NULL);
	g_return_if_fail (dir_path != NULL);

    while ((item_name = g_dir_read_name (dir_path)) != NULL) {

		full_filename = g_strconcat (directory, separator, item_name, NULL);

		if (!g_file_test (full_filename, G_FILE_TEST_IS_REGULAR)) {
			g_free (full_filename);
			continue;
		}

		if (!g_str_has_suffix (item_name, extension)) {
			g_free (full_filename);
			continue;
		}

		if (g_str_has_suffix (item_name, "osm")) {
			g_free (full_filename);
			full_filename = g_strconcat ("notes", separator, item_name, NULL);
			fitem = g_strdup (full_filename);
		} else {
			fitem = g_strdup (item_name);
		}

		g_free (full_filename);

        appGUI->file_list = g_slist_append (appGUI->file_list, fitem);
	}

	g_dir_close (dir_path);
}

/*------------------------------------------------------------------------------*/

void
backup_create (GUI *appGUI) {

gchar *home_dirname = NULL;
gchar *fitem = NULL;
gint i, ret, p1len, p2len, tErr;
GRG_CTX context;
GRG_KEY keyholder;
gchar *filename, *password, *bpass1, *bpass2, *tmp_filename;
gchar tmpbuf[BUFFER_SIZE];
gchar *contents;
gsize len;
GtkWidget *dialog, *passwd_dialog;
GtkWidget *vbox1, *alignment, *frame, *label;
GtkWidget *bck_p1_entry, *bck_p2_entry;

TAR *tArch;

	/* select filename and password */

    dialog = gtk_file_chooser_dialog_new (_("Save backup"),
                                          GTK_WINDOW(appGUI->main_window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(dialog), FALSE);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(dialog), 
									   utl_add_timestamp_to_filename ("osmobackup", "bck"));
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(dialog), TRUE);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if (ret == GTK_RESPONSE_CANCEL || ret == GTK_RESPONSE_DELETE_EVENT) {
        gtk_widget_destroy(dialog);
		return;
	}

	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);

	if (utl_gui_check_overwrite_file (filename, appGUI->main_window, appGUI) != 0) {
        return;
    } else {
        g_unlink (filename);
    }
	
    passwd_dialog = gtk_dialog_new_with_buttons (_("Password protection"), 
												 GTK_WINDOW(appGUI->main_window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
												 NULL);

	gtk_window_set_position (GTK_WINDOW(passwd_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size (GTK_WINDOW(passwd_dialog), 400, -1);
    gtk_dialog_set_default_response (GTK_DIALOG(passwd_dialog), GTK_RESPONSE_ACCEPT);

	vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(passwd_dialog)->vbox), vbox1, TRUE, TRUE, 16);

	frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);
	
    bck_p1_entry = gtk_entry_new ();
    gtk_widget_show (bck_p1_entry);
    gtk_container_add (GTK_CONTAINER (alignment), bck_p1_entry);
    gtk_entry_set_invisible_char (GTK_ENTRY (bck_p1_entry), 8226);
    gtk_entry_set_visibility (GTK_ENTRY (bck_p1_entry), FALSE);

    sprintf (tmpbuf, "<b>%s:</b>", _("Enter password"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);

    bck_p2_entry = gtk_entry_new ();
    gtk_widget_show (bck_p2_entry);
    gtk_container_add (GTK_CONTAINER (alignment), bck_p2_entry);
    gtk_entry_set_invisible_char (GTK_ENTRY (bck_p2_entry), 8226);
    gtk_entry_set_visibility (GTK_ENTRY (bck_p2_entry), FALSE);

    sprintf (tmpbuf, "<b>%s:</b>", _("Re-enter password"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	while (1) {

		ret = gtk_dialog_run(GTK_DIALOG(passwd_dialog)); 

		if (ret == GTK_RESPONSE_CANCEL || ret == GTK_RESPONSE_DELETE_EVENT) {
			gtk_widget_destroy(passwd_dialog);
			return;
		}

		bpass1 = g_strdup(gtk_entry_get_text(GTK_ENTRY(bck_p1_entry)));
		p1len = strlen(bpass1);
		bpass2 = g_strdup(gtk_entry_get_text(GTK_ENTRY(bck_p2_entry)));
		p2len = strlen(bpass2);

		if (p1len == 0 && p2len == 0) {
			utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Please enter the password"), GTK_WINDOW(passwd_dialog));
			gtk_widget_grab_focus (bck_p1_entry);
			g_free(bpass1);
			g_free(bpass2);
			continue;
		} else if (p1len != p2len) {
			utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Passwords do not match!"), GTK_WINDOW(passwd_dialog));
			gtk_widget_grab_focus (bck_p1_entry);
			g_free(bpass1);
			g_free(bpass2);
			continue;
		}

		if (g_utf8_collate (bpass1, bpass2) != 0) {
			utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Passwords do not match!"), GTK_WINDOW(passwd_dialog));
			gtk_widget_grab_focus (bck_p1_entry);
			g_free(bpass1);
			g_free(bpass2);
			continue;
		} else {
			g_free(bpass1);
			g_free(bpass2);
			break;
		}

	}

	password = g_strdup(gtk_entry_get_text(GTK_ENTRY(bck_p1_entry)));
	gtk_widget_destroy(passwd_dialog);

	/* generate file list */

	appGUI->file_list = NULL;

	home_dirname = g_strdup (prefs_get_config_dir(appGUI));
	add_all_files_to_list (home_dirname, FALSE, appGUI);
	g_free (home_dirname);

	/* create backup file */

	tmp_filename = g_strdup (prefs_get_config_filename ("backup.dat", appGUI));

	home_dirname = g_get_current_dir();
	g_chdir (prefs_get_config_dir(appGUI));     /* change directory to cfg home */
	g_unlink (tmp_filename);

	tErr = i = 0;

	if (tar_open (&tArch, tmp_filename, NULL, O_WRONLY | O_CREAT | O_EXCL, 0644, TAR_GNU) != -1) {

		while ((fitem = g_slist_nth_data (appGUI->file_list, i)) != NULL) {
			if (tar_append_file (tArch, fitem, fitem) != 0) {
				tErr = -1;
				break;
			}
			i++;
		}

	} else {
		tErr = -1;
	}

	g_chdir (home_dirname);
	g_free (home_dirname);

	if (tErr == -1) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Cannot create backup!"), GTK_WINDOW(appGUI->main_window));
		return;
	}

	if (g_file_get_contents (tmp_filename, &contents, &len, NULL) == FALSE) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Cannot create backup!"), GTK_WINDOW(appGUI->main_window));
		return;
	}

	context = grg_context_initialize_defaults ((unsigned char*) "BCK");
	keyholder = grg_key_gen ((unsigned char*) password, -1);

	if (keyholder == NULL || context == NULL) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Cannot create backup!"), GTK_WINDOW(appGUI->main_window));
		return;
	}

	grg_ctx_set_crypt_algo (context, get_enc_algorithm_value());
	grg_ctx_set_hash_algo (context, get_enc_hashing_value());
	grg_ctx_set_comp_algo (context, get_comp_algorithm_value());
	grg_ctx_set_comp_ratio (context, get_comp_ratio_value());
    grg_encrypt_file (context, keyholder, (unsigned char*) filename, (guchar *) contents, len);
    grg_free (context, contents, len);
	grg_key_free (context, keyholder);
	grg_context_free (context);

	g_unlink (tmp_filename);

	/* free strings */

	g_free (tmp_filename);

	g_free(filename);
	g_free(password);

	/* free list */

	if (appGUI->file_list != NULL) {
		g_slist_foreach (appGUI->file_list, (GFunc) g_free, NULL);
		g_slist_free (appGUI->file_list);
		appGUI->file_list = NULL;
	}
	
	utl_gui_create_dialog (GTK_MESSAGE_INFO, _("Backup file saved successfully!"), GTK_WINDOW(appGUI->main_window));
}

/*------------------------------------------------------------------------------*/

void
backup_restore (GUI *appGUI) {

gchar *filename, *tmp_filename;
gint ret, tErr, passlen;
GtkFileFilter *filter;
GRG_CTX context;
GRG_KEY keyholder;
unsigned char *arch;
long arch_len;
gchar *contents, *password;
gchar tmpbuf[BUFFER_SIZE];
GtkWidget *dialog, *passwd_dialog, *pass_entry;
GtkWidget *vbox1, *alignment, *frame, *label;

    dialog = gtk_file_chooser_dialog_new (_("Open backup file"),
                                          GTK_WINDOW(appGUI->main_window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(dialog), FALSE);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(dialog), TRUE);

	filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.[bB][cC][kK]");
    gtk_file_filter_set_name(GTK_FILE_FILTER(filter), _("Osmo backup files (*.bck)"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if (ret == GTK_RESPONSE_CANCEL || ret == GTK_RESPONSE_DELETE_EVENT) {
        gtk_widget_destroy(dialog);
		return;
	}

	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);
	
	tErr = -1;

	if (g_file_get_contents (filename, &contents, NULL, NULL) == TRUE) {

		if (contents[0] == 'B' && contents[1] == 'C' && contents[2] == 'K' && g_ascii_isalnum(contents[3])) {
			tErr = 0;
		}

		g_free (contents);
	}

	if (tErr == -1) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("This is not Osmo backup file"), GTK_WINDOW(appGUI->main_window));
		g_free (filename);
		return;
	}

    passwd_dialog = gtk_dialog_new_with_buttons (_("Password protection"), 
												 GTK_WINDOW(appGUI->main_window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
												 NULL);

	gtk_window_set_position (GTK_WINDOW(passwd_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size (GTK_WINDOW(passwd_dialog), 400, -1);
    gtk_dialog_set_default_response (GTK_DIALOG(passwd_dialog), GTK_RESPONSE_ACCEPT);

	vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(passwd_dialog)->vbox), vbox1, TRUE, TRUE, 8);

	frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox1), frame, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment);
    gtk_container_add (GTK_CONTAINER (frame), alignment);
    gtk_container_set_border_width (GTK_CONTAINER (alignment), 4);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);
	
    pass_entry = gtk_entry_new ();
    gtk_widget_show (pass_entry);
    gtk_container_add (GTK_CONTAINER (alignment), pass_entry);
    gtk_entry_set_invisible_char (GTK_ENTRY (pass_entry), 8226);
    gtk_entry_set_visibility (GTK_ENTRY (pass_entry), FALSE);

    sprintf (tmpbuf, "<b>%s:</b>", _("Enter password"));
    label = gtk_label_new (tmpbuf);
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	while (1) {

		ret = gtk_dialog_run(GTK_DIALOG(passwd_dialog)); 

		if (ret == GTK_RESPONSE_CANCEL || ret == GTK_RESPONSE_DELETE_EVENT) {
			gtk_widget_destroy(passwd_dialog);
			g_free(filename);
			return;
		}

		password = g_strdup(gtk_entry_get_text(GTK_ENTRY(pass_entry)));
		passlen = strlen(password);

		if (passlen == 0) {
			utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Please enter the password"), GTK_WINDOW(passwd_dialog));
			gtk_widget_grab_focus (pass_entry);
			g_free(password);
			continue;
		} else {
			break;
		}

	}

	gtk_widget_destroy(passwd_dialog);

	/* extracting encrypted data */

	tmp_filename = g_strdup (prefs_get_config_filename (BACKUP_TEMPLATE, appGUI));
	g_unlink (tmp_filename);

	context = grg_context_initialize_defaults ((unsigned char*) "BCK");
	keyholder = grg_key_gen ((unsigned char*) password, -1);

	if (keyholder == NULL || context == NULL) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Cannot create backup!"), GTK_WINDOW(appGUI->main_window));
		return;
	}

	grg_ctx_set_crypt_algo (context, get_enc_algorithm_value());
	grg_ctx_set_hash_algo (context, get_enc_hashing_value());
	grg_ctx_set_comp_algo (context, get_comp_algorithm_value());
	grg_ctx_set_comp_ratio (context, get_comp_ratio_value());

	ret = grg_decrypt_file (context, keyholder, (unsigned char *) filename, &arch, &arch_len);

	if (ret != GRG_OK) {
		utl_gui_create_dialog (GTK_MESSAGE_ERROR, _("Incorrect password!"), GTK_WINDOW(appGUI->main_window));
		grg_key_free (context, keyholder);
		grg_context_free (context);
		g_free(password);
		g_free(filename);
		g_free(tmp_filename);
		return;
	}
		
	tErr = -1;

    sprintf (tmpbuf, "%s\n\n%s", 
			 _("All your data will be replaced with backup file content."), _("Are you sure?"));

	ret = utl_gui_create_dialog (GTK_MESSAGE_QUESTION, tmpbuf, GTK_WINDOW(appGUI->main_window));
   
    if (ret == GTK_RESPONSE_YES) {
		/* save TAR file */
		g_file_set_contents (tmp_filename, (gchar *) arch, arch_len, NULL);
		tErr = 0;
	}

	g_free (arch);
	grg_key_free (context, keyholder);
	grg_context_free (context);

	/* free strings */

	g_free(password);
	g_free(filename);
	g_free(tmp_filename);

	if (!tErr) {
        sprintf (tmpbuf, "%s", _("Osmo has to be restarted now..."));
		utl_gui_create_dialog (GTK_MESSAGE_INFO, tmpbuf, GTK_WINDOW(appGUI->main_window));
		gui_quit_osmo (appGUI);
	}

}

/*------------------------------------------------------------------------------*/

void
backup_restore_run (GUI *appGUI) {

gint i;
gchar *tmp_filename, *home_dirname;
const gchar *item_name = NULL;
gchar *fitem = NULL, separator[2] = { G_DIR_SEPARATOR, '\0' };

TAR *tArch;

	/* is backup file available? */

	tmp_filename = g_strdup (prefs_get_config_filename (BACKUP_TEMPLATE, appGUI));

	if (g_file_test (tmp_filename, G_FILE_TEST_IS_REGULAR) == FALSE) {
		g_free (tmp_filename);
		return;
	}

	/* is it a TAR archive? */

	/* FIXME: How to check integrity of TAR archive? */

	/* removing old files */

	appGUI->file_list = NULL;

	home_dirname = g_strdup (prefs_get_config_dir(appGUI));
	add_files_to_list (home_dirname, ".xml", appGUI);

	item_name = g_strconcat (home_dirname, separator, "notes", NULL);
	g_free (home_dirname);
	home_dirname = g_strdup (item_name);
	
	add_files_to_list (home_dirname, ".osm", appGUI);

	g_free (home_dirname);

	/* change directory to cfg home */

	home_dirname = g_get_current_dir();
	g_chdir (prefs_get_config_dir(appGUI));

	i = 0;

	while ((fitem = g_slist_nth_data (appGUI->file_list, i)) != NULL) {
		g_unlink (fitem);
		i++;
	}

	/* untar files */

	if (tar_open (&tArch, tmp_filename, NULL, O_RDONLY, 0, TAR_GNU) >= 0) {
		tar_extract_all (tArch, ".");
		tar_close (tArch);
	}

	/* clean up */

	g_chdir (home_dirname);
	g_free (home_dirname);

	if (appGUI->file_list != NULL) {
		g_slist_foreach (appGUI->file_list, (GFunc) g_free, NULL);
		g_slist_free (appGUI->file_list);
		appGUI->file_list = NULL;
	}

	g_unlink (tmp_filename);
	g_free (tmp_filename);
}

/*------------------------------------------------------------------------------*/

#endif  /* BACKUP_SUPPORT && HAVE_LIBGRINGOTTS */

