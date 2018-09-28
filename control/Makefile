CCFLAGS = -I ../libmca66 -L ../libmca66
CLIBS = -lmca66
TARG = cntl

build: mca66cntl.c
	$(CC) -o $(TARG) $^ $(CCFLAGS) $(CFLAGS) $(CLIBS) $(LIBS)

clean: 
	rm cntl
