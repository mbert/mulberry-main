# Makefile for Mulberry Plugin builds

JX_MAKEMAKE = ../Libraries/JX/bin/makemake
PLUGINS_DIR := ./
PLUGINLIB_DIR := ${PLUGINS_DIR}/PluginLibrary

# Plugins to build
BUILD_PLUGINS := PluginLibrary CRAM-MD5 DIGEST-MD5 GSSAPI \
			CommunicatorAdbkIO Eudora4AdbkIO EudoraAdbkIO NetscapeAdbkIO \
			PineAdbkIO QuickmailAdbkIO vCardAdbkIO \
			ISpell ASpell \
			GPG SMIME SSL

# Plugins not built
NO_BUILD_PLUGINS := PGP QuickmailProAdbkIO POPPASSD URLPASSD

all: plugins

install:
	@cp -f */*.so ~/.mulberry/Plug-ins
	@strip ~/.mulberry/Plug-ins/*.so

installall: all install

clean: cleanplugins

cleanall:
	@for i in $(BUILD_PLUGINS); do \
		${MAKE} "PLUGIN=$$i" cleanplugin; \
	done

cleanplugins:
	@echo "--> Removing current plugins"
	@rm -f ${PLUGINS_DIR}/*/*Plugin.so
	@rm -f ${PLUGINLIB_DIR}/*.a

plugins:
	@for i in $(BUILD_PLUGINS); do \
		${MAKE} "PLUGIN=$$i" plugin; \
	done

plugin:
	@echo "-->Building ${PLUGIN}"
	@(cd ${PLUGINS_DIR}/${PLUGIN}; ../${JX_MAKEMAKE} --obj-dir BIN; mkdir -p .obj; ${MAKE})

cleanplugin:
	@echo "-->Cleaning ${PLUGIN}"
	@(cd ${PLUGINS_DIR}/${PLUGIN}; ${MAKE} clean)
