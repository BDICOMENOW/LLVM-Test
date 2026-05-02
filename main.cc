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
    std::unique_ptr<ExprAst> rootNode = parser.ParseExpression();

    if (rootNode != nullptr) {
        cout << "✅ 抽象语法树构建成功：" << endl;
        rootNode->Dump(); 
        
        cout << "\n🚀 === 对应的 LLVM IR 机器代码 ===" << endl;
        // 召唤翻译官！
        CodeGen codegen;
        // 调用我们刚刚写好的高阶编译入口
        // 它会在 LLVM 里建好 main 函数和基本块，防止段错误
        codegen.Compile(rootNode.get());
        codegen.GetModule()->print(llvm::outs(), nullptr);
        llvm::outs().flush();
        std::cout<<endl;
    } else {
        cout << "解析失败" << endl;
    }
}

int main()
{

    // 测试 1：最基础的算术
    Test("1 + 2 * 3");

    // 测试 2：带括号改变优先级的算术
    // Test("(1 + 2) * 3");

    // 测试 3：综合测试
    // Test("1 + 2 * 3 - 4 / 2");
    

    return 0;
}
