#pragma once
#include "Lexer.h"
#include "ast.h"
#include "Sema.h"
#include <vector> // 引入vector用来存放多行代码的树节点

class Parser
{
public:
	Parser() = default;
	Parser(Lexer lex);
	~Parser();

public:
	void Advance();
	bool Expect(TokenType tokenType);
	bool Consume(TokenType tokenType);

	int GetTokPrecedence();
	std::unique_ptr<ExprAst> ParseBinOpRhs(int exprPrec, std::unique_ptr<ExprAst> lHs);



public:
	std::unique_ptr<ExprAst> ParsePrimary(); // 任务处理分发
	std::unique_ptr<ExprAst> ParseParenExpr(); // 处理括号
	std::unique_ptr<ExprAst> ParseExpression();	// 处理表达式
	std::unique_ptr<ExprAst> ParserNumberExpr();
	std::vector<std::unique_ptr<ExprAst>> ParseProgram(); // 处理整个程序
	std::unique_ptr<ExprAst> ParseIdentifierExpr(); // 处理变量和赋值
	std::unique_ptr<ExprAst> ParseIfStmt();	// 处理if语句
	std::unique_ptr<ExprAst> ParseBlockStmt(); // 处理代码块
	std::unique_ptr<ExprAst> ParseForStmt(); // 处理for语句
	std::unique_ptr<ExprAst> ParseBreakStmt(); // 处理break语句
	std::unique_ptr<ExprAst> ParseContinueStmt(); // 处理continue语句
	std::unique_ptr<ExprAst> ParseUnaryExpr();	// 处理一元操作符

	std::shared_ptr<CType> ParseType(); // 解析类型

private:
	Lexer lexer;
	Token tok;
	Sema sema; // 安全检查
	
	// 用来记路的两个栈
	std::vector<ExprAst*> breakNodes;
	std::vector<ExprAst*> continueNodes;

};

