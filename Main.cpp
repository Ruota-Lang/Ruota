#include <iostream>
#include <cmath>
#include "Ruota/Ruota.h"

#ifdef _WIN32
	#include<windows.h>
	#include <conio.h>
	#pragma comment(lib, "User32.lib")
	void setColor(int k){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    	SetConsoleTextAttribute(hConsole, k);
	}
#else
	void setColor(int k){
		printf("\033[3%d;40mH", k);
	}
#endif

RuotaWrapper * rw;

void printToCoordinates(int x, int y, const std::string& text){
	printf("\033[%d;%dH%s\n", x, y, text.c_str());
}

std::vector<SP_Memory> __system(std::vector<SP_Memory> args) {
	return { new_memory(NUM, system(args[0]->toString().c_str())) };
}

std::vector<SP_Memory> __print(std::vector<SP_Memory> args) {
	std::cout << args[0]->toString();
	return { new_memory() };
}

std::vector<SP_Memory> __printat(std::vector<SP_Memory> args) {
	int pos_x = args[0]->getValue();
	int pos_y = args[1]->getValue();
	std::string line = args[2]->toString();
	printToCoordinates(pos_x, pos_y, line);
	return { new_memory() };
}

std::vector<SP_Memory> __exit(std::vector<SP_Memory> args) {
	quick_exit(0);
	return { new_memory() }; //superfluous?
}

std::vector<SP_Memory> __input_str(std::vector<SP_Memory> args) {
	String d;
	std::cin >> d;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return { new_memory(d) };
}

std::vector<SP_Memory> __input_line(std::vector<SP_Memory> args) {
	String d;
	std::getline(std::cin, d);
	return { new_memory(d) };
}

std::vector<SP_Memory> __random(std::vector<SP_Memory> args) {
	return { new_memory(NUM, (long double)rand() / RAND_MAX) };
}

std::vector<SP_Memory> __floor(std::vector<SP_Memory> args) {
	return { new_memory(NUM, std::floor(args[0]->getValue())) };
}

std::vector<SP_Memory> __color(std::vector<SP_Memory> args) {
	setColor(args[0]->getValue());
	return { new_memory() };
}

std::vector<SP_Memory> __raw_file(std::vector<SP_Memory> args) {
	std::ifstream myfile(args[0]->toString());
	std::string line = "";
	std::string content = "";
	if (myfile.is_open()){
		while (getline(myfile, line)){
			content += line + "\n";
		}
		myfile.close();
	} else {
		throw std::runtime_error("Error: cannot open file " + args[1]->toString() + "!");
	}
	return { new_memory(content) };
}

std::vector<SP_Memory> __key_down(std::vector<SP_Memory> args) {
	#ifdef _WIN32
		if(GetKeyState(args[0]->getValue()) & 0x8000)
			return {new_memory(NUM, 1)};
		else
			return {new_memory(NUM, 0)};
	#else
		return { new_memory(NUM, 0) };
	#endif
}

std::vector<SP_Memory> __milli(std::vector<SP_Memory> args) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	return { new_memory(NUM, timeMillis) };
}

int console(){
	String line;
	rw->runLine("args := [];");
	std::cout << "Ruota 0.8.0 Alpha - Copyright (C) 2018 - Benjamin Park" << std::endl;

	do {
		setColor(12);
		std::cout << "\n> ";
		setColor(7);
		std::getline(std::cin, line);

		try {
			SP_Memory res = rw->runLine(line);
			
			setColor(8);
			if (res->getArray().size() > 1) {
				int n = 1;
				for (auto &r : res->getArray()) {
					std::cout << "\t(" << n << ")\t" << r->toString() << std::endl;
					n++;
				}
			}
			else if(!res->getArray().empty()) {
				std::cout << "\t" << res->getArray()[0]->toString() << std::endl;
			}
			#ifdef DEBUG
			std::cout << "MEM:\t" << Memory::reference_count << std::endl;
			std::cout << "LAM:\t" << Lambda::reference_count << std::endl;
			std::cout << "NOD:\t" << Node::reference_count << std::endl;
			std::cout << "SCO:\t" << Scope::reference_count << std::endl;
			#endif
		}
		catch (std::runtime_error &e) {
			setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	} while (line != "");


	setColor(7);
	return 0;
}

int main(int argc, char * argv[]) {
	rw = new RuotaWrapper();
	Interpreter::addEmbed("console.print", &__print);
	Interpreter::addEmbed("console.printat", &__printat);
	Interpreter::addEmbed("console.system", &__system);
	Interpreter::addEmbed("console.exit", &__exit);
	Interpreter::addEmbed("console.input_str", &__input_str);
	Interpreter::addEmbed("console.input_line", &__input_line);
	Interpreter::addEmbed("console.random", &__random);
	Interpreter::addEmbed("console.floor", &__floor);
	Interpreter::addEmbed("console.color", &__color);
	Interpreter::addEmbed("console.raw_file", &__raw_file);
	Interpreter::addEmbed("console.key_down", &__key_down);
	Interpreter::addEmbed("console.milli", &__milli);

	if (argc >= 2) {
		std::string var = "[ ";
		for (int i = 2; i < argc; i++)
			var += "\"" + String(argv[i]) + "\" ";
		var += "]";
		try {
			rw->runLine("args := " + var + "; load \"" + String(argv[1]) + "\";");
		} catch (std::runtime_error &e) {
			setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	}else{
		console();
	}
	delete rw;
	setColor(7);
	return EXIT_SUCCESS;
}