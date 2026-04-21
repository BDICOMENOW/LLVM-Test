#pragma once
#include "Lexer.h"
class Parser
{
public:
	Parser(Lexer lex);
	~Parser();

public:
	void Advance();
	bool Expect(TokenType tokenType);
	bool Consume(TokenType tokenType);

	// shared_ptr<Expr> ParseFactor();

private:
	Lexer lexer;
	Token tok;
};

Parser::Parser(Lexer lex)
{
	lexer = lex;
	tok = lexer.GetNextToken();
}

Parser::~Parser()
{
}