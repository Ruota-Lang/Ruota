#include <iostream>
#include <cmath>
#include "Interpreter.h"

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
		double d;
		std::cin >> d;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return { std::make_shared<Memory>(d) };
		break;
	}
	case 3: {
		std::string d;
		std::cin >> d;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return { std::make_shared<Memory>(d) };
		break;
	}
	case 4: {
		std::string d;
		std::getline(std::cin, d);
		return { std::make_shared<Memory>(d) };
		break;
	}
	case 5: {
		return { std::make_shared<Memory>((double)rand() / RAND_MAX) };
		break;
	}
	case 6: {
		return { std::make_shared<Memory>(std::floor(args[1]->getValue())) };
		break;
	}
	default:
		return { std::make_shared<Memory>(1) };
		break;
	}
}

int main(void) {
	Interpreter * i = new Interpreter(&__send);

	std::string line;

	SP_Scope main = std::make_shared<Scope>(nullptr);
	i->generate("load \"RuotaCode\\System.ruo\";" , main, "");
	i->execute(main);

	do {
		std::cout << "> ";
		std::getline(std::cin, line);
		SP_Scope s = i->generate(line, main, "");

		try {
			SP_Memory res = i->execute(main);

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
		}
		catch (std::runtime_error &e) {
			std::cout << e.what() << std::endl;
		}
	} while (line != "");

	return 0;
}