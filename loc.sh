#!/bin/bash

wc \
    ace.conf \
    Makefile \
    src/libhtm/Makefile \
    src/libqthtm/Makefile \
    src/codec/Makefile \
    src/libhtm/src/*.{cpp,h} \
    src/libqthtm/src/*.{cpp,h} \
    src/codec/src/*.{cpp,h} \
    examples/*.cpp \
| awk '{print $1}' | tail -n 1

