#!/bin/bash

gcc demo/test_fps.c \
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
-obin/test_fps