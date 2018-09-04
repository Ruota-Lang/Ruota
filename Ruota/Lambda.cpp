#include "Ruota.h"

long Lambda::reference_add = 0;
long Lambda::reference_del = 0;

//	Executes the general function for a Lambda
//	Takes in the set of values to be used for the functions parameters
SP_MEMORY Lambda::execute(VEC_Memory params) {
	//	Clones the function into a new scope, so that its effects may be isolated
	//	from the original body
	if (base == nullptr)
		Interpreter::throwError("Error: Undefined Lambda Behavior", "");
	SP_SCOPE scope = NEW_SCOPE(parent, "$LAMBDA$");
	auto new_base = base->clone(scope);
	scope->main = new_base;
	auto idem = scope->declareVariable("idem");
	idem->set(NEW_MEMORY(to_this_ptr));

	//	Sets all the params as references to the old values or clones them, depending
	//	on what the function specified
	for (size_t i = 0; i < param_keys.size(); i++) {
		SP_MEMORY temp = scope->declareVariable(param_keys[i]);
		if (i < params.size()){
			if (param_types[i] == 0)
				temp->set(params[i]);
			else
				temp->refer(params[i]);
		} else {
			temp->set(default_params[i]);
		}
	}

	//	Deletes the cloned lambda and its corresponding scope from memory
	auto temp = scope->execute();
	idem->eraseLambda();
	//scope.reset();

	if (temp->getType() == RETURN_M)
		return NEW_MEMORY();
	else
		return temp;
}

//	Clones a Lambda
SP_LAMBDA Lambda::clone(const SP_SCOPE &parent) const {
	std::vector<SP_MEMORY> n_params;
	for (auto &p : default_params) {
		n_params.push_back(p->clone(parent));
	}
	return NEW_LAMBDA(parent, base->clone(parent), param_keys, param_types, n_params);
}

Lambda::Lambda(const SP_SCOPE &parent, const SP_NODE &base, std::vector<std::string> param_keys, std::vector<int> param_types, VEC_Memory default_params) {
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
	if (this->base != nullptr)
		this->base->destroy();
	this->base = nullptr;
	this->default_params.clear();
}