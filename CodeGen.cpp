#include "CodeGen.h"
#include <llvm/IR/Verifier.h>

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

    // 验证模块:安全检查
    llvm::verifyFunction(*mainFunction,&llvm::errs());

}

// 1.遇到数字，把他变成LLVM的32位整数指令
llvm::Value* CodeGen::VisitNumberExpr(NumberExprAst* expr) {
    int num = std::stoi(expr->val);
    return builder.getInt32(num);
}

// 2.遇到操作符，递归生成左右两边的代码，然后LLVM指令拼起来
llvm::Value* CodeGen::VisitBinaryExpr(BinaryExprAst* expr) {
    if (expr->op == "&&") {
        // 1.拿到main函数
        llvm::Function* theFunction = builder.GetInsertBlock()->getParent();
        // 2. 创建3个基本块
        llvm::BasicBlock* nextBB = llvm::BasicBlock::Create(context, "nextBB_and", theFunction);
        llvm::BasicBlock* falseBB = llvm::BasicBlock::Create(context, "falseBB_and", theFunction);
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "mergeBB_and", theFunction);
        // 3. 算左边 
        llvm::Value* leftVal = expr->left->Accept(this); // 仅解析左侧
        leftVal = builder.CreateICmpNE(leftVal, builder.getInt32(0), "ifcond");
        // 如果 A 是真，去 nextBB 算 B；如果 A 是假，直接短路飞向 falseBB
        builder.CreateCondBr(leftVal, nextBB, falseBB);
        // nextBB: 算右边
        builder.SetInsertPoint(nextBB);
        llvm::Value* rightVal = expr->right->Accept(this); // 递归算右边
        rightVal = builder.CreateICmpNE(rightVal, builder.getInt32(0)); // 转成 bool
        rightVal = builder.CreateZExt(rightVal, builder.getInt32Ty()); // 转成 i32
        // 短路飞向 mergeBB
        builder.CreateBr(mergeBB);
        // 因为算 B 的过程中可能产生了嵌套的新房间，我们要重新定位 nextBB 的真实出口在哪！
        nextBB = builder.GetInsertBlock();

        // falseBB: 短路飞向 mergeBB
        builder.SetInsertPoint(falseBB);
        builder.CreateBr(mergeBB);
        // mergeBB: 返回左右的逻辑与
        builder.SetInsertPoint(mergeBB);
        // 召唤 PHI 节点保安！(声明这是一个返回 i32 类型的 PHI，它有两个来源)
        llvm::PHINode* phi = builder.CreatePHI(builder.getInt32Ty(), 2, "and_res");
        // 告诉保安查监控的规则：
        phi->addIncoming(rightVal, nextBB);              // 如果从 nextBB 来，值为 B 的结果
        phi->addIncoming(builder.getInt32(0), falseBB);  // 如果从 falseBB 来，值为 0 (假)

        return phi; // 把保安手里拿到的最终值，交差！
    } else if (expr->op == "||") {
        // 1.拿到main函数
        llvm::Function* theFunction = builder.GetInsertBlock()->getParent();
        // 2. 创建3个基本块
        llvm::BasicBlock* trueBB = llvm::BasicBlock::Create(context, "trueBB_or", theFunction);
        llvm::BasicBlock* nextBB = llvm::BasicBlock::Create(context, "nextBB_or", theFunction);
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "mergeBB_or", theFunction);
        // 3. 算左边 
        llvm::Value* leftVal = expr->left->Accept(this); // 仅解析左侧
        leftVal = builder.CreateICmpNE(leftVal, builder.getInt32(0), "ifcond");
        // 如果 A 是真，去 trueBB, 如果 A 是假，直接短路飞向 nextBB
        builder.CreateCondBr(leftVal, trueBB, nextBB);
        // nextBB: 算右边
        builder.SetInsertPoint(nextBB);
        llvm::Value* rightVal = expr->right->Accept(this); // 递归算右边
        rightVal = builder.CreateICmpNE(rightVal, builder.getInt32(0)); // 转成 bool
        rightVal = builder.CreateZExt(rightVal, builder.getInt32Ty()); // 转成 i32
        // 短路飞向 mergeBB
        builder.CreateBr(mergeBB);
        // 因为算 B 的过程中可能产生了嵌套的新房间，我们要重新定位 nextBB 的真实出口在哪！
        nextBB = builder.GetInsertBlock();

        // trueBB: 短路飞向 mergeBB
        builder.SetInsertPoint(trueBB);
        builder.CreateBr(mergeBB);
        // mergeBB: 返回左右的逻辑或
        builder.SetInsertPoint(mergeBB);
        // 召唤 PHI 节点保安！(声明这是一个返回 i32 类型的 PHI，它有两个来源)
        llvm::PHINode* phi = builder.CreatePHI(builder.getInt32Ty(), 2, "or_res");
        // 告诉保安查监控的规则：
        phi->addIncoming(rightVal, nextBB);              // 如果从 nextBB 来，值为 B 的结果
        phi->addIncoming(builder.getInt32(1), trueBB);  // 如果从 trueBB 来，值为 1 (真)

        return phi; // 把保安手里拿到的最终值，交差！
    }

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
    } else if (expr->op == "<") {
        // CreateICmpSLT: Signed Less Than (有符号小于)
        llvm::Value* cmp = builder.CreateICmpSLT(left, right, "cmptmp");
        // 极其关键：LLVM的比较结果是 i1(1位布尔值)，我们要把它强转回 i32(32位整数) 保持类型统一！
        return builder.CreateIntCast(cmp, builder.getInt32Ty(), true, "casttmp");
    } else if (expr->op == ">") {
        // CreateICmpSGT: Signed Greater Than (有符号大于)
        llvm::Value* cmp = builder.CreateICmpSGT(left, right, "cmptmp");
        return builder.CreateIntCast(cmp, builder.getInt32Ty(), true, "casttmp");
    } else if (expr->op == "<<") {
        return builder.CreateShl(left, right, "shltmp");
    } else if (expr->op == ">>") {
        return builder.CreateAShr(left, right, "ashrtmp");
    } else if (expr->op == "&") {
        return builder.CreateAnd(left, right, "andtmp");
    } else if (expr->op == "|") {
        return builder.CreateOr(left, right, "ortmp");
    } else if (expr->op == "^") {
        return builder.CreateXor(left, right, "xortmp");
    } else if (expr->op == "==") {
        // CreateICmpEQ: Equal (等于)
        llvm::Value* cmp = builder.CreateICmpEQ(left, right, "cmptmp");
        return builder.CreateIntCast(cmp, builder.getInt32Ty(), true, "casttmp");
    } 
    return nullptr;
}

llvm::Value* CodeGen::VisitVariableDecl(VariableDeclAst* expr) {
    // 生成Alloca指令：在栈上分配32位整数（i32）空间,命名为expr->name

    llvm::Value* alloca = builder.CreateAlloca(expr->type->ToLLVMType(context), nullptr, expr->name);
    // 记录在 NamedValues 中
    NamedValues[expr->name] = alloca;
    return alloca;

}
    


llvm::Value* CodeGen::VisitVariableAccess(VariableAccessAst* expr) {
    // 从NamedValues（账本）里查到变量的坑位地址
    llvm::Value* varAddr = NamedValues[expr->name];
    if (!varAddr) {
        std::cerr << "致命错误：使用了未定义的变量 " << expr->name << std::endl;
        return nullptr;
    }
    if (expr->isLValue) {
        // （比如等号左边）物理地址交上去！
        return varAddr; 
    } else {
        // （比如等号右边），返回值！, 动态获取这个坑位当初挖的时候是什么类型 (getAllocatedType)！
        llvm::Type* allocTy = llvm::cast<llvm::AllocaInst>(varAddr)->getAllocatedType();
        return builder.CreateLoad(allocTy, varAddr, expr->name);
    }
}

llvm::Value* CodeGen::VisitAssignExpr(AssignExprAst* expr) {
    // 1. 递归算出等号右边的数值
    llvm::Value* rhs = expr->right->Accept(this);
    
    // 2. 拿到左边的坑位地址
    llvm::Value* lhsAddr = expr->left->Accept(this);
    if (!lhsAddr) return nullptr;
    
    // 3. 把泥土砸进坑里！
    builder.CreateStore(rhs, lhsAddr);
    
    return rhs; // 赋值表达式的结果就是赋过去的值
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
    if (!condVal) return nullptr;
    // 2. 拿到我们当前正在里面铺铁轨的函数 (也就是 main 函数)
    llvm::Function* theFunction = builder.GetInsertBlock()->getParent();

    // 3. 强行劈开三块内存空间 (创建三个基本块)
    // 注意：在创建的那一瞬间，它们就被挂载到 theFunction 的末尾了
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context, "then_branch", theFunction);
    llvm::BasicBlock* elseBB = expr->elseNode ? llvm::BasicBlock::Create(context, "else_branch", theFunction) : nullptr;
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "ifcont_branch", theFunction);

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


llvm::Value* CodeGen::VisitForStmt(ForStmtAst* expr) {
    // 0. 执行初始化 (例如 i = 0)
    if (expr->initNode) {
        expr->initNode->Accept(this);
    }

    // 1. 拿到当前的 main 函数
    llvm::Function* theFunction = builder.GetInsertBlock()->getParent();

    // 2. 劈开 4 个物理房间 (init 刚才在外面执行了，所以这里建 4 个)
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(context, "for.cond", theFunction);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(context, "for.body", theFunction);
    llvm::BasicBlock* incBB  = llvm::BasicBlock::Create(context, "for.inc", theFunction);
    llvm::BasicBlock* lastBB = llvm::BasicBlock::Create(context, "for.last", theFunction);

    // 3. 核心大招：泥瓦匠开始登记账本！
    // 记下：这个 For 图纸，它的越狱房间是 lastBB，跃迁房间是 incBB
    breakBBs[expr] = lastBB;
    continueBBs[expr] = incBB;

    // 4. 从当前位置，铺一条毫无保留的铁轨，直接开进 condBB (条件判断房间)
    builder.CreateBr(condBB);

    // ==========================================
    // 房间 A：修建 condBB (条件判断)
    // ==========================================
    builder.SetInsertPoint(condBB);
    if (expr->condNode) {
        llvm::Value* condVal = expr->condNode->Accept(this);
        // 如果条件解析失败，直接熔断，防止后续崩溃！
        if (!condVal) return nullptr;
        // 判断条件是否为真 (!= 0)
        condVal = builder.CreateICmpNE(condVal, builder.getInt32(0), "forcond");
        // 道岔：真就进 bodyBB，假就跳出循环进 lastBB
        builder.CreateCondBr(condVal, bodyBB, lastBB);
    } else {
        // 如果没有条件 (死循环 for(;;))，直接进 bodyBB
        builder.CreateBr(bodyBB);
    }

    // ==========================================
    // 房间 B：修建 bodyBB (循环体)
    // ==========================================
    builder.SetInsertPoint(bodyBB);
    if (expr->bodyNode) {
        expr->bodyNode->Accept(this); // 递归生成里面的机器码！(里面如果遇到break，就会去查账本)
    }
    // 循环体跑完，强制铺铁轨去 incBB (步进房间)
    builder.CreateBr(incBB);

    // ==========================================
    // 房间 C：修建 incBB (步进房间)
    // ==========================================
    builder.SetInsertPoint(incBB);
    if (expr->incNode) {
        expr->incNode->Accept(this); // 比如执行 i = i + 1
    }
    // 步进完，强制倒车开回 condBB！
    builder.CreateBr(condBB);

    // 5. 循环造完了，打扫战场，把这本账销毁
    breakBBs.erase(expr);
    continueBBs.erase(expr);

    // ==========================================
    // 房间 D：修建 lastBB (汇合大厅)
    // ==========================================
    // 把泥瓦匠传送到最后的大厅，准备接续后面的代码
    builder.SetInsertPoint(lastBB);

    return nullptr;
}


llvm::Value* CodeGen::VisitBreakStmt(BreakStmtAst* expr) {
    // 1. 认亲：从图纸上拿到亲爹 (ForStmtAst)
    ExprAst* targetFor = expr->target;
    
    // 2. 查账：从泥瓦匠的 breakBBs 账本里，查到该去哪个物理房间
    llvm::BasicBlock* targetBB = breakBBs[targetFor];

    // 3. 造门：砸下这扇越狱传送门
    builder.CreateBr(targetBB);

    // 4. 死亡垃圾桶机制：防止死神安检员 (Verifier) 暴走
    llvm::Function* theFunction = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* deadBB = llvm::BasicBlock::Create(context, "break.death", theFunction);
    builder.SetInsertPoint(deadBB); // 把泥瓦匠扔进垃圾桶，后面的废话代码全部砸在这里面

    return nullptr;
}


llvm::Value* CodeGen::VisitContinueStmt(ContinueStmtAst* expr) {
    // 1. 认亲：拿到亲爹
    ExprAst* targetFor = expr->target;
    
    // 2. 查账：从 continueBBs 账本里，查到步进房间 incBB
    llvm::BasicBlock* targetBB = continueBBs[targetFor];

    // 3. 造门：砸下这扇跃迁传送门
    builder.CreateBr(targetBB);

    // 4. 死亡垃圾桶机制
    llvm::Function* theFunction = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* deadBB = llvm::BasicBlock::Create(context, "continue.death", theFunction);
    builder.SetInsertPoint(deadBB);

    return nullptr;
}

llvm::Value* CodeGen::VisitUnaryExpr(UnaryExprAst* expr) 
{
    switch(expr->op) {
        // 取地址 &
        case UnaryOp::addr:{
            expr->node->isLValue = true;
            llvm::Value* ptrVal = expr->node->Accept(this);
            return ptrVal;
        }
        // 解引用 *
        case UnaryOp::deref:{
            expr->node->isLValue = false;
            llvm::Value* val = expr->node->Accept(this);
            if(expr->isLValue) {
                return val;
            } else {
                return builder.CreateLoad(llvm::PointerType::get(context, 0), val, "dereftmp");
            }
        }
        default:
            return nullptr;
    }
    std::cerr << "致命错误：遇到未知的一元操作符！" << std::endl;
    return nullptr;
}

llvm::Value* CodeGen::VisitArrayAccess(ArrayAccessAst* expr) {
    // 1. 找到基地址
    llvm::Value* varAddr = NamedValues[expr->arrayName];
    if (!varAddr) {
        std::cout << "致命错误：阵列 " << expr->arrayName << " 未定义！" << std::endl;
        return nullptr;
    }

    // 2. 查图纸：确认这块地皮当初是怎么批的（拿到 [10 x i32] 图纸）
    llvm::Type* allocTy = llvm::cast<llvm::AllocaInst>(varAddr)->getAllocatedType();

    // 3. 执行偏移量计算（算出中括号里的算式，比如 i+1 的结果）
    llvm::Value* indexVal = expr->indexExpr->Accept(this);
    if (!indexVal) return nullptr;

    // =======================================================
    // 4. GEP (GetElementPtr) 算坐标！
    // LLVM 的硬性规定：对于 Alloca 出来的数组，必须传两个参数：
    // 参数 0：表示“推开大门，进入这块地皮本身”。(builder.getInt32(0))
    // 参数 1：表示“进去之后，往前走几个坑”。(indexVal)
    // =======================================================
    std::vector<llvm::Value*> indices;
    indices.push_back(builder.getInt32(0)); 
    indices.push_back(indexVal);
    
    // 启动 GPS 定位，算出精确的坑位物理地址！
    llvm::Value* elementPtr = builder.CreateGEP(allocTy, varAddr, indices, "gep_tmp");

    // 5. 选择：要地址？还是要里面的值？
    if (expr->isLValue) {
        // 如果它在等号左边 (例如 arr[2] = 99)，它需要的是坑的物理地址！
        return elementPtr; 
    } else {
        // 如果它在等号右边 (例如 x = arr[2])，我们需要用铁锹把值挖出来！
        // 铁锹需要知道单个元素的大小，直接拿即可：getArrayElementType()
        llvm::Type* elementType = allocTy->getArrayElementType();
        return builder.CreateLoad(elementType, elementPtr, "load_tmp");
    }
}