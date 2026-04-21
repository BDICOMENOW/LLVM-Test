#include "parser.h"

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



