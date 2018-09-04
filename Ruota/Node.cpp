#include "Ruota.h"

long Node::reference_add = 0;
long Node::reference_del = 0;

/*
	Creates a Node that wraps itself around a memory unit with a specified 
	numerical value.
*/
Node::Node(long double data) {
	this->reference_add++;
	this->mem_data = NEW_MEMORY(NUM, data);
	this->nt = MEM;
}

/*
	Creates a wrapper node for a Memory object.
*/
Node::Node(SP_MEMORY m) {
	this->reference_add++;
	this->mem_data = m;
	this->nt = MEM;
}

/*
	Creates a Node with specified type and parameters.
*/
Node::Node(NodeType nt, VEC_Node params) {
	this->reference_add++;
	this->params = params;
	this->nt = nt;
}

/*
	Creates a Node for a vgiven ariable name.
*/
Node::Node(std::string key) {
	this->reference_add++;
	this->key = key;
	this->nt = VAR;
}

/*
	Creates a switch node.
*/
Node::Node(SP_NODE val, std::unordered_map<long double, SP_NODE> switch_values){
	this->reference_add++;
	this->switch_values = switch_values;
	this->nt = SWITCH;
	this->params.push_back(val);
}

/*
	Creates a Node for a Scope object.
*/
Node::Node(SP_SCOPE scope_ref) {
	this->reference_add++;
	this->scope_ref = scope_ref;
	this->nt = SCOPE;
}

/*
	Deconstructor
*/
Node::~Node(){
	this->reference_del++;
	destroy();
}

/*
	Clears all pointer values related to the object. Renders the Node object
	unusable.
*/
void Node::destroy() {
	//std::cout << " >> STARTING DELETION\n";
	this->mem_data = nullptr;
	if (nt == SWITCH) {
		for (auto &n : switch_values){
			if (n.second != nullptr)
				n.second->destroy();
		}
	}
	switch_values.clear();
	for (auto &n : params){
		if (n != nullptr)
			n->destroy();
	}
	params.clear();
	if (scope_ref!=nullptr){
		//this->scope_ref->parent = nullptr;
		if (this->scope_ref->main != nullptr)
			this->scope_ref->main->destroy();
		this->scope_ref->main = nullptr;
		this->scope_ref = nullptr;
	}
	//std::cout << " >> ENDING DELETION\n";
}

/*
	Node Evaluation Algorithm.
*/
SP_MEMORY Node::execute(const SP_SCOPE &scope) const {
	VEC_Memory executed;
	//	Certain types of nodes cannot be evaluated until after a particular
	//	prerequisite is met.
	if (nt != INDEX && nt != EXEC_ITER && nt != SET && nt != DECLARE && nt != TRY_CATCH && nt != SWITCH && nt != OBJ_LAM && nt != SET_STAT && nt != DETACH && nt != THREAD && nt != NEW && nt != DES && nt != LDES && nt != DOL && nt != THEN && nt != INDEX_OBJ && nt != OBJ_SET && nt != FROM) {
		for (const SP_NODE &n : params) {
			auto e = n->execute(scope);

			//	Break and return Nodes are marked and automatically returned.
			if (e->getType() == BREAK_M || e->getType() == RETURN_M)
				return e;

			//	Range Nodes are very specific in usage, if the above node is a
			//	list Node then the values are added sequentially, otherwise
			//	the values are just added as a another Node.
			if (n->nt == RANGE)
				for (auto &n2 : e->getArray())
					executed.push_back(n2);
			else
				executed.push_back(e);
		}
	}

	//	A temporary value makes some functions more efficient
	SP_MEMORY temp1 = NEW_MEMORY();
	switch (nt)
	{
	//	keys A
	case OBJ_KEYS:	{
		VEC_Memory new_list;
		for (auto e : executed[0]->getScope()->variables) {
			new_list.push_back(NEW_MEMORY(e.first));
		}
		return NEW_MEMORY(new_list);
	}
	//	eval(A)
	case EVAL:	{
		std::string last_dir = Interpreter::current_dir;
		std::string last_file = Interpreter::curr_file;
		auto n_s = NEW_SCOPE(scope, "$EVAL$");
		auto node = RuotaWrapper::interpreter->generate(executed[0]->toString(), n_s, "");
		temp1 = RuotaWrapper::interpreter->execute(node);
		Interpreter::current_dir = last_dir;
		Interpreter::curr_file = last_file;
		return temp1;
	}
	//	get A
	case GET_MET: {
		std::string key = executed[1]->toString();
		return executed[0]->getScope()->getVariable(key);
	}
	//	break
	case BREAK:		return temp1->setType(BREAK_M);
	//	returns
	case RETURN:	return temp1->setType(RETURN_M);
	//	varname
	case VAR:		return scope->getVariable(key);
	//	static A
	case SET_STAT:	{
		temp1 = scope->declareVariable(key);
		return temp1->setObjectMode(STATIC);
	}
	//	dynamic A
	case STRUCT:	{
		temp1 = scope->declareVariable(key);
		return temp1->setObjectMode(DYNAMIC);
	}
	//	virtual A
	case SET_VIR:	{
		temp1 = scope->declareVariable(key);
		return temp1->setObjectMode(VIRTUAL);
	}
	//	A &= B
	case REF_SET:	return executed[0]->refer(executed[1]);
	//	A = B
	case SET:		{
		executed.push_back(params[0]->execute(scope));
		executed.push_back(params[1]->execute(scope));
		return executed[0]->set(executed[1]);
	}
	// var A
	case DECLARE:	{
		if (params[0]->nt == LIST || params[0]->nt == SOFT_LIST){
			for (auto &v : params[0]->params)
				scope->declareVariable(v->key);
			return params[0]->execute(scope);
		} else
			return scope->declareVariable(params[0]->key);
	}
	//	Value Literal
	case MEM:		return this->mem_data;
	//	A + B
	case ADD:		switch(flag) {
		case 0: return executed[0]->add(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-add arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->add(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A - B
	case SUB:		switch(flag) {
		case 0: return executed[0]->sub(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-subtract arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->sub(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A * B
	case MUL:		switch(flag) {
		case 0: return executed[0]->mul(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-multiply arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->mul(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A / B
	case DIV:		switch(flag) {
		case 0: return executed[0]->div(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-divide arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->div(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A % B
	case MOD:		switch(flag) {
		case 0: return executed[0]->mod(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-mod arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->mod(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A ** B
	case POW:		switch(flag) {
		case 0: return executed[0]->pow(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-add arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->pow(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A == B
	case EQUAL:		switch(flag) {
		case 0: return NEW_MEMORY(NUM, executed[0]->equals(executed[1]));
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-add arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(NEW_MEMORY(NUM, executed[0]->getArray()[i]->equals(executed[1]->getArray()[i])));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	!A
	case NOT:		switch(flag) {
		case 0: return NEW_MEMORY(NUM, !executed[0]->equals(NEW_MEMORY(NUM, 1)));
		case 1: {
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(NEW_MEMORY(NUM, !executed[0]->getArray()[i]->equals(NEW_MEMORY(NUM, 1))));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A != B
	case NEQUAL:	switch(flag) {
		case 0: return NEW_MEMORY(NUM, !executed[0]->equals(executed[1]));
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-add arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(NEW_MEMORY(NUM, !executed[0]->getArray()[i]->equals(executed[1]->getArray()[i])));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A < B
	case LESS:		switch(flag) {
		case 0: return executed[0]->less(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-less-than arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->less(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A > B
	case MORE:		switch(flag) {
		case 0: return executed[0]->more(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-more-than arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->more(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A <= B
	case ELESS:		switch(flag) {
		case 0: return executed[0]->eless(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-less-than-or-equal arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->eless(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A >= B
	case EMORE:		switch(flag) {
		case 0: return executed[0]->emore(executed[1]);
		case 1: {
			if (executed[0]->getArray().size() != executed[1]->getArray().size())
				Interpreter::throwError("Error: cannot element-more-than-or-equal arrays of unequal sizes!", toString());
			VEC_Memory new_list;
			for (int i = 0; i < executed[0]->getArray().size(); i++)
				new_list.push_back(executed[0]->getArray()[i]->emore(executed[1]->getArray()[i]));
			return NEW_MEMORY(new_list);
		}
		default: Interpreter::throwError("Error: an undefined error has occured!", toString());
	}
	//	A && B
	case AND:		return NEW_MEMORY(NUM, executed[0]->getValue() && executed[1]->getValue());
	//	A || B
	case OR:		return NEW_MEMORY(NUM, executed[0]->getValue() || executed[1]->getValue());
	//	A .. B
	case STR_CAT:	return NEW_MEMORY(executed[0]->toString() + executed[1]->toString());
	//	_OUTER_CALL_(A)
	case OUT_CALL:	{
		std::string fname = executed[0]->toString();
		std::reverse(executed.begin(), executed.end());
		executed.pop_back();
		std::reverse(executed.begin(), executed.end());
		if (Interpreter::embedded.find(fname) == Interpreter::embedded.end())
			Interpreter::throwError	("Error: outer call `" + fname + "` does not exist!", toString());
		return NEW_MEMORY(Interpreter::embedded[fname](executed));
	}
	//	A switch {} >> B
	case SWITCH:	{
		executed.push_back(params[0]->execute(scope));
		long double switch_value = 0;
		if (flag == 0)
			switch_value = executed[0]->getValue();
		else
			switch_value = std::hash<std::string>{}(executed[0]->toString());
;		if (switch_values.find(switch_value) != switch_values.end()){
			temp1 = switch_values.at(switch_value)->execute(scope);
		}
		else if (params.size() > 1) {
			temp1 = params[1]->execute(scope);
		}
		return temp1;
	}
	//	pop A
	case POP_ARR:	{
		if (executed[0]->getArray().empty())
			Interpreter::throwError("Error: cannot pop empty array!", toString());
		return executed[0]->pop();
	}
	//	mov A
	case SHIFT_ARR:	{		
		if (executed[0]->getArray().empty())
			Interpreter::throwError("Error: cannot mov empty array!", toString());
		return executed[0]->shift();
	}
	//	alloc A
	case ALLOC: {
		if (executed[0]->getType() == ARR){
			SP_MEMORY base = NEW_MEMORY();
			VEC_Memory base_list = {base};
			for (auto m : executed[0]->getArray()){
				VEC_Memory curr_list;
				for (auto b : base_list) {
					VEC_Memory temp_list;
					for (int i = 0; i < m->getValue(); i++){
						curr_list.push_back(NEW_MEMORY());
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
				list.push_back(NEW_MEMORY());
			}
			return NEW_MEMORY(list);
		}
	}
	//	A push B
	case PUSH_ARR:	{
		if (executed[0]->getType() != ARR)
			Interpreter::throwError("Error: cannot push into a non-array value!", toString());
		return executed[0]->push(executed[1]);
	}
	//	A post B
	case UNSHIFT_ARR:	{
		if (executed[0]->getType() != ARR)
			Interpreter::throwError("Error: cannot post into a non-array value!", toString());
		return executed[0]->unshift(executed[1]);
	}
	//	num A
	case VALUE:		{
		try {
			if (executed[0]->getType() == CHA || executed[0]->getType() == NUL || executed[0]->getType() == OBJ)
				return NEW_MEMORY(NUM, executed[0]->getValue());
			return NEW_MEMORY(NUM,std::stold(executed[0]->toString()));
		} catch (...){
			Interpreter::throwError("Error: cannot convert string \"" + executed[0]->toString() + "\" to a numerical value!", toString());
		}
	}
	//	A(B)
	case EXEC:		{
		auto l = executed[0]->getLambda();
		if (l == nullptr)
			Interpreter::throwError("Error: Lambda does not exist!", toString());
		return l->execute(executed[1]->getArray());
	}
	//	str A
	case TOSTRING:	{
		if (executed[0]->getType() != ARR)
			return NEW_MEMORY(executed[0]->toString());
		else {
			std::string s = "";
			for (auto &v : executed[0]->getArray()){
				if (v->getType() != CHA && v->getType() != NUM)
					Interpreter::throwError("Error: Cannot convert multidimensional array to a string!", toString());
				s += std::string(1,v->getValue());
			}
			return NEW_MEMORY(s);
		}
	}
	//	arr A
	case TOARR:	{
		if (executed[0]->getType() != STR && executed[0]->getType() != ARR)
			return NEW_MEMORY();
		else if (executed[0]->getType() == ARR)
			return executed[0];
		else {
			VEC_Memory new_vec;
			for (auto &v : executed[0]->getArray()){
				new_vec.push_back(NEW_MEMORY(NUM, v->getValue()));
			}
			return NEW_MEMORY(new_vec);
		}
	}
	//	chr A
	case TOCHAR: {
		return NEW_MEMORY(CHA, executed[0]->getValue());
	}
	//	type A
	case TYPE:
		switch (executed[0]->getType())
		{
		case PTR:	return NEW_MEMORY("pointer");
		case NUM:	return NEW_MEMORY("number");
		case ARR:	return NEW_MEMORY("array");
		case STR:	return NEW_MEMORY("string");
		case CHA:	return NEW_MEMORY("char");
		case LAM: 	return NEW_MEMORY("lambda");
		case OBJ:	{
			return NEW_MEMORY(executed[0]->getScope()->key);
			//return NEW_MEMORY("object");
		}
		default:	return NEW_MEMORY("null");
		}
	//	A.B
	case INDEX_OBJ:
		temp1 = params[0]->execute(scope);
		if (temp1->getObjectMode() == DYNAMIC)
			Interpreter::throwError("Error: Cannot index properties of template object!", toString());
		if (temp1->getScope() == nullptr)
			temp1->makeScope(scope);
		if (params[1]->nt == VAR && temp1->getScope()->variables.find(params[1]->key) == temp1->getScope()->variables.end())
			temp1->getScope()->declareVariable(params[1]->key);
		return params[1]->execute(temp1->getScope());
	//	A :: B
	case OBJ_SET:
		temp1 = params[0]->execute(scope);
		if (params[1]->nt != INHERIT){
			temp1->setScope(params[1]->scope_ref);
			temp1->getScope()->execute();
		}else{
			temp1->setScope(params[1]->execute(scope)->getScope());
		}
		temp1->getScope()->key = params[0]->key;
		//temp1->getScope()->key = temp1->getScope()->getPath();
		return temp1;
	//	A +> B
	case INHERIT:{
		auto par = params[0]->execute(scope)->getScope();
		auto chi = params[1]->scope_ref;
		auto new_s = NEW_SCOPE(scope, par->key);
		for (auto &v : par->variables) {
			if (!v.second->isLocal())
				new_s->declareVariable(v.first)->set(v.second);
		}
		for (auto &v : chi->variables)
			new_s->declareVariable(v.first)->set(v.second);
		temp1->setScope(new_s);
		return temp1;
	}
	//	struct A
	case OBJ_LAM:{
		temp1 = NEW_MEMORY();
		temp1->setScope(params[0]->scope_ref);
		temp1->getScope()->execute();
		temp1->getScope()->key = "*";
		auto obj = temp1->getScope()->clone(scope);
		obj->variables["self"] = NEW_MEMORY(obj);
		return temp1;
	}
	//	new A
	case NEW: {
		if (params[0]->nt == EXEC){
			auto var = NEW_MEMORY();
			executed.push_back(params[0]->params[0]->execute(scope));
			if (executed[0]->getObjectMode() != DYNAMIC)
				Interpreter::throwError("Error: cannot instantiate a static object!", toString());
			if (executed[0]->getScope() == nullptr)
				Interpreter::throwError("Error: object template does not exist!", toString());
			auto obj = executed[0]->getScope()->clone(scope);
			obj->variables["self"] = NEW_MEMORY(obj);
			auto args = params[0]->params[1]->execute(scope)->getArray();
			var->setScope(obj);
			obj->variables["init"]->getLambda()->execute(args);
			return var;
		}else{
			auto var = NEW_MEMORY();
			executed.push_back(params[0]->execute(scope));
			if (executed[0]->getObjectMode() != DYNAMIC)
				Interpreter::throwError("Error: cannot instantiate a static object!", toString());
			if (executed[0]->getScope() == nullptr)
				Interpreter::throwError("Error: object template does not exist!", toString());
			auto obj = executed[0]->getScope()->clone(scope);
			obj->variables["self"] = NEW_MEMORY(obj);
			var->setScope(obj);
			return var;
		}
	}
	//	local A
	case LOCAL:
		return executed[0]->setLocal(true);
	//	{ ... }
	case SCOPE:
		scope_ref->parent = scope;
		temp1 = scope_ref->execute();
		if (temp1->getArray().empty())
			return temp1;
		return temp1->getArray()[0];
	//	A ++ B
	case ADD_ARR: {
		if (executed[0]->getType() != ARR || executed[1]->getType() != ARR)
			Interpreter::throwError("Error: cannot concatanate non-array values!", toString());
		auto p1 = executed[0]->getArray();
		auto p2 = executed[1]->getArray();
		VEC_Memory new_array;
		for (auto &m : p1) new_array.push_back(m);
		for (auto &m : p2) new_array.push_back(m);
		return NEW_MEMORY(new_array);
	}
	//	A -> B
	case DES: {
		std::vector<std::string> param_keys;
		std::vector<int> param_types;
		VEC_Memory default_params;
		for (auto &n : params[0]->params){
			if (n->nt == SET) {
				param_keys.push_back(n->params[0]->key);
				param_types.push_back(n->params[0]->flag);
				default_params.push_back(n->params[1]->execute(scope));
			} else {
				param_keys.push_back(n->key);
				param_types.push_back(n->flag);
				default_params.push_back(NEW_MEMORY());
			}
		}
		return NEW_MEMORY(NEW_LAMBDA(scope, params[1], param_keys, param_types, default_params));
	}
	//	A(B) => C
	case LDES: {
		std::vector<std::string> param_keys;
		std::vector<int> param_types;
		VEC_Memory default_params;
		for (auto &n : params[0]->params[1]->params){
			if (n->nt == SET) {
				param_keys.push_back(n->params[0]->key);
				param_types.push_back(n->params[0]->flag);
				default_params.push_back(n->params[1]->execute(scope));
			} else {
				param_keys.push_back(n->key);
				param_types.push_back(n->flag);
				default_params.push_back(NEW_MEMORY());
			}
		}
		auto lambda = NEW_MEMORY(NEW_LAMBDA(scope, params[1], param_keys, param_types, default_params));
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
	//	A in B ->> C
	case EXEC_ITER: {
		VEC_Memory list;	
		if (params[0]->nt == ITER) {
			SP_MEMORY var = params[0]->params[1]->execute(scope);
			SP_MEMORY iter_arr;
			SP_SCOPE inner_scope = NEW_SCOPE(scope, "$LOOP$");
			
			if (var->getType() == ARR || var->getType() == STR) {
				iter_arr = var;
				if (params[0]->params[0]->nt == VAR){
					std::string iter_key = params[0]->params[0]->key;
					for (auto &m : iter_arr->getArray()) {
						inner_scope->main = params[1]->clone(inner_scope);
						inner_scope->variables[iter_key] = m;
						SP_MEMORY v = inner_scope->execute();
						if (v->getType() == RETURN_M) return v;
						list.push_back(v);
						if (v->getType() == BREAK_M) break;
					}
				}else {
					std::vector<std::string> iter_keys;
					for (auto &v : params[0]->params[0]->params)
						iter_keys.push_back(v->key);
					VEC_Memory iter_values = iter_arr->getArray();
					for (int i = 0; i < iter_values[0]->getArray().size(); i++) {
						inner_scope->main = params[1]->clone(inner_scope);
						for (int j = 0; j < iter_keys.size(); j++)
							inner_scope->variables[iter_keys[j]] = iter_values[j]->getArray()[i];
						SP_MEMORY v = inner_scope->execute();
						if (v->getType() == RETURN_M) return v;
						list.push_back(v);
						if (v->getType() == BREAK_M) break;
					}				
				}
			}else if (var->getType() == OBJ && var->getScope()->variables.find("Iterator") != var->getScope()->variables.end()){
				//iter_arr = var->getScope()->variables["iterator"]->getLambda()->execute({});
				std::string iter_key = params[0]->params[0]->key;
				auto iter_obj = var->getScope()->variables["Iterator"]->getScope()->clone(inner_scope);
				iter_obj->key = "Iterator";
				iter_obj->variables["self"] = NEW_MEMORY(iter_obj);
				iter_obj->variables["init"]->getLambda()->execute({var});
				auto hasNext = iter_obj->variables["hasi"]->getLambda();
				auto next = iter_obj->variables["next"]->getLambda();
				auto geti = iter_obj->variables["geti"]->getLambda();

				while (hasNext->execute({})->getValue() != 0) {
					inner_scope->variables[iter_key] = geti->execute({});
					inner_scope->main = params[1]->clone(inner_scope);
					SP_MEMORY v = inner_scope->execute();
					next->execute({});
					if (v->getType() == RETURN_M) return v;
					list.push_back(v);
					if (v->getType() == BREAK_M) break;
				}
			}
			else
				Interpreter::throwError("Error: Cannot iterate over non-iterable value!", toString());
		} else {
			SP_MEMORY var = params[0]->execute(scope);
			SP_MEMORY iter_arr;
			SP_SCOPE inner_scope = NEW_SCOPE(scope, "$LOOP$");
			if (var->getType() == ARR || var->getType() == STR){ 
				iter_arr = var;
				for (auto &m : iter_arr->getArray()) {
					inner_scope->main = params[1]->clone(inner_scope);
					SP_MEMORY v = inner_scope->execute();
					if (v->getType() == RETURN_M) return v;
					list.push_back(v);
					if (v->getType() == BREAK_M) break;
				}
			}
			else if (var->getType() == OBJ && var->getScope()->variables.find("Iterator") != var->getScope()->variables.end()) {
				//iter_arr = var->getScope()->variables["iterator"]->getLambda()->execute({});
				auto iter_obj = var->getScope()->variables["Iterator"]->getScope()->clone(scope);
				iter_obj->key = "Iterator";
				iter_obj->variables["self"] = NEW_MEMORY(iter_obj);
				iter_obj->variables["init"]->getLambda()->execute({var});
				auto hasNext = iter_obj->variables["hasi"]->getLambda();
				auto next = iter_obj->variables["next"]->getLambda();
				auto geti = iter_obj->variables["geti"]->getLambda();

				while (hasNext->execute({})->getValue() != 0) {
					inner_scope->main = params[1]->clone(inner_scope);
					SP_MEMORY v = inner_scope->execute();
					next->execute({});
					if (v->getType() == RETURN_M) return v;
					list.push_back(v);
					if (v->getType() == BREAK_M) break;
				}
			}
			else
				Interpreter::throwError("Error: Cannot iterate over non-iterable value!", toString());
		}
		return NEW_MEMORY(list);
	}
	//	A do B
	case DOL: {
		auto dec = params[0];
		SP_SCOPE inner_scope = NEW_SCOPE(scope, "$DO$");
		while(dec->execute(scope)->getValue() != 0) {
			inner_scope->main = params[1]->clone(inner_scope);
			auto v = inner_scope->execute();
			if (v->getType() == BREAK_M)
				break;
		}
		return NEW_MEMORY();
	}
	//	A then B else C
	case THEN: {
		auto dec = params[0]->execute(scope);
		if (dec->getValue() != 0)
			return params[1]->execute(scope);
		else if (params.size() > 2)
			return params[2]->execute(scope);
		return NEW_MEMORY();
	}
	//	A : B
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
					range.push_back(NEW_MEMORY(NUM, i));
			}else{
				for (long double i = p1; i > p2; i += step)
					range.push_back(NEW_MEMORY(NUM, i));
			}
		}else if (flag == 1){
			if (step >= 0){
				for (long double i = p1; i <= p2; i += step)
					range.push_back(NEW_MEMORY(NUM, i));
			}else{
				for (long double i = p1; i >= p2; i += step)
					range.push_back(NEW_MEMORY(NUM, i));
			}
		}
		return NEW_MEMORY(range);
	}
	//	[]
	case LIST:
		return NEW_MEMORY(executed);
	//	len A
	case SIZE_O: {
		if (executed[0]->getType() == ARR || executed[0]->getType() == STR)
			return NEW_MEMORY(NUM, executed[0]->getArray().size());
		else if (executed[0]->getType() == OBJ) {
			if (executed[0]->getScope()->variables.find("size") != executed[0]->getScope()->variables.end()) {
				auto l = executed[0]->getScope()->variables["size"]->getLambda();
				if (l != nullptr)
					return l->execute({});
			}
		}else
			Interpreter::throwError("Error: cannot get size of non-array value, or object value where size() is undefined!", toString());
	}
	//	thread(A)
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
		return NEW_MEMORY();
	}
	//	detach A
	case DETACH:{
		#ifdef THREADING
		std::thread th(threadWrapper, params[0], scope);
		th.detach();
		#else
		params[0]->execute(scope);
		#endif
		return NEW_MEMORY();
	}
	//	try A catch B
	case TRY_CATCH: {
		try {
			return params[0]->execute(scope);
		} catch (std::runtime_error &e){
			if (params.size() > 1){
				SP_SCOPE ns = NEW_SCOPE(scope, "$TRY_CATCH$");
				ns->declareVariable("_err")->set(NEW_MEMORY(std::string(e.what())));
				return params[1]->execute(ns);
			}else
				return NEW_MEMORY();
		}
	}
	//	()
	case SOFT_LIST:
		if (executed.size() == 1)
			return executed[0];
		else
			return NEW_MEMORY(executed);
	//	A[B]
	case INDEX: {
		auto p1 = params[0]->execute(scope);
		SP_SCOPE s = NEW_SCOPE(scope, "$INDEX$");
		s->declareVariable("end")->set(NEW_MEMORY(NUM, p1->getArray().size() - 1));
		auto p2 = params[1]->execute(s);
		VEC_Memory new_arr;
		for (auto &i : p2->getArray()){
			auto temp = p1->index(i);
			if (temp == nullptr)
				Interpreter::throwError("Error: index out of bounds!", toString());
			new_arr.push_back(temp);
		}
		if (new_arr.size() > 1 || new_arr.size() == 0)
			return NEW_MEMORY(new_arr);
		else
			return new_arr[0];
	}
	//	A pull B
	case PULL_ARR: {
		auto p1 = params[0]->execute(scope);
		SP_SCOPE s = NEW_SCOPE(scope, "$INDEX$");
		s->declareVariable("end")->set(NEW_MEMORY(NUM, p1->getArray().size() - 1));
		auto p2 = params[1]->execute(s);
		return p1->steal(p2);
	}
	//	A from { ... }
	case FROM: {
		auto scope_inner = params[1]->scope_ref;
		scope_inner->variables[params[0]->key] = NEW_MEMORY();
		params[1]->execute(scope);
		temp1 = scope_inner->getVariable(params[0]->key);
		return temp1;
	}
	default: return NULL;
	}
}

void Node::threadWrapper(SP_NODE n, SP_SCOPE s){
	n->execute(s);
}

SP_NODE Node::clone(const SP_SCOPE &scope) const {
	VEC_Node new_params;
	for (auto &n : params)
		new_params.push_back(n->clone(scope));
	SP_NODE nn = NEW_NODE(this->nt, new_params);
	nn->key = this->key;
	nn->flag = this->flag;
	if (this->nt == SWITCH){
		std::unordered_map<long double, SP_NODE> nsw;
		for (auto &c : switch_values) {
			nsw[c.first] = c.second->clone(scope);
		}
		nn->switch_values = nsw;
	}
	if (this->mem_data != nullptr)
		nn->mem_data = this->mem_data->clone(scope);
	if (this->scope_ref != nullptr){
		nn->scope_ref = this->scope_ref->clone(scope);
		//nn->scope_ref->key = nn->scope_ref->getPath();
	}
	return nn;
}

const std::string Node::toString() const {
	switch(nt){
		case VAR:		return key;
		case MEM: 		return mem_data->toString();
		case DECLARE:	return "(var " + params[0]->toString() + ")";
		case ADD: 		return "(" + params[0]->toString() + " + " + params[1]->toString() + ")";
		case SUB: 		return "(" + params[0]->toString() + " - " + params[1]->toString() + ")";
		case MUL: 		return "(" + params[0]->toString() + " * " + params[1]->toString() + ")";
		case DIV: 		return "(" + params[0]->toString() + " / " + params[1]->toString() + ")";
		case POW: 		return "(" + params[0]->toString() + " ** " + params[1]->toString() + ")";
		case MOD: 		return "(" + params[0]->toString() + " % " + params[1]->toString() + ")";
		case SET: 		return "(" + params[0]->toString() + " = " + params[1]->toString() + ")";
		case REF_SET:	return "(" + params[0]->toString() + " &= " + params[1]->toString() + ")";
		case EQUAL:		return "(" + params[0]->toString() + " == " + params[1]->toString() + ")";
		case LESS:		return "(" + params[0]->toString() + " < " + params[1]->toString() + ")";
		case ELESS:		return "(" + params[0]->toString() + " <= " + params[1]->toString() + ")";
		case MORE:		return "(" + params[0]->toString() + " > " + params[1]->toString() + ")";
		case EMORE:		return "(" + params[0]->toString() + " >= " + params[1]->toString() + ")";
		case NEQUAL:	return "(" + params[0]->toString() + " != " + params[1]->toString() + ")";
		case OBJ_SET:	return "(" + params[0]->toString() + " :: " + params[1]->toString() + ")";
		case AND:		return "(" + params[0]->toString() + " && " + params[1]->toString() + ")";
		case OR:		return "(" + params[0]->toString() + " || " + params[1]->toString() + ")";
		case EXEC:		{
			auto args = params[1]->toString().substr(1);
			args = args.substr(0, args.size() - 1);
			return params[0]->toString() + "(" + args + ")";
		}
		case SWITCH:	return "(switch " + params[0]->toString() + ")";
		case CASE:		return "(" + params[0]->toString() + " >> " + params[1]->toString() + ")";
		case DES:		return "(" + params[0]->toString() + " -> " + params[1]->toString() + ")";
		case LDES:		return "(" + params[0]->toString() + " => " + params[1]->toString() + ")";
		case ADD_ARR:	return "(" + params[0]->toString() + " ++ " + params[1]->toString() + ")";
		case STR_CAT:	return "(" + params[0]->toString() + " .. " + params[1]->toString() + ")";
		case INDEX_OBJ:	return "(" + params[0]->toString() + " . " + params[1]->toString() + ")";
		case INDEX:		return "(" + params[0]->toString() + params[1]->toString() + ")";
		case DOL:		return "(" + params[0]->toString() + " do " + params[1]->toString() + ")";
		case EXEC_ITER:	return "(" + params[0]->toString() + " ->> " + params[1]->toString() + ")";
		case FROM:		return "(" + params[0]->toString() + " from " + params[1]->toString() + ")";
		case ITER:		return "(" + params[0]->toString() + " in " + params[1]->toString() + ")";
		case RANGE:		return "(" + params[0]->toString() + " : " + params[1]->toString() + (params.size() > 2 ? " : " + params[2]->toString() : "") + ")";
		case THEN:		return "(" + params[0]->toString() + " then " + params[1]->toString() + (params.size() > 2 ? " else " + params[2]->toString() : "") + ")";
		case SCOPE:		return scope_ref->toString();
		case PUSH_ARR:	return "(" + params[0]->toString() + " push " + params[1]->toString() + ")";
		case UNSHIFT_ARR:	return "(" + params[0]->toString() + " post " + params[1]->toString() + ")";
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
			std::string s = "[ ";
			for (auto &p : params)
				s += p->toString() + " ";
			return s + "]";
		}
		case SOFT_LIST:{
			std::string s = "( ";
			for (auto &p : params)
				s += p->toString() + " ";
			return s + ")";
		}
		default:
			return "NULL";
	}
}

void Node::weakListCheck(const SP_SCOPE &s) {
	SP_SCOPE scope;
	if (nt == SCOPE)
		scope = scope_ref;
	else
		scope = s;
	VEC_Node new_params;
	for (auto &p : params){\
		if (p == nullptr)
			Interpreter::throwError("Error: unbalanced operator!", "NA");
		SP_NODE nn = p;
		while (nn->nt == SOFT_LIST && nn->params.size() == 1)
			nn = nn->params[0]->clone(scope);
		nn->weakListCheck(scope);
		new_params.push_back(nn);
	}
	this->params = new_params;
}