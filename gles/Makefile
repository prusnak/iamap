CC=gcc
CFLAGS=$(shell pkg-config glesv2 sdl2 --cflags) -Wall
LIBS=$(shell pkg-config glesv2 sdl2 --libs) -lm
LDFLAGS=

all: testgles

testgles: testgles.o
	$(CC) testgles.o $(LDFLAGS) $(LIBS) -o testgles

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o testgles

