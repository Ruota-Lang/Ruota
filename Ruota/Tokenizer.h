#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<vector>
#include<unordered_map>
#include<string>
#include<algorithm>
#include<iostream>

class Tokenizer {
public:
	Tokenizer(std::unordered_map<std::string, int>);
	std::unordered_map<std::string, int>	operators;
	std::vector<std::string>	tokenize(const std::string);
	std::vector<std::string>	infixToPostfix(std::vector<std::string>);
	int			getPrecedence(std::string);
};

#endif