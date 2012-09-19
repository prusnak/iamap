NAME=sandbox
CC=gcc
CFLAGS=$(shell pkg-config libfreenect sdl --cflags) -Wall
LDFLAGS=
LIBS=$(shell pkg-config libfreenect sdl --libs) -lm

all: $(NAME)

$(NAME): $(NAME).c
	$(CC) $(NAME).c -o $(NAME) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(NAME)
