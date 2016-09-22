CC=gcc
#OPTS=-O4
#OPTS=-pg
OPTS=-ggdb
OPTS+=-D_FILE_OFFSET_BITS=64
OPTS+=-Wall

VERSION=0.0.1
DEST=capnet
LIBS=-lm -lpng12 -lgif
HFILES=config.h
OBJS=img.o capnet.o

#---------------------------------

all:	$(DEST)

$(DEST):	$(OBJS)
	$(CC) $(OPTS) -o $(DEST) $(OBJS) $(LIBS)

%.c:	

%.o:	%.c $(HFILES)
	$(CC) $(OPTS) -c $< -o $@

install:

tarball:
	tar cvplzf ../capnet-$(VERSION).tar.gz `cat .tarball`

clean:
	rm $(OBJS) $(DEST)