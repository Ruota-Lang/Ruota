CC=clang++
CFLAGS=-O3

all: bin/ruota.exe

bin/ruota.exe: Compiled/Main.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	$(CC) $(CFLAGS) -o bin/ruota.exe Compiled/Main.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o

Compiled/Main.o: Main.cpp
	$(CC) $(CFLAGS) Main.cpp -o Compiled/Main.o -c
	
Compiled/Interpreter.o: Interpreter.cpp
	$(CC) $(CFLAGS) Interpreter.cpp -o Compiled/Interpreter.o -c

Compiled/Tokenizer.o: Tokenizer.cpp
	$(CC) $(CFLAGS) Tokenizer.cpp -o Compiled/Tokenizer.o -c

Compiled/Scope.o: Scope.cpp
	$(CC) $(CFLAGS) Scope.cpp -o Compiled/Scope.o -c

Compiled/Lambda.o: Lambda.cpp
	$(CC) $(CFLAGS) Lambda.cpp -o Compiled/Lambda.o -c

Compiled/Node.o: Node.cpp
	$(CC) $(CFLAGS) Node.cpp -o Compiled/Node.o -c

Compiled/Memory.o: Memory.cpp
	$(CC) $(CFLAGS) Memory.cpp -o Compiled/Memory.o -c