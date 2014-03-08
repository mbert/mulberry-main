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

HAS_GNUG_3 := $(shell \
   if { $(CXX) --version | egrep '3\.2' > /dev/null; } \
   then { echo 1; } \
   else { echo 0; } fi)

HAS_GNUG_4 := $(shell \
   if { $(CXX) --version | egrep '4\.1' > /dev/null; } \
   then { echo 1; } \
   else { echo 0; } fi)

ifeq ($(HAS_GNUG_4),1)
# Do nothing
STL_INC := -fpermissive -Wno-deprecated
#else
ifeq ($(HAS_GNUG_3),1)
#STL_INC := -Wno-deprecated -I${DEV_ROOT}/STLport/stlport
else
#STL_INC := -nostdinc++ -I${DEV_ROOT}/STLport/stlport
endif
#STL_LIB := -L${DEV_ROOT}/STLport/lib -lstlport_gcc
endif

CFLAGS = ${STL_INC} -Wall -Wno-multichar -Wno-unused -Wno-unknown-pragmas \
	-g \
	-include ../Common/Plugin_Prefix_Linux.h -fPIC -D_REENTRANT ${CINCS}

LINK_STATIC := -Wl,-Bstatic
LINK_DYNAMIC := -Wl,-Bdynamic

DEPENDFLAGS = ${CFLAGS}
MAKEMAKE = makemake

LDFLAGS := -shared

LOADLIBES := -Wl,-Bsymbolic ${LINK_STATIC} \
				${STL_LIB} \
			${LINK_DYNAMIC} -lpthread
ifeq (${OSTYPE},solaris)
LOADLIBES := ${LOADLIBES} -nodefaultlibs -lc -lm -lgcc
endif

makeall: all

install: all
	cp *.so ~/.mulberry/Plug-ins
