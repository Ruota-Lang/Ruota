#include "Ruota.h"

const char * os_compiled = {
	#include "compiled/System.ruo"
};

std::vector<SP_Memory> __system(std::vector<SP_Memory> args) {
	return { new_memory(NUM, system(args[0]->toString().c_str())) };
}

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
	this->interpreter->generate(os_compiled, main_scope, "");
	this->interpreter->execute(main_scope);
}


SP_Memory RuotaWrapper::runLine(String line) {
	this->interpreter->generate(line , main_scope, "");
	return this->interpreter->execute(main_scope);
}