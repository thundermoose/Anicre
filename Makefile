
ifndef QUIET
QUIET=@
endif

all: mfr

OBJS = mfr_main.o mr_file_reader.o mr_base_reader.o \
	file_output.o sp_states.o pack_mp_state.o sp_pair_use.o \
	antoine_read.o missing_mpr.o repl_states.o \
	prepare_anicr.o \
	antoine_vector_read.o \
	colourtext.o markconvbold.o error.o

####################################################################

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  CXXFLAGS = -lm -O3 -g
  LINKFLAGS += -lm -g
  CXXFLAGS += -ansi -Wall -Wno-unused-function -Wno-unused-label \
        -W -Wshadow -Wwrite-strings -Wconversion \
        -Wno-non-template-friend 
endif
ifeq ($(UNAME_S),Darwin)
  CXXFLAGS = -O3 -g
  LINKFLAGS += -g
  CXXFLAGS += -ansi -Wall -Wno-unused-function -Wno-unused-label \
        -W -Wshadow -Wwrite-strings -Wconversion 
endif
#CXXFLAGS += -ansi 
#-Wall -Wno-unused-function -Wno-unused-label \
#        -W -Wshadow -Wwrite-strings -Wconversion \
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

all-anicr-%: %/tables_forw.h %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=n
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=p
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=pp
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=nn
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=np


####################################################################

n-anicr-%: %/tables_forw.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=n

####################################################################

p-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=p

####################################################################

nn-anicr-%: %/tables_forw.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=nn

####################################################################

pp-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=pp

####################################################################

np-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=np

####################################################################

pn-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=pn

####################################################################

n-tables-anicr-%: %/tables_forw.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=n_tables TABLE_PREFIX=n

####################################################################

nn-tables-anicr-%: %/tables_forw.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=nn_tables TABLE_PREFIX=nn

####################################################################

nnn-tables-anicr-%: %/tables_forw.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=nnn_tables TABLE_PREFIX=nnn

####################################################################

p-tables-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=p_tables TABLE_PREFIX=p

####################################################################

pp-tables-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=pp_tables TABLE_PREFIX=pp

####################################################################

ppp-tables-anicr-%: %/tables_rev.h
	make -C $* -f ../anicr/makever.mk ANICR_PREFIX=ppp_tables TABLE_PREFIX=ppp

####################################################################

clean-anicr-%:
	make -C $* -f ../anicr/makever.mk clean

####################################################################
