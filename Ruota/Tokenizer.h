#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<vector>
#include<unordered_map>
#include<string>
#include<algorithm>
#include<locale>
#include<codecvt>
#include<iostream>

static std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

static std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

typedef	std::wstring		String;
typedef	std::vector<String>	VEC_String;

#define new_string	String

class Tokenizer {
public:
	Tokenizer(std::unordered_map<String, int>);
	std::unordered_map<String, int>	operators;
	VEC_String	tokenize(const String);
	VEC_String	infixToPostfix(VEC_String);
	int			getPrecedence(String);
};

#endif