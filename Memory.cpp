#include "Interpreter.h"

Memory::Memory() {
#ifdef DEBUG
	reference_count++;
#endif // DEBUG

	this->data = 0;
	this->mt = NUL;
}

Memory::Memory(double data) {
#ifdef DEBUG
	reference_count++;
#endif // DEBUG
	this->data = data;
	this->mt = NUM;
}

Memory::Memory(SP_Lambda lambda) {
#ifdef DEBUG
	reference_count++;
#endif // DEBUG
	this->lambda = lambda;
	this->mt = LAM;
}

Memory::Memory(VEC_Memory arr_data) {
#ifdef DEBUG
	reference_count++;
#endif // DEBUG
	this->arr_data = arr_data;
	this->mt = ARR;
}

bool Memory::isStatic() {
	if (mt == REF)
		return reference->isStatic();
	return this->static_object;
}

bool Memory::isStruct() {
	if (mt == REF)
		return reference->isStruct();
	return this->struct_object;
}

SP_Scope Memory::getScope() {
	if (mt == REF)
		return reference->getScope();
	return this->obj_data;
}

void Memory::setType(MemType mt) {
	if (mt == REF) {
		reference->setType(mt);
		return;
	}
	this->mt = mt;
}

void Memory::setStatic(bool s) {
	if (mt == REF) {
		reference->setStatic(s);
		return;
	}
	this->static_object = s;
}

void Memory::setStruct(bool s) {
	if (mt == REF) {
		reference->setStruct(s);
		return;
	}
	this->struct_object = s;
}

void Memory::makeScope(SP_Scope parent) {
	if (mt == REF) {
		reference->makeScope(parent);
		return;
	}
	this->obj_data = std::make_shared<Scope>(parent);
	this->mt = OBJ;
}

Memory::Memory(std::string s) {
#ifdef DEBUG
	reference_count++;
#endif // DEBUG
	for (auto &c : s) {
		this->arr_data.push_back(std::make_shared<Memory>(c));
	}
	this->mt = ARR;
}

double Memory::getValue() {
	if (mt == REF)
		return reference->getValue();
	return this->data;
}

SP_Lambda Memory::getLambda() {
	if (mt == REF)
		return reference->getLambda();
	if (this->lambda == nullptr) {
		throw std::runtime_error("Error: Lambda does not exist!");
	}
	return this->lambda;
}
VEC_Memory Memory::getArray() {
	if (mt == REF)
		return reference->getArray();
	return this->arr_data;
}

void Memory::refer(SP_Memory m) {
	if (m->mt == NUL) {
		this->mt = NUL;
		return;
	}
	this->reference = m;
	this->mt = REF;
}

SP_Memory Memory::index(size_t pos) {
	if (mt == REF)
		return reference->index(pos);

	if (mt == ARR) {
		pos--;
		if (this->arr_data.size() <= pos || pos < 0) {
			throw std::runtime_error(("Error: Index out of range! (Pos=" + std::to_string(pos + 1) + ", Size=" + std::to_string(this->arr_data.size()) + ")").c_str());
		}
		return this->arr_data[pos];
	}
	else if (mt == OBJ) {
		if (obj_data->variables.find("index") != obj_data->variables.end()) {
			auto l = obj_data->variables["index"]->getLambda();
			if (l != nullptr) {
				return l->execute({std::make_shared<Memory>(pos)});
			}
		}
	}
	return std::make_shared<Memory>();
}

SP_Memory Memory::clone(SP_Scope parent) {
	switch (mt)
	{
	case REF:
		return reference->clone(parent);
	case NUM:
		return std::make_shared<Memory>(data);
	case ARR: {
		VEC_Memory new_arr;
		for (auto &v : arr_data)
			new_arr.push_back(v->clone(parent));
		return std::make_shared<Memory>(new_arr);
	}
	case LAM:
		return std::make_shared<Memory>(lambda->clone(parent));
	case NUL:
		return std::make_shared<Memory>();
	case OBJ: {
		auto temp = std::make_shared<Memory>();
		temp->mt = OBJ;
		temp->obj_data = obj_data->clone(parent);
		temp->obj_data->variables["self"] = shared_from_this();
		return temp;
	}
	}

	return nullptr;
}

bool Memory::equals(const SP_Memory a) {
	if (mt == REF)
		return reference->equals(a);

	if (mt != a->mt)
		return false;

	switch (mt)
	{
	case REF:
		return reference->equals(a);
	case NUM:
		if (getValue() != a->getValue())
			return false;
		break;
	case ARR: {
		if (arr_data.size() != a->arr_data.size())
			return false;
		for (int i = 0; i < arr_data.size(); i++) {
			if (!arr_data[i]->equals(a->arr_data[i]))
				return false;
		}
		break;
	}
	case LAM:
		if (lambda != a->lambda)
			return false;
		break;
	case NUL:
		return true;
	default:
		return false;
	}

	return true;
}

void Memory::set(const SP_Memory m) {
	if (mt == REF) {
		reference->set(m);
		return;
	}

	this->mt = m->mt;
	this->arr_data.clear();
	this->lambda = nullptr;
	switch (m->mt)
	{
	case REF:
		this->set(m->reference);
		break;
	case NUM:
		this->data = m->data;
		break;
	case ARR:
		for (auto &v : m->arr_data) {
			SP_Memory temp = std::make_shared<Memory>();
			temp->set(v);
			this->arr_data.push_back(temp);
		}
		break;
	case OBJ:
		this->obj_data = m->obj_data;
		break;
	case LAM:
		this->lambda = m->lambda;
		break;
	}
}

void Memory::setScope(SP_Scope scope) {
	if (mt == REF) {
		reference->setScope(scope);
		return;
	}
	obj_data = scope;
	mt = OBJ;
}

SP_Memory Memory::add(const SP_Memory a) {
	if (mt == REF)
		return reference->add(a);

	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return std::make_shared<Memory>(this->getValue() + a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->add(v));
			return std::make_shared<Memory>(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->add(a));
			return std::make_shared<Memory>(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::sub(const SP_Memory a) {
	if (mt == REF)
		return reference->sub(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return std::make_shared<Memory>(this->getValue() - a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->sub(v));
			return std::make_shared<Memory>(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->sub(a));
			return std::make_shared<Memory>(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::mul(const SP_Memory a) {
	if (mt == REF)
		return reference->mul(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return std::make_shared<Memory>(this->getValue() * a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mul(v));
			return std::make_shared<Memory>(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mul(a));
			return std::make_shared<Memory>(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::div(const SP_Memory a) {
	if (mt == REF)
		return reference->div(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return std::make_shared<Memory>(this->getValue() / a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->div(v));
			return std::make_shared<Memory>(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->div(a));
			return std::make_shared<Memory>(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::mod(const SP_Memory a) {
	if (mt == REF)
		return reference->mod(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return std::make_shared<Memory>((int)this->getValue() % (int)a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mod(v));
			return std::make_shared<Memory>(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mod(a));
			return std::make_shared<Memory>(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::pow(const SP_Memory a) {
	if (mt == REF)
		return reference->pow(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return std::make_shared<Memory>(std::pow(this->getValue(), a->getValue()));
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->pow(v));
			return std::make_shared<Memory>(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->pow(a));
			return std::make_shared<Memory>(new_arr);
		}
	}

	return nullptr;
}

std::string Memory::toString() {
	switch (this->mt)
	{
	case REF:
		return reference->toString();
	case NUM: {
		std::string s = std::to_string(data);
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.pop_back();
		return s;
	}
	case ARR: {
		std::string s = "";
		for (auto &m : arr_data) {
			s.push_back(m->getValue());
		}
		return s;
		/*std::string s = "[ ";
		for (auto &m : arr_data) {
			s += m->toString() + " ";
		}
		return s + "]";*/
	}
	case LAM: {
		Lambda * l = lambda.get();
		return "LAM-" + std::to_string((int)&l);
	}
	case OBJ: {
		if (obj_data->variables.find("string") != obj_data->variables.end()) {
			auto l = obj_data->variables["string"]->getLambda();
			if (l != nullptr) {
				return l->execute({})->toString();
			}
		}
		Memory * m = shared_from_this().get();
		return "OBJ-" + std::to_string((int)&m);
	}
	default:
		return "null";
	}
}

MemType Memory::getType() {
	if (mt == REF)
		return reference->getType();
	return this->mt;
}

long Memory::reference_count = 0;

Memory::~Memory() {
#ifdef DEBUG
	reference_count--;
	std::cout << " >> Deleting Memory: " << toString() << " ("<< reference_count << ")" << std::endl;
#endif // DEBUG
}