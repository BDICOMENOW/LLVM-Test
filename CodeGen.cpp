#include "CodeGen.h"

CodeGen::CodeGen() : builder(context) {
    // 创建一个模块，给编译器起名字
    module = std::make_unique<llvm::Module>("MyAwesomeComplier", context);
}

void CodeGen::Compile(const std::vector<std::unique_ptr<ExprAst>>& nodes) {
    // 1.定义main函数的类型，返回32位整数，不带参数（false）
    llvm::FunctionType* functionType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    // 2.创建main函数，给它起名字
    llvm::Function* mainFunction = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", module.get());
    // 3.创建一个BasicBlock，给它起名字叫entry
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(context, "entry", mainFunction);
    // 4.设置当前BasicBlock的上下文环境（让builder接下来所有的指令都在entryBB块里）
    builder.SetInsertPoint(entryBB);
    
    llvm::Value* lastVal = nullptr;
    // 循环遍历每一行代码
    for(auto &node : nodes) {
        lastVal = node->Accept(this);
    }
    // 只有当 lastVal 存在，并且它的类型真的是 32位整数 (i32) 时，才返回它
    if(lastVal && lastVal->getType()->isIntegerTy(32)) {
        builder.CreateRet(lastVal);
    } else {
        builder.CreateRet(builder.getInt32(0));
    }

    /* 
    让树的根节点接受翻译官的访问，翻译官遍历AST，拿到最终的结果（常量折叠后就是那个 7）
    //llvm::Value* retVal = root->Accept(this);
    // 生成一条机器指令 return retVal;
    //builder.CreateRet(retVal);
    */

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
    } else if (expr->op == "%") {
        return builder.CreateSRem(left, right, "modtmp");
    }
    return nullptr;
}

llvm::Value* CodeGen::VisitVariableDecl(VariableDeclAst* expr) {
    // 生成Alloca指令：在栈上分配32位整数（i32）空间,命名为expr->name
    llvm::Value* alloca = builder.CreateAlloca(builder.getInt32Ty(), nullptr, expr->name);
    // 记录在 NamedValues 中
    NamedValues[expr->name] = alloca;
    return alloca;

}
    

llvm::Value* CodeGen::VisitVariableAccess(VariableAccessAst* expr) {
    // 从NamedValues（账本）里查到变量的坑位地址
    llvm::Value* varAddr = NamedValues[expr->name];
    if (!varAddr) {
        return nullptr;
    }
    return builder.CreateLoad(builder.getInt32Ty(), varAddr, expr->name);
    
}

llvm::Value* CodeGen::VisitAssignExpr(AssignExprAst* expr) {
    // 1. 递归算出等号右边的数值
    llvm::Value* rightVal = expr->right->Accept(this);
    
    // 2. 拿到等号左边的变量名（强转一下拿到名字）
    VariableAccessAst* lhs = static_cast<VariableAccessAst*>(expr->left.get());
    
    // 3. 从账本里查到这个变量的坑位地址
    llvm::Value* varAddr = NamedValues[lhs->name];
    if (!varAddr) return nullptr;
    
    // 4. 生成 Store 指令：把右边的数字，死死地塞进左边的内存坑位里！
    builder.CreateStore(rightVal, varAddr);
    return rightVal;
}

llvm::Value* CodeGen::VisitBlockStmt(BlockStmtAst* expr) {
    llvm::Value* lastVal = nullptr;
    // 遍历大括号里的每一张图纸，依次交给泥瓦匠生成机器码
    for (auto& stmt : expr->stmtVec) {
        lastVal = stmt->Accept(this);
    }
    // 返回最后一条语句的结果
    return lastVal;
}

llvm::Value* CodeGen::VisitIfStmt(IfStmtAst* expr) {
    // 1. 让泥瓦匠先去算一算条件的值，比如 (b != 0)
    llvm::Value* condVal = expr->condNode->Accept(this);
    if (!condVal) return nullptr;

    // 因为你的算式算出来是 i32 (32位整数)，但 if 判断需要 i1 (布尔值真/假)
    // 所以强制加一条比较指令：判断条件的值是不是 不等于 0 (NE: Not Equal)
    condVal = builder.CreateICmpNE(condVal, builder.getInt32(0), "ifcond");

    // 2. 拿到我们当前正在里面铺铁轨的函数 (也就是 main 函数)
    llvm::Function* theFunction = builder.GetInsertBlock()->getParent();

    // 3. 强行劈开三块内存空间 (创建三个基本块)
    // 注意：在创建的那一瞬间，它们就被挂载到 theFunction 的末尾了
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context, "then", theFunction);
    llvm::BasicBlock* elseBB = expr->elseNode ? llvm::BasicBlock::Create(context, "else", theFunction) : nullptr;
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "ifcont", theFunction);

    // 4. 铺设道岔 (分支跳转指令)
    // 告诉 CPU：如果 condVal 为真，跳去 thenBB；否则跳去 elseBB（如果没有 else，直接跳去汇合点）
    if (expr->elseNode) {
        builder.CreateCondBr(condVal, thenBB, elseBB);
    } else {
        builder.CreateCondBr(condVal, thenBB, mergeBB);
    }

    // 5. 修建 thenBB 铁轨
    builder.SetInsertPoint(thenBB); // 把泥瓦匠传送到 then 块
    expr->thenNode->Accept(this);   // 生成 if 里面的机器码
    builder.CreateBr(mergeBB);      // 铁轨铺完，强制跳往汇合点

    // 6. 修建 elseBB 铁轨 (如果有的话)
    if (expr->elseNode) {
        builder.SetInsertPoint(elseBB); // 传送泥瓦匠
        expr->elseNode->Accept(this);
        builder.CreateBr(mergeBB);      // 强制跳往汇合点
    }

    // 7. 泥瓦匠来到汇合点，准备接续后面的代码
    builder.SetInsertPoint(mergeBB);

    // 因为 if 语句本身不产生可以直接拿来加减乘除的数值，所以返回空
    return nullptr; 
}
