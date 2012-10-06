CXX=g++
CXXFLAGS=$(shell pkg-config glesv2 sdl2 --cflags) -Wall
LIBS=$(shell pkg-config glesv2 sdl2 --libs) -lm
LDFLAGS=

all: testgles

testgles: testgles.o
	$(CXX) testgles.o $(LDFLAGS) $(LIBS) -o testgles

.c.o:
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f *.o testgles
