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
	TOKEN_LPAREN,	// 左括号 (
	TOKEN_RPAREN,	// 右括号 )
	TOKEN_IDENTIFIER, // 标识符( 变量名 )
	TOKEN_KW_INT,	// int关键字
	TOKEN_ASSIGN,	// 赋值符：=
	TOKEN_EQUAL_EQUAL, // 相等判断：==
	TOKEN_AMP_AMP,	// 逻辑与：&&
	TOKEN_PIPE_PIPE, // 逻辑或：||
	TOKEN_LESS,		// 小于符：<
	TOKEN_GREATER,	// 大于符：>
	TOKEN_PIPE,		// 管道符：|
	TOKEN_AMP,		// 与符：&
	TOKEN_CARET,	// 异或符：^
	TOKEN_LESS_LESS,	   // 左移符：<<
	TOKEN_GREATER_GREATER, // 右移符：>>
	TOKEN_SEMI,		// 分号
	TOKEN_COMMA,	// 逗号
	TOKEN_KW_IF,    // 新增：if 关键字
	TOKEN_KW_ELSE,  // 新增：else 关键字
	TOKEN_KW_FOR,	// 新增：for 关键字
	TOKEN_KW_BREAK,	// 新增：break 关键字
	TOKEN_KW_CONTINUE, // 新增：continue 关键字
	TOKEN_LBRACE,   // 新增：左大括号 {
	TOKEN_RBRACE,   // 新增：右大括号 }
	TOKEN_LBRACKET, // 新增：左中括号 [
    TOKEN_RBRACKET, // 新增：右中括号 ]
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