
ifndef QUIET
QUIET=@
endif

all: mfr

OBJS = mfr_main.o mr_file_reader.o mr_base_reader.o \
	antoine_read.o repl_states.o \
	colourtext.o markconvbold.o error.o

####################################################################

CXXFLAGS = -lm -O3 -g

LINKFLAGS += -g

CXXFLAGS += -ansi -Wall -Wno-unused-function -Wno-unused-label \
        -W -Wshadow -Wwrite-strings -Wconversion \
        -Wno-non-template-friend -Werror 

SRC_DIRS = src lu_common util

CXXFLAGS += $(addprefix -I ,$(SRC_DIRS))

####################################################################

MFR_OBJS = $(addprefix build_mfr/,$(OBJS))

MFR_AUTO_DEPS = $(MFR_OBJS:%.o=%.d)

include lu_common/makefile_use_curses.inc

####################################################################

-include $(MFR_AUTO_DEPS) # dependency files (.d)

# In such templates, all $ must be replaced by $$, to avoid evaluation
# at instantiation
define COMPILE_FROM_DIR_template
build_mfr/%.o: $(1)/%.cc build_mfr/%.d
	@echo "   CXX   $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CXX) $$(CXXFLAGS) $$< -c -o $$@

build_mfr/%.d: $(1)/%.cc
	@echo "  DEPS   $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CXX) $$(CXXFLAGS) -MM -MG $$< | \
	  sed -e 's,\($$(*F)\)\.o[ :]*,$$(dir $$@)$$*.o $$@ : ,g' \
	> $$@
endef

$(foreach dir,$(SRC_DIRS),$(eval $(call COMPILE_FROM_DIR_template,$(dir),)))

####################################################################

mfr: $(MFR_OBJS)
	@echo "   LD    $@"
	$(QUIET)$(CXX) -o $@ $(MFR_OBJS) $(LINKFLAGS) $(LIBS)

####################################################################

clean:
	rm -rf build_mfr/ mfr

####################################################################
