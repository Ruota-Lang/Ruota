#include "Ruota.h"

long Node::reference_count = 0;

Node::Node(long double data) {
	this->reference_count++;
	this->mem_data = new_memory(NUM, data);
	this->nt = MEM;
}

Node::Node(SP_Memory m) {
	this->reference_count++;
	this->mem_data = m;
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

Node::Node(SP_Node val, std::unordered_map<long double, SP_Node> switch_values){
	this->reference_count++;
	this->switch_values = switch_values;
	this->nt = SWITCH;
	this->params.push_back(val);
}

Node::Node(SP_Scope scope_ref) {
	this->reference_count++;
	this->scope_ref = scope_ref;
	this->nt = SCOPE;
}

Node::~Node(){
	this->reference_count--;
	this->scope_ref = nullptr;
	this->mem_data = nullptr;
	this->params.clear();
	this->switch_values.clear();
}

SP_Memory Node::execute(SP_Scope scope) {
	VEC_Memory executed;
	if (nt != SWITCH && nt != REF_SET_DEC && nt != OBJ_LAM && nt != SET_STAT && nt != DEC_SET && nt != DETACH && nt != THREAD && nt != NEW && nt != DES && nt != LDES && nt != DOL && nt != THEN && nt != INDEX_OBJ && nt != OBJ_SET && nt != LOCAL && nt != FROM) {
		for (auto &n : params) {
			if (n == nullptr)
				Interpreter::throwError("Error: unbalanced operator!", toString());
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
	case SET_STAT:	{
		temp1 = scope->declareVariable(key);
		return temp1->setStatic(true);
	}
	case STRUCT:	{
		temp1 = scope->declareVariable(key);
		return temp1->setStruct(true);
	}
	case REF_SET:	return executed[0]->refer(executed[1]);
	case REF_SET_DEC:	{
		String key = params[0]->key;
		temp1 = scope->declareVariable(key);
		return temp1->refer(params[1]->execute(scope));
	}
	case SET:		return executed[0]->set(executed[1]);
	case ARR_SET:	{
		if (executed[0]->getArray().size() != executed[1]->getArray().size())
			Interpreter::throwError("Error: cannot multi-set arrays of incompatible lengths!", toString());
		for (int i = 0; i < executed[0]->getArray().size(); i++){
			executed[0]->getArray()[i]->set(executed[1]->getArray()[i]);
		}
		return executed[0];
	}
	case DEC_SET:	{
		String key = params[0]->key;
		auto value = params[1]->execute(scope);
		temp1 = scope->declareVariable(key);
		temp1->set(value);
		return temp1;
	}
	case MEM:		return this->mem_data;
	case ADD:		return executed[0]->add(executed[1]);
	case SUB:		return executed[0]->sub(executed[1]);
	case MUL:		return executed[0]->mul(executed[1]);
	case DIV:		return executed[0]->div(executed[1]);
	case MOD:		return executed[0]->mod(executed[1]);
	case POW:		return executed[0]->pow(executed[1]);
	case EQUAL:		return new_memory(NUM, executed[0]->equals(executed[1]));
	case NOT:		return new_memory(NUM, !executed[0]->equals(new_memory(NUM, 1)));
	case NEQUAL:	return new_memory(NUM, !executed[0]->equals(executed[1]));
	case LESS:		return new_memory(NUM, executed[0]->getValue() < executed[1]->getValue());
	case MORE:		return new_memory(NUM, executed[0]->getValue() > executed[1]->getValue());
	case ELESS:		return new_memory(NUM, executed[0]->getValue() <= executed[1]->getValue());
	case EMORE:		return new_memory(NUM, executed[0]->getValue() >= executed[1]->getValue());
	case AND:		return new_memory(NUM, executed[0]->getValue() && executed[1]->getValue());
	case OR:		return new_memory(NUM, executed[0]->getValue() || executed[1]->getValue());
	case STR_CAT:	return new_memory(executed[0]->toString() + executed[1]->toString());
	case OUT_CALL:	{
		String fname = executed[0]->toString();
		std::reverse(executed.begin(), executed.end());
		executed.pop_back();
		std::reverse(executed.begin(), executed.end());
		if (Interpreter::embedded.find(fname) == Interpreter::embedded.end())
			Interpreter::throwError	("Error: outer call `" + fname + "` does not exist!", toString());
		return new_memory(Interpreter::embedded[fname](executed));
	}
	case SWITCH:	{
		executed.push_back(params[0]->execute(scope));
		if (switch_values.find(executed[0]->getValue()) != switch_values.end())
			temp1 = switch_values[executed[0]->getValue()]->execute(scope);
		else if (params.size() > 1)
			temp1 = params[1]->execute(scope);
		return temp1;
	}
	case POP_ARR:	{
		if (executed[0]->getArray().empty())
			Interpreter::throwError("Error: cannot pop empty array!", toString());
		return executed[0]->pop();
	}
	case SHIFT_ARR:	{		
		if (executed[0]->getArray().empty())
			Interpreter::throwError("Error: cannot mov empty array!", toString());
		return executed[0]->shift();
	}
	case ALLOC: {
		if (executed[0]->getType() == ARR){
			SP_Memory base = new_memory();
			VEC_Memory base_list = {base};
			for (auto m : executed[0]->getArray()){
				VEC_Memory curr_list;
				for (auto b : base_list) {
					VEC_Memory temp_list;
					for (int i = 0; i < m->getValue(); i++){
						curr_list.push_back(new_memory());
						temp_list.push_back(curr_list.back());
					}
					b->setArray(temp_list);
				}
				base_list = curr_list;
			}
			return base;
		} else {
			auto len = executed[0]->getValue();
			VEC_Memory list;
			for (auto i = 0; i < len; i++){
				list.push_back(new_memory());
			}
			return new_memory(list);
		}
	}
	case PUSH_ARR:	return executed[0]->push(executed[1]);
	case UNSHIFT_ARR:	return executed[0]->unshift(executed[1]);
	case LAST_ARR:	return executed[0]->getArray().back();
	case VALUE:		{
		try {
			if (executed[0]->getType() == CHA)
				return new_memory(NUM, executed[0]->getValue());
			return new_memory(NUM,std::stold(executed[0]->toString()));
		} catch (...){
			Interpreter::throwError("Error: cannot convert string \"" + executed[0]->toString() + "\" to a numerical value!", toString());
		}
	}
	case EXEC:		{
		auto l = executed[0]->getLambda();
		if (l == nullptr)
			Interpreter::throwError("Error: Lambda does not exist!", toString());
		return l->execute(executed[1]->getArray());
	}
	case TOSTRING:	{
		if (executed[0]->getType() != ARR)
			return new_memory(executed[0]->toString());
		else {
			String s = "";
			for (auto &v : executed[0]->getArray()){
				if (v->getType() != CHA && v->getType() != NUM)
					Interpreter::throwError("Error: Cannot convert multidimensional array to a string!", toString());
				s += String(1,v->getValue());
			}
			return new_memory(s);
		}
	}
	case TOARR:	{
		if (executed[0]->getType() != STR && executed[0]->getType() != ARR)
			return new_memory();
		else if (executed[0]->getType() == ARR)
			return executed[0];
		else {
			VEC_Memory new_vec;
			for (auto &v : executed[0]->getArray()){
				new_vec.push_back(new_memory(NUM, v->getValue()));
			}
			return new_memory(new_vec);
		}
	}
	case TOCHAR: {
		return new_memory(CHA, executed[0]->getValue());
	}
	case TYPE:
		switch (executed[0]->getType())
		{
		case NUM:	return new_memory("number");
		case ARR:	return new_memory("array");
		case STR:	return new_memory("string");
		case CHA:	return new_memory("char");
		case LAM: 	return new_memory("lambda");
		case OBJ:	return new_memory("object");
		default:	return new_memory("null");
		}
	case INDEX_OBJ:
		temp1 = params[0]->execute(scope);
		if (temp1->isStruct())
			Interpreter::throwError("Error: Cannot index properties of template object!", toString());
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
	case OBJ_LAM:{
		temp1 = new_memory();
		temp1->setScope(params[0]->scope_ref);
		temp1->getScope()->execute();
		auto obj = temp1->getScope()->clone(scope);
		obj->variables["self"] = new_memory(obj);
		return temp1;
	}
	case NEW: {
		if (params[0]->nt == EXEC){
			auto var = new_memory();
			executed.push_back(params[0]->params[0]->execute(scope));
			if (executed[0]->isStatic())
				Interpreter::throwError("Error: cannot instantiate a static object!", toString());
			if (executed[0]->getScope() == nullptr)
				Interpreter::throwError("Error: object template does not exist!", toString());
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
				Interpreter::throwError("Error: cannot instantiate a static object!", toString());
			if (executed[0]->getScope() == nullptr)
				Interpreter::throwError("Error: object template does not exist!", toString());
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
		if (executed[0]->getType() != ARR || executed[1]->getType() != ARR)
			Interpreter::throwError("Error: cannot concatanate non-array values!", toString());
		auto p1 = executed[0]->getArray();
		auto p2 = executed[1]->getArray();
		VEC_Memory new_array;
		for (auto &m : p1) new_array.push_back(m);
		for (auto &m : p2) new_array.push_back(m);
		return new_memory(new_array);
	}
	case DES: {
		VEC_String param_keys;
		std::vector<int> param_types;
		for (auto &n : params[0]->params){
			param_keys.push_back(n->key);
			param_types.push_back(n->flag);
		}
		return new_memory(new_lambda(scope, params[1], param_keys, param_types));
	}
	case LDES: {
		VEC_String param_keys;
		std::vector<int> param_types;
		for (auto &n : params[0]->params[1]->params){
			param_keys.push_back(n->key);
			param_types.push_back(n->flag);
		}
		auto lambda = new_memory(new_lambda(scope, params[1], param_keys, param_types));
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
			SP_Scope inner_scope = new_scope(scope);
			if (var->getType() == ARR || var->getType() == STR)
				iter_arr = var;
			else if (var->getType() == OBJ)
				iter_arr = var->getScope()->variables["iterator"]->getLambda()->execute({});
			else
				Interpreter::throwError("Error: Cannot iterate over non-iterable value!", toString());
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
			SP_Scope inner_scope = new_scope(scope);
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
			if (step >= 0){
				for (long double i = p1; i < p2; i += step)
					range.push_back(new_memory(NUM, i));
			}else{
				for (long double i = p1; i > p2; i += step)
					range.push_back(new_memory(NUM, i));
			}
		}else if (flag == 1){
			if (step >= 0){
				for (long double i = p1; i <= p2; i += step)
					range.push_back(new_memory(NUM, i));
			}else{
				for (long double i = p1; i >= p2; i += step)
					range.push_back(new_memory(NUM, i));
			}
		}
		return new_memory(range);
	}
	case LIST:
		return new_memory(executed);
	case SIZE_O: {
		if (executed[0]->getType() == ARR || executed[0]->getType() == STR)
			return new_memory(NUM, executed[0]->getArray().size());
		else if (executed[0]->getType() == OBJ) {
			if (executed[0]->getScope()->variables.find("size") != executed[0]->getScope()->variables.end()) {
				auto l = executed[0]->getScope()->variables["size"]->getLambda();
				if (l != nullptr)
					return l->execute({});
			}
		}else
			Interpreter::throwError("Error: cannot get size of non-array value, or object value where size() is undefined!", toString());
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
		for (auto &i : p2->getArray()){
			auto temp = p1->index(i);
			if (temp == nullptr)
				Interpreter::throwError("Error: index out of bounds!", toString());
			new_arr.push_back(temp);
		}
		if (new_arr.size() > 1 || new_arr.size() == 0)
			return new_memory(new_arr);
		else
			return new_arr[0];
	}
	case FROM: {
		auto scope_inner = params[1]->scope_ref;
		scope_inner->variables[params[0]->key] = new_memory();
		params[1]->execute(scope);
		temp1 = scope_inner->getVariable(params[0]->key);
		return temp1;
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
	SP_Node nn = new_node(this->nt, new_params);
	nn->key = this->key;
	nn->flag = this->flag;
	if (this->nt == SWITCH){
		std::unordered_map<long double, SP_Node> nsw;
		for (auto &c : switch_values) {
			nsw[c.first] = c.second->clone(scope);
		}
		nn->switch_values = nsw;
	}
	if (this->mem_data != nullptr)
		nn->mem_data = this->mem_data->clone(scope);
	if (this->scope_ref != nullptr)
		nn->scope_ref = this->scope_ref->clone(scope);
	return nn;
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
		case LESS:		return "(" + params[0]->toString() + " < " + params[1]->toString() + ")";
		case ELESS:		return "(" + params[0]->toString() + " <= " + params[1]->toString() + ")";
		case MORE:		return "(" + params[0]->toString() + " > " + params[1]->toString() + ")";
		case EMORE:		return "(" + params[0]->toString() + " >= " + params[1]->toString() + ")";
		case NEQUAL:	return "(" + params[0]->toString() + " != " + params[1]->toString() + ")";
		case OBJ_SET:	return "(" + params[0]->toString() + " :: " + params[1]->toString() + ")";
		case EXEC:		{
			auto args = params[1]->toString().substr(1);
			args = args.substr(0, args.size() - 1);
			return params[0]->toString() + "(" + args + ")";
		}
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
		case TYPE:		return "(type " + params[0]->toString() + ")";
		case SIZE_O:	return "(len " + params[0]->toString() + ")";
		case VALUE:		return "(num " + params[0]->toString() + ")";
		case POP_ARR:	return "(pop " + params[0]->toString() + ")";
		case SHIFT_ARR:	return "(mov " + params[0]->toString() + ")";
		case TOSTRING:	return "(str " + params[0]->toString() + ")";
		case SET_STAT:	return "(static " + key + ")";
		case OBJ_LAM:	return "(struct " + params[0]->toString() + ")";
		case STRUCT:	return "(dynamic " + key + ")";
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
		SP_Node nn = p;
		while (nn->nt == SOFT_LIST && nn->params.size() == 1){
			nn = nn->params[0];
		}
		nn->weakListCheck();
		new_params.push_back(nn);
	}
	this->params = new_params;
}