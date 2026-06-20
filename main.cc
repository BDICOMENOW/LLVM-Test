// complir_test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。


#include <iostream>
#include <ctype.h>
#include "parser.h"
#include "CodeGen.h"

void Test(const string& inputStr)
{
    cout << "-----------------------------------" << endl;
    cout << "正在编译: \"" << inputStr << "\"" << endl;
    
    Lexer lex(inputStr);
    Parser parser(lex);
    auto nodes = parser.ParseProgram();

    if (!nodes.empty()) {
        cout << "  抽象语法树构建成功：" << endl;
        for (auto& node : nodes) node->Dump(); 
        
        cout << "\n  === 对应的 LLVM IR 机器代码 ===\n" << endl;
        CodeGen codegen;
        codegen.Compile(nodes); 
        codegen.GetModule()->print(llvm::outs(), nullptr);
        llvm::outs().flush();
        std::cout << endl;
    }
}

int main()
{
    // 验证多级指针套娃、加载值以及以前的代码！
    Test(R"(    
        int a = 10;
        int* p = &a;       
        int** pp = &p;     
        int*** ppp = &pp;  

        ***ppp = 100;      
    )");
    return 0;
}
