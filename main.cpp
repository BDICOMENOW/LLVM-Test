// complir_test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。


#include <iostream>
#include <ctype.h>
#include"Parser.h"

void Test(const string& inputStr)
{
    Lexer lex(inputStr);
    Parser parser(lex);
    std::unique_ptr<ExprAst> firstNode = parser.ParseExpression();

    if (firstNode != nullptr) {
        cout << "构建了第一个数字节点" << endl;
        firstNode->Dump();
    }
    else {
        cout << "解析失败" << endl;
    }
}

int main()
{

    // 测试 1：最基础的算术
    Test("1 + 2 * 3");

    // 测试 2：带括号改变优先级的算术
    Test("(1 + 2) * 3");

    // 测试 3：综合测试
    Test("1 + 2 * 3 - 4 / 2");
    

    return 0;
}
