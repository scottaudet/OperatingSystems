CFLAGS = -std=c99 -g
LIBS = -lm

all: MyProj

MyProj: main.o Opcodes.o New.o
	$(CC) $(CFLAGS) -o MyProj Opcodes.o main.o New.o $(LIBS) 

main.o: main.c Vars.h
	$(CC) $(CFLAGS) -c main.c -o main.o

Opcodes.o: Opcodes.c Vars.h
	$(CC) $(CFLAGS) -c Opcodes.c -o Opcodes.o

New.o: New.c Vars.h
	$(CC) $(CFLAGS) -c New.c -o New.o