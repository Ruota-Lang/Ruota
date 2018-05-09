#include "Ruota.h"

void printToCoordinates(int x, int y, const std::string& text){
	printf("\033[%d;%dH%s\n", x, y, text.c_str());
}

std::vector<SP_Memory> __system(std::vector<SP_Memory> args) {
	return { new_memory(NUM, system(args[0]->toString().c_str())) };
}

#ifdef CONSOLE
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

std::vector<SP_Memory> __color(std::vector<SP_Memory> args) {
	setColor(args[0]->getValue());
	return { new_memory() };
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
#endif

std::vector<SP_Memory> __exit(std::vector<SP_Memory> args) {
	quick_exit(0);
	return { new_memory() }; //superfluous?
}

std::vector<SP_Memory> __random(std::vector<SP_Memory> args) {
	return { new_memory(NUM, (long double)rand() / RAND_MAX) };
}

std::vector<SP_Memory> __floor(std::vector<SP_Memory> args) {
	return { new_memory(NUM, std::floor(args[0]->getValue())) };
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

std::vector<SP_Memory> __milli(std::vector<SP_Memory> args) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	return { new_memory(NUM, timeMillis) };
}

RuotaWrapper::RuotaWrapper(String current_dir){
	#ifdef CONSOLE
	Interpreter::addEmbed("console.print", &__print);
	Interpreter::addEmbed("console.printat", &__printat);
	Interpreter::addEmbed("console.input_str", &__input_str);
	Interpreter::addEmbed("console.input_line", &__input_line);
	Interpreter::addEmbed("console.color", &__color);
	Interpreter::addEmbed("console.key_down", &__key_down);
	#endif
	Interpreter::addEmbed("console.system", &__system);
	Interpreter::addEmbed("console.exit", &__exit);
	Interpreter::addEmbed("console.random", &__random);
	Interpreter::addEmbed("console.floor", &__floor);
	Interpreter::addEmbed("console.raw_file", &__raw_file);
	Interpreter::addEmbed("console.milli", &__milli);
	this->current_dir = current_dir;
	while (this->current_dir.back() != '\\') {
		this->current_dir.pop_back();
	}
	this->interpreter = new Interpreter(this->current_dir);
	main_scope = new_scope(nullptr);
	#ifdef CONSOLE
	this->interpreter->generate("load \"" + this->current_dir + "defs\\Console\";", main_scope, "");
	this->interpreter->execute(main_scope);
	#endif
	this->interpreter->generate("load \"" + this->current_dir + "defs\\System\";", main_scope, "");
	this->interpreter->execute(main_scope);
}


SP_Memory RuotaWrapper::runLine(String line) {
	this->interpreter->generate(line , main_scope, "");
	return this->interpreter->execute(main_scope);
}