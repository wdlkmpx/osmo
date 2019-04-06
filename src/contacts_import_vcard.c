
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
 *           (C) 2008 Markus Dahms <mad@automagically.de>
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

#include "contacts_import_vcard.h"

#ifdef CONTACTS_ENABLED

/* these entries can be mapped directly from VCard field to Osmo field */
static OsmoVCardMapping vcard_mappings[] = {
	{ "TEL:HOME", COLUMN_HOME_PHONE_1 },
	{ "TEL:CELL", COLUMN_CELL_PHONE_1 }
};

static guint32 n_vcard_mappings = G_N_ELEMENTS(vcard_mappings);

static gchar *vcard_strdup_decode(gchar *text, OsmoVCardEncoding enc);

gboolean contacts_import_vcard(const gchar *vcard, GUI *appGUI)
{
	gchar **lines, **strp, **split2, *varname, *value;
	GtkTreeIter iter;
	gint32 i;
	OsmoVCardEncoding enc;

	g_assert(appGUI != NULL);
	g_assert(vcard != NULL);

	if(strncmp(vcard, "BEGIN:VCARD", 11) != 0) {
		/* not a VCARD */
		return FALSE;
	}

	/* add new contact */
	gtk_list_store_append(appGUI->cnt->contacts_list_store, &iter);

	/* FIXME: support for multi-line fields */
	strp = lines = g_strsplit(vcard, "\n", 0);

	while(strp && *strp) {
		/* split variable name and value */
		split2 = g_strsplit(*strp, ":", 2);
		if(g_strv_length(split2) < 2) {
			g_strfreev(split2);
			break;
		}
		varname = g_strdup(split2[0]);
		enc = VCARD_UNENCODED;
		if(strstr(varname, "ENCODING=QUOTED-PRINTABLE"))
			enc = VCARD_QUOTED_PRINTABLE;
		else if(strstr(varname, "BASE64"))
			enc = VCARD_BASE64;
		value = vcard_strdup_decode(split2[1], enc);
		g_strchomp(value);
		g_strfreev(split2);

		if((varname[0] == 'N') &&
			((varname[1] == '\0') || (varname[1] == ';'))) {
			/* name */
			split2 = g_strsplit(value, ";", 2);
			if(g_strv_length(split2) == 2) {
				gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter,
					COLUMN_FIRST_NAME, g_strdup(split2[1]),
					COLUMN_LAST_NAME, g_strdup(split2[0]), -1);
			}
			g_strfreev(split2);
		}
		else if(strncmp(varname, "ADR;HOME", 8) == 0) {
			gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter,
				COLUMN_HOME_ADDRESS, g_strdup(value), -1);
		}
		else {
			/* try directly mapped entries */
			for(i = 0; i < n_vcard_mappings; i ++) {
				if(strncmp(vcard_mappings[i].field, varname,
					strlen(vcard_mappings[i].field)) == 0) {
					gtk_list_store_set(appGUI->cnt->contacts_list_store, &iter,
						vcard_mappings[i].osmo_id, g_strdup(value), -1);
					break;
				}
			}
		}
		g_free(varname);
		g_free(value);

		strp ++;
	}

	g_strfreev(lines);

	return TRUE;
}

static gchar *vcard_strdup_decode(gchar *text, OsmoVCardEncoding enc)
{
	gchar *s, *pi = text, *po, shex[3] = { 'X', 'X', '\0' };
	guint32 n_eq = 0;

	switch(enc) {
		/* all non-ASCII characters should be encoded QP */
		case VCARD_QUOTED_PRINTABLE:
			/* count "=" chars */
			while(*pi != '\0') {
				if((*pi == '=') && (*(pi + 1) != '\n'))
					n_eq ++;
				pi ++;
			}
			/* allocate memory */
			po = s = g_new0(gchar, strlen(text) - (n_eq * 2) + 1);
			pi = text;
			/* copy and decode text */
			while(*pi != '\0') {
				if(*pi == '=') {
					if(*(pi + 1) == '\n')
						/* '=' at end of line => soft break */
						*po = ' ';
					else {
						shex[0] = *(++ pi);
						shex[1] = *(++ pi);
						*po = strtol(shex, NULL, 16);
					}
				}
				else
					*po = *pi;
				po ++;
				pi ++;
			}
			break;
		case VCARD_BASE64:
			/* base64 is used for photos and such stuff... */
			s = g_strdup("(BASE64 encoding currently unsupported)");
		default:
			s = g_strdup(text);
	}

	/* convert to UTF-8 if necessary */
	if(g_utf8_validate(s, -1, NULL) == FALSE) {
		po = g_convert_with_fallback(s, -1, "utf-8", "iso-8859-1",
			"?", NULL, NULL, NULL);
		g_free(s);
		s = po;
	}
	return s;
}

#endif  /* CONTACTS_ENABLED */
