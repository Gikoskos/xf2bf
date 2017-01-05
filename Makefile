CC              = gcc
CFLAGS          = -std=c11 -O3 -Wall -Wextra -pedantic
EXE             =

ifeq ($(OS),Windows_NT)
    EXE += xf2bf.exe
else
    EXE += xf2bf
endif

all: $(EXE)

$(EXE): xf2bf.c
	$(CC) $(CFLAGS) $^ -o $@ 

.PHONY: clean
clean:
	@del *.exe *.o
