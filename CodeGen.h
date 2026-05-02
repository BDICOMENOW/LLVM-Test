#pragma once
#include "ast.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h" // 禁用常量折叠特性

class CodeGen : public Visitor {
public:
    CodeGen();

    // 编译入口
    void Compile(ExprAst* root);


    // 具体工作说明书：怎么翻译数字，怎么翻译算式
    llvm::Value* VisitNumberExpr(NumberExprAst* expr) override;
    llvm::Value* VisitBinaryExpr(BinaryExprAst* expr) override;
    llvm::Module* GetModule() {
        return module.get();
    };
private:

    llvm::LLVMContext context;
    // llvm::IRBuilder<> builder;
    llvm::IRBuilder<llvm::NoFolder> builder;
    std::unique_ptr<llvm::Module> module;
};
