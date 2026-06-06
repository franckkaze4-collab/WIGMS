CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude
LDFLAGS =

SRC = src/main.c \
      src/person.c \
      src/category.c \
      src/priority.c \
      src/gift.c \
      src/parking.c \
      src/seating.c \
      src/schedule.c \
      src/ui_console.c

OBJ = $(SRC:.c=.o)

BIN_BASE = wigms_console
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
	del /Q src\*.o $(BIN) 2>NUL || rm -f src/*.o $(BIN)

run: all
	./$(BIN)

.PHONY: all clean run
