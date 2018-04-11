#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<vector>
#include<map>
#include<string>
#include<algorithm>
#include<iostream>

typedef	std::string			String;
typedef	std::vector<String>	VEC_String;

class Tokenizer {
public:
	Tokenizer(std::map<String, int>);
	std::map<String, int>	operators;
	VEC_String	tokenize(const String);
	VEC_String	infixToPostfix(VEC_String);
	int			getPrecedence(String);
};

#endif