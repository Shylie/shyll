#include <iostream>

#include "compiler.h"

Compiler::Compiler(const std::string& source) : currentToken(0), currentSymbol("!main"), hadError(false), panicMode(false), compiled(false)
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
	case Token::Type::AsDouble:
		EmitByte(OpCode::AsDouble);
		currentToken++;
		break;

	case Token::Type::AsLong:
		EmitByte(OpCode::AsLong);
		currentToken++;
		break;

	case Token::Type::AsString:
		EmitByte(OpCode::AsString);
		currentToken++;
		break;

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

	case Token::Type::Exponent:
		EmitByte(OpCode::Exponent);
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

	case Token::Type::LogicalNot:
		EmitByte(OpCode::LogicalNot);
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

	case Token::Type::Print:
		EmitByte(OpCode::Print);
		currentToken++;
		break;

	case Token::Type::PrintLn:
		EmitByte(OpCode::PrintLn);
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

	case Token::Type::Load:
		if (NextToken() && NextToken()->TokenType == Token::Type::Identifier)
		{
			if (NextToken()->HadWhitespace)
			{
				ErrorAt(*CurrentToken(), "Invalid trailing whitespace");
			}
			else
			{
				EmitConstant(NextToken()->Lexeme, OpCode::Load, OpCode::LoadLong);
			}
			currentToken += 2;
		}
		else
		{
			ErrorAt(*CurrentToken(), "Expected an identifier");
			currentToken++;
		}
		break;

	case Token::Type::Store:
		if (NextToken() && NextToken()->TokenType == Token::Type::Identifier)
		{
			if (NextToken()->HadWhitespace)
			{
				ErrorAt(*CurrentToken(), "Invalid trailing whitespace");
			}
			else
			{
				EmitConstant(NextToken()->Lexeme, OpCode::Store, OpCode::StoreLong);
			}
			currentToken += 2;
		}
		else
		{
			ErrorAt(*CurrentToken(), "Expected an identifier");
			currentToken++;
		}
		break;

	case Token::Type::Identifier:
	{
		if (NextToken())
		{
			currentToken++;
			switch (CurrentToken()->TokenType)
			{
			case Token::Type::Create:
			{
				if (CurrentToken()->HadWhitespace)
				{
					ErrorAt(*PreviousToken(), "Invalid trailing whitespace");
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
						currentToken += 2;
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
						currentToken++;
						break;
					}
					else
					{
						currentToken++;
						break;
					}
				}
			}

			case Token::Type::Delete:
				if (CurrentToken()->HadWhitespace)
				{
					ErrorAt(*PreviousToken(), "Invalid trailing whitespace");
				}
				else
				{
					EmitConstant(PreviousToken()->Lexeme, OpCode::Del, OpCode::DelLong);
					currentToken++;
				}
				break;

			default:
				ErrorAt(*PreviousToken(), "Invalid use of an identifier");
			}
		}
		else
		{
			ErrorAt(*CurrentToken(), "Invalid use of an identifier");
			currentToken++;
		}
		break;
	}

	case Token::Type::While:
	{
		Token whileStart = *CurrentToken();
		currentToken++;
		if (!CurrentToken())
		{
			ErrorAt(whileStart, "Unexpected end of file");
			break;
		}
		uint16_t whileHeaderStartOffset = EmitByte(OpCode::None);
		while (CurrentToken() && CurrentToken()->TokenType != Token::Type::Do)
		{
			if (CurrentToken()->TokenType == Token::Type::End)
			{
				ErrorAt(whileStart, "Unterminated while-loop header");
				break;
			}
			Token cur = *CurrentToken();
			if (!Instruction())
			{
				ErrorAt(whileStart, "Unterminated while-loop header");
				ErrorAt(cur, "Invalid instruction inside while-loop header");
				break;
			}
		}
		currentToken++;
		if (!CurrentToken())
		{
			ErrorAt(*PreviousToken(), "Unexpected end of file");
			break;
		}
		uint16_t whileHeaderEndOffset = EmitJump(OpCode::JumpIfFalse);
		while (CurrentToken() && CurrentToken()->TokenType != Token::Type::Loop)
		{
			if (CurrentToken()->TokenType == Token::Type::End)
			{
				ErrorAt(whileStart, "Unterminated while-loop body");
				break;
			}
			Token cur = *CurrentToken();
			if (!Instruction())
			{
				ErrorAt(whileStart, "Unterminated while-loop body");
				ErrorAt(cur, "Invalid instruction inside while-loop body");
				break;
			}
		}
		PatchJump(EmitJump(OpCode::Jump), whileHeaderStartOffset);
		PatchJump(whileHeaderEndOffset);
		currentToken++;
		break;
	}

	case Token::Type::If:
	{
		Token ifStart = *CurrentToken();
		Token elseStart;
		currentToken++;
		if (!CurrentToken())
		{
			ErrorAt(*PreviousToken(), "Unterminated if statement");
			break;
		}
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
			if (!CurrentToken())
			{
				ErrorAt(ifStart, "Unexpected end of file");
				break;
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
					elseStart = *CurrentToken();
					elseOffset = EmitJump(OpCode::Jump);
					PatchJump(ifOffset);
					ifPatched = true;
					currentToken++;
				}
			}
		}
		if (!CurrentToken())
		{
			ErrorAt((ifPatched ? elseStart : ifStart), (ifPatched ? "Unterminated else statement" : "Unterminated if statement"));
			break;
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

	case Token::Type::Error:
		ErrorAt(*CurrentToken(), CurrentToken()->Lexeme);
		currentToken++;
		break;

	default:
		if (CurrentToken()) { ErrorAt(*CurrentToken(), "Invalid token"); }
		currentToken++;
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
	if (currentSymbol != "!main")
	{
		EmitByte(OpCode::JumpToCallStackAddress);
	}
	else
	{
		EmitByte(OpCode::Return);
	}
#ifndef NDEBUG
	CurrentChunk()->Disassemble(currentSymbol);
	std::cerr << '\n';
#endif
}

void Compiler::WarnAt(const Token& token, const std::string& message)
{
	std::cerr << "[Line " << token.Line << "] Error";

	switch (token.TokenType)
	{
	case Token::Type::Error:
		// nothing
		break;

	default:
		std::cerr << " at '" << token.Lexeme << '\'';
	}

	std::cerr << ": " << message << '\n';
}

void Compiler::ErrorAt(const Token& token, const std::string& message)
{
	if (panicMode) { return; }
	//panicMode = true;

	WarnAt(token, message);

	hadError = true;
}