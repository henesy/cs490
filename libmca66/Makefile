AR=ar
ARFLAGS=cvq
TARG=libmca66
HFILES = mca66.h
CFLAGS=$(CLFAGS) -c

build: mca66.c
	$(CC) $^ $(CFLAGS) $(LIBS)
	$(AR) $(ARFLAGS) $(TARG).a *.o

clean: 
	rm $(TARG).a
	rm *.o

homeinstall:
	cp $(TARG).a $(HOME)/lib/$(TARG).a
	cp $(HFILES) $(HOME)/include/

homeuninstall:
	rm $(HOME)/lib/$(TARG).a
	rm $(HOME)/include/

install:
	cp $(TARG).a $(PREFIX)/lib/$(TARG).a
	cp $(HFILES) $(PREFIX)/include/
