#include "Ruota.h"

String Interpreter::path = L"\\";
String Interpreter::curr_file = L"";

std::unordered_map<String, int> Interpreter::operators = {
	{ L"load", 999 },
	{ L".index", 13 },
	{ L".exec", 13 },
	{ L"from", 12 },
	{ L"+>", 13 },
	{ L"::", 12 },
	{ L"new", 12 },
	{ L"struct", 12 },
	{ L"static", 14 },
	{ L"dynamic", 14 },
	{ L"var", -13 },
	{ L"local", -1 },
	{ L"!", -13 },
	{ L".!", -13 },
	{ L".", 13 },
	{ L"->", 11 },
	{ L"=>", 11 },
	{ L"type", -10 },
	{ L"len", -10 },
	{ L"&", -10 },

	{ L"num", -10 },
	{ L"str", -10 },
	{ L"arr", -10 },
	{ L"chr", -10 },
	{ L"alloc", -10 },

	{ L"pop", -10 },
	{ L"mov", -10 },
	{ L".negate", -10 },
	{ L".positive", -10 },
	{ L"**", -9 },
	{ L".**", -9 },
	{ L"*", 8 },
	{ L".*", 8 },
	{ L"/", 8 },
	{ L"./", 8 },
	{ L"%", 8 },
	{ L".%", 8 },
	{ L"+", 7 },
	{ L".+", 7 },
	{ L"++", 7 },
	{ L"..", 7 },
	{ L"-", 7 },
	{ L".-", 7 },
	{ L":", 6 },
	{ L"<:", 6 },
	{ L":>", 6 },
	{ L"<:>", 6 },
	{ L"==", 5 },
	{ L".=", 5 },
	{ L".==", 5 },
	{ L"!=", 5 },
	{ L".!=", 5 },
	{ L"<", 5 },
	{ L".<", 5 },
	{ L">", 5 },
	{ L".>", 5 },
	{ L"<=", 5 },
	{ L".<=", 5 },
	{ L">=", 5 },
	{ L".>=", 5 },
	{ L"&&", 4 },
	{ L"||", 4 },
	{ L"push", 4 },
	{ L"post", 4 },
	{ L"=", -3 },
	{ L"+=", -3},
	{ L".+=", -3},
	{ L"-=", -3},
	{ L".-=", -3},
	{ L"*=", -3},
	{ L".*=", -3},
	{ L"/=", -3},
	{ L"./=", -3},
	{ L"%=", -3},
	{ L".%=", -3},
	{ L"**=", -3},
	{ L".**=", -3},
	{ L"..=", -3},
	{ L"++=", -3},
	{ L":=", -3},
	{ L"&=", -3},
	{ L":&", -3},
	{ L":&=", -3},
	{ L">>", 2 },
	{ L"in", 2 },
	{ L"switch", 2 },
	{ L"do", 2 },
	{ L"->>", -2 },
	{ L"then", 2 },
	{ L"else", -2 },
	{ L"detach", -1 },
	{ L"try", -15 },
	{ L"catch", 14 },
	{ L",", 1 },
	{ L";", 0 }
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
		if (token == L"," || token == L";") continue;
		if (token == L"load") {
			std::string var_string = ws2s(stack.back()->execute(current)->toString());
			if (var_string.back() == '*'){
				std::vector<std::string> files;
				std::string path = ws2s(Interpreter::path.substr(1)) + var_string;
				path.pop_back();
				for (auto &p : std::filesystem::directory_iterator(path)){
					std::string file = p.path().string();
					if (file.length() > 4 && file.compare(file.length() - 4, 4, ".ruo") == 0)
						files.push_back(file);
				}

				for (auto &f : files) {
					std::string filename_raw = f;
					std::string path = ws2s(local_file);
					stack.pop_back();
					std::string filename = "";

					//if (local_file == L"") {
						path += filename_raw;
						while (path.back() != '\\' && path.back() != '/') {
							filename = std::string(1, path.back()) + filename;
							path.pop_back();
							if (path.empty()) break;
						}
					//}

					if (std::find(LOADED.begin(), LOADED.end(), s2ws(filename)) == LOADED.end()) {
						String content = L"";
						String line;
						std::wifstream myfile(path.substr(1) + filename);
						if (myfile.is_open()){
							while (getline(myfile, line))
								content += line + L"\n";
							myfile.close();
						}else{
							std::wifstream myfilelocal(ws2s(this->current_dir) + filename_raw);
							if(myfilelocal.is_open()){
								while (getline(myfilelocal, line))
									content += line + L"\n";
								myfilelocal.close();
							}else {
								throwError(L"Error: Cannot Load File: " + s2ws(filename) + L"!", L"load \"" + s2ws(filename_raw.substr(0, filename_raw.length() - 4)) + L"\";");
							}
						}
						LOADED.push_back(s2ws(filename));
						Interpreter::path = s2ws(path);
						String old_file = Interpreter::curr_file;
						Interpreter::curr_file = s2ws(filename);
						auto gen = new_node(generate(content, current, s2ws(path))->execute());
						Interpreter::path = local_file;
						Interpreter::curr_file = old_file;
						stack.push_back(gen);
					}
					else {
						stack.push_back(new_node(0));
					}
				}
			} else{
				std::string filename_raw = var_string + ".ruo";
				std::string path = ws2s(local_file);
				stack.pop_back();
				std::string filename = "";

				//if (local_file == L"") {
					path += filename_raw;
					while (path.back() != '\\' && path.back() != '/') {
						filename = std::string(1, path.back()) + filename;
						path.pop_back();
						if (path.empty()) break;
					}
				//}

				if (std::find(LOADED.begin(), LOADED.end(), s2ws(filename)) == LOADED.end()) {
					String content = L"";
					String line;
					std::wifstream myfile(path.substr(1) + filename);
					if (myfile.is_open()){
						while (getline(myfile, line))
							content += line + L"\n";
						myfile.close();
					}else{
						std::wifstream myfilelocal(ws2s(this->current_dir) + filename_raw);
						if(myfilelocal.is_open()){
							while (getline(myfilelocal, line))
								content += line + L"\n";
							myfilelocal.close();
						}else {
							throwError(L"Error: Cannot Load File: " + s2ws(filename) + L"!", L"load \"" + s2ws(filename_raw.substr(0, filename_raw.length() - 4)) + L"\";");
						}
					}
					LOADED.push_back(s2ws(filename));
					Interpreter::path = s2ws(path);
					String old_file = Interpreter::curr_file;
					Interpreter::curr_file = s2ws(filename);
					auto gen = new_node(generate(content, current, s2ws(path))->execute());
					Interpreter::path = local_file;
					Interpreter::curr_file = old_file;
					stack.push_back(gen);
				}
				else {
					stack.push_back(new_node(0));
				}
			}
		}
		else if (token == L"inf")
			stack.push_back(new_node((double)INFINITY));
		else if (token == L"true")
			stack.push_back(new_node(1));
		else if (token == L"false")
			stack.push_back(new_node(0));
		else if (token == L"break")
			stack.push_back(new_node(BREAK, nullvec));
		else if (token == L"return")
			stack.push_back(new_node(RETURN, nullvec));
		else if (token == L"null") {
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
			if (stack.empty())
				throwError(L"Error: unbalanced operator!", token);
			auto b = stack.back();
			stack.pop_back();
			SP_Node a = nullptr;
			if (!stack.empty()){
				a = stack.back(); 
				stack.pop_back();
			}
			VEC_Node params = { a, b };
			if (token == L"=")
				stack.push_back(new_node(SET, params));
			else if (token == L":=") {
				params = { a };
				auto dec = new_node(DECLARE, params);
				params = { dec, b };
				stack.push_back(new_node(SET, params));
			}
			else if (token == L"var") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(DECLARE, params));
			}
			else if (token == L"local") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(LOCAL, params));
			}
			else if (token == L"len") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(SIZE_O, params));
			}
			else if (token == L".negate") {
				if (a != nullptr)
					stack.push_back(a);
				params = { new_node(0), b };
				stack.push_back(new_node(SUB, params));
			}
			else if (token == L".positive") {
				if (a != nullptr)
					stack.push_back(a);
				params = { new_node(0), b };
				stack.push_back(new_node(ADD, params));
			}
			else if (token == L"&") {
				if (a != nullptr)
					stack.push_back(a);
				b->flag = 1;
				stack.push_back(b);
			}
			else if (token == L"type") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TYPE, params));
			}
			else if (token == L"num") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(VALUE, params));
			}
			else if (token == L"chr") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TOCHAR, params));
			}
			else if (token == L"alloc") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(ALLOC, params));
			}
			else if (token == L"pop") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(POP_ARR, params));
			}
			else if (token == L"mov") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(SHIFT_ARR, params));
			}
			else if (token == L"str") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TOSTRING, params));
			}
			else if (token == L"arr") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TOARR, params));
			}
			else if (token == L"static") {
				if (a != nullptr)
					stack.push_back(a);
				stack.push_back(new_node(b->key));
				stack.back()->nt = SET_STAT;
			}
			else if (token == L"dynamic") {
				if (a != nullptr)
					stack.push_back(a);
				stack.push_back(new_node(b->key));
				stack.back()->nt = STRUCT;
			}
			else if (token == L"struct") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(OBJ_LAM, params));
			}
			else if (token == L"new") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(NEW, params));
			}
			else if (token == L"detach") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(DETACH, params));
			}
			else if (token == L"try") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(TRY_CATCH, params));
			}
			else if (token == L"catch"){
				if (a->nt != TRY_CATCH)
					throw std::runtime_error("Error: no `try` section found!");
				a->params.push_back(b);
				stack.push_back(a);
			}
			else if (token == L"!") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(NOT, params));
			}
			else if (token == L".!") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(new_node(NOT, params));
				stack.back()->flag = 1;
			}
			else if (token == L"::")
				stack.push_back(new_node(OBJ_SET, params));
			else if (token == L"+>")
				stack.push_back(new_node(INHERIT, params));

		// ADDITION RELATED OPERATORS
			else if (token == L"+")
				stack.push_back(new_node(ADD, params));
			else if (token == L".+") {
				auto s = new_node(ADD, params);
				s->flag = 1;
				stack.push_back(s);
			}
			else if (token == L"+="){
				auto s = new_node(ADD, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L".+=") {
				auto s = new_node(ADD, params);
				s->flag = 1;
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
		
		// CONCATANATION RELATED OPERATORS
			else if (token == L"++")
				stack.push_back(new_node(ADD_ARR, params));
			else if (token == L"++="){
				auto s = new_node(ADD_ARR, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L"..")
				stack.push_back(new_node(STR_CAT, params));
			else if (token == L"..="){
				auto s = new_node(STR_CAT, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}

		// SUBTRACTION RELATED OPERATORS
			else if (token == L"-")
				stack.push_back(new_node(SUB, params));
			else if (token == L".-") {
				auto s = new_node(SUB, params);
				s->flag = 1;
				stack.push_back(s);
			}
			else if (token == L"-="){
				auto s = new_node(SUB, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L".-=") {
				auto s = new_node(SUB, params);
				s->flag = 1;
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}

		// MULTIPLICATION RELATED OPERATORS
			else if (token == L"*")
				stack.push_back(new_node(MUL, params));
			else if (token == L".*") {
				auto s = new_node(MUL, params);
				s->flag = 1;
				stack.push_back(s);
			}
			else if (token == L"*="){
				auto s = new_node(MUL, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L".*=") {
				auto s = new_node(MUL, params);
				s->flag = 1;
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}

		// DIVISION RELATED OPERATORS
			else if (token == L"/")
				stack.push_back(new_node(DIV, params));
			else if (token == L"./") {
				auto s = new_node(DIV, params);
				s->flag = 1;
				stack.push_back(s);
			}
			else if (token == L"/="){
				auto s = new_node(DIV, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L"./=") {
				auto s = new_node(DIV, params);
				s->flag = 1;
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}

		// MODULUS RELATED OPERATORS
			else if (token == L"%")
				stack.push_back(new_node(MOD, params));
			else if (token == L".%") {
				auto s = new_node(MOD, params);
				s->flag = 1;
				stack.push_back(s);
			}
			else if (token == L"%="){
				auto s = new_node(MOD, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L".%=") {
				auto s = new_node(MOD, params);
				s->flag = 1;
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}

		// POWER RELATED OPERATORS
			else if (token == L"**")
				stack.push_back(new_node(POW, params));
			else if (token == L".**") {
				auto s = new_node(POW, params);
				s->flag = 1;
				stack.push_back(s);
			}
			else if (token == L"**="){
				auto s = new_node(POW, params);
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}
			else if (token == L".**=") {
				auto s = new_node(POW, params);
				s->flag = 1;
				params = {a, s};
				stack.push_back(new_node(SET, params));
			}

		// EQUALITY RELATED OPERATORS
			else if (token == L"==")
				stack.push_back(new_node(EQUAL, params));
			else if (token == L".==") {
				auto s = new_node(EQUAL, params);
				s->flag = 1;
				stack.push_back(s);
			}
		
		// INEQUALITY RELATED OPERATORS
			else if (token == L"!=")
				stack.push_back(new_node(NEQUAL, params));
			else if (token == L".!=") {
				auto s = new_node(NEQUAL, params);
				s->flag = 1;
				stack.push_back(s);
			}
		
		// LESS THAN RELATED OPERATORS
			else if (token == L"<")
				stack.push_back(new_node(LESS, params));
			else if (token == L".<") {
				auto s = new_node(LESS, params);
				s->flag = 1;
				stack.push_back(s);
			}

		// MORE THAN RELATED OPERATORS
			else if (token == L">")
				stack.push_back(new_node(MORE, params));
			else if (token == L".>") {
				auto s = new_node(MORE, params);
				s->flag = 1;
				stack.push_back(s);
			}

		// ELESS THAN RELATED OPERATORS
			else if (token == L"<=")
				stack.push_back(new_node(ELESS, params));
			else if (token == L".<=") {
				auto s = new_node(ELESS, params);
				s->flag = 1;
				stack.push_back(s);
			}

		// EMORE THAN RELATED OPERATORS
			else if (token == L">=")
				stack.push_back(new_node(EMORE, params));
			else if (token == L".>=") {
				auto s = new_node(EMORE, params);
				s->flag = 1;
				stack.push_back(s);
			}

			else if (token == L"&&")
				stack.push_back(new_node(AND, params));
			else if (token == L"||")
				stack.push_back(new_node(OR, params));
			else if (token == L"push")
				stack.push_back(new_node(PUSH_ARR, params));
			else if (token == L"post")
				stack.push_back(new_node(UNSHIFT_ARR, params));
			else if (token == L"&=")
				stack.push_back(new_node(REF_SET, params));
			else if (token == L":&=") {			
				params = { a };
				auto dec = new_node(DECLARE, params);
				params = { dec, b };
				stack.push_back(new_node(REF_SET, params));
			}
			else if (token == L"->>")
				stack.push_back(new_node(EXEC_ITER, params));
			else if (token == L"then")
				stack.push_back(new_node(THEN, params));
			else if (token == L"switch"){
				std::unordered_map<long double, SP_Node> switch_values;
				for (auto &n : b->scope_ref->main->params) {
					switch_values[n->params[0]->mem_data->getValue()] = n->params[1];
				}
				stack.push_back(new_node(a, switch_values));
			}
			else if (token == L">>") {
				if (a->nt == SWITCH){
					a->params.push_back(b);
					stack.push_back(a);
				} else
					stack.push_back(new_node(CASE, params));
			}
			else if (token == L"else") {
				a->params.push_back(b);
				stack.push_back(a);
			}
			else if (token == L".index")	
				stack.push_back(new_node(INDEX, params));
			else if (token == L".")
				stack.push_back(new_node(INDEX_OBJ, params));
			else if (token == L".exec") {
				if (a->key == L"_OUTER_CALL_") {
					b->nt = OUT_CALL;
					stack.push_back(b);
				}
				else if (a->key == L"thread"){
					b->nt = THREAD;
					stack.push_back(b);
				}
				else {
					b->nt = LIST;
					stack.push_back(new_node(EXEC, params));
				}
			}
			else if (token == L"from")
				stack.push_back(new_node(FROM, params));
			else if (token == L"in")
				stack.push_back(new_node(ITER, params));
			else if (token == L"do")
				stack.push_back(new_node(DOL, params));
			else if (token == L":") {
				if (a->nt == RANGE) {
					a->params.push_back(b);
					stack.push_back(a);
				}else
					stack.push_back(new_node(RANGE, params));
			}
			else if (token == L":>") {
				stack.push_back(new_node(RANGE, params));
				stack.back()->flag = 1;
			}
			else if (token == L"=>")
				stack.push_back(new_node(LDES, params));
			else if (token == L"->") {
				a->nt = LIST;
				stack.push_back(new_node(DES, params));
			}
		}
		else if (token == L"[") {
			stack.push_back(new_node(M_B, nullvec));
		}
		else if (token == L"{") {
			stack.push_back(new_node(M_S, nullvec));
			current = new_scope(current);
		}
		else if (token == L"(") {
			stack.push_back(new_node(M_P, nullvec));
		}
		else if (token == L"]") {
			VEC_Node new_list;
			while (stack.back()->nt != M_B) {
				new_list.push_back(stack.back());
				stack.pop_back();
				if (stack.empty())
					throwError(L"Error: unmatched bracket!", token);
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			stack.push_back(new_node(LIST, new_list));
		}
		else if (token == L")") {
			VEC_Node new_list;
			while (stack.back()->nt != M_P) {
				new_list.push_back(stack.back());
				stack.pop_back();
				if (stack.empty())
					throwError(L"Error: unmatched parenthesis!", token);
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			stack.push_back(new_node(SOFT_LIST, new_list));
		}
		else if (token == L"}") {
			VEC_Node new_list;
			while (stack.back()->nt != M_S) {
				new_list.push_back(stack.back());
				stack.pop_back();
				if (stack.empty())
					throwError(L"Error: unmatched bracket!", token);
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
	throw std::runtime_error(ws2s(errorMessage + L"\n\t" + errorLine + L"\n\tFile: " + Interpreter::curr_file).c_str());
}