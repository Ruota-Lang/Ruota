#ifndef RUOTA_H
#define RUOTA_H

const char RUOTA_VERSION[] = "0.11.2.2 Alpha";

#define THREADING

#include "Tokenizer.h"
#include<string>
#include<unordered_map>
#include<vector>
#include<fstream>
#include<iostream>
#include <stdlib.h>
#include <stdio.h>
#include<algorithm>
#include<memory>
#include<time.h>
#include<functional>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include<chrono>
#include<cmath>
#include<regex>
#include <signal.h>
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
	PULL_ARR,	// pull
	LAST_ARR,	// last
	TOARR,		// arr
	TRY_CATCH,	// try .. catch
	DECLARE,	// var
	INHERIT,	// +>
	EVAL,		// eval()
	OBJ_KEYS,	// keys
	GET_MET,	// get

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


#define	SP_MEMORY	std::shared_ptr<Memory>
#define	SP_NODE		std::shared_ptr<Node>
#define	SP_SCOPE	std::shared_ptr<Scope>
#define	SP_LAMBDA	std::shared_ptr<Lambda>

typedef	std::vector<SP_MEMORY>	VEC_Memory;
typedef	std::vector<SP_NODE>	VEC_Node;

#define	NEW_MEMORY	std::make_shared<Memory>
#define	NEW_NODE	std::make_shared<Node>
#define	NEW_LAMBDA	std::make_shared<Lambda>
#define	NEW_SCOPE	std::make_shared<Scope>

#define to_this_ptr	shared_from_this()

struct Memory : std::enable_shared_from_this<Memory> {
private:
	long double	data = 0;
	char		char_data = 0;
	MemType		mt;
	VEC_Memory	arr_data;
	SP_SCOPE	obj_data;
	SP_LAMBDA	lambda;
	SP_MEMORY	reference;
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
	Memory(SP_SCOPE);
	Memory(SP_LAMBDA);
	Memory(VEC_Memory);
	Memory(void*);

	SP_MEMORY 	add(const SP_MEMORY&) const;
	SP_MEMORY 	sub(const SP_MEMORY&) const;
	SP_MEMORY 	mul(const SP_MEMORY&) const;
	SP_MEMORY 	div(const SP_MEMORY&) const;
	SP_MEMORY 	mod(const SP_MEMORY&) const;
	SP_MEMORY 	pow(const SP_MEMORY&) const;
	SP_MEMORY 	less(const SP_MEMORY&) const;
	SP_MEMORY 	more(const SP_MEMORY&) const;
	SP_MEMORY 	eless(const SP_MEMORY&) const;
	SP_MEMORY 	emore(const SP_MEMORY&) const;
	bool 		equals(const SP_MEMORY&) const;
	ObjectMode	getObjectMode() const;
	bool 		isLocal() const;
	SP_MEMORY	set(const SP_MEMORY&);
	long double	getValue() const;
	SP_MEMORY	index(const SP_MEMORY&);
	SP_MEMORY	steal(const SP_MEMORY&);
	SP_MEMORY	index(const std::string&);
	SP_MEMORY	setObjectMode(const ObjectMode&);
	SP_MEMORY	setLocal(const bool&);
	SP_MEMORY	setArray(VEC_Memory);
	SP_MEMORY	setValue(const long double&);
	const std::string		toString() const;
	SP_MEMORY	clone(const SP_SCOPE&) const;
	SP_LAMBDA	getLambda();
	SP_SCOPE	getScope();
	SP_MEMORY	setScope(const SP_SCOPE&);
	SP_MEMORY	makeScope(const SP_SCOPE&);
	SP_MEMORY	setType(const MemType&);
	SP_MEMORY	refer(const SP_MEMORY&);
	SP_MEMORY	pop();
	SP_MEMORY	shift();
	SP_MEMORY	push(SP_MEMORY&);
	SP_MEMORY	unshift(SP_MEMORY&);
	SP_MEMORY	eraseLambda();
	VEC_Memory	getArray();
	MemType		getType() const;
	void		clear();
	void *		getPointer();
};

struct Node : std::enable_shared_from_this<Node> {
	static long	reference_add;
	static long	reference_del;
	VEC_Node 	params;
	NodeType 	nt;
	SP_SCOPE 	scope_ref = nullptr;
	SP_MEMORY 	mem_data = nullptr;
	std::string		key;
	std::unordered_map<long double, SP_NODE> switch_values;
	int			flag = 0;

	Node(SP_SCOPE);
	Node(long double);
	Node(SP_NODE, std::unordered_map<long double, SP_NODE>);
	Node(std::string);
	Node(NodeType, VEC_Node);
	Node(SP_MEMORY);
	~Node();

	const std::string	toString() const;
	SP_MEMORY	execute(const SP_SCOPE&) const;
	SP_NODE		clone(const SP_SCOPE&) const;
	void		weakListCheck(const SP_SCOPE&);
	void		destroy();
	static void	threadWrapper(SP_NODE, SP_SCOPE);
};

struct Lambda : std::enable_shared_from_this<Lambda> {
	static long	reference_add;
	static long	reference_del;
	SP_NODE		base = nullptr;
	SP_SCOPE	parent = nullptr;
	std::vector<std::string>	param_keys;
	std::vector<int> param_types;
	VEC_Memory	default_params;

	Lambda(const SP_SCOPE&, const SP_NODE&, std::vector<std::string>, std::vector<int>, VEC_Memory);
	~Lambda();
	SP_MEMORY	execute(VEC_Memory);
	SP_LAMBDA	clone(const SP_SCOPE&) const;
};

struct Scope : std::enable_shared_from_this<Scope> {
	std::string	key;

	static long	reference_add;
	static long	reference_del;
	SP_SCOPE	parent = nullptr;
	SP_NODE		main = nullptr;
	std::unordered_map<std::string, SP_MEMORY> variables;

	Scope(SP_SCOPE, std::string);
	Scope(SP_SCOPE, std::string, SP_NODE);
	~Scope();

	SP_MEMORY	execute();
	SP_MEMORY	getVariable(std::string);
	SP_MEMORY	declareVariable(std::string);
	SP_SCOPE	clone(SP_SCOPE) const;
	std::string			getPath() const;
	const std::string	toString() const;
};

class Interpreter {
	friend Node;
	friend Lambda;
	friend Scope;
	friend Memory;
	friend RuotaWrapper;
private:
	static std::string current_dir;
	static std::unordered_map<std::string, VEC_Memory(*)(VEC_Memory)> embedded;
	static std::unordered_map<std::string, int> operators;
	std::vector<std::string> LOADED;
	Interpreter(std::string);
	SP_SCOPE generate(std::string, SP_SCOPE, std::string);
	SP_MEMORY execute(SP_SCOPE);
	static void throwError(std::string errorMessage, std::string errorLine);
public:	
	static std::string path;
	static std::string curr_file;
	static void addEmbed(std::string, VEC_Memory(*e)(VEC_Memory));
};

class RuotaWrapper {
private:
	SP_SCOPE main_scope;
	std::string current_dir;
public:
	static SP_LAMBDA on_exit;
	static void debugPrint();
	static Interpreter * interpreter;
	RuotaWrapper(std::string);
	SP_MEMORY runLine(std::string);
	~RuotaWrapper();
};

void signalCommand(int s);

#endif // !INTERPRETER_H
