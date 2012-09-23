CXX=g++
CFLAGS=$(shell pkg-config --cflags libfreenect) -Wall -fno-exceptions
CXXFLAGS=$(CFLAGS)
LDFLAGS=-pthread
LIBS=$(shell pkg-config --libs libfreenect) -lGL -lGLU -lglut
OBJ=armap.o sandbox.o

all: sandbox

sandbox: $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f sandbox $(OBJ)
