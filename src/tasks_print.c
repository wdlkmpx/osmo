
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

#include "i18n.h"
#include "utils.h"
#include "utils_gui.h"

#ifdef TASKS_ENABLED
#ifdef PRINTING_SUPPORT

/*------------------------------------------------------------------------------*/

static void
request_page_setup (GtkPrintOperation *operation, GtkPrintContext *context, int page_nr, GtkPageSetup *setup)
{
/*  GtkPaperSize *a4_size = gtk_paper_size_new ("iso_a4");*/
/*  gtk_page_setup_set_orientation (setup, GTK_PAGE_ORIENTATION_LANDSCAPE);*/
/*  gtk_page_setup_set_paper_size (setup, a4_size);*/
/*  gtk_paper_size_free (a4_size);*/
}

/*------------------------------------------------------------------------------*/

void
tasks_begin_print (GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data)
{
	PangoLayout *layout;
	PangoFontDescription *desc;
	gdouble page_height;
	gint letter_height, task_height;

	GUI *appGUI = (GUI *) user_data;

	layout = gtk_print_context_create_pango_layout (context);
	desc = pango_font_description_from_string ("mono");
	pango_font_description_set_size (desc, appGUI->print_font_size * PANGO_SCALE);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	pango_layout_set_text (layout, "M", -1);
	pango_layout_get_pixel_size (layout, NULL, &letter_height);
	g_object_unref (layout);
	
	task_height = 2 * letter_height;
	page_height = gtk_print_context_get_height (context);

	appGUI->print_nlines = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (appGUI->tsk->tasks_filter), NULL);
	appGUI->print_lines_per_page = floor (page_height / task_height) - 1;
	g_return_if_fail (appGUI->print_lines_per_page > 0);
	appGUI->print_npages = (appGUI->print_nlines - 1) / appGUI->print_lines_per_page + 1;
	gtk_print_operation_set_n_pages (operation, appGUI->print_npages);
}

/*------------------------------------------------------------------------------*/

void
tasks_draw_page (GtkPrintOperation *operation, GtkPrintContext *context, gint npage, gpointer user_data)
{
	PangoLayout *layout;
	PangoFontDescription *desc;
	GtkTreePath *sort_path, *filter_path, *path;
	GtkTreeIter iter;
	cairo_t *cr;
	gchar *summary, *date, *priority;
	gchar buffer[BUFFER_SIZE];
	gdouble page_width, page_height, ycursor;
	gint text_width, text_height, task_height;
	gint i, imin, imax;

	GUI *appGUI = (GUI *) user_data;

	cr = gtk_print_context_get_cairo_context (context);
	page_width = gtk_print_context_get_width (context);
	page_height = gtk_print_context_get_height (context);

	layout = gtk_print_context_create_pango_layout (context);

	desc = pango_font_description_from_string ("mono");
	pango_font_description_set_size (desc, appGUI->print_font_size * PANGO_SCALE);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	pango_layout_set_text (layout, "M", -1);
	pango_layout_get_pixel_size (layout, NULL, &task_height);
	task_height *= 2;

	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0);
	
	g_snprintf (buffer, BUFFER_SIZE, "<big><b>%s</b></big>", _("Tasks list"));
	pango_layout_set_markup (layout, buffer, -1);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);
	pango_cairo_show_layout (cr, layout);

	ycursor = task_height;

	if (appGUI->print_npages > 1) {
		ycursor = text_height;
		g_snprintf (buffer, BUFFER_SIZE, "<b>%d/%d</b>", npage + 1, appGUI->print_npages);
		pango_layout_set_markup (layout, buffer, -1);
		pango_layout_get_pixel_size (layout, &text_width, &text_height);
		cairo_move_to (cr, page_width - text_width - 4, ycursor - text_height);
		pango_cairo_show_layout (cr, layout);
		ycursor = task_height;
	}

	ycursor += 0.3 * task_height;

	cairo_rectangle (cr, 0, task_height, page_width, task_height);
	cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_stroke (cr);

	/* TRANSLATORS: "No." is an abbreviation of word "number" */
	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("No."));
	pango_layout_set_markup (layout, buffer, -1);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);
	cairo_move_to (cr, 0.05 * page_width - text_width, ycursor);
	pango_cairo_show_layout (cr, layout);

	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("Summary"));
	pango_layout_set_markup (layout, buffer, -1);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);
	cairo_move_to (cr, 0.07 * page_width, ycursor);
	pango_cairo_show_layout (cr, layout);

	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("Due date"));
	pango_layout_set_markup (layout, buffer, -1);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);
	cairo_move_to (cr, 0.66 * page_width, ycursor);
	pango_cairo_show_layout (cr, layout);

	g_snprintf (buffer, BUFFER_SIZE, "<b>%s</b>", _("Priority"));
	pango_layout_set_markup (layout, buffer, -1);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);
	cairo_move_to (cr, 0.88 * page_width, ycursor);
	pango_cairo_show_layout (cr, layout);

	ycursor += 0.7 * task_height;

	sort_path = gtk_tree_path_new_first ();

	i = 0;
	imin = appGUI->print_lines_per_page * npage;
	imax = appGUI->print_lines_per_page * (npage + 1);

	while (gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter, sort_path) == TRUE) {

		if (sort_path != NULL) {
			filter_path = gtk_tree_model_sort_convert_path_to_child_path (GTK_TREE_MODEL_SORT (appGUI->tsk->tasks_sort), sort_path);

			if (filter_path != NULL) {
				path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (appGUI->tsk->tasks_filter), filter_path);

				if (path != NULL) {
					if (i >= imin && i < imax) {
						gtk_tree_model_get_iter (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter, path);
						gtk_tree_model_get (GTK_TREE_MODEL (appGUI->tsk->tasks_list_store), &iter,
						                    TA_COLUMN_SUMMARY, &summary,
						                    TA_COLUMN_DUE_DATE, &date,
						                    TA_COLUMN_PRIORITY, &priority,
						                    -1);

						ycursor += 0.3 * task_height;

						g_snprintf (buffer, BUFFER_SIZE, "%d.", i + 1);
						pango_layout_set_markup (layout, buffer, -1);
						pango_layout_get_pixel_size (layout, &text_width, &text_height);
						cairo_move_to (cr, 0.05 * page_width - text_width, ycursor);
						pango_cairo_show_layout (cr, layout);

						g_snprintf (buffer, BUFFER_SIZE, "%.52s", summary);
						pango_layout_set_text (layout, buffer, -1);
						pango_layout_get_pixel_size (layout, &text_width, &text_height);
						cairo_move_to (cr, 0.07 * page_width, ycursor);
						pango_cairo_show_layout (cr, layout);

						pango_layout_set_text (layout, date, -1);
						pango_layout_get_pixel_size (layout, &text_width, &text_height);
						cairo_move_to (cr, 0.66 * page_width, ycursor);
						pango_cairo_show_layout (cr, layout);

						pango_layout_set_text (layout, priority, -1);
						pango_layout_get_pixel_size (layout, &text_width, &text_height);
						cairo_move_to (cr, 0.88 * page_width, ycursor);
						pango_cairo_show_layout (cr, layout);

						ycursor += 0.7 * task_height;
						cairo_move_to (cr, 0, ycursor);
						cairo_line_to (cr, page_width, ycursor);
						cairo_stroke (cr);
						
						g_free (summary);
						g_free (date);
						g_free (priority);
					}

					i++;
					gtk_tree_path_free (path);
				}

				gtk_tree_path_free (filter_path);
			}

		}

		gtk_tree_path_next (sort_path);
	}

	cairo_move_to (cr, 0, task_height);
	cairo_line_to (cr, 0, ycursor);
	cairo_move_to (cr, 0.06 * page_width, task_height);
	cairo_line_to (cr, 0.06 * page_width, ycursor);
	cairo_move_to (cr, 0.65 * page_width, task_height);
	cairo_line_to (cr, 0.65 * page_width, ycursor);
	cairo_move_to (cr, 0.87 * page_width, task_height);
	cairo_line_to (cr, 0.87 * page_width, ycursor);
	cairo_move_to (cr, page_width, task_height);
	cairo_line_to (cr, page_width, ycursor);
	cairo_stroke (cr);

	g_object_unref (layout);
}

/*------------------------------------------------------------------------------*/

void
tasks_print (GUI *appGUI) {

GtkPrintOperation *print;
GtkPrintOperationResult result;
GError *error = NULL;
gchar buffer[BUFFER_SIZE];

    print = gtk_print_operation_new ();

    appGUI->print_lines_per_page = 0;
    appGUI->print_nlines = 0;
    appGUI->print_npages = 0;

	g_signal_connect (print, "begin_print", G_CALLBACK (tasks_begin_print), appGUI);
	g_signal_connect (print, "draw_page", G_CALLBACK (tasks_draw_page), appGUI);
	g_signal_connect (print, "request_page_setup", G_CALLBACK (request_page_setup), NULL);

    result = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                      GTK_WINDOW (appGUI->main_window), &error);

    if (result == GTK_PRINT_OPERATION_RESULT_ERROR) {
		g_snprintf (buffer, BUFFER_SIZE, "%s: %s", _("Error printing"), error->message);
        utl_gui_create_dialog (GTK_MESSAGE_ERROR, buffer, GTK_WINDOW (appGUI->main_window));
        g_error_free (error);
    }

    g_object_unref (print);
}

/*------------------------------------------------------------------------------*/

#endif /* PRINTING_SUPPORT */
#endif  /* TASKS_ENABLED */

