#include "Ruota.h"

RuotaWrapper::RuotaWrapper(){
	this->interpreter = new Interpreter();
	main_scope = new_scope(nullptr);
}


SP_Memory RuotaWrapper::runLine(String line) {
	this->interpreter->generate(line , main_scope, "");
	return this->interpreter->execute(main_scope);
}