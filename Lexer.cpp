#include "Lexer.h"

Lexer::Lexer(std::string str)
{
	pos = 0;
    strInput = str;
}

// 有问题，后续再改，大致逻辑上是通的
Token Lexer::GetNextToken()
{
    Token tok;
    string strNum;
    // 判断空格
    while (pos >= strInput.size() && isspace(strInput[pos])) {
        pos++;
    }
    // 到字符串结尾（文件结尾）
    if (pos == strInput.size()) {
        tok.type = TOKEN_EOF;
        tok.value = "";
        return tok;
    }
    // 判断数字
    if (isdigit(strInput[pos])) {
        while (pos < strInput.size() && isdigit(strInput[pos])) {
            strNum += strInput[pos];
            pos++;
        }
        tok.type = TOKEN_NUMBER;
        tok.value = strNum;
    }
    else {
        switch (strInput[pos]) {
            
            // 判断加号
            case '+': {
                tok.type = TOKEN_PLUS;
                tok.value = "+";
                pos++;
                break;
            }
            // 判断减号
            case '-': {
                tok.type = TOKEN_MINUS;
                tok.value = "-";
                pos++;
                break;
            }
            // 判断乘号
            case '*': {
                tok.type = TOKEN_MUL;
                tok.value = "*";
                pos++;
                break;
            }
            // 判断除号
            case '/': {
                tok.type = TOKEN_DIV;
                tok.value = "/";
                pos++;
                break;
            }
            // 判断左括号
            case '(': {
                tok.type = TOKEN_LPAREN;
                tok.value = "(";
                pos++;
                break;
            }
            // 判断右括号
            case ')': {
                tok.type = TOKEN_RPAREN;
                tok.value = ")";
                pos++;
                break;
            }
            default:
                tok.type = TOKEN_ERROR;
                tok.value = "error";
                pos++;
                break;
        }
    }
    
    return tok;
}
