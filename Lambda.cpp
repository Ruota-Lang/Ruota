#include "Interpreter.h"

long Lambda::reference_count = 0;

SP_Memory Lambda::execute(VEC_Memory params) {
	SP_Scope scope = std::make_shared<Scope>(parent);
	auto new_base = base->clone(scope);
	scope->main = new_base;

	scope->variables["idem"] = std::make_shared<Memory>(shared_from_this());

	for (size_t i = 0; i < param_keys.size(); i++) {
		SP_Memory temp; 
		if (i < params.size())
			temp = params[i]->clone(scope);
		else
			temp = std::make_shared<Memory>();
		scope->variables[param_keys[i]] = temp;
	}

	auto temp = scope->execute();
	if (temp->getType() == RETURN_M)
		return std::make_shared<Memory>();
	else
		return temp;
}

SP_Lambda Lambda::clone(SP_Scope parent) {
	SP_Lambda nl = std::make_shared<Lambda>(parent, base->clone(parent), param_keys);
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