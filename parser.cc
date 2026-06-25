#include "parser.h"
#include <iostream>

Parser::Parser(Lexer lex)
{
	lexer = lex;
	tok = lexer.GetNextToken();
}

Parser::~Parser()
{
}

// 前进拿Token
void Parser::Advance()
{
	tok = lexer.GetNextToken();
}

// 判断是否为我想要的类型
bool Parser::Expect(TokenType tokenType)
{
	return tok.type == tokenType;
}

// 判断如果是我想要的类型就前进，如果不是返回false
bool Parser::Consume(TokenType tokenType)
{
	if (Expect(tokenType)) {
		Advance();
		return true;
	}
	return false;
}

int Parser::GetTokPrecedence()
{
	switch (tok.type) {
		case TOKEN_ASSIGN:{	// '='
			return 1;
		}
		case TOKEN_PIPE_PIPE:{	// '||'
			return 2;
		}
		case TOKEN_AMP_AMP: {	// '&&'
			return 3;
		}
		case TOKEN_PIPE: 	// '|'
		case TOKEN_AMP: 	// '&'
		case TOKEN_CARET: {	// '^'
			return 7;
		}
		case TOKEN_EQUAL_EQUAL: {	// '=='
			return 9;
		}
		case TOKEN_LESS:	// '<'
		case TOKEN_GREATER:	{// '>'
			return 12;
		}

		case TOKEN_LESS_LESS:	// '<<'
		case TOKEN_GREATER_GREATER: {	// '>>'
			return 15;
		}
		case TOKEN_PLUS:	// '+'
		case TOKEN_MINUS: {	// '-'
			return 18;
		}
		case TOKEN_MUL:		// '*'
		case TOKEN_DIV:		// '/'
		case TOKEN_MOD: {	// '%'
			return 22;
		}
		default:
			return -1;	// 不是二元操作符，返回-1
	}
	return 0;
}

// 核心算法：算符优先解析（操作符优先级查表法）
std::unique_ptr<ExprAst> Parser::ParseBinOpRhs(int exprPrec, std::unique_ptr<ExprAst> lHs)
{
	while (true) {
		// 1.获取当前操作符的战斗力
		int tokPrec = GetTokPrecedence();
		// 2.如果当前操作符的战斗力 < 我们设定的门槛，说明他该退场了
		//  （比如当前是 0 或者 -1，直接原封不动返回 LHS）
		if (tokPrec < exprPrec) {
			return lHs;
		}
		// 3.稳了，这个是一个二元操作符！存下它，然后让词法分析器吃掉它
		std::string binOp = tok.value;
		Advance();
		// 4.解析操作符右边的基础表达式
		auto rHs = ParsePrimary();
		if (!rHs) return nullptr;
		// 5.往后偷看下一个操作符的战斗力是多少？
		int nextPrec = GetTokPrecedence();
		// 6.如果当前战斗力 < 下一个操作符的战斗力（比如当前是 +，后面是 *）
		// 那么rHs就要被后面的高阶操作符抢走！递归调用，门槛设为tokPrec + 1
		if (tokPrec < nextPrec || (binOp == "=" && tokPrec == nextPrec)) {
			rHs = ParseBinOpRhs(tokPrec + 1, std::move(rHs));
			if (!rHs) return nullptr;
		}

		// 如果是等号，打包成赋值表达式
		if (binOp == "=") {
			lHs->isLValue = true; // 向左侧节点下达指令给我物理地址
			lHs = std::make_unique<AssignExprAst>(std::move(lHs), std::move(rHs));
		} else {
			// 如果不是等号，打包成二元表达式
			lHs = std::make_unique<BinaryExprAst>(binOp, std::move(lHs), std::move(rHs));
		}
	}
}

// 任务处理分发
std::unique_ptr<ExprAst> Parser::ParsePrimary()
{
	switch (tok.type) {
		case TOKEN_NUMBER: {
			// 如果是数字
			return ParserNumberExpr();
		}
		case TOKEN_LPAREN: {
			// 如果是左小括号，可能是小括号表达式
			return ParseParenExpr();
		}
		case TOKEN_IDENTIFIER: {
			// 如果是标识符，可能是变量或者赋值
			return ParseIdentifierExpr();
		}
		case TOKEN_KW_IF: {
			// if 语句
			return ParseIfStmt();
		}
		case TOKEN_LBRACE: {
			// 如果是左大括号，可能是大括号表达式
			return ParseBlockStmt();
		}
		case TOKEN_KW_FOR: {
			// for 语句
			return ParseForStmt();
		}
		case TOKEN_KW_BREAK: {
			// break 语句
			return ParseBreakStmt();
		}
		case TOKEN_KW_CONTINUE: {
			// continue 语句
			return ParseContinueStmt();
		}
		case TOKEN_AMP:	// &
		case TOKEN_MUL: //	*
		case TOKEN_MINUS: // -
        case TOKEN_PLUS:  // +
		{
			// 指针操作：解引用或者取地址
			return ParseUnaryExpr();
		}
		default: {
			cout << "语法错误，期望一个数字或左括号" << endl;
			return nullptr;
		}
	}
	return nullptr;
}

std::unique_ptr<ExprAst> Parser::ParseParenExpr()
{
	// 前进一个消耗掉左括号（
	Advance();

	auto expr = ParseExpression();
	if (!expr) return nullptr;

	// 解析完后必须接着一个右括号）
	if (!Consume(TOKEN_RPAREN)) {
		cout << "语法错误，期望')'" << endl;
		return nullptr;
	}
	return expr;
}

std::unique_ptr<ExprAst> Parser::ParseExpression()
{
	// 一切的开始: 先解析最左边的基础表达式
	auto lHs = ParsePrimary();
	if (!lHs)	return nullptr;

	// 把左子树交给二元操作符解析器，初始战斗力门槛设为 0 
	return ParseBinOpRhs(0, std::move(lHs));
}

std::unique_ptr<ExprAst> Parser::ParserNumberExpr()
{
	// 严谨起见：如果你调这个函数，说明你确信当前是一个数字
	if (tok.type != TOKEN_NUMBER) {
		std::cout << "Error: Expected a number!" << std::endl;
		return nullptr;
	}
	
	string numVal = tok.value;

	// 包装成Ast节点
	auto result = std::make_unique<NumberExprAst>(numVal);

	// 吃掉这个Token,前进
	Advance();

	return result;
}

std::unique_ptr<ExprAst> Parser::ParseIdentifierExpr()
{
	std::string varName = tok.value;
	Advance(); // 吃掉变量名

	if(!sema.CheckVariableUse(varName)) {
		return nullptr;
	}

	// 只返回变量节点
	return std::make_unique<VariableAccessAst>(varName);
	
}

std::shared_ptr<CType> Parser::ParseType(){

	std::shared_ptr<CType> type = nullptr;
	if(tok.type == TOKEN_KW_INT) {
		type = std::make_shared<IntType>();
		Advance();
	} else {
		return nullptr;
	}
	while(tok.type == TOKEN_MUL)
	{
		type = std::make_shared<PointerType>(type);
		Advance();
	}
	return type;
}

std::vector<std::unique_ptr<ExprAst>> Parser::ParseProgram() {

    std::vector<std::unique_ptr<ExprAst>> exprList;

    while (tok.type != TOKEN_EOF) {
        if (tok.type == TOKEN_SEMI) {
            Advance();
            continue;
        }

        // 1. 解析一下类型！
        std::shared_ptr<CType> declType = ParseType();

        // 2. 解析变量声明！
        if (declType != nullptr) {
            // 此时 ParseType 内部已经吃掉了 int 和 所有的 * 号！
            // 所以现在的 tok.type 恰好就是变量名！(比如 p)
            while (tok.type != TOKEN_SEMI) {
                if (tok.type == TOKEN_COMMA) Advance(); 
                std::string varName = tok.value;
                Advance(); // 吃掉变量名

				// 数组拦截器：吃掉变量名后，看看有没有 '['
				std::shared_ptr<CType> actualType = declType; // 默认拿着基础图                
                if (tok.type == TOKEN_LBRACKET) {
                    Advance(); // 吃掉 '['
                    if (tok.type != TOKEN_NUMBER) {
                        std::cout << "语法错误：数组大小必须是明确的数字！" << std::endl;
                        break;
                    }
                    int arrSize = std::stoi(tok.value); // 把字符串 "10" 转成整数 10
                    Advance(); // 吃掉数字
                    Consume(TOKEN_RBRACKET); // 吃掉 ']'
                    
                    // 把基础图纸包进数组图纸里！
                    actualType = std::make_shared<ArrayType>(actualType, arrSize);
                }

                // 3. 检查名字，类型登记
                if (!sema.CheckVariableDecl(varName, actualType)) break;

                // 4. 组装静态类型的声明
                exprList.push_back(std::make_unique<VariableDeclAst>(actualType, varName));
				// 处理等号 '=' 后面的表达式
                if(tok.type == TOKEN_ASSIGN) {
					Advance();
                    auto rhs = ParseExpression();
                    auto lhsNode = std::make_unique<VariableAccessAst>(varName);
                    lhsNode->isLValue = true; // 赋值时标记为左值
                    exprList.push_back(std::make_unique<AssignExprAst>(std::move(lhsNode), std::move(rhs)));
                }
            }
            Consume(TOKEN_SEMI); 
        } 
		// 拦截大括号
		else if(tok.type == TOKEN_LBRACE) {
			auto blockNode = ParseBlockStmt();
            if (blockNode) exprList.push_back(std::move(blockNode));
		}
        // =======================================
        // 如果不是声明，那就是普通表达式 (例如: *p = 20; 或 a + b;)
        // =======================================
        else {
            auto expr = ParseExpression();
            if (expr) {
                exprList.push_back(std::move(expr));
            }
            Consume(TOKEN_SEMI);
        }
    }

    return exprList;

}


std::unique_ptr<ExprAst> Parser::ParseBlockStmt() {
    auto blockNode = std::make_unique<BlockStmtAst>();

	sema.EnterScope();
    
    // 1. 进门：吃掉 '{'
    if (tok.type == TOKEN_LBRACE) {
        Advance(); 
    }

    // 2. 疯狂解析里面的语句，直到遇到 '}'
    while (tok.type != TOKEN_RBRACE && tok.type != TOKEN_EOF) {
        
        // 在大括号里获取类型
        std::shared_ptr<CType> declType = ParseType(); 
        
        if (declType != nullptr) {
            // 拿到类型了，说明这是局部变量声明 (如 int a = 2;)
            while (tok.type != TOKEN_SEMI) {
                if (tok.type == TOKEN_COMMA) Advance();
                
                std::string varName = tok.value;
                Advance(); // 吃掉变量名

				// 数组拦截器：吃掉变量名后，看看有没有 '['
				std::shared_ptr<CType> actualType = declType; // 默认拿着基础图                
                if (tok.type == TOKEN_LBRACKET) {
                    Advance(); // 吃掉 '['
                    if (tok.type != TOKEN_NUMBER) {
                        std::cout << "语法错误：数组大小必须是明确的数字！" << std::endl;
                        break;
                    }
                    int arrSize = std::stoi(tok.value); // 把字符串 "10" 转成整数 10
                    Advance(); // 吃掉数字
                    Consume(TOKEN_RBRACKET); // 吃掉 ']'
                    
                    // 神奇的物理升维：把基础图纸包进阵列图纸里！
                    actualType = std::make_shared<ArrayType>(actualType, arrSize);
                }

                if (!sema.CheckVariableDecl(varName, actualType)) break;
                
                blockNode->stmtVec.push_back(std::make_unique<VariableDeclAst>(actualType, varName));
                
                if (tok.type == TOKEN_ASSIGN) {
                    Advance();
                    auto rhs = ParseExpression();
                    auto lhsNode = std::make_unique<VariableAccessAst>(varName);
                    lhsNode->isLValue = true; // 坑位锁定
                    blockNode->stmtVec.push_back(std::make_unique<AssignExprAst>(std::move(lhsNode), std::move(rhs)));
                }
            }
            Consume(TOKEN_SEMI);
        } 
		// 拦截大括号
		else if(tok.type == TOKEN_LBRACE) {
			auto stmt = ParseBlockStmt();
            if (stmt) blockNode->stmtVec.push_back(std::move(stmt));
		}
		// 表达式
		else {
            // 普通表达式 (如 *p = a + b;)
            auto stmt = ParseExpression();
            if (stmt) blockNode->stmtVec.push_back(std::move(stmt));
            if (tok.type == TOKEN_SEMI) Advance();
        }
    }

    // 3. 吃掉 '}'
    if (tok.type == TOKEN_RBRACE) {
        Advance(); 
    }

	sema.ExitScope();
    return blockNode; // 组装完毕，交出大括号图纸
}

std::unique_ptr<ExprAst> Parser::ParseIfStmt() {
    // 1. 吃掉 "if" 和 "("
    Advance(); // 吃掉 if
    Advance(); // 吃掉 (
    
    // 2. 拿到条件 (condNode)
    auto condNode = ParseExpression(); 
    
    Advance(); // 吃掉 )

    // 3. 拿到成立时的动作 (thenNode)
    // 遇到大括号，直接调用我们上面刚写好的函数！
    std::unique_ptr<ExprAst> thenNode = nullptr;
    if (tok.type == TOKEN_LBRACE) {
        thenNode = ParseBlockStmt();
    } else {
        thenNode = ParseExpression(); // 有时候 if 后面只有一句话，没有大括号
    }

    // 4. 探头看一眼，有没有 else？
    std::unique_ptr<ExprAst> elseNode = nullptr;
    if (tok.type == TOKEN_KW_ELSE) {
        Advance(); // 吃掉 else
        
        if (tok.type == TOKEN_LBRACE) {
            elseNode = ParseBlockStmt();
        } else {
            elseNode = ParseExpression();
        }
    }

    // 5. 将三个零件通过 std::move 硬生生砸进 IfStmtAst 的肚子里！
    return std::make_unique<IfStmtAst>(std::move(condNode), std::move(thenNode), std::move(elseNode));
}


std::unique_ptr<ExprAst> Parser::ParseForStmt() {
    Advance(); // 吃掉 for
    Consume(TOKEN_LPAREN); // 吃掉 (

    // 1. 初始化表达式 (例如 i = 0)
    std::unique_ptr<ExprAst> initNode = nullptr;
    if (tok.type != TOKEN_SEMI) {
        initNode = ParseExpression();
    }
    Consume(TOKEN_SEMI); // 吃掉第一个 ;

    // 2. 条件表达式 (例如 i < 100)
    std::unique_ptr<ExprAst> condNode = nullptr;
    if (tok.type != TOKEN_SEMI) {
        condNode = ParseExpression();
    }
    Consume(TOKEN_SEMI); // 吃掉第二个 ;

    // 3. 步进表达式 (例如 i = i + 1)
    std::unique_ptr<ExprAst> incNode = nullptr;
    if (tok.type != TOKEN_RPAREN) {
        incNode = ParseExpression();
    }
    Consume(TOKEN_RPAREN); // 吃掉 )

    // 先把大箱子造出来
    auto forNode = std::make_unique<ForStmtAst>(std::move(initNode), std::move(condNode), std::move(incNode), nullptr);

    // 压栈动作！
    // 车间主任大喊一声：“里面的 break 和 continue 听着，你们的爹是我！”
    breakNodes.push_back(forNode.get());
    continueNodes.push_back(forNode.get());

    // 4. 进去解析循环体
    std::unique_ptr<ExprAst> bodyNode = nullptr;
    if (tok.type == TOKEN_LBRACE) {
        bodyNode = ParseBlockStmt();
    } else {
        bodyNode = ParseExpression(); // 支持没有大括号的单行 for 循环
    }
    forNode->bodyNode = std::move(bodyNode);

    // 解析完循环体，弹栈！
    // 退出这个循环了，我不再是后面代码的爹了。
    breakNodes.pop_back();
    continueNodes.pop_back();

    return forNode;
}

std::unique_ptr<ExprAst> Parser::ParseBreakStmt() {
    Advance(); // 吃掉 break

    auto node = std::make_unique<BreakStmtAst>();
    
    // 核心认亲逻辑：回头看一眼栈顶是谁！
    if (breakNodes.empty()) {
        std::cout << "语义错误：break 必须放在循环内部！" << std::endl;
    } else {
        node->target = breakNodes.back(); // 死死绑定最近的那个 For 节点
    }
    
    return node;
}

std::unique_ptr<ExprAst> Parser::ParseContinueStmt() {
    Advance(); // 吃掉 continue

    auto node = std::make_unique<ContinueStmtAst>();
    
    // 核心认亲逻辑：回头看一眼栈顶是谁！
    if (continueNodes.empty()) {
        std::cout << "语义错误：continue 必须放在循环内部！" << std::endl;
    } else {
        node->target = continueNodes.back(); // 死死绑定最近的那个 For 节点
    }
    
    return node;
}


std::unique_ptr<ExprAst> Parser::ParseUnaryExpr()
{
	UnaryOp op;
	// 先看看是 * 还是 &
	if (tok.type == TOKEN_MUL) op = UnaryOp::deref;
    else if (tok.type == TOKEN_AMP) op = UnaryOp::addr;
	
	Advance(); // 吃掉 * 或者 &
	// 1.解析操作符右边的基础表达式
	auto node = ParsePrimary();
	if (!node) return nullptr;
	return std::make_unique<UnaryExprAst>(op, std::move(node));
}
