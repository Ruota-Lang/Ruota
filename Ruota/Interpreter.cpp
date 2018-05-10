#include "Ruota.h"

std::unordered_map<String, int> Interpreter::operators = {
	{ "load", 999 },
	{ ".index", 13 },
	{ ".exec", 13 },
	{ "from", 12 },
	{ "->>", 12 },
	{ "::", 12 },
	{ "new", 12 },
	{ "struct", 12 },
	{ "static", 14 },
	{ "dynamic", 14 },
	{ "!", -13 },
	{ ".", 13 },
	{ "->", 11 },
	{ "=>", 11 },
	{ "end", -10 },
	{ "type", -10 },
	{ "len", -10 },
	{ "&", -10 },

	{ "num", -10 },
	{ "str", -10 },
	{ "arr", -10 },
	{ "chr", -10 },
	{ "alloc", -10 },

	{ "pop", -10 },
	{ "mov", -10 },
	{ ".negate", -10 },
	{ "**", -9 },
	{ "*", 8 },
	{ "/", 8 },
	{ "%", 8 },
	{ "+", 7 },
	{ "++", 7 },
	{ "..", 7 },
	{ "-", 7 },
	{ ":", 6 },
	{ "<:", 6 },
	{ ":>", 6 },
	{ "<:>", 6 },
	{ "==", 5 },
	{ "!=", 5 },
	{ "<", 5 },
	{ ">", 5 },
	{ "<=", 5 },
	{ ">=", 5 },
	{ "&&", 4 },
	{ "||", 4 },
	{ "push", 4 },
	{ "post", 4 },
	{ "=", -3 },
	{ "+=", -3},
	{ "-=", -3},
	{ "*=", -3},
	{ "/=", -3},
	{ "%=", -3},
	{ "**=", -3},
	{ "..=", -3},
	{ "++=", -3},
	{ ":=", -3},
	{ "&=", -3},
	{ ":&", -3},
	{ ":&=", -3},
	{ ".=", -3},
	{ ">>", 2 },
	{ "in", 2 },
	{ "switch", 2 },
	{ "do", 2 },
	{ "then", 2 },
	{ "else", -2 },
	{ "detach", -1 },
	{ ",", 1 },
	{ ";", 0 }
};

std::unordered_map<String, VEC_Memory(*)(VEC_Memory)> Interpreter::embedded = {};

void Interpreter::addEmbed(String s, VEC_Memory(*e)(VEC_Memory)) {
	embedded[s] = e;
}

Interpreter::Interpreter(String current_dir) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	srand(timeMillis);
	this->current_dir = current_dir;
}

SP_Scope Interpreter::generate(String code, SP_Scope main, String local_file) {
	Tokenizer * t = new Tokenizer(operators);
	auto tokenized = t->tokenize(code);
	auto tokens = t->infixToPostfix(tokenized);
	SP_Scope current = main;
	VEC_Node stack;
	VEC_Node nullvec;

	for (auto &token : tokens) {
		if (token == "," || token == ";") continue;
		if (token == "load") {
			String filename_raw = stack.back()->execute(current)->toString() + ".ruo";
			String path = local_file;
			stack.pop_back();
			String filename = "";

			//if (local_file == "") {
				path += filename_raw;
				while (path.back() != '\\' && path.back() != '/') {
					filename = String(1, path.back()) + filename;
					path.pop_back();
					if (path.empty()) break;
				}
			//}

			if (std::find(LOADED.begin(), LOADED.end(), filename) == LOADED.end()) {
				String content = "";
				String line;
				std::ifstream myfile(path.substr(1) + filename);
				if (myfile.is_open()){
					while (getline(myfile, line))
						content += line + "\n";
					myfile.close();
				}else{
					std::ifstream myfilelocal(this->current_dir + filename_raw);
					if(myfilelocal.is_open()){
						while (getline(myfilelocal, line))
							content += line + "\n";
						myfilelocal.close();
					}else {
						throwError("Error: Cannot Load File: " + filename + "!", "load \"" + filename_raw.substr(0, filename_raw.length() - 4) + "\";");
					}
				}
				LOADED.push_back(filename);
				auto gen = generate(content, current, path);
				stack.push_back(gen->main);
			}
			else {
				stack.push_back(new_node(0));
			}
		}
		else if (token == "true")
			stack.push_back(new_node(1));
		else if (token == "false")
			stack.push_back(new_node(0));
		else if (token == "break")
			stack.push_back(new_node(BREAK, nullvec));
		else if (token == "return")
			stack.push_back(new_node(RETURN, nullvec));
		else if (token == "null") {
			SP_Memory nullmem = new_memory();
			nullmem->setValue(1);
			SP_Node nullnode = new_node(0);
			nullnode->mem_data = nullmem;
			stack.push_back(nullnode);
		}
		else if (isdigit(token[0]))
			stack.push_back(new_node(stod(token)));
		else if (token[0] == '\''){
			stack.push_back(new_node(new_memory(CHA, token[1])));
		}
		else if (token[0] == '\"') {
			stack.push_back(new_node(new_memory(token.substr(1))));
		}
		else if (operators.find(token) != operators.end()) {
			auto b = stack.back(); 
			stack.pop_back();
			SP_Node a = nullptr;
			if (!stack.empty()){
				a = stack.back(); 
				stack.pop_back();
			}
			VEC_Node params = { a, b };
			if (token == "=")
				stack.push_back(new_node(SET, params));
			else if (token == ":=")
				stack.push_back(new_node(DEC_SET, params));
			else if (token == "len") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(SIZE_O, params));
			}
			else if (token == ".negate") {
				if (a != nullptr)
					stack.push_back(a);
				params = { new_node(0), b };
				stack.push_back(new_node(SUB, params));
			}
			else if (token == "&") {
				if (a != nullptr)
					stack.push_back(a);
				b->flag = 1;
				stack.push_back(b);
			}
			else if (token == "type") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TYPE, params));
			}
			else if (token == "num") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(VALUE, params));
			}
			else if (token == "chr") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TOCHAR, params));
			}
			else if (token == "alloc") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(ALLOC, params));
			}
			else if (token == "end") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(LAST_ARR, params));
			}
			else if (token == "pop") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(POP_ARR, params));
			}
			else if (token == "mov") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(SHIFT_ARR, params));
			}
			else if (token == "str") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TOSTRING, params));
			}
			else if (token == "arr") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TOARR, params));
			}
			else if (token == "static") {
				if (a != nullptr)
					stack.push_back(a);
				stack.push_back(new_node(b->key));
				stack.back()->nt = SET_STAT;
			}
			else if (token == "dynamic") {
				if (a != nullptr)
					stack.push_back(a);
				stack.push_back(new_node(b->key));
				stack.back()->nt = STRUCT;
			}
			else if (token == "struct") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(OBJ_LAM, params));
			}
			else if (token == "new") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(NEW, params));
			}
			else if (token == "detach") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(DETACH, params));
			}
			else if (token == "!") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(NOT, params));
			}
			else if (token == "::")
				stack.push_back(new_node(OBJ_SET, params));
			else if (token == "+")
				stack.push_back(new_node(ADD, params));
			else if (token == "+="){
				auto s = new_node(ADD, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "++")
				stack.push_back(new_node(ADD_ARR, params));
			else if (token == "++="){
				auto s = new_node(ADD_ARR, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "..")
				stack.push_back(new_node(STR_CAT, params));
			else if (token == "..="){
				auto s = new_node(STR_CAT, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "-")
				stack.push_back(new_node(SUB, params));
			else if (token == "-="){
				auto s = new_node(SUB, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "*")
				stack.push_back(new_node(MUL, params));
			else if (token == "*="){
				auto s = new_node(MUL, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "/")
				stack.push_back(new_node(DIV, params));
			else if (token == "/="){
				auto s = new_node(DIV, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "%")
				stack.push_back(new_node(MOD, params));
			else if (token == "%="){
				auto s = new_node(MOD, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "**")
				stack.push_back(new_node(POW, params));
			else if (token == "**="){
				auto s = new_node(POW, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == "==")
				stack.push_back(new_node(EQUAL, params));
			else if (token == "!=")
				stack.push_back(new_node(NEQUAL, params));
			else if (token == "<")
				stack.push_back(new_node(LESS, params));
			else if (token == ">")
				stack.push_back(new_node(MORE, params));
			else if (token == "<=")
				stack.push_back(new_node(ELESS, params));
			else if (token == ">=")
				stack.push_back(new_node(EMORE, params));
			else if (token == "&&")
				stack.push_back(new_node(AND, params));
			else if (token == "||")
				stack.push_back(new_node(OR, params));
			else if (token == "push")
				stack.push_back(new_node(PUSH_ARR, params));
			else if (token == "post")
				stack.push_back(new_node(UNSHIFT_ARR, params));
			else if (token == "&=")
				stack.push_back(new_node(REF_SET, params));
			else if (token == ".=")
				stack.push_back(new_node(ARR_SET, params));
			else if (token == ":&=")
				stack.push_back(new_node(REF_SET_DEC, params));
			else if (token == "->>")
				stack.push_back(new_node(EXEC_ITER, params));
			else if (token == "then")
				stack.push_back(new_node(THEN, params));
			else if (token == "switch"){
				std::unordered_map<long double, SP_Node> switch_values;
				for (auto &n : b->scope_ref->main->params) {
					switch_values[n->params[0]->mem_data->getValue()] = n->params[1];
				}
				stack.push_back(new_node(a, switch_values));
			}
			else if (token == ">>") {
				if (a->nt == SWITCH){
					a->params.push_back(b);
					stack.push_back(a);
				} else
					stack.push_back(new_node(CASE, params));
			}
			else if (token == "else") {
				a->params.push_back(b);
				stack.push_back(a);
			}
			else if (token == ".index")	
				stack.push_back(new_node(INDEX, params));
			else if (token == ".")
				stack.push_back(new_node(INDEX_OBJ, params));
			else if (token == ".exec") {
				if (a->key == "_OUTER_CALL_") {
					b->nt = OUT_CALL;
					stack.push_back(b);
				}
				else if (a->key == "thread"){
					b->nt = THREAD;
					stack.push_back(b);
				}
				else {
					b->nt = LIST;
					stack.push_back(new_node(EXEC, params));
				}
			}
			else if (token == "from")
				stack.push_back(new_node(FROM, params));
			else if (token == "in")
				stack.push_back(new_node(ITER, params));
			else if (token == "do")
				stack.push_back(new_node(DOL, params));
			else if (token == ":") {
				if (a->nt == RANGE) {
					a->params.push_back(b);
					stack.push_back(a);
				}else
					stack.push_back(new_node(RANGE, params));
			}
			else if (token == ":>") {
				stack.push_back(new_node(RANGE, params));
				stack.back()->flag = 1;
			}
			else if (token == "=>")
				stack.push_back(new_node(LDES, params));
			else if (token == "->") {
				a->nt = LIST;
				stack.push_back(new_node(DES, params));
			}
		}
		else if (token == "[") {
			stack.push_back(new_node(M_B, nullvec));
		}
		else if (token == "{") {
			stack.push_back(new_node(M_S, nullvec));
			current = new_scope(current);
		}
		else if (token == "(") {
			stack.push_back(new_node(M_P, nullvec));
		}
		else if (token == "]") {
			VEC_Node new_list;
			while (stack.back()->nt != M_B) {
				new_list.push_back(stack.back());
				stack.pop_back();
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			stack.push_back(new_node(LIST, new_list));
		}
		else if (token == ")") {
			VEC_Node new_list;
			while (stack.back()->nt != M_P) {
				new_list.push_back(stack.back());
				stack.pop_back();
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			stack.push_back(new_node(SOFT_LIST, new_list));
		}
		else if (token == "}") {
			VEC_Node new_list;
			while (stack.back()->nt != M_S) {
				new_list.push_back(stack.back());
				stack.pop_back();
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			auto mainOfScope = new_node(LIST, new_list);
			current->main = mainOfScope;
			stack.push_back(new_node(current));
			current = current->parent;
		}
		else {
			stack.push_back(new_node(token));
		}
	}

	main->main = new_node(LIST, stack);
	main->main->weakListCheck();
	return main;
}

SP_Memory Interpreter::execute(SP_Scope main) {
	return main->execute();
}

void Interpreter::throwError(String errorMessage, String errorLine){
	while (errorLine[0] == '(' && errorLine.back() == ')')
		errorLine = errorLine.substr(1, errorLine.size() - 2);
	throw std::runtime_error(errorMessage + "\n\t" + errorLine);
}