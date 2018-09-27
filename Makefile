CC=gcc
CFLAGS=-ggdb
ICFLAGS=-pthread -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/freetype2 -I/usr/include/libpng16
ILIBS=-lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lfontconfig -lfreetype
LIBS=

cntl: mca66cntl.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

daemon: mca66d.go
	# Build and install dependencies if missing
	go build -i mca66d.go

interface: interface.c
	$(CC) -o $@ $^ $(CFLAGS) $(ICFLAGS) $(LIBS) $(ILIBS)

htd: htd.c htd_defs.h
	$(CC) -o $@ htd.c $(CFLAGS) $(LIBS)

all: cntl daemon interface

clean:
	rm interface htd cntl mca66d *.o

