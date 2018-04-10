#ifndef INTERPRETER_H
#define INTERPRETER_H

#include<string>
#include<map>
#include<vector>
#include<fstream>
#include<iostream>
#include<memory>
#include "Tokenizer.h"
#include<time.h>
#include <windows.h>
#include<chrono>
#include<cmath>

#undef	max
#undef	min

class Interpreter;
struct Memory;
struct Scope;
struct Node;
struct Lambda;

enum NodeType {
	M_B,
	M_S,
	M_P,

	MEM,
	VAR,
	LIST,
	SOFT_LIST,
	SCOPE,
	SET,
	SET_STAT,
	OBJ_SET,
	REF_SET,
	ADD,
	ADD_ARR,
	STR_CAT,
	SUB,
	MUL,
	DIV,
	MOD,
	POW,

	EQUAL,
	NEQUAL,
	LESS,
	MORE,
	ELESS,
	EMORE,
	AND,
	OR,

	INDEX,
	INDEX_OBJ,
	SIZE_O,
	EXEC,
	EXEC_ITER,
	FROM,
	DES,
	LDES,

	DOL,
	THEN,
	ITER,
	RANGE,

	TYPE,
	OUT_CALL,
	NEW,
	LOCAL,
	STRUCT,

	BREAK,
	RETURN
};

enum MemType {
	BREAK_M,
	RETURN_M,

	NUL,
	NUM,
	ARR,
	LAM,
	OBJ,
	REF
};

#define	SP_Memory	std::shared_ptr<Memory>
#define SP_Node		std::shared_ptr<Node>
#define SP_Scope	std::shared_ptr<Scope>
#define SP_Lambda	std::shared_ptr<Lambda>

#define VEC_Memory	std::vector<SP_Memory>
#define VEC_Node	std::vector<SP_Node>


struct Memory : std::enable_shared_from_this<Memory> {
private:
	static long	reference_count;
	long double		data = 0;
	MemType		mt;
	VEC_Memory	arr_data;
	SP_Scope	obj_data = nullptr;
	SP_Lambda	lambda = nullptr;
	SP_Memory	reference = nullptr;
	bool		static_object = false;
	bool		struct_object = false;
public:
	Memory();
	~Memory();
	Memory(std::string);
	Memory(long double);
	Memory(SP_Lambda);
	Memory(VEC_Memory);
	SP_Memory add(const SP_Memory);
	SP_Memory sub(const SP_Memory);
	SP_Memory mul(const SP_Memory);
	SP_Memory div(const SP_Memory);
	SP_Memory mod(const SP_Memory);
	SP_Memory pow(const SP_Memory);
	bool equals(const SP_Memory);
	bool isStatic();
	bool isStruct();
	void set(const SP_Memory);
	long double getValue();
	SP_Memory index(size_t pos);
	SP_Memory index(std::string);
	void setStatic(bool);
	void setStruct(bool);
	std::string toString();
	SP_Memory clone(SP_Scope);
	SP_Lambda getLambda();
	SP_Scope getScope();
	void setScope(SP_Scope);
	void makeScope(SP_Scope);
	void setType(MemType);
	void refer(SP_Memory);
	VEC_Memory getArray();
	MemType getType();
};

struct Node : std::enable_shared_from_this<Node> {
	VEC_Node params;
	NodeType nt;
	SP_Scope scope_ref = nullptr;
	SP_Memory mem_data = nullptr;
	std::string key;
	SP_Memory execute(SP_Scope);
	SP_Node clone(SP_Scope);

	Node(SP_Scope);
	Node(long double);
	Node(std::string);
	Node(NodeType, VEC_Node);

	std::string toString();
};

struct Lambda : std::enable_shared_from_this<Lambda> {
	SP_Node base = nullptr;
	SP_Scope parent = nullptr;
	std::vector<std::string> param_keys;

	Lambda(SP_Scope, SP_Node, std::vector<std::string>);
	SP_Memory execute(VEC_Memory);
	SP_Lambda clone(SP_Scope);
};

struct Scope : std::enable_shared_from_this<Scope> {
	SP_Scope parent = nullptr;
	SP_Node main = nullptr;
	std::map<std::string, SP_Memory> variables;
	Scope(SP_Scope);
	Scope(SP_Scope, SP_Node);
	SP_Memory execute();
	SP_Memory getVariable(std::string);

	SP_Scope clone(SP_Scope);

	std::string toString();
};

class Interpreter {	
	friend Node;
	friend Lambda;
	friend Scope;
	friend Memory;
private:
	static VEC_Memory(*__send)(VEC_Memory);
	static std::map<std::string, int> operators;
	std::vector<std::string> LOADED;
public:
	Interpreter(VEC_Memory (*__send)(VEC_Memory));
	SP_Scope generate(std::string, SP_Scope, std::string);
	SP_Memory execute(SP_Scope);
};

#endif // !INTERPRETER_H
