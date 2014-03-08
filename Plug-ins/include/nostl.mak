#########################################
### 
###      M A K E F I L E
###
###
###

# compiler flags ------------------------------

DEV_ROOT=../../..

CPP = g++
LINKER = ${CPP}
CFLAGS = -Wall -Wno-multichar -Wno-unused -Wno-unknown-pragmas \
	-g \
	-include ../Common/Plugin_Prefix_Linux.h -fPIC -D_REENTRANT ${CINCS}

DEPENDFLAGS = ${CFLAGS}
MAKEMAKE = makemake

LDFLAGS := -shared -fPIC

ifeq (${OSTYPE},solaris)
LOADLIBES := -nodefaultlibs -lc -lm -lgcc -lresolv
else
LOADLIBES := -lresolv
endif

makeall: all

install: all
	cp *.so ~/.mulberry/Plug-ins

