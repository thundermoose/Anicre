
ifndef QUIET
QUIET=@
endif

all: anicr

OBJS = anicr_main.o anicr_tables.o create.o couple.o packed_create.o \
	accumulate.o util.o

####################################################################

CFLAGS = -lm -O3 -g -std=c99 # -pg

LINKFLAGS += -g # -pg

CFLAGS += -ansi -Wall -Wno-unused-function -Wno-unused-label \
	-W -Wshadow -Wwrite-strings -Wconversion \
	-Werror 

SRC_DIRS = ../anicr/src

CFLAGS += $(addprefix -I ,$(SRC_DIRS))
CFLAGS += -I .

CFLAGS    += `gsl-config --cflags`
LINKFLAGS += `gsl-config --libs`

####################################################################

ANICR_OBJS = $(addprefix build_anicr/,$(OBJS))

ANICR_AUTO_DEPS = $(ANICR_OBJS:%.o=%.d)

####################################################################

-include $(ANICR_AUTO_DEPS)

####################################################################

# In such templates, all $ must be replaced by $$, to avoid evaluation
# at instantiation
define COMPILE_FROM_DIR_template
build_anicr/%.o: $(1)/%.c build_anicr/%.d
	@echo "   CC    $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) $$< -c -o $$@

build_anicr/%.d: $(1)/%.c
	@echo "  DEPS   $$@"
	@mkdir -p $$(dir $$@)
	$$(QUIET)$$(CC) $$(CFLAGS) -MM -MG $$< | \
	  sed -e 's,\($$(*F)\)\.o[ :]*,$$(dir $$@)$$*.o $$@ : ,g' \
	> $$@
endef

$(foreach dir,$(SRC_DIRS),$(eval $(call COMPILE_FROM_DIR_template,$(dir),)))

####################################################################

anicr: $(ANICR_OBJS)
	@echo "   LD    $@"
	$(QUIET)$(CC) -o $@ $(ANICR_OBJS) $(LINKFLAGS) $(LIBS)

####################################################################

clean:
	rm -rf build_anicr/ anicr

####################################################################
