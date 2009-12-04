#
# General rules, common to all plugins...
#

#
# Automatic stuff, should not be changed:
#
SOURCES = $(OBJS:$O=.cpp)
DEPENDS = $(OBJS:$O=.dd)

#
# rules:
#
$(LIBRARY)$A: $(OBJS)
ifdef PRELINK
	$(PRELINK) $^
endif
	$(AR) crs $@ $^

$(LIBRARY)$D: $(OBJS) 
	$(CXX) -shared -o $@ $^ $(SO_LDFLAGS)

install: all
	$(INSTALL) -d $(LIBDIR)/Plug-ins
	$(INSTALL) $(LIBRARY)$D $(LIBDIR)/Plug-ins/
	if [ -n "$(STRIP)" ]; then $(STRIP) $(LIBDIR)/Plug-ins/$(LIBRARY)$D; else true; fi

uninstall:
	rm -f $(LIBDIR)/Plug-ins/$(LIBRARY)$D
	-rmdir $(LIBDIR)/Plug-ins 2>/dev/null

clean:
	-rm -rf $(OBJS) $(LIBRARY)$A $(LIBRARY)$D *~

distclean:
	-$(MAKE) clean
	-rm -f config.cache config.status config.log Makefile

PluginDLLmain$O: ../Common/PluginDLLmain.cp sources/PluginDLLmain.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

#
# Automatic targets and rules:
#
%$O: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%$O: %.cp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%_plugin$O: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%_plugin$O: %.cp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%$O: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%_plugin$O: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.dd: %.cp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%.dd: %.cpp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%.dd: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%_plugin.dd: %.cp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%_plugin.dd: %.cpp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%_plugin.dd: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'
