#include <iostream>
#include <cmath>
#include "Ruota/Ruota.h"

const char * console_compiled = {
	#include "Console.ruo"
};

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

void printToCoordinates(int x, int y, const std::string& text){
	printf("\033[%d;%dH%s\n", x, y, text.c_str());
}

VEC_Memory __print(VEC_Memory args) {
	std::cout << args[0]->toString();
	return { new_memory() };
}

VEC_Memory __printat(VEC_Memory args) {
	int pos_x = args[0]->getValue();
	int pos_y = args[1]->getValue();
	std::string line = args[2]->toString();
	printToCoordinates(pos_x, pos_y, line);
	return { new_memory() };
}

VEC_Memory __color(VEC_Memory args) {
	setColor(args[0]->getValue());
	return { new_memory() };
}

VEC_Memory __input_str(VEC_Memory args) {
	String d;
	std::cin >> d;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return { new_memory(d) };
}

VEC_Memory __input_line(VEC_Memory args) {
	String d;
	std::getline(std::cin, d);
	return { new_memory(d) };
}

VEC_Memory __key_down(VEC_Memory args) {
	#ifdef _WIN32
		if(GetKeyState(args[0]->getValue()) & 0x8000)
			return {new_memory(NUM, 1)};
		else
			return {new_memory(NUM, 0)};
	#else
		return { new_memory(NUM, 0) };
	#endif
}

RuotaWrapper * rw;

int checkClosed(String s) {
	int p_count = 0;
	int b_count = 0;
	int c_count = 0;
	char in_p = 0;
	for (auto &c : s) {
		if (in_p == 0) {
			switch(c){
			case '(':
				p_count++;
				break;
			case '[':
				b_count++;
				break;
			case '{':
				c_count++;
				break;
			case ')':
				p_count--;
				break;
			case ']':
				b_count--;
				break;
			case '}':
				c_count--;
				break;
			case '\"':
			case '\'':
				in_p = c;
				break;
			}
			if (p_count < 0 || b_count < 0 || c_count < 0)
				return -1;
		} else {
			if (c == in_p){
				in_p = 0;
			}
		}
	}
	if (in_p != 0)
		return 0;
	if (p_count == 0 && b_count == 0 && c_count == 0)
		return 1;
	return 0;
}

int console(){
	String line;
	rw->runLine("args := [];");
	std::cout << "Ruota 0.10.5.0 Alpha - Copyright (C) 2018 - Benjamin Park" << std::endl;

	while (true) {
		do {
			setColor(12);
			std::cout << "\n> ";
			setColor(7);
			std::getline(std::cin, line);

			String curr_line = "";
			while (checkClosed(line) == 0) {
				setColor(12);
				std::cout << ">> ";
				setColor(7);
				std::getline(std::cin, curr_line);
				line += "\n" + curr_line;
			}

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
			catch (const std::runtime_error &e) {
				setColor(12);
				std::cout << "\t" << e.what() << std::endl;
			} catch (const std::exception& e) {
				setColor(12);
				std::cout << "\t" << e.what() << std::endl;				
			} catch (const std::string& e) {
				setColor(12);
				std::cout << "\t" << e << std::endl;	
			} catch (...){
				setColor(12);
				std::cout << "\tAn undefined error has occured" << std::endl;
			}
    // ...
		} while (line != "");
		setColor(12);
		std::cout << "Quit (y/N) ?> ";
		setColor(7);
		char c = getchar();
		if (c == 'y' || c == 'Y')
			break;
	}


	setColor(7);
	return 0;
}

int main(int argc, char * argv[]) {
	rw = new RuotaWrapper(argv[0]);
	Interpreter::addEmbed("console.print", &__print);
	Interpreter::addEmbed("console.printat", &__printat);
	Interpreter::addEmbed("console.input_str", &__input_str);
	Interpreter::addEmbed("console.input_line", &__input_line);
	Interpreter::addEmbed("console.color", &__color);
	Interpreter::addEmbed("console.key_down", &__key_down);
	rw->runLine(console_compiled);

	if (argc >= 2) {
		std::string var = "[ ";
		for (int i = 2; i < argc; i++)
			var += "\"" + String(argv[i]) + "\" ";
		var += "]";
		try {
			rw->runLine("args := " + var + ";");
			rw->runLine("load \"" + String(argv[1]) + "\";");
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