#pragma once
#include<string>
#include<vector>
using namespace std;

enum TokenType {
	TOKEN_NUMBER = 0,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_MOD,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_EOF,
	TOKEN_ERROR
};

struct Token{
	TokenType type;
	string value;
};

class Lexer{
public:
	Lexer()=default;
	Lexer(std::string str);
	Token GetNextToken();
private:
	
	int pos;    // 扫描仪的位置
	string strInput;// 获取字符串
};