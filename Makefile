CC=g++
CFLAGS=-Ofast -std=gnu++17 -lboost_system -lboost_filesystem -lboost_thread -DBOOST_SYSTEM_NO_DEPRECATED -lws2_32

all: win32 linux

win32: bin/ruota.exe
linux: bin/ruota.out

bin/ruota.exe: Compiled/Main.o Compiled/FileIO.o Compiled/Tree.o Compiled/Network.o Compiled/RuotaWrapper.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	$(CC) -o bin/ruota.exe Compiled/Main.o Compiled/FileIO.o Compiled/Tree.o Compiled/Network.o Compiled/RuotaWrapper.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o $(CFLAGS)

bin/ruota.out: Compiled/Main.o Compiled/FileIO.o Compiled/Tree.o Compiled/Network.o Compiled/RuotaWrapper.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	$(CC) -o bin/ruota.out Compiled/Main.o Compiled/FileIO.o Compiled/Tree.o Compiled/Network.o Compiled/RuotaWrapper.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o $(CFLAGS)

Compiled/FileIO.o: Ruota/FILE_IO/FileIO.cpp
	$(CC) Ruota/FILE_IO/FileIO.cpp -o Compiled/FileIO.o -c $(CFLAGS)

Compiled/Network.o: Ruota/NETWORK/Network.cpp
	$(CC) Ruota/NETWORK/Network.cpp -o Compiled/Network.o -c $(CFLAGS)

Compiled/Tree.o: Ruota/TREE/Tree.cpp
	$(CC) Ruota/TREE/Tree.cpp -o Compiled/Tree.o -c $(CFLAGS)

Compiled/Main.o: Main.cpp Console.ruo
	$(CC) Main.cpp -o Compiled/Main.o -c $(CFLAGS)
	
Compiled/RuotaWrapper.o: Ruota/RuotaWrapper.cpp Ruota/compiled/System.ruo
	$(CC) Ruota/RuotaWrapper.cpp -o Compiled/RuotaWrapper.o -c $(CFLAGS)

Compiled/Interpreter.o: Ruota/Interpreter.cpp
	$(CC) Ruota/Interpreter.cpp -o Compiled/Interpreter.o -c $(CFLAGS)

Compiled/Tokenizer.o: Ruota/Tokenizer.cpp
	$(CC) Ruota/Tokenizer.cpp -o Compiled/Tokenizer.o -c $(CFLAGS)

Compiled/Scope.o: Ruota/Scope.cpp
	$(CC) Ruota/Scope.cpp -o Compiled/Scope.o -c $(CFLAGS)

Compiled/Lambda.o: Ruota/Lambda.cpp
	$(CC) Ruota/Lambda.cpp -o Compiled/Lambda.o -c $(CFLAGS)

Compiled/Node.o: Ruota/Node.cpp
	$(CC) Ruota/Node.cpp -o Compiled/Node.o -c $(CFLAGS)

Compiled/Memory.o: Ruota/Memory.cpp
	$(CC) Ruota/Memory.cpp -o Compiled/Memory.o -c $(CFLAGS)