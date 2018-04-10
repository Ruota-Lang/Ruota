all: ruota.exe

ruota.exe: Main.o Tokenizer.o Interpreter.o Scope.o Lambda.o Node.o Memory.o
	g++ Compiled/Main.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o -o ruota.exe -std=c++11

Main.o: Main.cpp
	g++ Main.cpp -o Compiled/Main.o -c -std=c++11
	
Interpreter.o: Interpreter.cpp
	g++ Interpreter.cpp -o Compiled/Interpreter.o -c -std=c++11

Tokenizer.o: Tokenizer.cpp
	g++ Tokenizer.cpp -o Compiled/Tokenizer.o -c -std=c++11

Scope.o: Scope.cpp
	g++ Scope.cpp -o Compiled/Scope.o -c -std=c++11

Lambda.o: Lambda.cpp
	g++ Lambda.cpp -o Compiled/Lambda.o -c -std=c++11

Node.o: Node.cpp
	g++ Node.cpp -o Compiled/Node.o -c -std=c++11

Memory.o: Memory.cpp
	g++ Memory.cpp -o Compiled/Memory.o -c -std=c++11