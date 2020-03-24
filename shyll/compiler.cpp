#include <iostream>
#include "compiler.h"

Compiler::Compiler(const std::string& source) : currentToken(0), currentSymbol("main"), hadError(false), panicMode(false), compiled(false)
{
	Scanner scanner(source);
	Token token;
	while ((token = scanner.ScanToken()).TokenType != Token::Type::End) { tokens.push_back(token); }
}

std::map<std::string, Chunk>& Compiler::Compile(bool& success)
{
	if (compiled)
	{
		success = !hadError;
		return symbols;
	}

	compiled = true;

	while (CurrentToken()) { Instruction(); }
	currentToken--;

	EndSymbol();
	success = !hadError;
	return symbols;
}

Chunk* Compiler::CurrentChunk()
{
	return &symbols[currentSymbol];
}

const Token* Compiler::PreviousToken()
{
	return (currentToken - 1 >= tokens.size()) ? nullptr : &tokens[currentToken - 1];
}

const Token* Compiler::CurrentToken()
{
	return (currentToken >= tokens.size()) ? nullptr : &tokens[currentToken];
}

const Token* Compiler::NextToken()
{
	return (currentToken + 1 >= tokens.size()) ? nullptr : &tokens[currentToken + 1];
}

const Token* Compiler::TokenRelative(int offset)
{
	return (currentToken + offset >= tokens.size()) ? nullptr : &tokens[currentToken + offset];
}

bool Compiler::Instruction()
{
	switch (CurrentToken()->TokenType)
	{
	case Token::Type::Long:
		EmitConstant(std::stol(CurrentToken()->Lexeme), OpCode::Constant, OpCode::ConstantLong);
		currentToken++;
		break;

	case Token::Type::Double:
		EmitConstant(std::stod(CurrentToken()->Lexeme), OpCode::Constant, OpCode::ConstantLong);
		currentToken++;
		break;

	case Token::Type::String:
		EmitConstant(CurrentToken()->Lexeme.substr(1, CurrentToken()->Lexeme.length() - 2), OpCode::Constant, OpCode::ConstantLong);
		currentToken++;
		break;

	case Token::Type::True:
		EmitConstant(true, OpCode::Constant, OpCode::ConstantLong);
		currentToken++;
		break;

	case Token::Type::False:
		EmitConstant(false, OpCode::Constant, OpCode::ConstantLong);
		currentToken++;
		break;

	case Token::Type::Add:
		EmitByte(OpCode::Add);
		currentToken++;
		break;

	case Token::Type::Subtract:
		EmitByte(OpCode::Subtract);
		currentToken++;
		break;

	case Token::Type::Multiply:
		EmitByte(OpCode::Multiply);
		currentToken++;
		break;

	case Token::Type::Divide:
		EmitByte(OpCode::Divide);
		currentToken++;
		break;

	case Token::Type::LessThan:
		EmitByte(OpCode::LessThan);
		currentToken++;
		break;

	case Token::Type::LessThanEqual:
		EmitByte(OpCode::LessThanEqual);
		currentToken++;
		break;

	case Token::Type::GreaterThan:
		EmitByte(OpCode::GreaterThan);
		currentToken++;
		break;

	case Token::Type::GreaterThanEqual:
		EmitByte(OpCode::GreaterThanEqual);
		currentToken++;
		break;

	case Token::Type::Equal:
		EmitByte(OpCode::Equal);
		currentToken++;
		break;

	case Token::Type::NotEqual:
		EmitByte(OpCode::NotEqual);
		currentToken++;
		break;

	case Token::Type::LogicalAnd:
		EmitByte(OpCode::LogicalAnd);
		currentToken++;
		break;

	case Token::Type::LogicalOr:
		EmitByte(OpCode::LogicalOr);
		currentToken++;
		break;

	case Token::Type::Duplicate:
		EmitByte(OpCode::Duplicate);
		currentToken++;
		break;

	case Token::Type::Pop:
		EmitByte(OpCode::Pop);
		currentToken++;
		break;

	case Token::Type::Trace:
		EmitByte(OpCode::Trace);
		currentToken++;
		break;

	case Token::Type::ShowTraceLog:
		EmitByte(OpCode::ShowTraceLog);
		currentToken++;
		break;

	case Token::Type::ClearTraceLog:
		EmitByte(OpCode::ClearTraceLog);
		currentToken++;
		break;

	case Token::Type::Identifier:
	{
		currentToken++;
		if (currentToken < tokens.size())
		{
			switch (CurrentToken()->TokenType)
			{
			case Token::Type::Create:
			{
				if (PreviousToken()->HadWhitespace)
				{
					ErrorAt(*TokenRelative(-2), "Invalid trailing whitespace");
					break;
				}
				else
				{
					Token variable = *PreviousToken();
					EmitConstant(variable.Lexeme, OpCode::Create, OpCode::CreateLong);
					if (NextToken() && NextToken()->TokenType == Token::Type::Do)
					{
						Token doStart = *NextToken();
						EmitConstant("!" + variable.Lexeme, OpCode::Create, OpCode::CreateLong);
						EmitConstant(variable.Lexeme, OpCode::Store, OpCode::StoreLong);
						EmitConstant("!" + variable.Lexeme, OpCode::Store, OpCode::StoreLong);
						uint16_t beginLoopOffset = EmitConstant("!" + variable.Lexeme, OpCode::Load, OpCode::LoadLong);
						EmitConstant(variable.Lexeme, OpCode::Load, OpCode::LoadLong);
						EmitByte(OpCode::GreaterThan);
						uint16_t endLoopOffset = EmitJump(OpCode::JumpIfFalse);
						currentToken++;
						while (CurrentToken() && CurrentToken()->TokenType != Token::Type::Loop)
						{
							if (CurrentToken()->TokenType == Token::Type::End)
							{
								ErrorAt(doStart, "Unterminated do-loop");
								break;
							}
							Token cur = *CurrentToken();
							if (!Instruction())
							{
								ErrorAt(doStart, "Unterminated do-loop");
								ErrorAt(cur, "Invalid instruction inside do-loop");
								break;
							}
						}
						EmitConstant(variable.Lexeme, OpCode::Load, OpCode::LoadLong);
						EmitConstant(1L, OpCode::Constant, OpCode::ConstantLong);
						EmitByte(OpCode::Add);
						EmitConstant(variable.Lexeme, OpCode::Store, OpCode::StoreLong);
						PatchJump(EmitJump(OpCode::Jump), beginLoopOffset);
						PatchJump(endLoopOffset);
						EmitConstant("!" + variable.Lexeme, OpCode::Del, OpCode::DelLong);
					}
					else
					{
						break;
					}
				}
			}

			case Token::Type::Delete:
				currentToken++;
				if (CurrentToken()->HadWhitespace)
				{
					ErrorAt(*PreviousToken(), "Invalid trailing whitespace");
				}
				else
				{
					EmitConstant(CurrentToken()->Lexeme, OpCode::Del, OpCode::DelLong);
				}
				break;

			case Token::Type::Load:
				currentToken++;
				if (CurrentToken()->HadWhitespace)
				{
					ErrorAt(*PreviousToken(), "Invalid trailing whitespace");
				}
				else
				{
					EmitConstant(CurrentToken()->Lexeme, OpCode::Load, OpCode::LoadLong);
				}
				break;

			case Token::Type::Store:
				currentToken++;
				if (CurrentToken()->HadWhitespace)
				{
					ErrorAt(*PreviousToken(), "Invalid trailing whitespace");
				}
				else
				{
					EmitConstant(CurrentToken()->Lexeme, OpCode::Store, OpCode::StoreLong);
				}
				break;

			default:
				ErrorAt(*CurrentToken(), "Invalid use of an identifier");
			}

		}
		break;
	}

	case Token::Type::If:
	{
		Token ifStart = *CurrentToken();
		currentToken++;
		uint16_t ifOffset = EmitJump(OpCode::JumpIfFalse);
		uint16_t elseOffset = 0;

		bool ifPatched = false;
		while (CurrentToken() && CurrentToken()->TokenType != Token::Type::EndIf)
		{
			if (CurrentToken()->TokenType == Token::Type::End)
			{
				ErrorAt(ifStart, "Unterminated if statement");
				break;
			}
			Token cur = *CurrentToken();
			if (!Instruction())
			{
				ErrorAt(cur, "Invalid instruction inside if statement");
			}
			if (CurrentToken()->TokenType == Token::Type::Else)
			{
				if (ifPatched)
				{
					ErrorAt(*CurrentToken(), "Invalid else statement");
					currentToken++;
					break;
				}
				else
				{
					elseOffset = EmitJump(OpCode::Jump);
					PatchJump(ifOffset);
					ifPatched = true;
					currentToken++;
				}
			}
		}
		PatchJump(ifPatched ? elseOffset : ifOffset);
		break;
	}

	case Token::Type::Else:
		break;

	case Token::Type::EndIf:
		currentToken++;
		break;

	case Token::Type::FunctionCall:
		if (NextToken() && NextToken()->TokenType == Token::Type::Identifier)
		{
			if (NextToken()->HadWhitespace)
			{
				ErrorAt(*CurrentToken(), "Invalid trailing whitespace");
				currentToken += 2;
			}
			else
			{
				EmitByte(OpCode::PushJumpAddress);
				CurrentChunk()->AddMeta(EmitJump(OpCode::Jump), NextToken()->Lexeme);
				currentToken += 2;
			}
		}
		else
		{
			ErrorAt(*CurrentToken(), "Expected an identifier");
			currentToken++;
		}
		break;

	case Token::Type::FunctionHeader:
		if (NextToken() && NextToken()->TokenType == Token::Type::Identifier)
		{
			if (NextToken()->HadWhitespace)
			{
				ErrorAt(*CurrentToken(), "Invalid trailing whitespace");
				currentToken += 2;
			}
			else
			{
				EndSymbol();
				currentToken += 2;
				currentSymbol = PreviousToken()->Lexeme;
			}
		}
		else
		{
			ErrorAt(*CurrentToken(), "Expected an identifier");
			currentToken++;
		}
		return false;

	default:
#ifdef _DEBUG
		if (CurrentToken()) { WarnAt(*CurrentToken(), "Invalid token"); }
#endif
		return false;
	}
	return true;
}

size_t Compiler::EmitByte(uint8_t byte)
{
	return CurrentChunk()->Write(byte, CurrentToken()->Line);
}

size_t Compiler::EmitByte(OpCode op)
{
	return CurrentChunk()->Write(op, CurrentToken()->Line);
}

size_t Compiler::EmitConstant(const Value& value, OpCode ifShort, OpCode ifLong)
{
	using namespace std::string_literals;
	bool success;
	size_t ret = CurrentChunk()->AddConstant(value, CurrentToken()->Line, ifShort, ifLong, success);
	CurrentChunk()->AddMeta(ret, "!constant"s);
	CurrentChunk()->AddMeta(ret + 1, value);
	if (!success) { ErrorAt(*CurrentToken(), "Too many constants in one chunk"); }
	return ret;
}

uint16_t Compiler::EmitJump(OpCode op)
{
	EmitByte(op);
	EmitByte(0xFF); // -2 offset points to this byte
	EmitByte(0xFF);
	return CurrentChunk()->Size() - 2;
}

void Compiler::PatchJump(uint16_t address)
{
	int16_t jump = CurrentChunk()->Size() - address - 2;

	CurrentChunk()->ModifyLong(address, jump);
}

void Compiler::PatchJump(uint16_t address, uint16_t jumpAddress)
{
	int16_t jump = jumpAddress - address - 2;

	CurrentChunk()->ModifyLong(address, jump);
}

void Compiler::EndSymbol()
{
	if (currentSymbol != "main")
	{
		EmitByte(OpCode::JumpToCallStackAddress);
	}
	else
	{
		EmitByte(OpCode::Return);
	}
#ifdef _DEBUG
	CurrentChunk()->Disassemble(currentSymbol);
	std::cout << '\n';
#endif
}

void Compiler::WarnAt(const Token& token, const std::string& message)
{
	std::cout << "[Line " << token.Line << "] Error";

	switch (token.TokenType)
	{
	case Token::Type::Error:
		// nothing
		break;

	default:
		std::cout << " at '" << token.Lexeme << '\'';
	}

	std::cout << ": " << message << '\n';
}

void Compiler::ErrorAt(const Token& token, const std::string& message)
{
	if (panicMode) { return; }
	//panicMode = true;

	WarnAt(token, message);

	hadError = true;
}