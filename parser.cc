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
		case TOKEN_PLUS:
		case TOKEN_MINUS: {
			return 10;
		}
		case TOKEN_MUL:
		case TOKEN_DIV:
		case TOKEN_MOD: {
			return 20;
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
		if (tokPrec < nextPrec) {
			rHs = ParseBinOpRhs(tokPrec + 1, std::move(rHs));
			if (!rHs) return nullptr;
		}

		// 7.把LHS和RHS组装
		lHs = std::make_unique<BinaryExprAst>(binOp, std::move(lHs), std::move(rHs));
	}
}

// 任务处理分发
std::unique_ptr<ExprAst> Parser::ParsePrimary()
{
	switch (tok.type) {
		case TOKEN_NUMBER: {
			return ParserNumberExpr();
		}
		case TOKEN_LPAREN: {
			return ParseParenExpr();
		}
		case TOKEN_IDENTIFIER: {
			// 如果是标识符，可能是变量或者赋值
			return ParseIdentifierExpr();
		}
		case TOKEN_KW_IF: {
			// 如果是 if 关键字，可能是 if 语句
			return ParseIfStmt();
		}
		case TOKEN_LBRACE: {
			// 如果是左大括号，可能是大括号表达式
			return ParseBlockStmt();
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
	// 前进一个消耗掉左括号
	Advance();

	auto expr = ParseExpression();
	if (!expr) return nullptr;

	// 解析完后必须接着一个右括号
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

	if(tok.type == TOKEN_ASSIGN) {
		Advance(); // 吃掉赋值符 "="
		auto rhs = ParseExpression(); // 解析等号右边的算式
		if (!rhs) return nullptr;
		return std::make_unique<AssignExprAst>(std::make_unique<VariableAccessAst>(varName), std::move(rhs));
	}
	// 如果没有等号，那就只是一个变量： aa + 1 里面的 aa
	return std::make_unique<VariableAccessAst>(varName);
	
}

std::vector<std::unique_ptr<ExprAst>> Parser::ParseProgram()
{
	std::vector<std::unique_ptr<ExprAst>> exprList;
	while (tok.type != TOKEN_EOF) {
		// 1.忽略多余的分号
		if(tok.type == TOKEN_SEMI) {
			Advance();
			continue;
		}

		// 2. 如果遇到int，进入”变量声明“模式
		if(tok.type == TOKEN_KW_INT) {
			Advance(); // 吃掉int
			
			// 循环处理逗号分隔的变量名 aa = 3,b = 4; aa + b * 4 + 5;
			while(tok.type != TOKEN_SEMI) {
				if(tok.type == TOKEN_COMMA) {
					Advance(); // 吃掉逗号
				}
				std::string varName = tok.value;
				Advance(); // 吃掉变量名
				if(!sema.CheckVariableDecl(varName)) {
					break;
				}
				// 只要安检通过，就生成一个“声明节点”放进列表
				exprList.push_back(std::make_unique<VariableDeclAst>(varName));

				// 如果声明时顺带赋值了 (比如 int aa = 3)
				if(tok.type == TOKEN_ASSIGN) {
					Advance(); // 吃掉等号
					auto rhs = ParseExpression(); // 解析等号右边的算式
					exprList.push_back(std::make_unique<AssignExprAst>(
						    std::make_unique<VariableAccessAst>(varName), std::move(rhs)));
				}
			}
			Consume(TOKEN_SEMI); // 处理完这一行，吃掉分号
		} else {
			// 3. 不是变量声明，那就是一个表达式
			auto expr = ParseExpression();
			if (expr) {
				exprList.push_back(std::move(expr));
			}
			Consume(TOKEN_SEMI); // 吃掉行尾的分号
		}
	}
	return exprList;
}

std::unique_ptr<ExprAst> Parser::ParseBlockStmt() {
    auto blockNode = std::make_unique<BlockStmtAst>();
    
    // 1. 进门：吃掉 '{'
    if (tok.type == TOKEN_LBRACE) {
        Advance(); // 或者 Eat(TOKEN_LBRACE);
    }

    // 2. 疯狂解析里面的语句，直到遇到 '}'
    while (tok.type != TOKEN_RBRACE && tok.type != TOKEN_EOF) {
		
        auto stmt = ParseExpression(); 
        if (stmt) {
            blockNode->stmtVec.push_back(std::move(stmt));
        }
    }

    // 3. 出门：吃掉 '}'
    if (tok.type == TOKEN_RBRACE) {
        Advance(); 
    }

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