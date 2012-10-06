CXX=g++
CFLAGS=$(shell pkg-config --cflags libfreenect glesv2 sdl2) -Wall -fno-exceptions
CXXFLAGS=$(CFLAGS)
LDFLAGS=-pthread
LIBS=$(shell pkg-config --libs libfreenect glesv2 sdl2) -lm
OBJ=armap.o sandbox.o

all: sandbox

sandbox: $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f sandbox $(OBJ)
