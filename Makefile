CC=clang++
CFLAGS=-O2 -std=c++17

all: win32 linux

win32: bin/ruota.exe
linux: bin/ruota.out

bin/ruota.exe: Compiled/Main.o Compiled/RuotaWrapper.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	$(CC) $(CFLAGS) -o bin/ruota.exe Compiled/Main.o Compiled/RuotaWrapper.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o

bin/ruota.out: Compiled/Main.o Compiled/RuotaWrapper.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	$(CC) $(CFLAGS) -o bin/ruota.out Compiled/Main.o Compiled/RuotaWrapper.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o

Compiled/Main.o: Main.cpp Console.ruo
	$(CC) $(CFLAGS) Main.cpp -o Compiled/Main.o -c
	
Compiled/RuotaWrapper.o: Ruota/RuotaWrapper.cpp Ruota/compiled/System.ruo
	$(CC) $(CFLAGS) Ruota/RuotaWrapper.cpp -o Compiled/RuotaWrapper.o -c

Compiled/Interpreter.o: Ruota/Interpreter.cpp
	$(CC) $(CFLAGS) Ruota/Interpreter.cpp -o Compiled/Interpreter.o -c

Compiled/Tokenizer.o: Ruota/Tokenizer.cpp
	$(CC) $(CFLAGS) Ruota/Tokenizer.cpp -o Compiled/Tokenizer.o -c

Compiled/Scope.o: Ruota/Scope.cpp
	$(CC) $(CFLAGS) Ruota/Scope.cpp -o Compiled/Scope.o -c

Compiled/Lambda.o: Ruota/Lambda.cpp
	$(CC) $(CFLAGS) Ruota/Lambda.cpp -o Compiled/Lambda.o -c

Compiled/Node.o: Ruota/Node.cpp
	$(CC) $(CFLAGS) Ruota/Node.cpp -o Compiled/Node.o -c

Compiled/Memory.o: Ruota/Memory.cpp
	$(CC) $(CFLAGS) Ruota/Memory.cpp -o Compiled/Memory.o -c