#
# Common rules for the libraries
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

debug:
	$(MAKE) "CXXDEBUG=-g -DDEBUG" all

clean:
	-rm -rf $(OBJS) $(LIBRARY)$A $(LIBRARY)$D *~

distclean:
	-$(MAKE) clean
	-rm -f config.cache config.status config.log Makefile

#
# Automatic targets and rules:
#
%$O: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%$O: %.cp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.dd: %.cpp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%.dd: %.cp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%.dd: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s|\($*\)\.o[ :]*|\1.o $@ : |g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

