CXX=g++
CFLAGS=$(shell pkg-config --cflags libfreenect) -Wall
CXXFLAGS=$(CFLAGS)
LDFLAGS=-pthread
LIBS=$(shell pkg-config --libs libfreenect) -lGL -lglut
OBJ=armap.o sandbox.o

all: sandbox

sandbox: $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f sandbox $(OBJ)
