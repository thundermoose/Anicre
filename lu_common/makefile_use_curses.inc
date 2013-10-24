# -*- Makefile -*-

#########################################
# We generally want to use curses, if we can find it.

FILE_CURSES_CONFIG=$(shell which ncurses5-config 2> /dev/null)

ifneq (,$(FILE_CURSES_CONFIG))
CXXFLAGS     += -DUSE_CURSES=$(USING_CURSES) -DUSING_CURSES=$(USING_CURSES) \
        $(shell $(FILE_CURSES_CONFIG) --cflags)
LIBS         += $(shell $(FILE_CURSES_CONFIG) --libs)
USING_CURSES=1
else 

# Can we compile a small test program, then we are ok

HAVE_CURSES := $(shell gcc -o /dev/null -lcurses \
	$(LAND02SRCDIR)/lu_common/curses_test.c \
	2> /dev/null && echo -DHAVE_CURSES)

# If, then we claim explicit request.

ifneq (,$(HAVE_CURSES))
USING_CURSES = YES
endif

# Is CURSES explicitly requested?
ifdef USING_CURSES

ifeq ($(USING_CURSES),NO)
$(error USING_CURSES set to NO, please do not define it at all)
endif

CXXFLAGS     += -DUSE_CURSES=$(USE_CURSES) -DUSING_CURSES=$(USING_CURSES)
LIBS         += -lcurses
USING_CURSES=1

endif # USING_CURSES
endif # (,$(FILE_CURSES_CONFIG))

#########################################
