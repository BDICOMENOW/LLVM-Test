// complir_test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。


#include <iostream>
#include<ctype.h>
#include "Lexer.h"
#include"Parser.h"

int main()
{
    // 完成 Parser 生成
    string curPtr = "1 +  ";

    Parser parser(curPtr);


    /* Lexer lex(curPtr);
    std::vector<Token> vecTok;
    Token tokCur = lex.GetNextToken();
    while (tokCur.type != TOKEN_EOF) {
        if (tokCur.type == TOKEN_ERROR) {
            cout << "Get Input Token Error!" << endl;
            break;
        }
        vecTok.push_back(tokCur);
        tokCur = lex.GetNextToken();
    }
    
    for (auto tmp : vecTok) {
        cout << "type: " << tmp.type << " value: " << tmp.value << endl;
    }
    */
    return 0;
}
