BUILDDIR ?= bin

MAKE_ALL := dpu host

FLAGS :=

.PHONY: all debug clean $(MAKE_ALL)

__dirs := $(shell mkdir -p ${BUILDDIR})

all: $(MAKE_ALL)

debug: FLAGS += debug
debug: $(MAKE_ALL)

$(MAKE_ALL):
	$(MAKE) $(FLAGS) -C $@

clean:
	$(RM) -r $(BUILDDIR)