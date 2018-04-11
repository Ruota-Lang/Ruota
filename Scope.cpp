#include "Interpreter.h"
#include <iostream>

Scope::Scope(SP_Scope parent) {
	this->parent = parent;
}

Scope::Scope(SP_Scope parent, SP_Node main) {
	this->parent = parent;
	this->main = main;
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
#ifdef DEBUG
	std::cout << " >> Retrieving Variable: " << key << std::endl;
#endif // DEBUG

	SP_Scope current = shared_from_this();

	while (current != nullptr) {
		auto vars = current->variables;

		if (vars.find(key) != vars.end())
			break;

		current = current->parent;
	}

	if (current == nullptr) {
#ifdef DEBUG
		std::cout << " >> Creating Variable: " << key << std::endl;
#endif // DEBUG
		this->variables[key] = std::make_shared<Memory>();
		return this->variables[key];
	}

	return current->variables[key];
}

String Scope::toString(){
	return "{ " + main->toString() + " }";
}