
ifndef QUIET
QUIET=@
endif

all: $(ANICR_PREFIX)_anicr dumpnlj obs 

OBJS = anicr_main.o anicr_tables.o anicr_tables_sp.o accumulate.o\
	create.o packed_create.o\
	util.o couple.o

NLJ_OBJS = anicr_tables_sp.o util.o nlj.o dumpnlj.o

OBS_OBJS = anicr_tables_sp.o util.o nlj.o calobop.o caltbop.o obs.o 

####################################################################

CFLAGS = -O3 -g -std=c99 

LINKFLAGS += -g  -pg

CFLAGS += -Wall -Wno-unused-function -Wno-unused-label \
	-W -Wshadow -Wwrite-strings -Wconversion \


SRC_DIRS = ../anicr/src

CFLAGS += $(addprefix -I ,$(SRC_DIRS))
CFLAGS += -I .

CFLAGS    += `gsl-config --cflags`
LINKFLAGS += `gsl-config --libs`

####################################################################

CFLAGS += -DFILENAME_CONFIG_H=\"config_$(ANICR_PREFIX).h\"

####################################################################

ifneq (,$(TABLE_PREFIX))
CFLAGS += -DTABLE_PREFIX=\"$(TABLE_PREFIX)\"
endif

####################################################################

ANICR_OBJS = $(addprefix build_$(ANICR_PREFIX)_anicr/,$(OBJS))

ANICR_AUTO_DEPS = $(ANICR_OBJS:%.o=%.d)

DUMPNLJ_OBJS = $(addprefix build_dumpnlj/,$(NLJ_OBJS))

DUMPNLJ_AUTO_DEPS = $(DUMPNLJ_OBJS:%.o=%.d)

COMPOBS_OBJS = $(addprefix build_obs/,$(OBS_OBJS))

COMPOBS_AUTO_DEPS = $(COMPOBS_OBJS:%.o=%.d)

####################################################################

-include $(ANICR_AUTO_DEPS)

-include $(DUMPNLJ_AUTO_DEPS)
-include $(COMPOBS_AUTO_DEPS)

####################################################################

# In such templates, all $ must be replaced by $$, to avoid evaluation
# at instantiation
define COMPILE_FROM_DIR_template
build_$(ANICR_PREFIX)_anicr/%.o: $(1)/%.c  build_$(ANICR_PREFIX)_anicr/%.d
	@echo "   CC    $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) $$< -c -o $$@

build_$(ANICR_PREFIX)_anicr/%.d: $(1)/%.c
	@echo "  DEPS   $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) -MM -MG $$< | \
	  sed -e 's,\($$(*F)\)\.o[ :]*,$$(dir $$@)$$*.o $$@ : ,g' \
	> $$@
endef

$(foreach dir,$(SRC_DIRS),$(eval $(call COMPILE_FROM_DIR_template,$(dir),)))
	
# In such templates, all $ must be replaced by $$, to avoid evaluation
# at instantiation
define COMPILE_FROM_DIR_template_nlj
build_dumpnlj/%.o: $(1)/%.c  build_dumpnlj/%.d
	@echo "   CC    $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) $$< -c -o $$@

build_dumpnlj/%.d: $(1)/%.c
	@echo "  DEPS   $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) -MM -MG $$< | \
	  sed -e 's,\($$(*F)\)\.o[ :]*,$$(dir $$@)$$*.o $$@ : ,g' \
	> $$@
endef

$(foreach dir,$(SRC_DIRS),$(eval $(call COMPILE_FROM_DIR_template_nlj,$(dir),)))
###################################################################
define COMPILE_FROM_DIR_template_obs
build_obs/%.o: $(1)/%.c  build_obs/%.d
	@echo "   CC    $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) $$< -c -o $$@

build_obs/%.d: $(1)/%.c 
	@echo "  DEPS   $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) -MM -MG $$< | \
	 sed -e 's,\($$(*F)\)\.o[ :]*,$$(dir $$@)$$*.o $$@ : ,g' \
	> $$@
endef

$(foreach dir,$(SRC_DIRS),$(eval $(call COMPILE_FROM_DIR_template_obs,$(dir),)))


####################################################################


$(ANICR_PREFIX)_anicr: $(ANICR_OBJS)
	@echo "   LD    $@"
	$(QUIET)$(CC) -o $@ $(ANICR_OBJS) $(LINKFLAGS) $(LIBS)

dumpnlj: $(DUMPNLJ_OBJS)
	@echo "   LD    $@"
	$(QUIET)$(CC) -o $@ $(DUMPNLJ_OBJS) $(LINKFLAGS) $(LIBS)

obs: $(COMPOBS_OBJS)
	@echo "   LD    $@"
	$(QUIET)$(CC) -o $@ $(COMPOBS_OBJS) $(LINKFLAGS) $(LIBS)
####################################################################

clean:
	rm -rf build_anicr/ anicr

####################################################################
