# Build for codec

NAME    = codec

# Verify the make parameter arguments.
ifeq (install, $(firstword $(MAKECMDGOALS)))
ifndef INSTALL_DIR
    $(error *** Missing INSTALL_DIR argument)
else
    $(info *** Installing in $(INSTALL_DIR))
endif
else
ifeq ($(NAME), $(firstword $(MAKECMDGOALS)))
ifndef HTMHEADERS
   	$(error *** You forgot to define the HTMHEADERS parameter)
else
    $(info *** Including $(HTMHEADERS) for header files)
endif
endif
endif

CC       = g++
CFLAGS   = -g -Wall -O2 -I$(HTMHEADERS)

LIBS = -lelf

SRC  = src/$(NAME).cpp

OBJ  = obj/$(NAME).o

#---------------------------------

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ) $(LIBS)

#src/%.cpp:	

obj/%.o:	src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

#obj/bitmap.o: src/bitmap.cpp
#	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: install
install:
	cp $(OBJ) $(INSTALL_DIR)

clean:
	rm -f $(OBJ)

