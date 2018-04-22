all: bin/ruota.exe

bin/ruota.exe: Compiled/Main.o Compiled/Tokenizer.o Compiled/Interpreter.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o
	clang++ Compiled/Main.o Compiled/Interpreter.o Compiled/Tokenizer.o Compiled/Scope.o Compiled/Lambda.o Compiled/Node.o Compiled/Memory.o -o bin/ruota.exe -std=c++17

Compiled/Main.o: Main.cpp
	clang++ Main.cpp -o Compiled/Main.o -c -std=c++17
	
Compiled/Interpreter.o: Interpreter.cpp
	clang++ Interpreter.cpp -o Compiled/Interpreter.o -c -std=c++17

Compiled/Tokenizer.o: Tokenizer.cpp
	clang++ Tokenizer.cpp -o Compiled/Tokenizer.o -c -std=c++17

Compiled/Scope.o: Scope.cpp
	clang++ Scope.cpp -o Compiled/Scope.o -c -std=c++17

Compiled/Lambda.o: Lambda.cpp
	clang++ Lambda.cpp -o Compiled/Lambda.o -c -std=c++17

Compiled/Node.o: Node.cpp
	clang++ Node.cpp -o Compiled/Node.o -c -std=c++17

Compiled/Memory.o: Memory.cpp
	clang++ Memory.cpp -o Compiled/Memory.o -c -std=c++17