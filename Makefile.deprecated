export CC=gcc
export CFLAGS=-ggdb
export LIBS=
export PREFIX=/usr

SUBDIRS = interface mca66d libmca66 control

# Please fix these rules

cntl:
	$(MAKE) -C control build

oldcntl:
	$(MAKE) -C cntl build

lib: 
	$(MAKE) -C libmca66 build

libinst:
	$(MAKE) -C libmca66 install

libuninst:
	$(MAKE) -C libmca66 uninstall


daemon:
	$(MAKE) -C mca66d build

daemoninst: 
	$(MAKE) -C mca66d install

daemonuninst:
	$(MAKE) -C mca66d uninstall


interface:
	$(MAKE) -C interface build


# Yes, this does not work
htd:
	$(MAKE) -C $@ build


all: 
	$(foreach DIR, $(SUBDIRS), $(MAKE) -C $(DIR) build;)

cleanall:
	$(foreach DIR, $(SUBDIRS), $(MAKE) -C $(DIR) clean;)

.PHONY: clean
.PHONY: cntl
.PHONY: libmca66
.PHONY: interface 
.PHONY: daemon daemoninst daemonuninst
.PHONY: htd
