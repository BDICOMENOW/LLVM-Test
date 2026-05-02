#include "CodeGen.h"

CodeGen::CodeGen() : builder(context) {
    // 创建一个模块，给编译器起名字
    module = std::make_unique<llvm::Module>("MyAwesomeComplier", context);
}

void CodeGen::Compile(ExprAst* root) {
    // 1.定义main函数的类型，返回32位整数，不带参数（false）
    llvm::FunctionType* functionType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    // 2.创建main函数，给它起名字
    llvm::Function* mainFunction = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", module.get());
    // 3.创建一个BasicBlock，给它起名字叫entry
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(context, "entry", mainFunction);
    // 4.设置当前BasicBlock的上下文环境（让builder接下来所有的指令都在entryBB块里）
    builder.SetInsertPoint(entryBB);
    // 5.让树的根节点接受翻译官的访问，翻译官遍历AST，拿到最终的结果（常量折叠后就是那个 7）
    llvm::Value* retVal = root->Accept(this);
    // 6.生成一条机器指令 return retVal;
    builder.CreateRet(retVal);

}

// 1.遇到数字，把他变成LLVM的32位整数指令
llvm::Value* CodeGen::VisitNumberExpr(NumberExprAst* expr) {
    int num = std::stoi(expr->val);
    return builder.getInt32(num);
}

// 2.遇到操作符，递归生成左右两边的代码，然后LLVM指令拼起来
llvm::Value* CodeGen::VisitBinaryExpr(BinaryExprAst* expr) {
    // 递归！先让翻译官把左右两边翻译
    llvm::Value* left = expr->left->Accept(this);
    llvm::Value* right = expr->right->Accept(this);
    if (!left || !right) {
        return nullptr;
    }
    // 根据符号，生成LLVM算数指令
    if (expr->op == "+") {
        return builder.CreateNSWAdd(left, right, "addtmp");
    } else if (expr->op == "-") {
        return builder.CreateNSWSub(left, right, "subtmp");
    } else if (expr->op == "*") {
        return builder.CreateNSWMul(left, right, "multmp");
    } else if (expr->op == "/") {
        return builder.CreateSDiv(left, right, "divtmp");
    }
    return nullptr;
}