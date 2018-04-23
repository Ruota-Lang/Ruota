#include "Interpreter.h"
#include <iostream>

long Scope::reference_count = 0;

Scope::Scope(SP_Scope parent) {
	this->reference_count++;
	this->parent = parent;
}

Scope::Scope(SP_Scope parent, SP_Node main) {
	this->reference_count++;
	this->parent = parent;
	this->main = main;
}

Scope::~Scope(){
	this->reference_count--;
	this->parent = nullptr;
	this->main = nullptr;
}

SP_Memory Scope::execute() {
	return main->execute(shared_from_this());
}

SP_Scope Scope::clone(SP_Scope parent) {
	SP_Scope scope;
	if (main != nullptr)
		scope = std::make_shared<Scope>(parent, main->clone(parent));
	else
		scope = std::make_shared<Scope>(parent);

	for (auto &v : variables)
		scope->variables[v.first] = v.second->clone(scope);
	return scope;
}

SP_Memory Scope::getVariable(String key) {
	//std::cout << " >> Retrieving Variable: " << key << std::endl;

	SP_Scope current = shared_from_this();

	while (current != nullptr) {
		auto vars = current->variables;

		/*std::cout << " >> VAR SELECTION:\n";
		for (auto &v : vars) {
			std::cout << v.first << "\t" << v.second->toString() << std::endl;;
		}*/

		if (vars.find(key) != vars.end())
			break;
		current = current->parent;
	}

	if (current == nullptr) {
		//std::cout << " >> Creating Variable: " << key << std::endl;
		this->variables[key] = std::make_shared<Memory>();
		return this->variables[key];
	}

	return current->variables[key];
}

String Scope::toString(){
	return "{ " + main->toString() + " }";
}