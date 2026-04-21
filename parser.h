#pragma once
#include "Lexer.h"
class Parser
{
public:
	Parser(string inputStr);
	~Parser();

public:
	void Advance();
	bool Expect(TokenType tokenType);
	bool Consume(TokenType tokenType);

private:
	Lexer lexer;
	Token tok;
};

Parser::Parser(string inputStr)
{
	lexer = Lexer(inputStr);
}

Parser::~Parser()
{
}