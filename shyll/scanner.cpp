#include "scanner.h"

Token::Token() : TokenType(Type::Error), Lexeme(""), Line(1), HadWhitespace(false)
{
}

Token::Token(Type type, const std::string& lexeme, int line, bool hadWhitespace) : TokenType(type), Lexeme(lexeme), Line(line), HadWhitespace(hadWhitespace)
{
}

Scanner::Scanner(const std::string& source) : source(source), start(0), current(0), line(1), hadWhitespace(false)
{
}

Token Scanner::ScanToken()
{
	hadWhitespace = false;

	if (IsAtEnd()) return MakeToken(Token::Type::End);

	SkipWhitespace();
	start = current;

	char c = Advance();

	switch (c)
	{
	case '-':
		if (Match('>')) { return MakeToken(Token::Type::Load); }
		if (Match('-')) { return MakeToken(Token::Type::Delete); }
		break;

	case '+':
		if (Match('+')) { return MakeToken(Token::Type::Create); }
		break;

	case '<':
		if (Match('-')) { return MakeToken(Token::Type::Store); }
		break;

	case ':':
		return MakeToken(Token::Type::FunctionHeader);

	case '@':
		return MakeToken(Token::Type::FunctionCall);

	case '"':
		while (Peek() != '"' && Peek() != '\n' && !IsAtEnd())
		{
			Advance();
		}

		if (Peek(-1) == '\n' || IsAtEnd()) return ErrorToken("Unterminated string.");

		Advance();
		return MakeToken(Token::Type::String);

	case '\0':
		return MakeToken(Token::Type::End);

	default:
		if (IsDigit(c))
		{
			while (IsDigit(Peek())) { Advance(); }

			if (Peek() == '.' && IsDigit(Peek(1)))
			{
				Advance();

				while (IsDigit(Peek())) { Advance(); }

				return MakeToken(Token::Type::Double);
			}

			return MakeToken(Token::Type::Long);
		}
		else if (IsAlpha(c))
		{
			while (IsAlpha(Peek()) || IsDigit(Peek())) { Advance(); }

			return MakeToken(IdentifierType());
		}
	}

	using namespace std::string_literals;
	return ErrorToken("Unexpected character '"s + c + "'"s);
}

char Scanner::Advance()
{
	return IsAtEnd() ? '\0' : source[current++];
}

char Scanner::Peek(int offset) const
{
	if (current + offset < 0 || current + offset >= source.length())
	{
		return '\0';
	}
	else
	{
		return source[current + offset];
	}
}

bool Scanner::IsAtEnd() const
{
	return current >= source.length();
}

bool Scanner::Match(char expected)
{
	if (IsAtEnd()) { return false; }
	if (source[current] != expected) { return false; }

	current++;
	return true;
}

void Scanner::SkipWhitespace()
{
	for (;;)
	{
		char c = Peek();
		switch (c)
		{
		case ' ':
		case '\r':
		case '\t':
			hadWhitespace = true;
			Advance();
			break;

		case '\n':
			line++;
			hadWhitespace = true;
			Advance();
			break;

		case '#':
			hadWhitespace = true;
			while (Peek() != '\n' && !IsAtEnd()) { Advance(); }

		default:
			return;
		}
	}
}

Token::Type Scanner::IdentifierType() const
{
	switch (source[start])
	{
	case 'a':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 'd':
				return CheckKeyword("add", Token::Type::Add);
			case 'n':
				return CheckKeyword("and", Token::Type::LogicalAnd);
			}
		}
		break;
	case 'c':
		return CheckKeyword("cleartracelog", Token::Type::ClearTraceLog);
	case 'd':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 'i':
				return CheckKeyword("div", Token::Type::Divide);
			case 'o':
				return CheckKeyword("do", Token::Type::Do);
			case 'u':
				return CheckKeyword("dup", Token::Type::Duplicate);
			}
		}
		break;
	case 'e':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 'l':
				return CheckKeyword("else", Token::Type::Else);
			case 'n':
				return CheckKeyword("endif", Token::Type::EndIf);
			case 'q':
				return CheckKeyword("eq", Token::Type::Equal);
			}
		}
		break;
	case 'f':
		return CheckKeyword("false", Token::Type::False);
	case 'g':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 't':
				if (current - start > 1)
				{
					switch (source[start + 2])
					{
					case 'e':
						return CheckKeyword("gte", Token::Type::GreaterThanEqual);
					}
				}
				return CheckKeyword("gt", Token::Type::GreaterThan);
			}
		}
		break;
	case 'i':
		return CheckKeyword("if", Token::Type::If);
	case 'l':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 'o':
				return CheckKeyword("loop", Token::Type::Loop);
			case 't':
				if (current - start > 1)
				{
					switch (source[start + 2])
					{
					case 'e':
						return CheckKeyword("lte", Token::Type::LessThanEqual);
					}
				}
				return CheckKeyword("lt", Token::Type::LessThan);
			}
		}
		break;
	case 'm':
		return CheckKeyword("mul", Token::Type::Multiply);
	case 'n':
		return CheckKeyword("neq", Token::Type::NotEqual);
	case 'o':
		return CheckKeyword("or", Token::Type::LogicalOr);
	case 'p':
		return CheckKeyword("pop", Token::Type::Pop);
	case 's':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 'h':
				return CheckKeyword("showtracelog", Token::Type::ShowTraceLog);
			case 'u':
				return CheckKeyword("sub", Token::Type::Subtract);
			}
		}
		break;
	case 't':
		if (current - start > 0)
		{
			switch (source[start + 1])
			{
			case 'r':
				if (current - start > 1)
				{
					switch (source[start + 2])
					{
					case 'a':
						return CheckKeyword("trace", Token::Type::Trace);
					case 'u':
						return CheckKeyword("true", Token::Type::True);
					}
				}
			}
		}
		break;
	}

	return Token::Type::Identifier;
}

Token::Type Scanner::CheckKeyword(const std::string& expected, Token::Type type) const
{
	return ((current - start == expected.length() && source.substr(start, current - start) == expected) ? type : Token::Type::Identifier);
}

Token Scanner::MakeToken(Token::Type type) const
{
	return Token(type, source.substr(start, current - start), line, hadWhitespace);
}

Token Scanner::ErrorToken(const std::string& message) const
{
	return Token(Token::Type::Error, message, line, hadWhitespace);
}

bool Scanner::IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool Scanner::IsAlpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}