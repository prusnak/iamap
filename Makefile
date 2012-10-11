CXX=g++
CXXFLAGS=-Iarmap $(shell pkg-config --cflags libfreenect sdl2) -Wall -fno-exceptions
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
OBJ_BOOK=armap/armap.o book/book.o
OBJ_SANDBOX=armap/armap.o sandbox/sandbox.o

all: book/book sandbox/sandbox

book/book: $(OBJ_BOOK)
	$(CXX) $(LDFLAGS) $(OBJ_BOOK) $(LIBS) -o $@

sandbox/sandbox: $(OBJ_SANDBOX)
	$(CXX) $(LDFLAGS) $(OBJ_SANDBOX) $(LIBS) -o $@


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f book/book sandbox/sandbox $(OBJ_BOOK) $(OBJ_SANDBOX)
