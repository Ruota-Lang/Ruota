#include "Interpreter.h"

Node::Node(long double data) {
	this->mem_data = std::make_shared<Memory>(data);
	this->nt = MEM;
}

Node::Node(NodeType nt, VEC_Node params) {
	this->params = params;
	this->nt = nt;
}

Node::Node(String key) {
	this->key = key;
	this->nt = VAR;
}

Node::Node(SP_Scope scope_ref) {
	this->scope_ref = scope_ref;
	this->nt = SCOPE;
}

SP_Memory Node::execute(SP_Scope scope) {
	VEC_Memory executed;
	if (nt != NEW && nt != DES && nt != LDES && nt != DOL && nt != THEN && nt != INDEX_OBJ && nt != OBJ_SET && nt != LOCAL && nt != FROM) {
		for (auto &n : params) {
			auto e = n->execute(scope);
			if (e->getType() == BREAK_M || e->getType() == RETURN_M)
				return e;

			if (n->nt == RANGE) {
				for (auto &n2 : e->getArray()) {
					executed.push_back(n2);
				}
			}else
				executed.push_back(e);
		}
	}

	switch (nt)
	{
	case BREAK: {
		auto temp = std::make_shared<Memory>();
		temp->setType(BREAK_M);
		return temp;
	}
	case RETURN: {
		auto temp = std::make_shared<Memory>();
		temp->setType(RETURN_M);
		return temp;
	}
	case VAR:
		return scope->getVariable(key);
	case TYPE:
		switch (executed[0]->getType())
		{
		case NUM:
			return std::make_shared<Memory>("number");
		case ARR:
			return std::make_shared<Memory>("array");
		case LAM:
			return std::make_shared<Memory>("lambda");
		case OBJ:
			return std::make_shared<Memory>("object");
		case NUL:
			return std::make_shared<Memory>("null");
		}
		break;
	case INDEX_OBJ: {
		auto e = params[0]->execute(scope);

		if (e->isStruct())
			throw std::runtime_error("Error: Cannot index properties of template object!");

		if (e->getScope() == nullptr)
			e->makeScope(scope);
		if (params[1]->nt == VAR && e->getScope()->variables.find(params[1]->key) == e->getScope()->variables.end())
			e->getScope()->variables[params[1]->key] = std::make_shared<Memory>();
		return params[1]->execute(e->getScope());
	}
	case OBJ_SET: {
		auto obj = params[0]->execute(scope);
		auto obj_data = params[1]->scope_ref;
		obj->setScope(obj_data);
		obj->getScope()->execute();
		return obj;
	}
	case NEW: {
		if (params[0]->nt == EXEC){
			auto var = std::make_shared<Memory>();
			executed.push_back(params[0]->params[0]->execute(scope));
			if (executed[0]->isStatic())
				throw std::runtime_error("Error: cannot instantiate a static object!");
			auto obj = executed[0]->getScope()->clone(scope);
			obj->variables["self"] = std::make_shared<Memory>(obj);
			auto args = params[0]->params[1]->execute(scope)->getArray();
			var->setScope(obj);
			obj->variables["init"]->getLambda()->execute(args);
			return var;
		}else{
			executed.push_back(params[0]->execute(scope));
			auto var = std::make_shared<Memory>();
			if (executed[0]->isStatic())
				throw std::runtime_error("Error: cannot instantiate a static object!");
			auto obj = executed[0]->getScope()->clone(scope);
			obj->variables["self"] = std::make_shared<Memory>(obj);
			var->setScope(obj);
			return var;
		}
	}
	case SET_STAT: {
		auto var = executed[0];
		var->setStatic(true);
		return var;
	}
	case STRUCT: {
		auto var = executed[0];
		var->setStruct(true);
		return var;
	}
	case LOCAL: {
		scope->variables[params[0]->key] = std::make_shared<Memory>();
		return scope->variables[params[0]->key];
	}
	case MEM:
		return this->mem_data;
	case SET: {
		auto temp = executed[0];
		temp->set(executed[1]);
		return temp;
	}
	case REF_SET: {
		auto temp = executed[0];
		temp->refer(executed[1]);
		return temp;
	}
	case SCOPE: {
		scope_ref->parent = scope;
		auto temp = scope_ref->execute();
		if (temp->getArray().empty())
			return temp;
		return temp->getArray()[0];
	}
	case ADD: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return p1->add(p2);
	}
	case STR_CAT: {
		auto p1 = executed[0]->toString();
		auto p2 = executed[1]->toString();
		return std::make_shared<Memory>(p1 + p2);
	}
	case ADD_ARR: {
		auto p1 = executed[0]->getArray();
		auto p2 = executed[1]->getArray();
		VEC_Memory new_array;
		for (auto &m : p1)
			new_array.push_back(m);
		for (auto &m : p2)
			new_array.push_back(m);
		return std::make_shared<Memory>(new_array);
	}
	case SUB: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return p1->sub(p2);
	}
	case MUL: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return p1->mul(p2);
	}
	case DIV: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return p1->div(p2);
	}
	case MOD: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return p1->mod(p2);
	}
	case POW: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return p1->pow(p2);
	}
	case EQUAL: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->equals(p2));
	}
	case NEQUAL: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(!p1->equals(p2));
	}
	case LESS: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->getValue() < p2->getValue());
	}
	case MORE: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->getValue() > p2->getValue());
	}
	case ELESS: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->getValue() <= p2->getValue());
	}
	case EMORE: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->getValue() >= p2->getValue());
	}
	case AND: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->getValue() && p2->getValue());
	}
	case OR: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		return std::make_shared<Memory>(p1->getValue() || p2->getValue());
	}
	case DES: {
		std::vector<String> param_keys;
		for (auto &n : params[0]->params) {
			param_keys.push_back(n->key);
		}
		return std::make_shared<Memory>(std::make_shared<Lambda>(scope, params[1], param_keys));
	}
	case LDES: {
		std::vector<String> param_keys;
		for (auto &n : params[0]->params[1]->params) {
			param_keys.push_back(n->key);
		}
		auto lambda = std::make_shared<Memory>(std::make_shared<Lambda>(scope, params[1], param_keys));
		if (params[0]->params[0]->nt != VAR) {
			auto e = params[0]->params[0]->execute(scope);
			e->set(lambda);
			return e;
		}
		else {
			auto varname = params[0]->params[0]->key;
			scope->variables[varname] = lambda->clone(scope);
			return scope->variables[varname];
		}
	}
	case EXEC: {
		auto lambda = executed[0]->getLambda();
		auto args = executed[1]->getArray();
		return lambda->execute(args);
	}
	case EXEC_ITER: {
		auto arg_arr = executed[0]->getArray();
		auto lambda = executed[1]->getLambda();
		VEC_Memory new_arr;

		for (auto &a : arg_arr) {
			new_arr.push_back(lambda->execute({ a, std::make_shared<Memory>(new_arr) }));
		}

		return std::make_shared<Memory>(new_arr);
	}
	case DOL: {
		if (params[0]->nt == ITER) {
			auto arr = params[0]->params[1]->execute(scope);
			auto iter_key = params[0]->params[0]->key;
			SP_Scope inner_scope = std::make_shared<Scope>(scope);
			for (auto &m : arr->getArray()) {
				inner_scope->main = params[1]->clone(inner_scope);
				inner_scope->variables[iter_key] = m;
				auto v = inner_scope->execute();
				if (v->getType() == BREAK_M)
					break;
				if (v->getType() == RETURN_M)
					return v;
			}
			return std::make_shared<Memory>();
		}
		else {
			auto dec = params[0];
			SP_Scope inner_scope = std::make_shared<Scope>(scope);
			while(dec->execute(scope)->getValue() != 0) {
				inner_scope->main = params[1]->clone(inner_scope);
				auto v = inner_scope->execute();
				if (v->getType() == BREAK_M)
					break;
			}
			return std::make_shared<Memory>();
		}
	}
	case THEN: {
		auto dec = params[0]->execute(scope);
		if (dec->getValue() != 0) {
			return params[1]->execute(scope);
		}
		else if (params.size() > 2) {
			return params[2]->execute(scope);
		}
		return std::make_shared<Memory>();
	}
	case RANGE: {
		auto p1 = executed[0]->getValue();
		auto p2 = executed[1]->getValue();
		long double step = 1;
		if (executed.size() > 2)
			step = executed[2]->getValue();

		VEC_Memory range;
		if (p1 <= p2 || executed.size() > 2) {
			for (long double i = p1; i <= p2; i += step) {
				range.push_back(std::make_shared<Memory>(i));
			}
		}
		else {
			for (long double i = p1; i >= p2; i -= step) {
				range.push_back(std::make_shared<Memory>(i));
			}
		}
		return std::make_shared<Memory>(range);
	}
	case LIST:
		return std::make_shared<Memory>(executed);
	case SIZE_O: {
		if (executed[0]->getType() == ARR)
			return std::make_shared<Memory>(executed[0]->getArray().size());
		else if (executed[0]->getType() == OBJ) {
			if (executed[0]->getScope()->variables.find("size") != executed[0]->getScope()->variables.end()) {
				auto l = executed[0]->getScope()->variables["size"]->getLambda();
				if (l != nullptr) {
					return l->execute({});
				}
			}
		}
		return std::make_shared<Memory>(0);
	}
	case SOFT_LIST:
		if (executed.size() == 1)
			return executed[0];
		else
			return std::make_shared<Memory>(executed);
	case INDEX: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		VEC_Memory new_arr;
		for (auto i : p2->getArray()) {
			new_arr.push_back(p1->index(i));
		}
		if (new_arr.size() > 1)
			return std::make_shared<Memory>(new_arr);
		else
			return new_arr[0];
	}
	case FROM: {
		auto scope_inner = params[1]->scope_ref;
		scope_inner->variables[params[0]->key] = std::make_shared<Memory>();
		params[1]->execute(scope);
		return scope_inner->variables[params[0]->key];
	}
	case OUT_CALL: {
		auto ret = Interpreter::__send(executed);
		return std::make_shared<Memory>(ret);
	}
	default:
		return NULL;
		break;
	}
}

SP_Node Node::clone(SP_Scope scope) {
	VEC_Node new_params;
	for (auto &n : params)
		new_params.push_back(n->clone(scope));
	SP_Node new_node = std::make_shared<Node>(this->nt, new_params);
	new_node->key = this->key;
	if (this->mem_data != nullptr)
		new_node->mem_data = this->mem_data->clone(scope);
	if (this->scope_ref != nullptr)
		new_node->scope_ref = this->scope_ref->clone(scope);
	return new_node;
}

String Node::toString() {
	switch(nt){
		case VAR:
			return key;
		case MEM:
			return mem_data->toString();
		case ADD:
			return "(" + params[0]->toString() + " + " + params[1]->toString() + ")";
		case SUB:
			return "(" + params[0]->toString() + " - " + params[1]->toString() + ")";
		case MUL:
			return "(" + params[0]->toString() + " * " + params[1]->toString() + ")";
		case DIV:
			return "(" + params[0]->toString() + " / " + params[1]->toString() + ")";
		case POW:
			return "(" + params[0]->toString() + " ** " + params[1]->toString() + ")";
		case MOD:
			return "(" + params[0]->toString() + " % " + params[1]->toString() + ")";
		case SET:
			return "(" + params[0]->toString() + " = " + params[1]->toString() + ")";
		case EQUAL:
			return "(" + params[0]->toString() + " == " + params[1]->toString() + ")";
		case NEQUAL:
			return "(" + params[0]->toString() + " != " + params[1]->toString() + ")";
		case OBJ_SET:
			return "(" + params[0]->toString() + " :: " + params[1]->toString() + ")";
		case EXEC:
			return "(" + params[0]->toString() + " !! " + params[1]->toString() + ")";
		case DES:
			return "(" + params[0]->toString() + " -> " + params[1]->toString() + ")";
		case LDES:
			return "(" + params[0]->toString() + " => " + params[1]->toString() + ")";
		case ADD_ARR:
			return "(" + params[0]->toString() + " ++ " + params[1]->toString() + ")";
		case STR_CAT:
			return "(" + params[0]->toString() + " .. " + params[1]->toString() + ")";
		case INDEX_OBJ:
			return "(" + params[0]->toString() + " . " + params[1]->toString() + ")";
		case INDEX:
			return "(" + params[0]->toString() + params[1]->toString() + ")";
		case DOL:
			return "(" + params[0]->toString() + " do " + params[1]->toString() + ")";
		case FROM:
			return "(" + params[0]->toString() + " from " + params[1]->toString() + ")";
		case ITER:
			return "(" + params[0]->toString() + " in " + params[1]->toString() + ")";
		case RANGE:
			return "(" + params[0]->toString() + " : " + params[1]->toString() + (params.size() > 2 ? " : " + params[2]->toString() : "") + ")";
		case THEN:
			return "(" + params[0]->toString() + " then " + params[1]->toString() + (params.size() > 2 ? " else " + params[2]->toString() : "") + ")";
		case SCOPE:
			return scope_ref->toString();
		case TYPE:
			return "(&" + params[0]->toString() + ")";
		case SIZE_O:
			return "(#" + params[0]->toString() + ")";
		case LOCAL:
			return "(local " + params[0]->toString() + ")";
		case SET_STAT:
			return "(static " + params[0]->toString() + ")";
		case OUT_CALL:
			return "(_OUTER_CALL_ !! (" + params[0]->toString() + "))";
		case RETURN:
			return "return";
		case BREAK:
			return "break";
		case LIST:{
			String s = "[ ";
			for (auto &p : params){
				s += p->toString() + " ";
			}
			return s + "]";
		}
		case SOFT_LIST:{
			String s = "( ";
			for (auto &p : params){
				s += p->toString() + " ";
			}
			return s + ")";
		}
		default:
			return "NULL";
	}
}