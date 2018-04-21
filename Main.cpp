#include <iostream>
#include <cmath>
#include "Interpreter.h"

#ifdef WIN32
	#include<windows.h>
	void setColor(int k){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    	SetConsoleTextAttribute(hConsole, k);
	}
#endif

Interpreter * i;
SP_Scope main_scope;

std::vector<SP_Memory> __send(std::vector<SP_Memory> args) {
	switch ((int)args[0]->getValue())
	{
	case 0:
		return { std::make_shared<Memory>(system(args[1]->toString().c_str())) };
		break;
	case 1:
		std::cout << args[1]->toString();
		return { std::make_shared<Memory>() };
		break;
	case 2: {
		long double d;
		std::cin >> d;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return { std::make_shared<Memory>(d) };
		break;
	}
	case 3: {
		String d;
		std::cin >> d;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return { std::make_shared<Memory>(d) };
		break;
	}
	case 4: {
		String d;
		std::getline(std::cin, d);
		return { std::make_shared<Memory>(d) };
		break;
	}
	case 5: {
		return { std::make_shared<Memory>((long double)rand() / RAND_MAX) };
		break;
	}
	case 6: {
		return { std::make_shared<Memory>(std::floor(args[1]->getValue())) };
		break;
	}
	case 7: {
		#ifdef WIN32
		setColor(args[1]->getValue());
		return { std::make_shared<Memory>() };
		#else
		return { std::make_shared<Memory>(1) };
		#endif
	}
	default:
		return { std::make_shared<Memory>(1) };
		break;
	}
}

int console(){
	String line;
	//i->generate("load \"RuotaCode\\System.ruo\";" , main_scope, "");
	//i->execute(main_scope);

	std::cout << "Ruota 0.2.1 Alpha - Runtime Console" << std::endl;

	do {
		#ifdef WIN32
		setColor(7);
		#endif
		std::cout << "> ";
		std::getline(std::cin, line);

		try {
			SP_Scope s = i->generate(line, main_scope, "");
			SP_Memory res = i->execute(main_scope);
			
			#ifdef WIN32
			setColor(11);
			#endif

			if (res->getArray().size() > 1) {
				int n = 1;
				for (auto &r : res->getArray()) {
					std::cout << "(" << n << ")\t" << r->toString() << std::endl;
					n++;
				}
			}
			else if(!res->getArray().empty()) {
				std::cout << res->getArray()[0]->toString() << std::endl;
			}
			#ifdef DEBUG
			std::cout << "MEM:\t" << Memory::reference_count << std::endl;
			std::cout << "LAM:\t" << Lambda::reference_count << std::endl;
			std::cout << "NOD:\t" << Node::reference_count << std::endl;
			std::cout << "SCO:\t" << Scope::reference_count << std::endl;
			#endif
		}
		catch (std::runtime_error &e) {
			setColor(4);
			std::cout << e.what() << std::endl;
		}
	} while (line != "");


	#ifdef WIN32
	setColor(7);
	#endif
	return 0;
}

int main(int argc, char * argv[]) {
	i = new Interpreter(&__send);
	main_scope = std::make_shared<Scope>(nullptr);

	if (argc == 2) {
		i->generate("load \"" + String(argv[1]) + "\";" , main_scope, "");
		i->execute(main_scope);
	}else{
		return console();
	}
	delete i;
	return EXIT_SUCCESS;
}