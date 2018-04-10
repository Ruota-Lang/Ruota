#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<vector>
#include<map>
#include<string>
#include<algorithm>
#include<iostream>

class Tokenizer {
public:
	std::map<std::string, int> operators;
	Tokenizer(std::map<std::string, int>);
	std::vector<std::string> tokenize(const std::string);
	std::vector<std::string> infixToPostfix(std::vector<std::string>);
	int getPrecedence(std::string);
};

#endif