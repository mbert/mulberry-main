# Makefile for Mulberry misc. files
#
# Compatibility: GNU
#
# $Date$
# $Revision$
#

#
# Installation dirs...
#
prefix = /usr
DESTDIR = 
LIBDIR = $(DESTDIR)$(prefix)/lib/mulberry
HELPDIR = $(DESTDIR)$(prefix)/share/doc/mulberry

INSTALL = /usr/bin/install -c

HELPFILES := $(shell ls -1 Bits/CHANGES* | grep '[0-9a-zA-Z]$$' | tr '\n' ' ') Bits/Mulberry_License

.PHONY: install uninstall

install:
	$(INSTALL) -d $(LIBDIR)/icons $(LIBDIR)/Plug-ins/Timezones $(HELPDIR)
	$(INSTALL) $(HELPFILES) $(HELPDIR)
	$(INSTALL) Bits/icons/* $(LIBDIR)/icons/
	$(INSTALL) ./Bits/Timezones/vtimezones.ics $(LIBDIR)/Plug-ins/Timezones 

uninstall:
	rm -rf $(LIBDIR)/icons
	for i in $(HELPFILES); do \
		rm -f $(HELPDIR)/$$i; \
	done
	-rmdir $(HELPDIR) 2>/dev/null

