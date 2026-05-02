#pragma once
#include "Lexer.h"
#include "ast.h"
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
	std::unique_ptr<ExprAst> ParsePrimary();
	std::unique_ptr<ExprAst> ParseParenExpr();
	std::unique_ptr<ExprAst> ParseExpression();
	std::unique_ptr<ExprAst> ParserNumberExpr();

private:
	Lexer lexer;
	Token tok;
};

