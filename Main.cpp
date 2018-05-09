#include <iostream>
#include <cmath>
#include "Ruota/Ruota.h"

#ifdef _WIN32
	#include<windows.h>
	#include <conio.h>
	#pragma comment(lib, "User32.lib")
	void _setColor(int k){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    	SetConsoleTextAttribute(hConsole, k);
	}
#else
	void _setColor(int k){
		printf("\033[3%d;40mH", k);
	}
#endif

RuotaWrapper * rw;

int console(){
	String line;
	rw->runLine("args := [];");
	std::cout << "Ruota 0.8.1 Alpha - Copyright (C) 2018 - Benjamin Park" << std::endl;

	do {
		_setColor(12);
		std::cout << "\n> ";
		_setColor(7);
		std::getline(std::cin, line);

		try {
			SP_Memory res = rw->runLine(line);
			
			_setColor(8);
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
			_setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	} while (line != "");


	_setColor(7);
	return 0;
}

int main(int argc, char * argv[]) {
	rw = new RuotaWrapper(argv[0]);

	if (argc >= 2) {
		std::string var = "[ ";
		for (int i = 2; i < argc; i++)
			var += "\"" + String(argv[i]) + "\" ";
		var += "]";
		try {
			rw->runLine("args := " + var + "; load \"" + String(argv[1]) + "\";");
		} catch (std::runtime_error &e) {
			_setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	}else{
		console();
	}
	delete rw;
	_setColor(7);
	return EXIT_SUCCESS;
}