# Copyright (C) 1996-2006 Markus F.X.J. Oberhumer
#
# this is modelled after build.sh; needs GNU make 3.80

SHELL = /bin/sh

##
## setup flags
##

##top_srcdir ?= ../..
top_srcdir ?= $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))../..
AR ?= ar
CC ?= cc
CFLAGS ?= -O
CC.c = $(CC)
CC.cpp = $(CXX)

CFI = -I$(top_srcdir)/include -I$(top_srcdir)
BNAME = lzopro
BLIB = lib$(BNAME).a

CF = $(CPPFLAGS) $(CFI) $(CFLAGS)
LF = $(LDFLAGS) $(LZO_EXTRA_LDFLAGS)
LL = $(BLIB) $(LIBS) $(LZO_EXTRA_LIBS)


##
## setup files
##

__map = $(foreach __map_v,$(2),$(call $(1),$(__map_v)))
__object_filename = $(addsuffix .o,$(basename $(notdir $(1))))

BLIB_SOURCES := $(wildcard $(top_srcdir)/src/*.c)
BLIB_SOURCES += $(LZO_EXTRA_SOURCES)
BLIB_OBJECTS := $(call __map,__object_filename,$(BLIB_SOURCES))
PROGRAMS = lzopack.out precomp.out selftest.out simple.out lzotest.out


##
## rules
##

all.targets ?= $(BLIB) $(PROGRAMS)
all: $(all.targets)

clean:
	rm -f *.d *.o $(BLIB) $(PROGRAMS)

$(BLIB): $(BLIB_OBJECTS) $(LZO_EXTRA_OBJECTS)
	rm -f $@
	$(AR) rcs $@ $^

# dynamically construct object rules
define BLIB_OBJECT_template
    $(call __object_filename,$(1)) : $(1) ; $$(CC$(suffix $1)) $$(CF) $$(LZO_EXTRA_CPPFLAGS) $$(LZO_EXTRA_CFLAGS) -c $$<
endef
$(foreach s,$(BLIB_SOURCES),$(eval $(call BLIB_OBJECT_template,$(s))))

lzopack.out: $(top_srcdir)/examples/lzopack.c
	$(CC) $(CF) $(LF) -o $@ $< $(LL)
precomp.out: $(top_srcdir)/examples/precomp.c
	$(CC) $(CF) $(LF) -o $@ $< $(LL)
selftest.out: $(top_srcdir)/examples/selftest.c
	$(CC) $(CF) $(LF) -o $@ $< $(LL)
simple.out: $(top_srcdir)/examples/simple.c
	$(CC) $(CF) $(LF) -o $@ $< $(LL)
lzotest.out: $(top_srcdir)/lzotest/lzotest.c
	$(CC) $(CF) $(LF) -o $@ $< $(LL)


# dependencies
$(PROGRAMS): $(BLIB)
# object dependencies are generated via -MMD
-include *.d


# debug
show_vars:
	@echo "BLIB_SOURCES = $(BLIB_SOURCES)"
	@echo "BLIB_OBJECTS = $(BLIB_OBJECTS)"
	@echo "PROGRAMS = $(PROGRAMS)"
	@echo "LZO_EXTRA_SOURCES = $(LZO_EXTRA_SOURCES)"
	@echo "LZO_EXTRA_OBJECTS = $(LZO_EXTRA_OBJECTS)"

.PHONY: all clean show_vars

