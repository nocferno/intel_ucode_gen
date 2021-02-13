
#!/bin/bash

COMPILER="gcc "
FLAGS="-O2 "
SRC_FILES="src/ucode_gen.c src/rand_32.c main.c "
OUTPUT="-o ucode_gen "

$COMPILER $FLAGS $SRC_FILES $OUTPUT

