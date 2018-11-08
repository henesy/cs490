CCFLAGS = -ggdb -I ../libmca66 -L ../libmca66
CLIBS = -lmca66
TARG = cntl
PREFIX = /usr

build: mca66cntl.c
	$(CC) -o $(TARG) $^ $(CCFLAGS) $(CFLAGS) $(CLIBS) $(LIBS)

install: build
	cp ./cntl $(PREFIX)/bin

clean: cntl
	rm cntl
