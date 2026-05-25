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
	TOKEN_IDENTIFIER, // 标识符
	TOKEN_KW_INT,	// int关键字
	TOKEN_ASSIGN,	// 赋值符：=
	TOKEN_SEMI,		// 分号
	TOKEN_COMMA,	// 逗号
	TOKEN_KW_IF,    // 新增：if 关键字
	TOKEN_KW_ELSE,  // 新增：else 关键字
	TOKEN_LBRACE,   // 新增：左大括号 {
	TOKEN_RBRACE,   // 新增：右大括号 }
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