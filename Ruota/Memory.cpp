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
	obj_data = nullptr;
	reference = nullptr;
	lambda = nullptr;
	data = 0;
	char_data = 0;
	arr_data.clear();
}

Memory::Memory(SP_SCOPE scope) {
	reference_add++;
	obj_data = scope;
	mt = OBJ;
}

Memory::Memory(SP_LAMBDA lambda) {
	reference_add++;
	this->lambda = lambda;
	this->mt = LAM;
}

Memory::Memory(VEC_Memory arr_data) {
	reference_add++;
	this->arr_data = arr_data;
	this->mt = ARR;
}

ObjectMode Memory::getObjectMode() const {
	if (mt == REF) 	return reference->getObjectMode();
	return this->om;
}

bool Memory::isLocal() const {
	if (mt == REF) 	return reference->isLocal();
	return this->local_var;
}

SP_SCOPE Memory::getScope() {
	if (mt == REF)	return reference->getScope();
	return this->obj_data;
}

SP_MEMORY Memory::setType(const MemType &mt) {
	if (mt == REF)	reference->setType(mt);
	else			this->mt = mt;
	return to_this_ptr;
}

SP_MEMORY Memory::setObjectMode(const ObjectMode &om) {
	if (mt == REF)	reference->setObjectMode(om);
	else			this->om = om;
	return to_this_ptr;
}

SP_MEMORY Memory::makeScope(const SP_SCOPE &parent) {
	if (mt == REF) {
		reference->makeScope(parent);
		return to_this_ptr;
	}
	this->clear();
	this->obj_data = NEW_SCOPE(parent, "$INNER$");
	this->mt = OBJ;
	return to_this_ptr;
}

Memory::Memory(const std::string &s) {
	reference_add++;
	for (auto &c : s){
		this->arr_data.push_back(NEW_MEMORY(CHA, c));
		this->arr_data.back()->setObjectMode(STATIC);
	}
	this->mt = STR;
}

SP_MEMORY Memory::pop() {
	this->arr_data.pop_back();
	return to_this_ptr;
}

SP_MEMORY Memory::shift() {
	std::reverse(arr_data.begin(), arr_data.end());
	this->arr_data.pop_back();
	std::reverse(arr_data.begin(), arr_data.end());
	return to_this_ptr;
}

SP_MEMORY Memory::push(SP_MEMORY &m) {
	this->arr_data.push_back(m);
	return to_this_ptr;
}

SP_MEMORY Memory::unshift(SP_MEMORY &m) {
	std::reverse(arr_data.begin(), arr_data.end());
	this->arr_data.push_back(m);
	std::reverse(arr_data.begin(), arr_data.end());
	return to_this_ptr;
}

long double Memory::getValue() const {
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

SP_LAMBDA Memory::getLambda() {
	if (mt == REF)	return reference->getLambda();
	return this->lambda;
}
VEC_Memory Memory::getArray() {
	if (mt == REF)	return reference->getArray();
	return this->arr_data;
}

SP_MEMORY Memory::setArray(VEC_Memory arr_data) {
	this->clear();
	this->mt = ARR;
	this->arr_data = arr_data;
	return to_this_ptr;
}

SP_MEMORY Memory::setLocal(const bool &b) {
	this->local_var = b;
	return to_this_ptr;
}

SP_MEMORY Memory::refer(const SP_MEMORY &m) {
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

SP_MEMORY Memory::index(const SP_MEMORY &m) {
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

SP_MEMORY Memory::clone(const SP_SCOPE &parent) const {
	SP_MEMORY m;
	switch (mt)
	{
	case PTR:	m = NEW_MEMORY((void*)ptr_data); break;
	case REF:	m = reference->clone(parent); break;
	case CHA:	m = NEW_MEMORY(CHA, char_data); break;
	case NUM:	m = NEW_MEMORY(NUM, data); break;
	case LAM:	m = NEW_MEMORY(lambda->clone(parent)); break;
	case NUL:	m = NEW_MEMORY(); break;
	case STR:	m = NEW_MEMORY(this->toString()); break;
	case ARR: {
		VEC_Memory new_arr;
		for (auto &v : arr_data)
			new_arr.push_back(v->clone(parent));
		m = NEW_MEMORY(new_arr);
		break;
	}
	case OBJ: {
		auto temp = NEW_MEMORY();
		temp->mt = OBJ;
		temp->om = this->om;
		temp->obj_data = obj_data->clone(parent);
		temp->obj_data->variables["self"] = NEW_MEMORY(obj_data);
		m = temp;
		break;
	}
	default: m = nullptr;
	}
	m->local_var = this->local_var;
	return m;
}

bool Memory::equals(const SP_MEMORY &a) const {
	if (mt == REF)		return reference->equals(a);
	if (a->mt == REF)	return equals(a->reference);
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

SP_MEMORY Memory::setValue(const long double &data){
	if (mt == REF)	this->reference->setValue(data);
	else			this->data = data;
	return to_this_ptr;
}

SP_MEMORY Memory::set(const SP_MEMORY &m) {
	if (to_this_ptr == m)
		return to_this_ptr;
	if (mt == REF) {
		reference->set(m);
		return to_this_ptr;
	} else if (m->mt == REF){
		return this->set(m->reference);
	}

	if (om == STATIC && mt == CHA){
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
				SP_MEMORY temp = NEW_MEMORY();
				temp->set(v);
				if (temp->getType() != CHA)
					this->mt = ARR;
				else
					temp->setObjectMode(STATIC);
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
				SP_MEMORY temp = NEW_MEMORY();
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
		this->om = m->om;
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

SP_MEMORY Memory::setScope(const SP_SCOPE &scope) {
	if (mt == REF) {
		reference->setScope(scope);
		return to_this_ptr;
	}
	obj_data = scope;
	mt = OBJ;
	return to_this_ptr;
}

SP_MEMORY Memory::add(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->add(a);

	switch (mt)
	{
	default:
		switch (a->mt)
		{ 
		default:
			return NEW_MEMORY(this->mt == CHA ? CHA : NUM, this->getValue() + a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->add(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->add(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::sub(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->sub(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(this->mt == CHA ? CHA : NUM, this->getValue() - a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->sub(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->sub(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::mul(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->mul(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(this->mt == CHA ? CHA : NUM, this->getValue() * a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mul(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mul(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::div(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->div(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(this->mt == CHA ? CHA : NUM, this->getValue() / a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->div(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->div(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::mod(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->mod(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(this->mt == CHA ? CHA : NUM, (int)this->getValue() % (int)a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mod(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mod(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::pow(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->pow(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(this->mt == CHA ? CHA : NUM, std::pow(this->getValue(), a->getValue()));
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->pow(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->pow(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::less(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->less(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(NUM, this->getValue() < a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->less(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->less(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::eless(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->eless(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(NUM, this->getValue() <= a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->eless(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->eless(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::more(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->more(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(NUM, this->getValue() > a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->more(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->more(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

SP_MEMORY Memory::emore(const SP_MEMORY &a) const {
	if (mt == REF)
		return reference->emore(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return NEW_MEMORY(NUM, this->getValue() >= a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->emore(v));
			return NEW_MEMORY(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->emore(a));
			return NEW_MEMORY(new_arr);
		}
	}

	return nullptr;
}

void * Memory::getPointer() {
	return this->ptr_data;
}

const std::string Memory::toString() const {
	switch (this->mt)
	{
	case PTR:
		return "PTR";
	case REF:
		return reference->toString();
	case CHA:
		return std::string(1, char_data);
	case NUM: {
		std::string s = std::to_string(data);
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.pop_back();
		return s;
	}
	case STR: {
		std::string s = "";
		for (auto &m : arr_data) {
			s.push_back(m->getValue());
		}
		return s;
	}
	case ARR:{
		std::string s = "[ ";
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
		std::string s = "(";
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
		if (om != VIRTUAL && om != DYNAMIC && obj_data->variables.find("string") != obj_data->variables.end()) {
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

MemType Memory::getType() const {
	if (mt == REF)
		return reference->getType();
	return this->mt;
}


SP_MEMORY Memory::eraseLambda() {
	this->clear();
	this->mt = NUL;
	return to_this_ptr;
}

long Memory::reference_add = 0;
long Memory::reference_del = 0;

Memory::~Memory() {
	reference_del++;
	this->arr_data.clear();
	this->reference = nullptr;
	this->lambda = nullptr;
	this->obj_data = nullptr;
}