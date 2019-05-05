#!/bin/bash

gcc demo/test_grab_sys.c \
-Isrc \
-Wall \
-Wextra \
-Wundef \
-Wshadow \
-Wpointer-arith \
-Wcast-align \
-Wstrict-prototypes \
-Wstrict-overflow=5 \
-Wwrite-strings \
-Waggregate-return \
-Wcast-qual \
-Wswitch-default \
-Wswitch-enum \
-Wconversion \
-Wunreachable-code \
-fsanitize=signed-integer-overflow \
-otest_grab_sys