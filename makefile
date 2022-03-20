CC=gcc
CFLAGS=
LIBS=

a2r3to2:
	$(CC) $(CFLAGS) $(LIBS) a2r3to2.c -o a2r3to2

all:
	make clean; make a2r3to2

clean:
	rm a2r3to2
