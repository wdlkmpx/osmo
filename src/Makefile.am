AM_CPPFLAGS = -DLOCALEDIR=\"$(datadir)/locale\" -DDATADIR=\"$(datadir)\" \
			  -DSOUNDSDIR=\"$(datadir)/sounds\" \
			  @GTK_CFLAGS@ @XML_CPPFLAGS@ -Wall
			  dnl -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DG_DISABLE_CAST_CHECKS
AM_CFLAGS = $(CFLAGS)

bin_PROGRAMS = osmo
osmo_SOURCES = about.c about.h \
			   backup.c backup.h \
			   calendar.c calendar.h \
			   calendar_calc.c calendar_calc.h \
			   calendar_fullyear.c calendar_fullyear.h \
			   calendar_ical.c calendar_ical.h \
			   calendar_jumpto.c calendar_jumpto.h \
			   calendar_moon.h \
			   calendar_notes.c calendar_notes.h \
			   calendar_print.c calendar_print.h \
			   calendar_preferences_gui.c calendar_preferences_gui.h \
			   calendar_timeline.c calendar_timeline.h \
			   calendar_utils.c calendar_utils.h \
			   calendar_widget.c calendar_widget.h \
			   check_events.c check_events.h \
			   contacts.c contacts.h \
			   contacts_birthdays.c contacts_birthdays.h \
			   contacts_import.c contacts_import.h \
			   contacts_import_csv.c contacts_import_csv.h \
			   contacts_export.c contacts_export.h \
			   contacts_items.c contacts_items.h \
			   contacts_preferences_gui.c contacts_preferences_gui.h \
			   gui.c gui.h \
			   gui_icon.h gui_logo.h \
			   i18n.h \
			   main.c \
			   notes.c notes.h \
			   notes_items.c notes_items.h \
			   notes_preferences_gui.c notes_preferences_gui.h \
			   options_prefs.c options_prefs.h \
			   preferences_gui.c preferences_gui.h \
			   stock_icons.c stock_icons.h stock_icons_data.h \
			   tasks.c tasks.h \
			   tasks_export.c tasks_export.h \
			   tasks_items.c tasks_items.h \
			   tasks_print.c tasks_print.h \
			   tasks_preferences_gui.c tasks_preferences_gui.h \
			   tasks_utils.c tasks_utils.h \
			   utils.c utils.h \
			   utils_gui.c utils_gui.h \
			   utils_date.c utils_date.h \
			   utils_date_time.c utils_date_time.h \
			   utils_time.c utils_time.h

# contacts_import_vcard.[ch] does not require libsyncml, but unless it is
# used otherwise only compile it if needed
if HAVE_LIBSYNCML
osmo_SOURCES += \
	contacts_import_syncml.c contacts_import_syncml.h \
	contacts_import_vcard.c contacts_import_vcard.h
AM_CFLAGS += @LIBSYNCML_CFLAGS@
LIBS += @LIBSYNCML_LIBS@
endif

if HAVE_LIBNOTIFY
AM_CFLAGS += @LIBNOTIFY_CFLAGS@
LIBS += @LIBNOTIFY_LIBS@
endif

if HAVE_LIBGTKHTML
AM_CFLAGS += @LIBGTKHTML_CFLAGS@
LIBS += @LIBGTKHTML_LIBS@
endif

if HAVE_LIBICAL
AM_CFLAGS += @LIBICAL_CFLAGS@
LIBS += @LIBICAL_LIBS@
endif

if HAVE_GTKSPELL
AM_CFLAGS += @GTKSPELL_CFLAGS@
LIBS += @GTKSPELL_LIBS@
endif

if CONFIGPATH
    AM_CFLAGS += -DCONFIG_PATH=\"$(configpath)\"
endif
if CONFIGDIR
    AM_CFLAGS += -DCONFIG_DIR=\"$(configdir)\"
endif

osmo_LDADD = $(LIBINTL) @GTK_LIBS@ @XML_LIBS@ @GTHREAD_LIBS@ -lm

#if WIN32
#AM_CFLAGS = \
#        -mms-bitfields  \
#        -mwindows
#endif
