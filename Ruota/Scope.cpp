#include "Ruota.h"

long Scope::reference_add = 0;
long Scope::reference_del = 0;

Scope::Scope(SP_Scope parent) {
	this->reference_add++;
	this->parent = parent;
}

Scope::Scope(SP_Scope parent, SP_Node main) {
	this->reference_add++;
	this->parent = parent;
	this->main = main;
}

Scope::~Scope(){
	this->reference_del++;
	this->main = nullptr;
	this->parent = nullptr;
	this->variables.clear();
}

SP_Memory Scope::execute() {
	return main->execute(to_this_ptr);
}

SP_Scope Scope::clone(SP_Scope parent) const {
	SP_Scope scope;
	if (main != nullptr)
		scope = new_scope(parent, main->clone(parent));
	else
		scope = new_scope(parent);

	for (auto &v : variables)
		scope->variables[v.first] = v.second->clone(scope);
	return scope;
}

SP_Memory Scope::declareVariable(std::string key) {
	this->variables[key] = new_memory();
	return this->variables[key];
}

SP_Memory Scope::getVariable(std::string key) {
	SP_Scope current = to_this_ptr;

	while (current != nullptr) {
		auto vars = current->variables;
		if (vars.find(key) != vars.end())
			break;
		current = current->parent;
	}

	if (current == nullptr) {
		Interpreter::throwError("Error: variable `" + key + "` undeclared!", key + " = ?");
	}

	return current->variables[key];
}

const std::string Scope::toString() const {
	return "{ " + main->toString() + " }";
}