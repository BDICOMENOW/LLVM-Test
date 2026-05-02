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
		case TOKEN_DIV: {
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



