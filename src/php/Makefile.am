# Transform this in a real Makefile.am with libtool thing!

flom_wrap.c: flom.i
	swig -php -I.. flom.i

flom_wrap.o: flom_wrap.c
	gcc @PHP_INCLUDES@ -fpic -c flom_wrap.c

flom.so: flom_wrap.o
	gcc -shared -L/usr/local/lib -lflom -lgthread-2.0 -o flom.so flom_wrap.o
