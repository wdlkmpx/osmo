
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

#include "contacts_import_csv.h"

#ifdef CONTACTS_ENABLED

/*------------------------------------------------------------------------------*/

gchar *
csv_get_line (guint line, GUI *appGUI) {

gchar *line_buffer;
gint c, i, n;
gboolean line_found = FALSE, quotation_found;

    line_buffer = NULL;

    i = 1;
    n = 0;
    quotation_found = FALSE;

    while (n != appGUI->cnt->file_length) {
        c = appGUI->cnt->file_buffer[n];
        if (i == line) {
            line_found = TRUE;
            break;
        }
        if (c == '"') {
            quotation_found = !quotation_found;
        }
        if (c == '\n' && quotation_found == FALSE) i++;
        n++;
    }

    if (line_found == TRUE) {

        line_buffer = g_malloc0 (MAX_LINE_LENGTH);
        if (line_buffer != NULL) {

            /* get single or multi line */

            quotation_found = FALSE;
            i = 0;
            do {
                c = appGUI->cnt->file_buffer[n];

                if (c == '"') {
                    quotation_found = !quotation_found;
                }

                if (c == '\n' && quotation_found == FALSE) break;

                line_buffer[i++] = c;
                n++;

            } while (n != appGUI->cnt->file_length && i < MAX_LINE_LENGTH);

            line_buffer[i] = 0;
        }
    }

    return line_buffer;
}

/*------------------------------------------------------------------------------*/

gchar *
csv_get_field (gchar *line_buffer, guint field) {

gchar *field_buffer;
gint i, j, current_field;
gboolean quotation_found;


    field_buffer = g_malloc0 (MAX_FIELD_LENGTH);
    if (field_buffer != NULL) {

        quotation_found = FALSE;
        i = j = 0;
        current_field = 1;

        while (line_buffer[i]) {

            if (j >= MAX_FIELD_LENGTH-1) break;

            if (line_buffer[i] == '"' && line_buffer[i+1] == '"') {
                field_buffer[j++] = '"';
                i++;
            } else if (line_buffer[i] == '"' && line_buffer[i+1] != '"') {
                quotation_found = !quotation_found;
            } else if (line_buffer[i] == FIELD_SEPARATOR) {
                if (quotation_found == FALSE) {
                    if (current_field == field) break;
                    current_field++;
                    j = 0;
                } else {
                    /* ignore white chars */
                    field_buffer[j++] = line_buffer[i];
                    if (j == 1 && (line_buffer[i] == ' ' || line_buffer[i] == '\t')) --j;
                    if (i > 0 && line_buffer[i] != 0) {
                        if (line_buffer[i] == '"' && line_buffer[i-1] == FIELD_SEPARATOR) --j;
                        if (line_buffer[i] == '"' && line_buffer[i+1] == 0) --j;
                    }
                }
            } else {
                /* ignore white chars */
                field_buffer[j++] = line_buffer[i];
                if (j == 1 && (line_buffer[i] == ' ' || line_buffer[i] == '\t')) --j;
                if (i > 0 && line_buffer[i] != 0) {
                    if (line_buffer[i] == '"' && line_buffer[i-1] == FIELD_SEPARATOR) --j;
                    if (line_buffer[i] == '"' && line_buffer[i+1] == 0) --j;
                }
            }

            i++;
        }

        field_buffer[j] = 0;

        if (current_field != field) {
            g_free(field_buffer);
            field_buffer = NULL;
        }
    }

    return field_buffer;
}

/*------------------------------------------------------------------------------*/

guint
get_number_of_records (GUI *appGUI) {

guint lines = 0, i;
gboolean quotation_found = FALSE;

    for (i=0; i != appGUI->cnt->file_length; i++) {
       if (appGUI->cnt->file_buffer[i] == '"') {
            quotation_found = !quotation_found;
       }
       if (appGUI->cnt->file_buffer[i] == '\n' && quotation_found == FALSE) {
           lines++;
       }
    }

    return lines;
}

/*------------------------------------------------------------------------------*/

#endif  /* CONTACTS_ENABLED */

