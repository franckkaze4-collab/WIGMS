CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

SRC = src/main.c \
      src/person.c \
      src/category.c \
      src/priority.c \
      src/gift.c \
      src/parking.c \
      src/seating.c \
      src/schedule.c \
      src/ui_gtk.c

OBJ = $(SRC:.c=.o)

BIN_BASE = wigms
EXEEXT =
ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif
BIN = $(BIN_BASE)$(EXEEXT)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BIN)

run: all
	./$(BIN)

.PHONY: all clean run

