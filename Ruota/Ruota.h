#ifndef RUOTA_H
#define RUOTA_H

#define THREADING
#define FILE_IO

#include "Tokenizer.h"
#include<string>
#include<unordered_map>
#include<vector>
#include<fstream>
#include<iostream>
#include<algorithm>
#include<memory>
#include<time.h>
#include<filesystem>
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <dlfcn.h>
#endif
#include<chrono>
#include<cmath>
#include<regex>
#ifdef THREADING
	#include<thread>
#endif

#undef	max
#undef	min

class	RuotaWrapper;
class	Interpreter;
struct	Memory;
struct	Scope;
struct	Node;
struct	Lambda;

enum NodeType {
	M_B,
	M_S,
	M_P,

	MEM,
	VAR,
	LIST,		// [ ]
	SOFT_LIST,	// ( )
	SCOPE,		// { }
	SET,		// =	:=
	SET_STAT,	// static
	OBJ_SET,	// ::
	OBJ_LAM,	// struct
	REF_SET,	// &=	:&=
	ADD,		// +
	ADD_ARR,	// ++
	STR_CAT,	// ..
	SUB,		// -
	MUL,		// *
	DIV,		// /
	MOD,		// %
	POW,		// **

	SWITCH,		// switch
	CASE,		// >>

	EQUAL,		// ==
	NEQUAL,		// !=
	LESS,		// <
	MORE,		// >
	NOT,		// !
	ELESS,		// <=
	EMORE,		// >=
	AND,		// &&
	OR,			// ||

	INDEX,		// []
	INDEX_OBJ,	// .
	SIZE_O,		// #
	EXEC,		// ()
	EXEC_ITER,	// ->>
	FROM,		// from
	DES,		// ->
	LDES,		// =>

	DOL,		// do
	THEN,		// then
	ITER,		// in
	RANGE,		// :

	TYPE,		// type
	OUT_CALL,	// _OUTER_CALL_
	NEW,		// new
	LOCAL,		// local
	STRUCT,		// dynamic
	SET_VIR,	// virtual
	THREAD,		// thread()
	DETACH,		// detach
	VALUE,		// val
	TOSTRING,	// str
	ALLOC,		// alloc
	TOCHAR,		// chr
	POP_ARR,	// pop
	SHIFT_ARR,	// shift
	UNSHIFT_ARR,// post
	PUSH_ARR,	// push
	LAST_ARR,	// last
	TOARR,		// arr
	TRY_CATCH,	// try .. catch
	DECLARE,	// var
	INHERIT,	// +>

	BREAK,		// break
	RETURN		// return
};

enum ObjectMode {
	UNDEF,
	DYNAMIC,
	STATIC,
	VIRTUAL
};

enum MemType {
	BREAK_M,
	RETURN_M,

	NUL,
	NUM,
	ARR,
	LAM,
	OBJ,
	REF,
	CHA,
	STR,
	UND,
	PTR
};


typedef	std::shared_ptr<Memory>	SP_Memory;
typedef	std::shared_ptr<Node>	SP_Node;
typedef	std::shared_ptr<Scope>	SP_Scope;
typedef	std::shared_ptr<Lambda>	SP_Lambda;

typedef	std::vector<SP_Memory>	VEC_Memory;
typedef	std::vector<SP_Node>	VEC_Node;

#define	new_memory	std::make_shared<Memory>
#define	new_node	std::make_shared<Node>
#define	new_lambda	std::make_shared<Lambda>
#define	new_scope	std::make_shared<Scope>

#define to_this_ptr	shared_from_this()

struct Memory : std::enable_shared_from_this<Memory> {
private:
	long double	data = 0;
	char		char_data = 0;
	MemType		mt;
	VEC_Memory	arr_data;
	SP_Scope	obj_data;
	SP_Lambda	lambda;
	SP_Memory	reference;
	ObjectMode	om = UNDEF;
	bool		local_var = false;
	void *		ptr_data = NULL;
public:	
	static long	reference_add;
	static long	reference_del;
	Memory();
	~Memory();
	Memory(const std::string&);
	Memory(MemType, const long double&);
	Memory(SP_Scope);
	Memory(SP_Lambda);
	Memory(VEC_Memory);
	Memory(void*);

	SP_Memory 	add(const SP_Memory&);
	SP_Memory 	sub(const SP_Memory&);
	SP_Memory 	mul(const SP_Memory&);
	SP_Memory 	div(const SP_Memory&);
	SP_Memory 	mod(const SP_Memory&);
	SP_Memory 	pow(const SP_Memory&);
	SP_Memory 	less(const SP_Memory&);
	SP_Memory 	more(const SP_Memory&);
	SP_Memory 	eless(const SP_Memory&);
	SP_Memory 	emore(const SP_Memory&);
	bool 		equals(const SP_Memory&);
	ObjectMode	getObjectMode();
	bool 		isLocal();
	SP_Memory	set(const SP_Memory&);
	long double	getValue();
	SP_Memory	index(const SP_Memory&);
	SP_Memory	index(const std::string&);
	SP_Memory	setObjectMode(const ObjectMode&);
	SP_Memory	setLocal(const bool&);
	SP_Memory	setArray(VEC_Memory);
	SP_Memory	setValue(const long double&);
	std::string		toString();
	SP_Memory	clone(const SP_Scope&);
	SP_Lambda	getLambda();
	SP_Scope	getScope();
	SP_Memory	setScope(const SP_Scope&);
	SP_Memory	makeScope(const SP_Scope&);
	SP_Memory	setType(const MemType&);
	SP_Memory	refer(const SP_Memory&);
	SP_Memory	pop();
	SP_Memory	shift();
	SP_Memory	push(SP_Memory&);
	SP_Memory	unshift(SP_Memory&);
	SP_Memory	eraseLambda();
	VEC_Memory	getArray();
	MemType		getType();
	void		clear();
	void *		getPointer();
};

struct Node : std::enable_shared_from_this<Node> {
	static long	reference_add;
	static long	reference_del;
	VEC_Node 	params;
	NodeType 	nt;
	SP_Scope 	scope_ref = nullptr;
	SP_Memory 	mem_data = nullptr;
	std::string		key;
	std::unordered_map<long double, SP_Node> switch_values;
	int			flag = 0;

	Node(SP_Scope);
	Node(long double);
	Node(SP_Node, std::unordered_map<long double, SP_Node>);
	Node(std::string);
	Node(NodeType, VEC_Node);
	Node(SP_Memory);
	~Node();

	std::string		toString();
	SP_Memory	execute(const SP_Scope&);
	SP_Node		clone(const SP_Scope&);
	void		weakListCheck();
	static void	threadWrapper(SP_Node, SP_Scope);
};

struct Lambda : std::enable_shared_from_this<Lambda> {
	static long	reference_add;
	static long	reference_del;
	SP_Node		base = nullptr;
	SP_Scope	parent = nullptr;
	std::vector<std::string>	param_keys;
	std::vector<int> param_types;
	VEC_Memory	default_params;

	Lambda(const SP_Scope&, const SP_Node&, std::vector<std::string>, std::vector<int>, VEC_Memory);
	~Lambda();
	SP_Memory	execute(VEC_Memory);
	SP_Lambda	clone(const SP_Scope&);
};

struct Scope : std::enable_shared_from_this<Scope> {
	static long	reference_add;
	static long	reference_del;
	SP_Scope	parent = nullptr;
	SP_Node		main = nullptr;
	std::unordered_map<std::string, SP_Memory> variables;

	Scope(SP_Scope);
	Scope(SP_Scope, SP_Node);
	~Scope();

	SP_Memory	execute();
	SP_Memory	getVariable(std::string);
	SP_Memory	declareVariable(std::string);
	SP_Scope	clone(SP_Scope);
	std::string		toString();
};

class Interpreter {
	friend Node;
	friend Lambda;
	friend Scope;
	friend Memory;
	friend RuotaWrapper;
private:
	std::string current_dir;
	static std::unordered_map<std::string, VEC_Memory(*)(VEC_Memory)> embedded;
	static std::unordered_map<std::string, int> operators;
	std::vector<std::string> LOADED;
	Interpreter(std::string);
	SP_Scope generate(std::string, SP_Scope, std::string);
	SP_Memory execute(SP_Scope);
	static void throwError(std::string errorMessage, std::string errorLine);
public:	
	static std::string path;
	static std::string curr_file;
	static void addEmbed(std::string, VEC_Memory(*e)(VEC_Memory));
};

class RuotaWrapper {
private:
	Interpreter * interpreter;
	SP_Scope main_scope;
	std::string current_dir;
public:
	RuotaWrapper(std::string);
	SP_Memory runLine(std::string);
	~RuotaWrapper();
};

#endif // !INTERPRETER_H
