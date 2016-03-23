# Build for libhtm

LIBNAME = libhtm

CC      = g++
AR      = ar
CFLAGS  = -g -Wall -O2
LDFLAGS = -fPIC -shared -Wl,-soname,$(LIBNAME).so

LIBS = 

HFILES  = src/cell.h \
          src/column.h \
          src/dendritesegment.h \
          src/genericregion.h \
          src/htm.h \
          src/sensoryinput.h \
          src/synapse.h \
          src/codec_base.h \
          src/genericinput.h \
          src/htmregion.h \
          src/sensoryregion.h \
          src/util.h

OBJS    = obj/cell.o \
          obj/column.o \
          obj/dendritesegment.o \
          obj/genericregion.o \
          obj/htm.o \
          obj/sensoryinput.o \
          obj/synapse.o \
          obj/codec_base.o \
          obj/genericinput.o \
          obj/htmregion.o \
          obj/sensoryregion.o \
          obj/util.o

INSTALL = bin/

#---------------------------------

all:	$(OBJS) $(LIBNAME).so $(LIBNAME).a

$(LIBNAME).so:
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/$(LIBNAME).so $(OBJS) $(LIBS)

$(LIBNAME).a:
	$(AR) rcs bin/$(LIBNAME).a $(OBJS)

src/%.cpp:	

obj/%.o:	%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/cell.o:   src/cell.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/column.o:   src/column.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/dendritesegment.o:   src/dendritesegment.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/genericregion.o:   src/genericregion.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/htm.o:   src/htm.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/sensoryinput.o:   src/sensoryinput.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/synapse.o:   src/synapse.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/codec_base.o:   src/codec_base.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/genericinput.o:   src/genericinput.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/htmregion.o:   src/htmregion.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/sensoryregion.o:   src/sensoryregion.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/util.o:   src/util.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: install
install:
	cp bin/$(LIBNAME).so /usr/lib

HEADERS_DIR=/usr/include/HTM
.PHONY: headers_install
headers_install:
	if [ ! -d $(HEADERS_DIR) ]; then mkdir -p $(HEADERS_DIR); fi
	cp src/*.h $(HEADERS_DIR)

tarball:
	tar cvplzf ../$(LIBNAME)-$(VERSION).tar.gz `cat .tarball`

clean:
	rm -f $(OBJS) bin/*

