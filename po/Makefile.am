DOMAIN = osmo
POTFILES = $(wildcard ../src/*.c ../src/*.h)

MSGFMT = msgfmt
XGETTEXT = xgettext
MSGMERGE = msgmerge

XGETTEXT_OPTS = --keyword=_ --keyword=N_ --keyword=X_ --add-comments=TRANSLATORS: --no-location
POFILES = bg.po ca.po cs.po da.po de.po el.po en_GB.po es.po fi.po fr.po hu.po it.po ja.po lt.po nl.po pl.po pt.po ru.po sv.po tr.po uk.po zh_CN.po
MOFILES = $(POFILES:.po=.mo)

EXTRA_DIST = $(POFILES) $(MOFILES)
LOCALEDIR = $(datadir)/locale

all: $(MOFILES)

update-po: $(DOMAIN).pot $(POFILES) $(MOFILES)

%.mo: skip
	@po=$(@:.mo=.po); if test $$po -nt $@ ; then $(MSGFMT) -c --statistics $$po -o $@; echo "$@ updated."; fi

%.po: $(DOMAIN).pot
	$(MSGMERGE) $@ $< -o $@.in && mv $@.in $@

$(DOMAIN).pot: $(POTFILES)
	@$(XGETTEXT) $(XGETTEXT_OPTS) $(POTFILES) -o $(DOMAIN).pot

install-data-local: $(MOFILES)
	for i in $(MOFILES); do \
		lang=`echo $$i | sed 's/\.mo$$//'`; \
		$(mkinstalldirs) ${DESTDIR}${LOCALEDIR}/$$lang/LC_MESSAGES; \
		$(INSTALL_DATA) $$i ${DESTDIR}${LOCALEDIR}/$$lang/LC_MESSAGES/$(DOMAIN).mo; \
	done

uninstall-local:
	for i in $(MOFILES); do \
		lang=`echo $$i | sed 's/\.mo$$//'`; \
		rm -f ${DESTDIR}${LOCALEDIR}/$$lang/LC_MESSAGES/$(DOMAIN).mo; \
	done

clean-local:
	rm -rf *.mo

skip:
