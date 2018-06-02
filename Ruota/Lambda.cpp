#include "Ruota.h"

long Lambda::reference_add = 0;
long Lambda::reference_del = 0;

SP_Memory Lambda::execute(VEC_Memory params) {
	SP_Scope scope = new_scope(parent);
	auto new_base = base->clone(scope);
	scope->main = new_base;
	auto idem = scope->declareVariable("idem");
	idem->set(new_memory(to_this_ptr));

	for (size_t i = 0; i < param_keys.size(); i++) {
		SP_Memory temp = scope->declareVariable(param_keys[i]);
		if (i < params.size()){
			if (param_types[i] == 0)
				temp->set(params[i]);
			else
				temp->refer(params[i]);
		} else {
			temp->set(default_params[i]);
		}
	}

	auto temp = scope->execute();
	idem->eraseLambda();
	if (temp->getType() == RETURN_M)
		return new_memory();
	else
		return temp;
}

SP_Lambda Lambda::clone(const SP_Scope &parent) const {
	return new_lambda(parent, base->clone(parent), param_keys, param_types, default_params);
}

Lambda::Lambda(const SP_Scope &parent, const SP_Node &base, std::vector<std::string> param_keys, std::vector<int> param_types, VEC_Memory default_params) {
	this->reference_add++;
	this->param_keys = param_keys;
	this->param_types = param_types;
	this->parent = parent;
	this->base = base;
	this->default_params = default_params;
}

Lambda::~Lambda(){
	this->reference_del++;
	this->parent = nullptr;
	this->base = nullptr;
	this->default_params.clear();
}