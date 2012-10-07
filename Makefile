CXX=g++
CXXFLAGS=$(shell pkg-config --cflags libfreenect sdl2) -Wall -fno-exceptions
LDFLAGS=-pthread
LIBS=$(shell pkg-config --libs libfreenect sdl2) -lm
ifeq ($(shell test -d /opt/vc/include && echo 1),1)
CXXFLAGS+=-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -DRPI=1
LDFLAGS+=-L/opt/vc/lib
LIBS+=-lGLESv2
else
CXXFLAGS+=$(shell pkg-config --cflags glesv2)
LIBS+=$(shell pkg-config --libs glesv2)
endif
OBJ=armap.o sandbox.o

all: sandbox

sandbox: $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LIBS) -o $@


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f sandbox $(OBJ)
