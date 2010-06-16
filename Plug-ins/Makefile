# Makefile for Mulberry Plugin builds

O = .o

# Plugins to build
BUILD_PLUGINS := PluginLibrary CRAM-MD5 DIGEST-MD5 GSSAPI  \
			CommunicatorAdbkIO Eudora4AdbkIO EudoraAdbkIO NetscapeAdbkIO \
			PineAdbkIO QuickmailAdbkIO vCardAdbkIO \
			ISpell ASpell \
			GPG SMIME SSL

# Plugins not built
NO_BUILD_PLUGINS := PGP QuickmailProAdbkIO POPPASSD URLPASSD

.PHONY: all install uninstall clean distclean

all:
	for i in $(BUILD_PLUGINS); do \
		$(MAKE) -C $$i $@ || exit 1; \
	done

install: all
	for i in $(BUILD_PLUGINS); do \
		$(MAKE) -C $$i $@ || exit 1; \
	done

uninstall: all
	for i in $(BUILD_PLUGINS); do \
		$(MAKE) -C $$i $@; \
	done

distclean:
	@for i in $(BUILD_PLUGINS); do \
		$(MAKE) -C $$i $@; \
	done
clean:
	@for i in $(BUILD_PLUGINS); do \
		$(MAKE) -C $$i $@; \
	done

