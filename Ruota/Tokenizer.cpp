#include "Tokenizer.h"

Tokenizer::Tokenizer(std::unordered_map<std::string, int> operators) {
	this->operators = operators;
}

std::vector<std::string> Tokenizer::tokenize(const std::string str) {
	char pChar = 0;
	short mode = 0;
	char last_c = 0;
	bool comment = false;
	std::vector<std::string> tokens;

	for (const char c : str) {
		if (comment) {
			if (c == '\n' || c == '\r') {
				comment = false;
				mode = 0;
			}
			continue;
		}
		if (pChar != 0) {
			if (c == pChar && last_c != '\\')
				pChar = 0;
			else {
				if (last_c == '\\') {
					tokens.back().pop_back();
					switch (c)
					{
					case '\\':
						tokens.back().push_back('\\');
						last_c = 0;
						continue;
						break;
					case 'n':
						tokens.back().push_back('\n');
						break;
					case 't':
						tokens.back().push_back('\t');
						break;
					case 'r':
						tokens.back().push_back('\r');
						break;
					case '\'':
						tokens.back().push_back('\'');
						break;
					case '"':
						tokens.back().push_back('"');
						break;
					default:
						tokens.back().push_back('\\');
						tokens.back().push_back(c);
						break;
					}
				}else
					tokens.back().push_back(c);
			}
		}
		else {
			if (c == '#') {
				comment = true;
				continue;
			}
			if (isspace(c)) {
				mode = 0;
				continue;
			}
			else if (c == '\"' || c == '\'' || c == '`') {
				mode = 0;
				pChar = c;
				tokens.push_back(std::string(1, c));
				continue;
			}
			switch (mode)
			{
			case 0:
				tokens.push_back(std::string(1, c));
				break;
			case 1:
				if ((isalnum(c) || c == '_'))
					tokens.back().push_back(c);
				else
					tokens.push_back(std::string(1, c));
				break;
			case 2:
				if (!(isalnum(c) || c == '_') && operators.find(tokens.back() + std::string(1,c)) != operators.end())
					tokens.back().push_back(c);
				else
					tokens.push_back(std::string(1, c));
				break;
			}
			mode = !(isalnum(c) || c == '_') + 1;
		}
		last_c = c;
	}

	std::vector<std::string> new_tokens;
	std::string last = "+";
	for (auto t : tokens) {
		if (t == "[" && ((isalnum(last[0]) || last[0] == '_') || last == ")" || last == "]" || last == "}") && operators.find(last) == operators.end())
			new_tokens.push_back(".index");
		if (t == "(" && ((isalnum(last[0]) || last[0] == '_') || last == ")" || last == "]" || last == "}") && operators.find(last) == operators.end())
			new_tokens.push_back(".exec");
		if (t == "-" && (operators.find(last) != operators.end() || last == "(" || last == "[" || last == "{")) {
			new_tokens.push_back(".negate");
			last = t;
			continue;
		}
		if (isdigit(t[0]) && last == "." && isdigit(new_tokens[new_tokens.size() - 2][0])) {
			new_tokens.pop_back();
			new_tokens.back().push_back('.');
			new_tokens.back() = new_tokens.back() + t;
			last = t;
			continue;
		}
		new_tokens.push_back(t);
		last = t;
	}

	return new_tokens;
}

std::vector<std::string> Tokenizer::infixToPostfix(std::vector<std::string> tokens) {
	std::vector<std::string> stack;
	std::vector<std::string> output;
	int p_count = 0;
	int b_count = 0;
	int c_count = 0;

	for (auto token : tokens) {
		if (((isalnum(token[0]) || token[0] == '_') || token[0] == '\"' || token[0] == '\'' || token[0] == '`') && operators.find(token) == operators.end())
			output.push_back(token);
		else if (operators.find(token) != operators.end()) {
			if (!stack.empty()) {
				auto sb = stack.back();
				while ((std::abs(getPrecedence(token)) < std::abs(getPrecedence(sb))
					|| (getPrecedence(sb) > 0
						&& std::abs(getPrecedence(token)) == std::abs(getPrecedence(sb))))
					&& (sb != "(" && sb != "[" && sb != "{")) {
					output.push_back(sb);
					stack.pop_back();
					if (stack.empty())
						break;
					else
						sb = stack.back();
				}
			}
			stack.push_back(token);
		}
		else if (token == "(" || token == "[" || token == "{") {
			stack.push_back(token);
			output.push_back(token);
			switch (token[0]) {
				case '(': p_count++; break;
				case '[': b_count++; break;
				case '{': c_count++; break;
			}
		}
		else if (token == ")" || token == "]" || token == "}") {
			switch (token[0]) {
			case ')':
				p_count--;
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched paranthesis!");
				while (stack.back() != "(") {
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched paranthesis!");
				}
				break;
			case ']':
				b_count--;
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched bracket!");
				while (stack.back() != "[") {
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched bracket!");
				}
				break;
			case '}':
				c_count--;
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched scoping bracket!");
				while (stack.back() != "{") {
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched scoping bracket!");
				}
				break;
			}
			stack.pop_back();
			output.push_back(token);
		}
	}

	if (p_count != 0) throw std::runtime_error("Error: Unmatched paranthesis!");
	if (b_count != 0) throw std::runtime_error("Error: Unmatched bracket!");
	if (c_count != 0) throw std::runtime_error("Error: Unmatched scoping bracket!");

	while (!stack.empty()) {
		output.push_back(stack.back());
		stack.pop_back();
	}

	return output;
}

int Tokenizer::getPrecedence(std::string token) {
	if (operators.find(token) != operators.end())
		return operators[token];

	return 999;
}