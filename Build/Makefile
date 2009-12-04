# Makefile for Mulberry builds

include make.local
include make.version

DEVHOME=../..
MULBERRY_DIR := ..

APP_DIR := ${MULBERRY_DIR}/Linux
APP_NAME := mulberry

PLUGINS_DIR := ${MULBERRY_DIR}/Plug-ins
PLUGINLIB_DIR := ${PLUGINS_DIR}/PluginLibrary

JX_DIR := ${MULBERRY_DIR}/Libraries/JX

BITS_DIR := ./Bits
TIMEZONE_DIR := ./Bits/Timezones
RESOURCES_DIR := ${MULBERRY_DIR}/Sources_Common/Resources
LICENSE_FILE := Mulberry_License
CHANGES_FILE := CHANGES
CHANGES_FILE_DEV := CHANGES-DEV
CHANGES_FILE_V3 := CHANGES-v3
CHANGES_FILE_DEV_V3 := CHANGES-DEV-v3
CHANGES_FILE_V4 := CHANGES-v4
CHANGES_FILE_DEV_V4 := CHANGES-DEV-v4

OS_NAME := $(shell echo "`uname -s`")
ifeq (${OS_NAME},SunOS)
PROC_NAME := -$(shell echo "`uname -p`")
REL_NAME := -$(shell echo "`uname -r`")
else
ifeq (${OS_NAME},Linux)
PROC_NAME := -$(shell echo "`uname -m`")
KERNEL := $(shell echo "`uname -r`")
REL_NAME := 
endif
endif

RELEASE_NAME := ${OS_NAME}${PROC_NAME}${REL_NAME}
RELEASE_DIR := ./${RELEASE_NAME}
LIB_DIR := ${RELEASE_DIR}/lib
HIDDEN_DIR := .mulberry
PLUGIN_RELEASE_DIR := ${RELEASE_DIR}/${HIDDEN_DIR}/Plug-ins
RESOURCE_RELEASE_DIR := ${RELEASE_DIR}/${HIDDEN_DIR}/Resources
TIMEZONE_RELEASE_DIR := ${PLUGIN_RELEASE_DIR}/Timezones
LINK_DIR := ./obj
RPM_DIR := ./rpmdir
RPM_SOURCES_DIR := ${RPM_DIR}/SOURCES
RPM_BUILD_DIR := ${RPM_DIR}/BUILD

TAR_FILE := mulberry-${MUL_VERSION}-${ARCHIVE_SUFFIX}.tar
GZIP_FILE := mulberry-${MUL_VERSION}-${ARCHIVE_SUFFIX}.tgz

TAR_ITEMS := ${APP_NAME} ${LICENSE_FILE} ${HIDDEN_DIR}
TAR_ITEMS_LIB := ${APP_NAME} ${LICENSE_FILE} ${HIDDEN_DIR} lib

ACE_LIB := ${JX_DIR}/ACE/ACE_wrappers/ace/libACE.so
ACE_LIB_RENAME := libACE-5_4_7.so
JCORE_LIB := ${JX_DIR}/libjcore/libjcore-2_5_0.so
JX_LIB := ${JX_DIR}/libjx/libjx-2_5_0.so
STL_LIB := ${DEVHOME}/STLport/lib/libstlport_gcc.so
GCCSTD_LIB := /usr/local/lib/libstdc++.so.2.10.0

STATIC_FLAGS := static=1
NODEBUG_FLAGS := nodebug=1

help:
	@echo "Targets:"
	@echo "svnjx    - do SVN update of JX."
	@echo "jx       - build the JX Libraries."
	@echo "svn      - do svn update of all mulberry files."
	@echo ""
	@echo "static   - build static application package."
	@echo "staticcw - build static application package without app build"
	@echo ""
	@echo "staticnoplugins     - build static application package without plugin build"
	@echo "staticnopluginscw   - build static application package without app/plugin build"
	@echo ""
	@echo "rpm      - build binary rpm package."
	@echo ""

svnjx:
	@echo "--> Updating JX"
	@(cd ${JX_DIR}; svn update)

jx:
	@echo "--> Building JX"
	@(cd ${JX_DIR}; ./build)

svn:
	@echo "--> Updating Libraries"
	@(cd ${DEVHOME}/Libraries/CICalendar; svn update)
	@(cd ${DEVHOME}/Libraries/XMLLib; svn update)
	@echo "--> Updating Sources_Common"
	@(cd ${MULBERRY_DIR}/Sources_Common; svn update)
	@echo "--> Updating Linux"
	@(cd ${MULBERRY_DIR}/Linux; svn update)
	@echo "--> Updating Plug-ins"
	@(cd ${MULBERRY_DIR}/Plug-ins; svn update)
	@echo "--> Updating Bits"
	@(svn update make.version)
	@(cd Bits; svn update)

static: cleanrelease plugins appstatic archivestatic

staticnoplugins: cleanrelease pluginsnobuild appstatic archivestatic

dynamic: cleanrelease plugins app lib archivedynamic

cleanall: cleanapp cleanplugins cleanrelease

cleanmost: cleanplugins cleanrelease

cleanapp:
	@echo ""
	@echo "--> Removing current mulberry binary"
	@rm -f ${APP_DIR}/${APP_NAME}

cleanplugins:
	@echo ""
	@echo "--> Removing current plugins"
	@(cd ${PLUGINS_DIR}; ${MAKE} clean)
	@rm -f ${PLUGINS_DIR}/*/*Plugin.so
	@rm -f ${PLUGINLIB_DIR}/*.a

cleanrelease:
	@echo ""
	@echo "--> Removing ${RELEASE_NAME}"
	@rm -rf ${RELEASE_DIR}
	@rm -f ${LINK_DIR}

createrelease:
	@echo "--> Creating ${RELEASE_NAME}"
	@mkdir -p ${RELEASE_DIR}
	@mkdir -p ${RELEASE_DIR}/${HIDDEN_DIR}
	@mkdir -p ${RESOURCE_RELEASE_DIR}
	@ln -sf ${RELEASE_DIR} ${LINK_DIR}

plugins: createrelease
	@echo "-->Building Plugins"
	@rm -rf ${PLUGIN_RELEASE_DIR}; mkdir ${PLUGIN_RELEASE_DIR}
	@(cd ${PLUGINS_DIR}; ${MAKE} all)
	@echo "-->Copying Plugins"
	@cp ${PLUGINS_DIR}/*/*Plugin.so ${PLUGIN_RELEASE_DIR}
	@strip ${PLUGIN_RELEASE_DIR}/*Plugin.so

pluginsnobuild: createrelease
	@echo "-->Copying Plugins"
	@cp ${PLUGINS_DIR}/*/*Plugin.so ${PLUGIN_RELEASE_DIR}
	@strip ${PLUGIN_RELEASE_DIR}/*Plugin.so

appstatic:
	@${MAKE} "BUILD_STATIC=1" app

app: createrelease
	@echo ""
	@echo "-->Building Mulberry"
	@rm -f ${RELEASE_DIR}/${APP_NAME}
	@(cd ${APP_DIR}; ${MAKE})
	@echo "-->Copying Mulberry"
	@cp ${APP_DIR}/${APP_NAME} ${RELEASE_DIR}
	@strip ${RELEASE_DIR}/${APP_NAME}

bits:
	@echo ""
	@echo "-->Copying Bits to appropriate locations"
	@cp ${BITS_DIR}/${LICENSE_FILE} ${RELEASE_DIR}
	@cp -r ${BITS_DIR}/icons ${RELEASE_DIR}/${HIDDEN_DIR}
	@rm -rf ${RELEASE_DIR}/${HIDDEN_DIR}/icons/CVS
	@cp ${RESOURCES_DIR}/*.xml ${RESOURCE_RELEASE_DIR}
	@cp -r ${TIMEZONE_DIR} ${PLUGIN_RELEASE_DIR}
	@rm -rf ${TIMEZONE_RELEASE_DIR}/CVS
	@rm -rf ${TIMEZONE_RELEASE_DIR}/.AppleDouble
	@cp ${BITS_DIR}/${CHANGES_FILE} ${RELEASE_DIR}/${HIDDEN_DIR}
	@cp ${BITS_DIR}/${CHANGES_FILE_DEV} ${RELEASE_DIR}/${HIDDEN_DIR}
	@cp ${BITS_DIR}/${CHANGES_FILE_V3} ${RELEASE_DIR}/${HIDDEN_DIR}
	@cp ${BITS_DIR}/${CHANGES_FILE_DEV_V3} ${RELEASE_DIR}/${HIDDEN_DIR}
	@cp ${BITS_DIR}/${CHANGES_FILE_V4} ${RELEASE_DIR}/${HIDDEN_DIR}
	@cp ${BITS_DIR}/${CHANGES_FILE_DEV_V4} ${RELEASE_DIR}/${HIDDEN_DIR}

lib:
	@echo ""
	@echo "-->Copying dynamic libraries to appropriate locations"
	@rm -rf ${LIB_DIR}
	@mkdir ${LIB_DIR}
	@cp ${ACE_LIB} ${LIB_DIR}/${ACE_LIB_RENAME}
	@cp ${JCORE_LIB} ${LIB_DIR}/
	@cp ${JX_LIB} ${LIB_DIR}/
	@cp ${STL_LIB} ${LIB_DIR}/
	@cp ${GCCSTD_LIB} ${LIB_DIR}/
	@strip ${LIB_DIR}/*

archivestatic: bits
	@(cd ${RELEASE_DIR}; \
	echo ""; \
	echo "-->Creating tar file"; \
	rm -f ${TAR_FILE} ${GZIP_FILE}; \
	tar cf ${TAR_FILE} ${TAR_ITEMS}; \
	echo "-->Creating gzip file"; \
	gzip -c ${TAR_FILE} > ${GZIP_FILE}; \
	rm -f ${TAR_FILE})

archivedynamic: bits
	@(cd ${RELEASE_DIR}; \
	echo ""; \
	echo "-->Creating tar file"; \
	rm -f ${TAR_FILE} ${GZIP_FILE}; \
	tar cf ${TAR_FILE} ${TAR_ITEMS_LIB}; \
	echo "-->Creating gzip file"; \
	gzip -c ${TAR_FILE} > ${GZIP_FILE}; \
	rm -f ${TAR_FILE})

rpm:
	echo ""
	echo "-->Creating rpm file"
	@rm -rf ${RPM_DIR}
	@mkdir ${RPM_DIR}
	@mkdir ${RPM_SOURCES_DIR}
	@touch ${RPM_SOURCES_DIR}/mulberry.tar
	@mkdir ${RPM_BUILD_DIR}
	@rpm -bb ./mulberry.spec

clean:
	@rm -rf ${RELEASE_DIR}
	@rm -rf ${RPM_DIR}

install:
	@cp ${RELEASE_DIR}/${GZIP_FILE} ~
	@(cd ~; tar zxf ${GZIP_FILE}; rm ${GZIP_FILE})
	