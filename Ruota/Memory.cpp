#include "Ruota.h"

Memory::Memory() {
	reference_add++;
	this->data = 0;
	this->mt = NUL;
}

Memory::Memory(MemType mt, const long double &data) {
	reference_add++;
	this->mt = mt;
	switch(mt){
		case NUM:
		this->data = data;
		break;
		case CHA:
		this->char_data = data;
		break;
	}
}

Memory::Memory(void * ptr_data) {
	this->ptr_data = ptr_data;
	this->mt = PTR;
}

void Memory::clear() {
	if (obj_data != nullptr)
		obj_data->parent = nullptr;
	obj_data = nullptr;
	reference = nullptr;
	if (lambda != nullptr)
		lambda->parent = nullptr;
	lambda = nullptr;
	data = 0;
	char_data = 0;
	arr_data.clear();
}

Memory::Memory(SP_Scope scope) {
	reference_add++;
	obj_data = scope;
	mt = OBJ;
}

Memory::Memory(SP_Lambda lambda) {
	reference_add++;
	this->lambda = lambda;
	this->mt = LAM;
}

Memory::Memory(VEC_Memory arr_data) {
	reference_add++;
	this->arr_data = arr_data;
	this->mt = ARR;
}

bool Memory::isStatic() {
	if (mt == REF) 	return reference->isStatic();
	return this->static_object;
}

bool Memory::isStruct() {
	if (mt == REF) 	return reference->isStruct();
	return this->struct_object;
}

bool Memory::isLocal() {
	if (mt == REF) 	return reference->isLocal();
	return this->local_var;
}

SP_Scope Memory::getScope() {
	if (mt == REF)	return reference->getScope();
	return this->obj_data;
}

SP_Memory Memory::setType(const MemType &mt) {
	if (mt == REF)	reference->setType(mt);
	else			this->mt = mt;
	return to_this_ptr;
}

SP_Memory Memory::setStatic(const bool &s) {
	if (mt == REF)	reference->setStatic(s);
	else			this->static_object = s;
	return to_this_ptr;
}

SP_Memory Memory::setStruct(const bool &s) {
	if (mt == REF) 	reference->setStruct(s);
	else			this->struct_object = s;
	return to_this_ptr;
}

SP_Memory Memory::makeScope(const SP_Scope &parent) {
	if (mt == REF) {
		reference->makeScope(parent);
		return to_this_ptr;
	}
	this->clear();
	this->obj_data = new_scope(parent);
	this->mt = OBJ;
	return to_this_ptr;
}

Memory::Memory(const String &s) {
	reference_add++;
	for (auto &c : s){
		this->arr_data.push_back(new_memory(CHA, c));
		this->arr_data.back()->setStatic(true);
	}
	this->mt = STR;
}

SP_Memory Memory::pop() {
	this->arr_data.pop_back();
	return to_this_ptr;
}

SP_Memory Memory::shift() {
	std::reverse(arr_data.begin(), arr_data.end());
	this->arr_data.pop_back();
	std::reverse(arr_data.begin(), arr_data.end());
	return to_this_ptr;
}

SP_Memory Memory::push(SP_Memory &m) {
	this->arr_data.push_back(m);
	return to_this_ptr;
}

SP_Memory Memory::unshift(SP_Memory &m) {
	std::reverse(arr_data.begin(), arr_data.end());
	this->arr_data.push_back(m);
	std::reverse(arr_data.begin(), arr_data.end());
	return to_this_ptr;
}

long double Memory::getValue() {
	if (mt == REF)	return reference->getValue();
	switch(mt){
		case NUL:
		return 0;
		case CHA:
		return this->char_data;
		case NUM:
		return this->data;
		case STR:
		return std::stold(this->toString());
		case OBJ: {
			if (obj_data->variables.find("value") != obj_data->variables.end()) {
				auto l = obj_data->variables["value"]->getLambda();
				if (l != nullptr) {
					return l->execute({})->getValue();
				}
			}
		}
		default:
		Interpreter::throwError("Error: value is not scalar!", toString());
	}
}

SP_Lambda Memory::getLambda() {
	if (mt == REF)	return reference->getLambda();
	return this->lambda;
}
VEC_Memory Memory::getArray() {
	if (mt == REF)	return reference->getArray();
	return this->arr_data;
}

SP_Memory Memory::setArray(VEC_Memory arr_data) {
	this->clear();
	this->mt = ARR;
	this->arr_data = arr_data;
	return to_this_ptr;
}

SP_Memory Memory::setLocal(const bool &b) {
	this->local_var = b;
	return to_this_ptr;
}

SP_Memory Memory::refer(const SP_Memory &m) {
	if (m->mt == NUL && m->data == 1) {
		this->mt = NUL;
	}else{
		auto rp = m;
		while (rp->mt == REF) rp = rp->reference;
		if (rp == to_this_ptr)
			throw std::runtime_error("Error: cannot set a variable's reference to itself!");
		this->clear();
		this->reference = rp;
		this->mt = REF;
	}
	return to_this_ptr;
}

SP_Memory Memory::index(const SP_Memory &m) {
	if (mt == REF)	return reference->index(m);

	if (mt == ARR || mt == STR) {
		size_t pos = m->getValue();
		if (this->arr_data.size() <= pos || pos < 0)
			return nullptr;
		return this->arr_data[pos];
	} else if (mt == OBJ) {
		if (obj_data->variables.find("index") != obj_data->variables.end()) {
			auto l = obj_data->variables["index"]->getLambda();
			if (l != nullptr)
				return l->execute({m});
		}
	} else {
		Interpreter::throwError("Error: value type is not indexable!", toString());
	}
}

SP_Memory Memory::clone(const SP_Scope &parent) {
	SP_Memory m;
	switch (mt)
	{
	case PTR:	m = new_memory((void*)ptr_data); break;
	case REF:	m = reference->clone(parent); break;
	case CHA:	m = new_memory(CHA, char_data); break;
	case NUM:	m = new_memory(NUM, data); break;
	case LAM:	m = new_memory(lambda->clone(parent)); break;
	case NUL:	m = new_memory(); break;
	case STR:	m = new_memory(this->toString()); break;
	case ARR: {
		VEC_Memory new_arr;
		for (auto &v : arr_data)
			new_arr.push_back(v->clone(parent));
		m = new_memory(new_arr);
		break;
	}
	case OBJ: {
		auto temp = new_memory();
		temp->mt = OBJ;
		temp->obj_data = obj_data->clone(parent);
		temp->obj_data->variables["self"] = to_this_ptr;
		m = temp;
		break;
	}
	default: m = nullptr;
	}
	m->local_var = this->local_var;
	return m;
}

bool Memory::equals(const SP_Memory &a) {
	if (mt == REF)		return reference->equals(a);
	if (a->mt == REF)	return a->reference->equals(to_this_ptr);
	if (mt != a->mt)	return false;

	switch (a->mt)
	{
	case PTR:	return (ptr_data == a->ptr_data);
	case CHA:
	case NUM:	if (getValue() != a->getValue())	return false;
	case STR:
	case ARR: {
		if (arr_data.size() != a->arr_data.size())	return false;
		for (int i = 0; i < arr_data.size(); i++)
			if (!arr_data[i]->equals(a->arr_data[i]))
				return false;
		return true;
	}
	case LAM:	if (lambda != a->lambda)	return false;
	case NUL:	return true;
	case OBJ:	{
		if (obj_data->variables.find("equals") != obj_data->variables.end()) {
			auto l = obj_data->variables["equals"]->getLambda();
			if (l != nullptr)
				return l->execute({a})->getValue();
		} else {
			return obj_data == a->obj_data;
		}
	}
	default:	return false;
	}
}

SP_Memory Memory::setValue(const long double &data){
	if (mt == REF)	this->reference->setValue(data);
	else			this->data = data;
	return to_this_ptr;
}

SP_Memory Memory::set(const SP_Memory &m) {
	if (to_this_ptr == m)
		return to_this_ptr;
	if (mt == REF) {
		reference->set(m);
		return to_this_ptr;
	} else if (m->mt == REF){
		return this->set(m->reference);
	}

	if (isStatic() && mt == CHA){
		try {
		this->char_data = (char)m->getValue();
		} catch (...){
			throw std::runtime_error("Error: Cannot set a string-dependent char to a non-char value!");
		}
		return to_this_ptr;
	}

	this->mt = m->mt;
	if (this->arr_data.size() != m->arr_data.size())
		this->arr_data.clear();
	this->lambda = nullptr;
	this->reference = nullptr;
	this->data = 0;
	this->char_data = 0;
	switch (m->mt)
	{
	case PTR:
		this->ptr_data = m->ptr_data;
		break;
	case CHA:
		this->char_data = m->getValue();
		break;
	case NUM:
		this->data = m->getValue();
		break;
	case STR:
		if (this->arr_data.empty()) {
			for (auto &v : m->arr_data) {
				SP_Memory temp = new_memory();
				temp->set(v);
				if (temp->getType() != CHA)
					this->mt = ARR;
				else
					temp->setStatic(true);
				this->arr_data.push_back(temp);
			}
		}else {
			for (int i = 0; i < arr_data.size(); i++){
				this->arr_data[i]->set(m->arr_data[i]);
			}
		}
		break;
	case ARR:
		if (this->arr_data.empty()) {
			for (auto &v : m->arr_data) {
				SP_Memory temp = new_memory();
				temp->set(v);
				this->arr_data.push_back(temp);
			}
		}else {
			for (int i = 0; i < arr_data.size(); i++){
				this->arr_data[i]->set(m->arr_data[i]);
			}
		}
		break;
	case OBJ:
		this->obj_data = m->obj_data;
		break;
	case LAM:
		this->lambda = m->lambda;
		break;
	default:
		break;
	}
	return to_this_ptr;
}

SP_Memory Memory::setScope(const SP_Scope &scope) {
	if (mt == REF) {
		reference->setScope(scope);
		return to_this_ptr;
	}
	obj_data = scope;
	mt = OBJ;
	return to_this_ptr;
}

SP_Memory Memory::add(const SP_Memory &a) {
	if (mt == REF)
		return reference->add(a);

	switch (mt)
	{
	default:
		switch (a->mt)
		{ 
		default:
			return new_memory(this->mt == CHA ? CHA : NUM, this->getValue() + a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->add(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->add(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::sub(const SP_Memory &a) {
	if (mt == REF)
		return reference->sub(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->mt == CHA ? CHA : NUM, this->getValue() - a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->sub(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->sub(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::mul(const SP_Memory &a) {
	if (mt == REF)
		return reference->mul(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->mt == CHA ? CHA : NUM, this->getValue() * a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mul(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mul(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::div(const SP_Memory &a) {
	if (mt == REF)
		return reference->div(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->mt == CHA ? CHA : NUM, this->getValue() / a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->div(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->div(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::mod(const SP_Memory &a) {
	if (mt == REF)
		return reference->mod(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->mt == CHA ? CHA : NUM, (int)this->getValue() % (int)a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mod(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mod(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::pow(const SP_Memory &a) {
	if (mt == REF)
		return reference->pow(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->mt == CHA ? CHA : NUM, std::pow(this->getValue(), a->getValue()));
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->pow(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->pow(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::less(const SP_Memory &a) {
	if (mt == REF)
		return reference->less(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(NUM, this->getValue() < a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->less(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->less(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::eless(const SP_Memory &a) {
	if (mt == REF)
		return reference->eless(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(NUM, this->getValue() <= a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->eless(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->eless(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::more(const SP_Memory &a) {
	if (mt == REF)
		return reference->more(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(NUM, this->getValue() > a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->more(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->more(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::emore(const SP_Memory &a) {
	if (mt == REF)
		return reference->emore(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(NUM, this->getValue() >= a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->emore(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->emore(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

void * Memory::getPointer() {
	return this->ptr_data;
}

String Memory::toString() {
	switch (this->mt)
	{
	case PTR:
		return "PTR";
	case REF:
		return reference->toString();
	case CHA:
		return std::string(1, char_data);
	case NUM: {
		String s = std::to_string(data);
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.pop_back();
		return s;
	}
	case STR: {
		String s = "";
		for (auto &m : arr_data) {
			s.push_back(m->getValue());
		}
		return s;
	}
	case ARR:{
		String s = "[ ";
		for (auto &m : arr_data) {
			if (m->getType() == CHA)
				s += "'" + m->toString() + "' ";
			else if (m->getType() == STR)
				s += "\"" + m->toString() + "\" ";
			else
				s += m->toString() + " ";
		}
		return s + "]";
	}
	case LAM: {
		String s = "(";
		for (int i = 0; i < lambda->param_keys.size(); i++){
			s += " ";
			if (lambda->param_types[i] == 1) 
				s += "&";
			s += lambda->param_keys[i];
		}
		s += " ) -> " + lambda->base->toString();
		return s;
	}
	case OBJ: {
		if (!struct_object && obj_data->variables.find("string") != obj_data->variables.end()) {
			auto l = obj_data->variables["string"]->getLambda();
			if (l != nullptr) {
				return l->execute({})->toString();
			}
		}
		return "OBJ";
	}
	case NUL:
		return "null";
	default:
		return "error-type";
	}
}

MemType Memory::getType() {
	if (mt == REF)
		return reference->getType();
	return this->mt;
}

long Memory::reference_add = 0;
long Memory::reference_del = 0;

Memory::~Memory() {
	reference_del++;
	this->arr_data.clear();
	this->reference = nullptr;
	if (this->lambda != nullptr)
		this->lambda->parent = nullptr;
	this->lambda = nullptr;
	this->obj_data = nullptr;
}