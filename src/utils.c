
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

#include "utils.h"
#include "utils_time.h"
#include "utils_date.h"
#include "i18n.h"
#include "options_prefs.h"

/*------------------------------------------------------------------------------*/

gchar *
utl_text_replace (const gchar *text, const gchar *regex, const gchar *replacement)
{
	GRegex *reg = g_regex_new (regex, 0, 0, NULL);
	gchar *buffer = g_regex_replace_literal (reg, text, -1, 0, replacement, 0, NULL);
	g_regex_unref (reg);

	return buffer;
}

/*------------------------------------------------------------------------------*/

gchar *
utl_text_to_html (const gchar *text, gboolean ignoreBR)
{

const gchar *pairs[] = {
	"&",    "&amp;", 
	"\"",   "&quot;", 
	"<",    "&lt;", 
	">",    "&gt;",
	"\\n",  "<br />"    /* must be the last entry here */
};

GRegex *reg;
gint i = 0;
gchar *buffer = NULL, *temp = NULL;
guint n_pairs = G_N_ELEMENTS (pairs) / 2;

	temp = g_strdup(text);

	if (ignoreBR) --n_pairs;

	for (i=0; i < n_pairs; i++) {
		reg = g_regex_new (pairs[i*2+0], 0, 0, NULL);
		buffer = g_regex_replace_literal (reg, temp, -1, 0, pairs[i*2+1], 0, NULL);
		g_free (temp);
		temp = buffer;
		g_regex_unref (reg);
	}

	return temp;
}

/*------------------------------------------------------------------------------*/

gchar *
utl_get_day_name (guint day, gboolean short_name)
{
	static gchar buffer[BUFFER_SIZE];
	GDate *tmpdate = NULL;

	g_return_val_if_fail (day > 0 && day <= 31, buffer);

	tmpdate = g_date_new_dmy (day, 1, 2007);
	g_return_val_if_fail (tmpdate != NULL, buffer);

	g_date_strftime (buffer, BUFFER_SIZE, short_name ? "%a" : "%A", tmpdate);
	g_date_free (tmpdate);

	return buffer;
}

/*------------------------------------------------------------------------------*/

gchar *
utl_get_julian_day_name (guint32 julian)
{
	static gchar buffer[BUFFER_SIZE];
	GDate *tmpdate = NULL;

	buffer[0] = '\0';
	g_return_val_if_fail (g_date_valid_julian (julian) == TRUE, buffer);

	tmpdate = g_date_new_julian (julian);
	g_return_val_if_fail (tmpdate != NULL, buffer);

	g_date_strftime (buffer, BUFFER_SIZE, "%A", tmpdate);
	g_date_free (tmpdate);

	return buffer;
}

/*------------------------------------------------------------------------------*/

gchar *
utl_get_date_name (const GDate *date)
{
	static gchar buffer[BUFFER_SIZE];

	g_date_strftime (buffer, BUFFER_SIZE, "%e %B %Y", date);     /* e.g. 1 August 1999 */
	return buffer;
}

/*------------------------------------------------------------------------------*/

gchar *
utl_get_date_name_format (const GDate *date, gchar *fmt)
{
	static gchar buffer[BUFFER_SIZE];

	g_date_strftime (buffer, BUFFER_SIZE, fmt, date);
	return buffer;
}

/*------------------------------------------------------------------------------*/

guint
utl_get_weekend_days_in_month (const GDate *date)
{
	GDate *tmpdate = NULL;
	guint i, day, days, weekend_days;

	tmpdate = g_date_new_dmy (1, g_date_get_month (date), g_date_get_year (date));
	g_return_val_if_fail (tmpdate != NULL, 0);

	days = utl_date_get_days_in_month (tmpdate);
	weekend_days = 0;

	for (i = 1; i <= days; i++) {
		g_date_set_day (tmpdate, i);
		day = g_date_get_weekday (tmpdate);
		if (day == G_DATE_SATURDAY || day == G_DATE_SUNDAY) {
			weekend_days++;
		}
	}

	g_date_free (tmpdate);
	return weekend_days;
}

/*------------------------------------------------------------------------------*/

guint
utl_get_weekend_days_in_month_my (guint month, guint year)
{
	GDate *tmpdate = NULL;
	guint i, day, days, weekend_days;

	g_return_val_if_fail (g_date_valid_dmy (1, month, year) == TRUE, 0);

	tmpdate = g_date_new_dmy (1, month, year);
	g_return_val_if_fail (tmpdate != NULL, 0);

	days = utl_date_get_days_in_month (tmpdate);
	weekend_days = 0;

	for (i = 1; i <= days; i++) {
		g_date_set_day (tmpdate, i);
		day = g_date_get_weekday (tmpdate);
		if (day == G_DATE_SATURDAY || day == G_DATE_SUNDAY) {
			weekend_days++;
		}
	}

	g_date_free (tmpdate);
	return weekend_days;
}

/*------------------------------------------------------------------------------*/

guint
utl_get_days_per_year (guint year)
{
	return (g_date_is_leap_year (year) ? 366 : 365);
}

/*------------------------------------------------------------------------------*/

void
utl_subtract_from_date (guint32 date, gint time, gint days, gint seconds, guint32 *new_date, gint *new_time)
{
	*new_date = date - days;

	if (time >= 0) {
		*new_time = time - seconds;

		if (*new_time < 0) {
			*new_time = (*new_time) + 24 * 3600;
			*new_date = (*new_date) - 1;
		}
	} else {
		*new_time = -1;
	}
}

/*------------------------------------------------------------------------------*/
/*  This routine has been taken from http://www.voidware.com/moon_phase.htm
    calculates the moon phase (0-7), accurate to 1 segment: 0 = > new moon, 4 => full moon.
*/

guint
utl_calc_moon_phase (const GDate *date)
{
	gdouble jd;
	gint day, month, year;
	gint b, c, e;

	utl_date_get_dmy (date, &day, &month, &year);

	if (month < 3) {
		year--;
		month += 12;
	}
	month++;
	c = 365.25 * year;
	e = 30.6 * month;
	jd = c + e + day - 694039.09;   /* jd is total days elapsed */
	jd /= 29.53;                    /* divide by the moon cycle (29.53 days) */
	b = jd;                         /* int(jd) -> b, take integer part of jd */
	jd -= b;                        /* subtract integer part to leave fractional part of original jd */
	b = jd * 8 + 0.5;               /* scale fraction from 0-8 and round by adding 0.5 */
	b = b & 7;                      /* 0 and 8 are the same so turn 8 into 0 */

	return b;
}

/*------------------------------------------------------------------------------*/

gchar*
utl_get_moon_phase_name (gint phase)
{
	const gchar *phase_names[] = {
		N_("New Moon"), N_("Waxing Crescent Moon"), N_("Quarter Moon"), N_("Waxing Gibbous Moon"),
		N_("Full Moon"), N_("Waning Gibbous Moon"), N_("Last Quarter Moon"), N_("Waning Crescent Moon")
	};

	return (gchar *) gettext (phase_names[phase]);
}

/*------------------------------------------------------------------------------*/

void
utl_name_strcat (gchar *first, gchar *second, gchar *buffer)
{
	gchar tmpbuff[BUFFER_SIZE];
	gboolean flag;

	buffer[0] = '\0';
	g_return_if_fail (first != NULL || second != NULL);

	g_snprintf (tmpbuff, BUFFER_SIZE, "(%s)", _("None"));
	flag = FALSE;

	if (first != NULL) {

		if (strcmp (first, tmpbuff) != 0) {
			flag = TRUE;
			g_strlcpy (buffer, first, BUFFER_SIZE);
		}

		g_free (first);
	}

	if (second != NULL) {

		if (strcmp (second, tmpbuff) != 0) {
			if (flag == TRUE) {
				g_strlcat (buffer, " ", BUFFER_SIZE);
				g_strlcat (buffer, second, BUFFER_SIZE);
			} else {
				g_strlcpy (buffer, second, BUFFER_SIZE);
			}
		}

		g_free (second);
	}

	g_return_if_fail (strlen (buffer) > 0);
}

/*------------------------------------------------------------------------------*/

void
utl_xml_get_int (gchar *name, gint *iname, xmlNodePtr node)
{
	xmlChar *key;

	if ((xmlStrcmp (node->name, (const xmlChar *) name)) == 0) {
		key = xmlNodeGetContent (node->xmlChildrenNode);
		if (key != NULL) {
			*iname = atoi ((gchar *) key);
			xmlFree (key);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
utl_xml_get_uint (gchar *name, guint *uname, xmlNodePtr node)
{
	xmlChar *key;

	if ((xmlStrcmp (node->name, (const xmlChar *) name)) == 0) {
		key = xmlNodeGetContent (node->xmlChildrenNode);
		if (key != NULL) {
			*uname = (guint) atoi ((gchar *) key);
			xmlFree (key);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
utl_xml_get_char (gchar *name, gchar *cname, xmlNodePtr node)
{
	xmlChar *key;

	if ((xmlStrcmp (node->name, (const xmlChar *) name)) == 0) {
		key = xmlNodeGetContent (node->xmlChildrenNode);
		if (key != NULL) {
			*cname = key[0];
			xmlFree (key);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
utl_xml_get_str (gchar *name, gchar **sname, xmlNodePtr node)
{
	xmlParserCtxtPtr context;
	xmlChar *key, *out;

	if ((xmlStrcmp (node->name, (const xmlChar *) name)) == 0) {

		key = xmlNodeGetContent (node->xmlChildrenNode);
		context = xmlCreateDocParserCtxt (key);
		out = (xmlChar*) xmlStringDecodeEntities (context, key, XML_SUBSTITUTE_REF, 0, 0, 0);
		xmlFreeParserCtxt (context);
		xmlFree (key);

		if (out != NULL) {
			*sname = g_strdup ((gchar *) out);
			xmlFree (out);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
utl_xml_get_strn (gchar *name, gchar *sname, gint buffer_size, xmlNodePtr node)
{
	xmlParserCtxtPtr context;
	xmlChar *key, *out;

	if ((xmlStrcmp (node->name, (const xmlChar *) name)) == 0) {

		key = xmlNodeGetContent (node->xmlChildrenNode);
		context = xmlCreateDocParserCtxt (key);
		out = (xmlChar*) xmlStringDecodeEntities (context, key, XML_SUBSTITUTE_REF, 0, 0, 0);
		xmlFreeParserCtxt (context);
		xmlFree (key);

		if (out != NULL) {
			g_strlcpy (sname, (gchar *) out, buffer_size);
			xmlFree (out);
		}
	}
}

/*------------------------------------------------------------------------------*/

void
utl_xml_put_int (gchar *name, gint value, xmlNodePtr node)
{
gchar buffer[32];

	g_snprintf (buffer, 32, "%d", value);
	xmlNewChild (node, NULL, (const xmlChar *) name, (xmlChar *) buffer);
}

/*------------------------------------------------------------------------------*/

void
utl_xml_put_uint (gchar *name, guint value, xmlNodePtr node)
{
gchar buffer[32];

	g_snprintf (buffer, 32, "%d", value);
	xmlNewChild (node, NULL, (const xmlChar *) name, (xmlChar *) buffer);
}

/*------------------------------------------------------------------------------*/

void
utl_xml_put_char (gchar *name, gchar character, xmlNodePtr node, xmlDocPtr doc)
{
	gchar buffer[32];
	xmlChar *escaped;

	g_snprintf (buffer, 32, "%c", character);
	escaped = xmlEncodeSpecialChars(doc, (const xmlChar *) buffer);
	xmlNewTextChild (node, NULL, (const xmlChar *) name, (xmlChar *) escaped);
	xmlFree (escaped);
}

/*------------------------------------------------------------------------------*/

void
utl_xml_put_str (gchar *name, gchar *string, xmlNodePtr node, xmlDocPtr doc)
{
	xmlChar *escaped;

	escaped = xmlEncodeSpecialChars(doc, (const xmlChar *) string);
	xmlNewTextChild (node, NULL, (const xmlChar *) name, (xmlChar *) escaped);
	xmlFree (escaped);
}

/*------------------------------------------------------------------------------*/

void
utl_xml_put_strn (gchar *name, gchar *string, gint buffer_size, xmlNodePtr node, xmlDocPtr doc)
{
	gchar buffer[BUFFER_SIZE];
	xmlChar *escaped;

	if (buffer_size > BUFFER_SIZE) buffer_size = BUFFER_SIZE;
	g_snprintf (buffer, buffer_size, "%s", string);
	escaped = xmlEncodeSpecialChars(doc, (const xmlChar *) buffer);
	xmlNewTextChild (node, NULL, (const xmlChar *) name, (xmlChar *) escaped);
	xmlFree (escaped);
}

/*------------------------------------------------------------------------------*/

gboolean
utl_is_valid_command (gchar *command) {

gchar *found_path;

    found_path = g_find_program_in_path (command);

    if (found_path != NULL) {
		g_free (found_path);
		return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------*/

void
utl_run_command (gchar *command, gboolean sync) {

gchar *cmdline[4];

    cmdline[0] = "sh";
    cmdline[1] = "-c";
    cmdline[2] = command;
    cmdline[3] = 0;

	if (sync == FALSE) {
		g_spawn_async (NULL, (gchar **)cmdline, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL, 
					   NULL, NULL, NULL, NULL);
	} else {
		g_spawn_sync (NULL, (gchar **)cmdline, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL, 
				      NULL, NULL, NULL, NULL, NULL, NULL);
	}
}

/*------------------------------------------------------------------------------*/

gboolean
utl_run_helper (gchar *parameter, gint helper) {

gchar command[PATH_MAX];
gboolean sync = FALSE;

    if (helper == EMAIL) {
        sprintf (command, config.email_client, parameter);
    } else if (helper == WWW) {
        sprintf (command, config.web_browser, parameter);
    } else if (helper == SOUND) {
        sprintf (command, config.sound_player, parameter);
		sync = TRUE;
    } else {
        return FALSE;
    }

    utl_run_command (command, sync);

    return TRUE;
}

/*------------------------------------------------------------------------------*/

gint
utl_get_link_type (gchar *link)
{
	gint i, n;

	g_return_val_if_fail (link != NULL, UNKNOWN);

	for (i = n = 0; i < strlen (link); i++)
		if (link[i] == '@') n++;

	if (!strncasecmp (link, "https://", 8) || !strncasecmp (link, "http://", 7) || !strncasecmp(link, "www", 3)) return WWW;
	else if (n == 1) return EMAIL;
	else return UNKNOWN;
}

/*------------------------------------------------------------------------------*/

void
utl_cairo_set_color (cairo_t *cr, GdkColor *color, gint alpha)
{
	cairo_set_source_rgba (cr, (double) color->red / 65535.0,
	                           (double) color->green / 65535.0,
	                           (double) color->blue / 65535.0,
	                           (double) alpha / 65535.0);
}

/*------------------------------------------------------------------------------*/

void
utl_cairo_draw (cairo_t *cr, gint stroke)
{
	if (stroke) {
		cairo_set_line_width (cr, stroke);
		cairo_stroke (cr);
	} else {
		cairo_fill (cr);
	}
}

/*------------------------------------------------------------------------------*/

void
utl_draw_rounded_rectangle (cairo_t *cr, gint x, gint y, gint w, gint h, gint a, gint s)
{
	cairo_move_to (cr, x + a + s, y + s);
	cairo_line_to (cr, x + w - a - s, y + s);
	cairo_arc (cr, x + w - a - s, y + a + s, a, 1.5 * M_PI, 2.0 * M_PI);
	cairo_line_to (cr, x + w - s, y + h - a - s);
	cairo_arc (cr, x + w - a - s, y + h - a - s, a, 0.0 * M_PI, 0.5 * M_PI);
	cairo_line_to (cr, x + a + s, y + h - s);
	cairo_arc (cr, x + a + s, y + h - a - s, a, 0.5 * M_PI, 1.0 * M_PI);
	cairo_line_to (cr, x + s, y + a + s);
	cairo_arc (cr, x + a + s, y + a + s, a, 1.0 * M_PI, 1.5 * M_PI);
}

/*------------------------------------------------------------------------------*/

void
utl_draw_left_arrow (cairo_t *cr, gdouble x, gdouble y, gdouble w, gdouble h, gdouble a)
{
	cairo_move_to (cr, x, y);
	cairo_line_to (cr, x + w * a, y + h * 0.50);
	cairo_line_to (cr, x + w * a, y + h * 0.25);
	cairo_line_to (cr, x + w * 1, y + h * 0.25);
	cairo_line_to (cr, x + w * 1, y - h * 0.25);
	cairo_line_to (cr, x + w * a, y - h * 0.25);
	cairo_line_to (cr, x + w * a, y - h * 0.50);
	cairo_close_path (cr);
}

/*------------------------------------------------------------------------------*/

gpointer 
utl_snd_play_thread (gpointer *data) {

gchar sound_filename[PATH_MAX];
gint i;
		
    g_snprintf (sound_filename, PATH_MAX, "%s%c%s%c%s", SOUNDSDIR, G_DIR_SEPARATOR, "osmo", 
				G_DIR_SEPARATOR, "alarm.wav");

	for (i=0; i < (gint) data; i++) {
		utl_run_helper (sound_filename, SOUND);
	}
		
	return NULL;
}


void
utl_play_alarm_sound (guint repetitions) {

GThread *snd_thread = NULL;

	if (repetitions == 0) return;

	snd_thread = g_thread_create ((GThreadFunc)utl_snd_play_thread, (gpointer) repetitions, FALSE, NULL);
	if (snd_thread == NULL) return;

}

/*------------------------------------------------------------------------------*/

gchar*
utl_add_timestamp_to_filename (gchar *filename, gchar *extension) {

static gchar filename_buffer[BUFFER_SIZE];

	g_snprintf (filename_buffer, BUFFER_SIZE, "%s-%4d%02d%02d%02d%02d.%s", 
				filename, 
				utl_date_get_current_year(), utl_date_get_current_month(), utl_date_get_current_day(), 
				utl_time_get_current_hour(), utl_time_get_current_minute(),
				extension);

	return filename_buffer;
}

/*------------------------------------------------------------------------------*/

