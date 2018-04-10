all: bin/ruota.exe

bin/ruota.exe: Compiled/Main.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	g++ Compiled/Main.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o -o bin/ruota.exe -std=c++11

Compiled/Main.o: Main.cpp
	g++ Main.cpp -o Compiled/Main.o -c -std=c++11
	
Compiled/Interpreter.o: Interpreter.cpp
	g++ Interpreter.cpp -o Compiled/Interpreter.o -c -std=c++11

Compiled/Tokenizer.o: Tokenizer.cpp
	g++ Tokenizer.cpp -o Compiled/Tokenizer.o -c -std=c++11

Compiled/Scope.o: Scope.cpp
	g++ Scope.cpp -o Compiled/Scope.o -c -std=c++11

Compiled/Lambda.o: Lambda.cpp
	g++ Lambda.cpp -o Compiled/Lambda.o -c -std=c++11

Compiled/Node.o: Node.cpp
	g++ Node.cpp -o Compiled/Node.o -c -std=c++11

Compiled/Memory.o: Memory.cpp
	g++ Memory.cpp -o Compiled/Memory.o -c -std=c++11