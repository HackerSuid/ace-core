# Generic Makefile for ACE.

# Directory containing HTM source directory.
ifndef HTMDIR
    $(error *** Missing HTMDIR definition)
endif
# Directory containing Codec source directory.
ifndef CODECDIR
    $(error *** Missing CODECDIR definition)
endif
# Directory containing Qt UI source directory.
ifndef UIDIR
    $(error *** Missing UIDIR definition)
endif

CC               = g++
CFLAGS           = -g -Wall -O2

RELEASE_INCFLAGS = -I$(HTMDIR)/src
DEBUG_INCFLAGS   = $(RELEASE_INCFLAGS) -I$(UIDIR)/src -I/opt/qt4/include

RELEASE_LDFLAGS  = -L$(HTMDIR)/bin
DEBUG_LDFLAGS    = $(RELEASE_LDFLAGS) -L$(UIDIR)/bin -L/opt/qt4/lib

RELEASE_LIBS     = $(HTMDIR)/bin/libhtm.so
DEBUG_LIBS       = $(RELEASE_LIBS) $(UIDIR)/bin/libqthtm.so -lQtGui -lQtCore -lelf

ACE_SRCS         = main.cpp
ACE_OBJS         = obj/main.o

CODEC_OBJS       = $(CODECDIR)/obj/*.o

DESTDIR          = build
BIN              = ace

# Release build directory (no UI included).
RELEASE_TGT=release
# Debug build directory (includes the UI).
DEBUG_TGT=debug

# Compile

all: $(DEBUG_TGT)

# Default build: debug
.PHONY: $(DEBUG_TGT)
$(DEBUG_TGT): $(HTMDIR) $(CODECDIR) $(UIDIR) $(ACE_OBJS)
	if [ ! -d build ]; then mkdir -p build; fi
	$(CC) $(CFLAGS) $(DEBUG_LDFLAGS) -o $(DESTDIR)/$(BIN) $(ACE_OBJS) \
        $(CODEC_OBJS) $(DEBUG_INCFLAGS) $(DEBUG_LIBS)

# release target doesn't work right now.
.PHONY: $(RELEASE_TGT)
$(RELEASE_TGT): $(HTMDIR) $(CODECDIR)
	$(MAKE) -C $(RELEASE_TGT) HTMDIR=$(HTMDIR) CODECDIR=$(CODECDIR)

.PHONY: $(HTMDIR)
$(HTMDIR):
	$(MAKE) -C $(HTMDIR)

.PHONY: $(UIDIR)
$(UIDIR):
	$(MAKE) -C $(UIDIR) HTMHEADERS=$(HTMDIR)/src

.PHONY: $(CODECDIR)
$(CODECDIR):
	$(MAKE) -C $(CODECDIR) HTMHEADERS=$(HTMDIR)/src

.PHONY: headers_install
headers_install:
	$(MAKE) -C $(HTMDIR) headers_install
	$(MAKE) -C $(UIDIR) headers_install

obj/main.o:   src/main.cpp
	if [ ! -d obj ]; then mkdir -p obj; fi
	$(CC) $(CFLAGS) $(DEBUG_INCFLAGS) -c $< -o $@

# Install

.PHONY: installall
installall:
	$(MAKE) -C $(HTMDIR) install
	$(MAKE) -C $(UIDIR) install

.PHONY: install
install:
	$(MAKE) -C $(HTMDIR) install

# Clean

.PHONY: cleanall
cleanall:
	$(MAKE) -C $(DEBUG_TGT) clean
	$(MAKE) -C $(HTMDIR) clean
	$(MAKE) -C $(UIDIR) clean
	$(MAKE) -C $(CODECDIR) clean
	rm -f $(ACE_OBJS)
	rm -f $(DESTDIR)/$(BIN)

.PHONY: clean
clean:
	$(MAKE) -C $(RELEASE_TGT) clean
	$(MAKE) -C $(HTMDIR) clean
	$(MAKE) -C $(CODECDIR) clean
	rm -f $(ACE_OBJS)

