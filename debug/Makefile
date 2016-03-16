# Build for debug version of ACE.

# HTMDIR, UIDIR, and CODECDIR must be defined by the top-level script.

LIBNAME  = libhtm

CC       = g++
CFLAGS   = -g -Wall -O2
# Include some non-standard Qt header locations.
INCFLAGS = -I/opt/qt4/include#/usr/include/qt4 -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui
LDFLAGS  = -L$(HTMDIR)/bin -L$(UIDIR)/bin -L/opt/qt4/lib

LIBS     = $(HTMDIR)/bin/libhtm.a $(UIDIR)/bin/libqthtm.a -lQtGui -lQtCore

# External object files needing linked.
EXTOBJS  = $(CODECDIR)/obj/*.o
# Internal source and object files needing linked.
INTSRCS  = src/main.cpp
INTOBJS  = obj/main.o

OBJS     = $(EXTOBJS) $(INTOBJS)

DEST     = build/librarian

# Build targets

all: $(DEST)

$(DEST): $(INTOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(DEST) $(OBJS) $(LIBS)

obj/main.o:   src/main.cpp
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

# Install (nothing yet)

install:

.PHONY: clean
clean:
	rm -f $(INTOBJS) $(DEST)

