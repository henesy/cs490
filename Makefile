export CC=gcc
export CFLAGS=-ggdb
export LIBS=

SUBDIRS = cntl interface mca66d

cntl:
	$(MAKE) -C $@ $(MAKECMDGOALS)

daemon:
	$(MAKE) -C mca66d $(MAKECMDGOALS)

interface:
	$(MAKE) -C $@ $(MAKECMDGOALS)

htd:
	$(MAKE) -C $@ $(MAKECMDGOALS)

all: cntl daemon interface

clean:
	$(foreach DIR, $(SUBDIRS), $(MAKE) -C $(DIR) $@;)

.PHONY: clean
.PHONY: cntl interface daemon htd
