CC=clang
CFLAGS=-ggdb
LIBS=


cntl: mca66cntl.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

interface: interface.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

htd: htd.c htd_defs.h
	$(CC) -o $@ htd.c $(CFLAGS) $(LIBS)

clean:
	rm interface htd cntl *.o

