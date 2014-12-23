#!/bin/sh
swig -php -I/usr/local/include flom.i
gcc $(php-config --includes) -fpic -c flom_wrap.c
gcc -shared -L/usr/local/lib -lflom -lgthread-2.0 -o flom.so flom_wrap.o
