#include "Interpreter.h"

long Lambda::reference_count = 0;

SP_Memory Lambda::execute(VEC_Memory params) {
	SP_Scope scope = new_scope(parent);
	auto new_base = base->clone(scope);
	scope->main = new_base;

	scope->variables["idem"] = new_memory(shared_from_this());

	for (size_t i = 0; i < param_keys.size(); i++) {
		SP_Memory temp; 
		if (i < params.size())
			temp = params[i]->clone(scope);
		else
			temp = new_memory();
		scope->variables[param_keys[i]] = temp;
	}

	auto temp = scope->execute();
	if (temp->getType() == RETURN_M)
		return new_memory();
	else
		return temp;
}

SP_Lambda Lambda::clone(SP_Scope parent) {
	SP_Lambda nl = new_lambda(parent, base->clone(parent), param_keys);
	return nl;
};

Lambda::Lambda(SP_Scope parent, SP_Node base, std::vector<std::string> param_keys) {
	this->reference_count++;
	this->param_keys = param_keys;
	this->parent = parent;
	this->base = base;
}

Lambda::~Lambda(){
	this->reference_count--;
	this->parent = nullptr;
	this->base = nullptr;
}