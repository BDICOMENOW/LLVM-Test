#pragma once
#include "ast.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h" // 禁用常量折叠特性
#include <map>
#include <vector>

class CodeGen : public Visitor {
public:
    CodeGen();

    // 编译入口
    void Compile(const std::vector<std::unique_ptr<ExprAst>>& nodes);


    // 具体工作说明书：怎么翻译数字，怎么翻译算式
    llvm::Value* VisitNumberExpr(NumberExprAst* expr) override;
    llvm::Value* VisitBinaryExpr(BinaryExprAst* expr) override;
    llvm::Value* VisitVariableDecl(VariableDeclAst* expr) override;
    llvm::Value* VisitVariableAccess(VariableAccessAst* expr) override;
    llvm::Value* VisitAssignExpr(AssignExprAst* expr) override;
    llvm::Value* VisitBlockStmt(BlockStmtAst* expr) override;
    llvm::Value* VisitIfStmt(IfStmtAst* expr) override;
    llvm::Value* VisitForStmt(ForStmtAst* expr) override;
    llvm::Value* VisitBreakStmt(BreakStmtAst* expr) override;
    llvm::Value* VisitContinueStmt(ContinueStmtAst* expr) override;

    llvm::Module* GetModule() {
        return module.get();
    };
private:

    llvm::LLVMContext context;
    // llvm::IRBuilder<> builder;
    llvm::IRBuilder<llvm::NoFolder> builder;
    std::unique_ptr<llvm::Module> module;

    // 记录变量名->内存地址
    std::map<std::string, llvm::Value*> NamedValues;
    // 两个寻路账本
    // 键：For 节点的图纸指针； 值：真实的物理房间 (BasicBlock)
    std::map<ExprAst*, llvm::BasicBlock*> breakBBs;
    std::map<ExprAst*, llvm::BasicBlock*> continueBBs;
};
