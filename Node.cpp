#include "Interpreter.h"

long Node::reference_count = 0;

Node::Node(long double data) {
	this->reference_count++;
	this->mem_data = new_memory(data);
	this->nt = MEM;
}

Node::Node(NodeType nt, VEC_Node params) {
	this->reference_count++;
	this->params = params;
	this->nt = nt;
}

Node::Node(String key) {
	this->reference_count++;
	this->key = key;
	this->nt = VAR;
}

Node::Node(SP_Scope scope_ref) {
	this->reference_count++;
	this->scope_ref = scope_ref;
	this->nt = SCOPE;
}

Node::~Node(){
	this->reference_count--;
}

SP_Memory Node::execute(SP_Scope scope) {
	//std::cout << " >> " << this->toString() << std::endl;
	VEC_Memory executed;
	if (nt != DETACH && nt != THREAD && nt != NEW && nt != DES && nt != LDES && nt != DOL && nt != THEN && nt != INDEX_OBJ && nt != OBJ_SET && nt != LOCAL && nt != FROM) {
		for (auto &n : params) {
			auto e = n->execute(scope);
			if (e->getType() == BREAK_M || e->getType() == RETURN_M)
				return e;

			if (n->nt == RANGE)
				for (auto &n2 : e->getArray())
					executed.push_back(n2);
			else
				executed.push_back(e);
		}
	}

	SP_Memory temp1 = new_memory();
	switch (nt)
	{
	case BREAK:		return temp1->setType(BREAK_M);
	case RETURN:	return temp1->setType(RETURN_M);
	case VAR:		return scope->getVariable(key);
	case SET_STAT:	return executed[0]->setStatic(true);
	case STRUCT:	return executed[0]->setStruct(true);
	case SET:		return executed[0]->set(executed[1]);
	case REF_SET:	return executed[0]->refer(executed[1]);
	case MEM:		return this->mem_data;
	case ADD:		return executed[0]->add(executed[1]);
	case SUB:		return executed[0]->sub(executed[1]);
	case MUL:		return executed[0]->mul(executed[1]);
	case DIV:		return executed[0]->div(executed[1]);
	case MOD:		return executed[0]->mod(executed[1]);
	case POW:		return executed[0]->pow(executed[1]);
	case EQUAL:		return new_memory(executed[0]->equals(executed[1]));
	case NEQUAL:	return new_memory(!executed[0]->equals(executed[1]));
	case LESS:		return new_memory(executed[0]->getValue() < executed[1]->getValue());
	case MORE:		return new_memory(executed[0]->getValue() > executed[1]->getValue());
	case ELESS:		return new_memory(executed[0]->getValue() <= executed[1]->getValue());
	case EMORE:		return new_memory(executed[0]->getValue() >= executed[1]->getValue());
	case AND:		return new_memory(executed[0]->getValue() && executed[1]->getValue());
	case OR:		return new_memory(executed[0]->getValue() || executed[1]->getValue());
	case EXEC:		return executed[0]->getLambda()->execute(executed[1]->getArray());
	case STR_CAT:	return new_memory(executed[0]->toString() + executed[1]->toString());
	case OUT_CALL:	return new_memory(Interpreter::__send(executed));
	case TYPE:
		switch (executed[0]->getType())
		{
		case NUM:	return new_memory("number");
		case ARR:	return new_memory("array");
		case LAM: 	return new_memory("lambda");
		case OBJ:	return new_memory("object");
		default:	return new_memory("null");
		}
	case INDEX_OBJ:
		temp1 = params[0]->execute(scope);
		if (temp1->isStruct())
			throw std::runtime_error("Error: Cannot index properties of template object!");
		if (temp1->getScope() == nullptr)
			temp1->makeScope(scope);
		if (params[1]->nt == VAR && temp1->getScope()->variables.find(params[1]->key) == temp1->getScope()->variables.end())
			temp1->getScope()->variables[params[1]->key] = new_memory();
		return params[1]->execute(temp1->getScope());
	case OBJ_SET:
		temp1 = params[0]->execute(scope);
		temp1->setScope(params[1]->scope_ref);
		temp1->getScope()->execute();
		return temp1;
	case NEW: {
		if (params[0]->nt == EXEC){
			auto var = new_memory();
			executed.push_back(params[0]->params[0]->execute(scope));
			if (executed[0]->isStatic())
				throw std::runtime_error("Error: cannot instantiate a static object!");
			auto obj = executed[0]->getScope()->clone(scope);
			obj->variables["self"] = new_memory(obj);
			auto args = params[0]->params[1]->execute(scope)->getArray();
			var->setScope(obj);
			obj->variables["init"]->getLambda()->execute(args);
			return var;
		}else{
			executed.push_back(params[0]->execute(scope));
			auto var = new_memory();
			if (executed[0]->isStatic())
				throw std::runtime_error("Error: cannot instantiate a static object!");
			auto obj = executed[0]->getScope()->clone(scope);
			obj->variables["self"] = new_memory(obj);
			var->setScope(obj);
			return var;
		}
	}
	case LOCAL:
		scope->variables[params[0]->key] = new_memory();
		return scope->variables[params[0]->key];
	case SCOPE:
		scope_ref->parent = scope;
		temp1 = scope_ref->execute();
		if (temp1->getArray().empty())
			return temp1;
		return temp1->getArray()[0];
	case ADD_ARR: {
		auto p1 = executed[0]->getArray();
		auto p2 = executed[1]->getArray();
		VEC_Memory new_array;
		for (auto &m : p1) new_array.push_back(m);
		for (auto &m : p2) new_array.push_back(m);
		return new_memory(new_array);
	}
	case DES: {
		VEC_String param_keys;
		for (auto &n : params[0]->params) param_keys.push_back(n->key);
		return new_memory(std::make_shared<Lambda>(scope, params[1], param_keys));
	}
	case LDES: {
		VEC_String param_keys;
		for (auto &n : params[0]->params[1]->params)
			param_keys.push_back(n->key);
		auto lambda = new_memory(std::make_shared<Lambda>(scope, params[1], param_keys));
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
	case EXEC_ITER: {
		auto arg_arr = executed[0]->getArray();
		auto lambda = executed[1]->getLambda();
		VEC_Memory new_arr;

		for (auto &a : arg_arr)
			new_arr.push_back(lambda->execute({ a, new_memory(new_arr) }));

		return new_memory(new_arr);
	}
	case DOL: {
		if (params[0]->nt == ITER) {
			SP_Memory var = params[0]->params[1]->execute(scope);
			SP_Memory iter_arr;
			String iter_key = params[0]->params[0]->key;
			SP_Scope inner_scope = std::make_shared<Scope>(scope);
			if (var->getType() == ARR)
				iter_arr = var;
			else if (var->getType() == OBJ)
				iter_arr = var->getScope()->variables["iterator"]->getLambda()->execute({});
			else
				throw std::runtime_error("Error: Cannot iterate over non-iterable value!");
			for (auto &m : iter_arr->getArray()) {
				inner_scope->main = params[1]->clone(inner_scope);
				inner_scope->variables[iter_key] = m;
				SP_Memory v = inner_scope->execute();
				if (v->getType() == BREAK_M) break;
				if (v->getType() == RETURN_M) return v;
			}
			return new_memory();
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
			return new_memory();
		}
	}
	case THEN: {
		auto dec = params[0]->execute(scope);
		if (dec->getValue() != 0)
			return params[1]->execute(scope);
		else if (params.size() > 2)
			return params[2]->execute(scope);
		return new_memory();
	}
	case RANGE: {
		auto p1 = executed[0]->getValue();
		auto p2 = executed[1]->getValue();
		long double step = 1;
		if (executed.size() > 2)
			step = executed[2]->getValue();

		VEC_Memory range;
		if (flag == 0){
			for (long double i = p1; i <= p2; i += step)
				range.push_back(new_memory(i));
		}else if (flag == 1){
			for (long double i = p1; i < p2; i += step)
				range.push_back(new_memory(i));
		}
		return new_memory(range);
	}
	case LIST:
		return new_memory(executed);
	case SIZE_O: {
		if (executed[0]->getType() == ARR)
			return new_memory(executed[0]->getArray().size());
		else if (executed[0]->getType() == OBJ) {
			if (executed[0]->getScope()->variables.find("size") != executed[0]->getScope()->variables.end()) {
				auto l = executed[0]->getScope()->variables["size"]->getLambda();
				if (l != nullptr)
					return l->execute({});
			}
		}
		return new_memory(0);
	}
	case THREAD:{
		#ifdef THREADING
		std::vector<std::thread> threads;
		for (auto &n : params){
			std::thread th(threadWrapper, n, scope);
			threads.emplace_back(std::move(th));
		}
		for (std::thread & t : threads) {
			t.join();
		}
		threads.clear();
		#else
		for (auto &n : params){
			n->execute(scope);
		}
		#endif
		return new_memory();
	}
	case DETACH:{
		#ifdef THREADING
		std::thread th(threadWrapper, params[0], scope);
		th.detach();
		#else
		params[0]->execute(scope);
		#endif
		return new_memory();
	}
	case SOFT_LIST:
		if (executed.size() == 1)
			return executed[0];
		else
			return new_memory(executed);
	case INDEX: {
		auto p1 = executed[0];
		auto p2 = executed[1];
		VEC_Memory new_arr;
		for (auto &i : p2->getArray())
			new_arr.push_back(p1->index(i));
		if (new_arr.size() > 1 || new_arr.size() == 0)
			return new_memory(new_arr);
		else
			return new_arr[0];
	}
	case FROM: {
		auto scope_inner = params[1]->scope_ref;
		scope_inner->variables[params[0]->key] = new_memory();
		params[1]->execute(scope);
		return scope_inner->variables[params[0]->key];
	}
	default: return NULL;
	}
}

void Node::threadWrapper(SP_Node n, SP_Scope s){
	n->execute(s);
}

SP_Node Node::clone(SP_Scope scope) {
	VEC_Node new_params;
	for (auto &n : params)
		new_params.push_back(n->clone(scope));
	SP_Node new_node = std::make_shared<Node>(this->nt, new_params);
	new_node->key = this->key;
	new_node->flag = this->flag;
	if (this->mem_data != nullptr)
		new_node->mem_data = this->mem_data->clone(scope);
	if (this->scope_ref != nullptr)
		new_node->scope_ref = this->scope_ref->clone(scope);
	return new_node;
}

String Node::toString() {
	switch(nt){
		case VAR:		return key;
		case MEM: 		return mem_data->toString();
		case ADD: 		return "(" + params[0]->toString() + " + " + params[1]->toString() + ")";
		case SUB: 		return "(" + params[0]->toString() + " - " + params[1]->toString() + ")";
		case MUL: 		return "(" + params[0]->toString() + " * " + params[1]->toString() + ")";
		case DIV: 		return "(" + params[0]->toString() + " / " + params[1]->toString() + ")";
		case POW: 		return "(" + params[0]->toString() + " ** " + params[1]->toString() + ")";
		case MOD: 		return "(" + params[0]->toString() + " % " + params[1]->toString() + ")";
		case SET: 		return "(" + params[0]->toString() + " = " + params[1]->toString() + ")";
		case EQUAL:		return "(" + params[0]->toString() + " == " + params[1]->toString() + ")";
		case NEQUAL:	return "(" + params[0]->toString() + " != " + params[1]->toString() + ")";
		case OBJ_SET:	return "(" + params[0]->toString() + " :: " + params[1]->toString() + ")";
		case EXEC:		return "(" + params[0]->toString() + " !! " + params[1]->toString() + ")";
		case DES:		return "(" + params[0]->toString() + " -> " + params[1]->toString() + ")";
		case LDES:		return "(" + params[0]->toString() + " => " + params[1]->toString() + ")";
		case ADD_ARR:	return "(" + params[0]->toString() + " ++ " + params[1]->toString() + ")";
		case STR_CAT:	return "(" + params[0]->toString() + " .. " + params[1]->toString() + ")";
		case INDEX_OBJ:	return "(" + params[0]->toString() + " . " + params[1]->toString() + ")";
		case INDEX:		return "(" + params[0]->toString() + params[1]->toString() + ")";
		case DOL:		return "(" + params[0]->toString() + " do " + params[1]->toString() + ")";
		case FROM:		return "(" + params[0]->toString() + " from " + params[1]->toString() + ")";
		case ITER:		return "(" + params[0]->toString() + " in " + params[1]->toString() + ")";
		case RANGE:		return "(" + params[0]->toString() + " : " + params[1]->toString() + (params.size() > 2 ? " : " + params[2]->toString() : "") + ")";
		case THEN:		return "(" + params[0]->toString() + " then " + params[1]->toString() + (params.size() > 2 ? " else " + params[2]->toString() : "") + ")";
		case SCOPE:		return scope_ref->toString();
		case TYPE:		return "(&" + params[0]->toString() + ")";
		case SIZE_O:	return "(#" + params[0]->toString() + ")";
		case LOCAL:		return "(local " + params[0]->toString() + ")";
		case SET_STAT:	return "(static " + params[0]->toString() + ")";
		case NEW:		return "(new " + params[0]->toString() + ")";
		case OUT_CALL:	return "(_OUTER_CALL_ !! (" + params[0]->toString() + "))";
		case RETURN:	return "return";
		case BREAK:		return "break";
		case LIST:{
			String s = "[ ";
			for (auto &p : params)
				s += p->toString() + " ";
			return s + "]";
		}
		case SOFT_LIST:{
			String s = "( ";
			for (auto &p : params)
				s += p->toString() + " ";
			return s + ")";
		}
		default:
			return "NULL";
	}
}

void Node::weakListCheck() {
	VEC_Node new_params;
	for (auto &p : params){
		SP_Node new_node = p;
		while (new_node->nt == SOFT_LIST && new_node->params.size() == 1){
			new_node = new_node->params[0];
		}
		new_node->weakListCheck();
		new_params.push_back(new_node);
	}
	this->params = new_params;
}