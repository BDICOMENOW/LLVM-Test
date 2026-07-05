#include "Lexer.h"
#include <cctype>

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
    while (pos < strInput.size() && isspace(strInput[pos])) {
        pos++;
    }
    // 到字符串结尾（文件结尾）
    if (pos >= strInput.size()) {
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
      // 判断字母 （用于解析标识符（变量）或关键字 int）
    } else if(isalpha(strInput[pos]) || strInput[pos] == '_') {
        string strId = "";
        // 只要是字母，数字，下划线，就是合法标识符
        while (pos < strInput.size() && (isalnum(strInput[pos]) || strInput[pos] == '_')) {
            strId += strInput[pos];
            pos++;
        }
        if(strId == "int") {
            tok.type = TOKEN_KW_INT;
        }else if (strId == "if") {      // 新增：认出 if
            tok.type = TOKEN_KW_IF;
        } else if (strId == "else") {    // 新增：认出 else
            tok.type = TOKEN_KW_ELSE;
        } else if (strId == "for") {          // <-- [新增] FOR
            tok.type = TOKEN_KW_FOR;
        } else if (strId == "break") {        // <-- [新增] BREAK
            tok.type = TOKEN_KW_BREAK;
        } else if (strId == "continue") {     // <-- [新增] CONTINUE
            tok.type = TOKEN_KW_CONTINUE;
        } else {
            tok.type = TOKEN_IDENTIFIER;
        }
        tok.value = strId;
    } // 判断符号
    else {
        switch (strInput[pos]) {
            
            // 判断加号
            case '+': {
                // 这里是为了兼容 C 语言中的 ++
                if( pos + 1 < strInput.size() && strInput[pos + 1] == '+') {
                    tok.type = TOKEN_PLUS_PLUS;
                    tok.value = "++";
                    pos += 2;
                }
                // 这里是为了兼容 C 语言中的 += 
                else if(pos + 1 < strInput.size() && strInput[pos + 1] == '=') {
                    tok.type = TOKEN_PLUS_EQUAL;
                    tok.value = "+=";
                    pos += 2;
                }
                // + 
                else {
                    tok.type = TOKEN_PLUS;
                    tok.value = "+";
                    pos++;
                }
                break;
            }
            // 判断减号
            case '-': {
                // 这里是为了兼容 C 语言中的 --
                if( pos + 1 < strInput.size() && strInput[pos + 1] == '-') {
                    tok.type = TOKEN_MINUS_MINUS;
                    tok.value = "--";
                    pos += 2;
                }
                // 这里是为了兼容 C 语言中的 -= 
                else if(pos + 1 < strInput.size() && strInput[pos + 1] == '=') {
                    tok.type = TOKEN_MINUS_EQUAL;
                    tok.value = "-=";
                    pos += 2;
                }
                else {
                    tok.type = TOKEN_MINUS;
                    tok.value = "-";
                    pos++;
                }
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
            // 判断取模
            case '%': {
                tok.type = TOKEN_MOD;
                tok.value = "%";
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
            // 判断左中括号
            case '[': {
                tok.type = TOKEN_LBRACKET;
                tok.value = "[";
                pos++;
                break;
            }
            // 判断右中括号
            case ']': {
                tok.type = TOKEN_RBRACKET;
                tok.value = "]";
                pos++;
                break;
            }
            // 判断赋值符号 = 或者 比较符号 ==
            case '=': {
                // 探头往后多看一眼，如果是 ==
                if (pos + 1 < strInput.size() && strInput[pos + 1] == '=') {
                    tok.type = TOKEN_EQUAL_EQUAL;
                    tok.value = "==";
                    pos += 2; // 一次性吃掉两个字符
                } else {
                    tok.type = TOKEN_ASSIGN;
                    tok.value = "=";
                    pos++;
                }
                break;
            }
            // 判断管道号 |
            case '|': {
                // 探头往后多看一眼，如果是 ||
                if (pos + 1 < strInput.size() && strInput[pos + 1] == '|') {
                    tok.type = TOKEN_PIPE_PIPE;
                    tok.value = "||";
                    pos += 2;
                } else {
                    tok.type = TOKEN_PIPE;
                    tok.value = "|";
                    pos++;
                }
                break;
            }
            // 判断与号 &
            case '&': {
                // 探头往后多看一眼，如果是 &&
                if (pos + 1 < strInput.size() && strInput[pos + 1] == '&') {
                    tok.type = TOKEN_AMP_AMP;
                    tok.value = "&&";
                    pos += 2;
                } else {
                    tok.type = TOKEN_AMP;
                    tok.value = "&";
                    pos++;
                }
                break;
            }
            // 判断异或 ^
            case '^': {
                tok.type = TOKEN_CARET;
                tok.value = "^";
                pos++;
                break;
            }
            // 判断小于号 <
            case '<': {
                // 探头往后多看一眼，如果是 <<
                if (pos + 1 < strInput.size() && strInput[pos + 1] == '<') {
                    tok.type = TOKEN_LESS_LESS;
                    tok.value = "<<";
                    pos += 2;
                } else {
                    tok.type = TOKEN_LESS;
                    tok.value = "<";
                    pos++;
                }
                break;
            }
            // 判断大于号 >
            case '>':{
                // 探头往后多看一眼，如果是 >>
                if (pos + 1 < strInput.size() && strInput[pos + 1] == '>') {
                    tok.type = TOKEN_GREATER_GREATER;
                    tok.value = ">>";
                    pos += 2;
                } else {
                    tok.type = TOKEN_GREATER;
                    tok.value = ">";
                    pos++;
                }
                break;
            }
            // 判断分号
            case ';': {
                tok.type = TOKEN_SEMI;
                tok.value = ";";
                pos++;
                break;
            }
            // 判断逗号
            case ',': {
                tok.type = TOKEN_COMMA;
                tok.value = ",";
                pos++;
                break;
            }
            // 判断左大括号
            case '{':{
                tok.type = TOKEN_LBRACE;
                tok.value = "{";
                pos++;
                break;
            }
            // 判断右大括号
            case '}':{
                tok.type = TOKEN_RBRACE;
                tok.value = "}";
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
