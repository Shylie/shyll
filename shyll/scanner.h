#pragma once

#include <string>

class Token
{
public:
	enum class Type
	{
		Long,
		Double,
		String,
		True,
		False,
		Identifier,

		AsLong,
		AsDouble,
		AsString,

		Store,
		Load,
		Delete,
		Create,
		FunctionCall,
		FunctionHeader,

		Add,
		Subtract,
		Multiply,
		Divide,
		Exponent,
		LessThan,
		LessThanEqual,
		GreaterThan,
		GreaterThanEqual,
		Equal,
		NotEqual,
		LogicalAnd,
		LogicalOr,
		LogicalNot,

		Duplicate,
		Pop,

		If,
		Else,
		EndIf,
		Do,
		While,
		Loop,

		Print,
		PrintLn,
		Trace,
		ShowTraceLog,
		ClearTraceLog,

		Comment,

		Error,
		End
	} TokenType;
	std::string Lexeme;
	int Line;
	bool HadWhitespace;

	Token();
	Token(Type type, const std::string& lexeme, int line, bool hadWhitespace);
};

class Scanner
{
public:
	Scanner(const std::string& source);

	Token ScanToken();

private:
	std::string source;
	size_t start;
	size_t current;
	int line;
	bool hadWhitespace;

	char Advance();
	char Peek(int offset = 0) const;

	bool IsAtEnd() const;
	bool Match(char expected);

	void SkipWhitespace();

	Token::Type IdentifierType() const;
	Token::Type CheckKeyword(const std::string& expected, Token::Type type) const;

	Token MakeToken(Token::Type type) const;
	Token ErrorToken(const std::string& message) const;

	static bool IsDigit(char c);
	static bool IsAlpha(char c);
};