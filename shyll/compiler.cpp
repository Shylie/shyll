#include <iostream>
#include "compiler.h"

Compiler::Compiler(const std::string& source) : source(source), scanner(source), hadError(false), panicMode(false), compilingChunk(nullptr)
{
}

bool Compiler::Compile(Chunk* chunk)
{
	if (!chunk)
	{
		return false;
	}
	else
	{
		compilingChunk = chunk;
	}

	Advance();
	Advance();

	while (current.TokenType != Token::Type::End)
	{
		Instruction();
	}
	Instruction();

	EndCompile();
	return !hadError;
}

Chunk* Compiler::CurrentChunk()
{
	return compilingChunk;
}

void Compiler::Advance(bool read)
{
	if (!read) { return; }

	previousPrevious = previous;
	previous = current;

	for (;;)
	{
		current = scanner.ScanToken();
		if (current.TokenType != Token::Type::Error) { break; }

		ErrorAt(current, current.Lexeme);
	}
}

void Compiler::Consume(Token::Type type, const std::string& message, bool read)
{
	if (current.TokenType == type)
	{
		Advance(read);
	}
	else
	{
		Error(message);
	}
}

void Compiler::Instruction(bool read)
{
	switch (previous.TokenType)
	{
	case Token::Type::Long:
		EmitConstant(std::stol(previous.Lexeme), OpCode::Constant, OpCode::ConstantLong);
		break;

	case Token::Type::Double:
		EmitConstant(std::stod(previous.Lexeme), OpCode::Constant, OpCode::ConstantLong);
		break;

	case Token::Type::String:
		EmitConstant(previous.Lexeme.substr(1, previous.Lexeme.length() - 2), OpCode::Constant, OpCode::ConstantLong);
		break;

	case Token::Type::True:
		EmitConstant(true, OpCode::Constant, OpCode::ConstantLong);
		break;

	case Token::Type::False:
		EmitConstant(false, OpCode::Constant, OpCode::ConstantLong);
		break;

	case Token::Type::Add:
		EmitByte(OpCode::Add);
		break;

	case Token::Type::Subtract:
		EmitByte(OpCode::Subtract);
		break;

	case Token::Type::Multiply:
		EmitByte(OpCode::Multiply);
		break;

	case Token::Type::Divide:
		EmitByte(OpCode::Divide);
		break;

	case Token::Type::LessThan:
		EmitByte(OpCode::LessThan);
		break;

	case Token::Type::LessThanEqual:
		EmitByte(OpCode::LessThanEqual);
		break;

	case Token::Type::GreaterThan:
		EmitByte(OpCode::GreaterThan);
		break;

	case Token::Type::GreaterThanEqual:
		EmitByte(OpCode::GreaterThanEqual);
		break;

	case Token::Type::Equal:
		EmitByte(OpCode::Equal);
		break;

	case Token::Type::NotEqual:
		EmitByte(OpCode::NotEqual);
		break;

	case Token::Type::LogicalAnd:
		EmitByte(OpCode::LogicalAnd);
		break;

	case Token::Type::LogicalOr:
		EmitByte(OpCode::LogicalOr);
		break;

	case Token::Type::Duplicate:
		EmitByte(OpCode::Duplicate);
		break;

	case Token::Type::Pop:
		EmitByte(OpCode::Pop);
		break;

	case Token::Type::Trace:
		EmitByte(OpCode::Trace);
		break;

	case Token::Type::ShowTraceLog:
		EmitByte(OpCode::ShowTraceLog);
		break;

	case Token::Type::ClearTraceLog:
		EmitByte(OpCode::ClearTraceLog);
		break;

	case Token::Type::Store:
		Consume(Token::Type::Identifier, "Expected an identifier", read);
		if (previous.HadWhitespace)
		{
			ErrorAt(previousPrevious, "Invalid trailing whitespace");
		}
		else
		{
			EmitConstant(previous.Lexeme, OpCode::Store, OpCode::StoreLong);
		}
		break;

	case Token::Type::Load:
		Consume(Token::Type::Identifier, "Expected an identifier", read);
		if (previous.HadWhitespace)
		{
			ErrorAt(previousPrevious, "Invalid trailing whitespace");
		}
		else
		{
			EmitConstant(previous.Lexeme, OpCode::Load, OpCode::LoadLong);
		}
		break;

	case Token::Type::Delete:
		Consume(Token::Type::Identifier, "Expected an identifier", read);
		if (previous.HadWhitespace)
		{
			ErrorAt(previousPrevious, "Invalid trailing whitespace");
		}
		else
		{
			EmitConstant(previous.Lexeme, OpCode::Del, OpCode::DelLong);
		}
		break;

	case Token::Type::Create:
		Consume(Token::Type::Identifier, "Expected an identifier", read);
		if (previous.HadWhitespace)
		{
			ErrorAt(previousPrevious, "Invalid trailing whitespace");
		}
		else
		{
			Token variable = previous;
			EmitConstant(variable.Lexeme, OpCode::Create, OpCode::CreateLong);
			if (current.TokenType == Token::Type::Do)
			{
				Token doStart = current;
				EmitConstant("!" + variable.Lexeme, OpCode::Create, OpCode::CreateLong);
				EmitConstant(variable.Lexeme, OpCode::Store, OpCode::StoreLong);
				EmitConstant("!" + variable.Lexeme, OpCode::Store, OpCode::StoreLong);
				uint16_t beginLoopOffset = EmitConstant("!" + variable.Lexeme, OpCode::Load, OpCode::LoadLong);
				EmitConstant(variable.Lexeme, OpCode::Load, OpCode::LoadLong);
				EmitByte(OpCode::GreaterThan);
				uint16_t endLoopOffset = EmitJump(OpCode::JumpIfFalse);
				Advance(read);
				while (previous.TokenType != Token::Type::Loop)
				{
					Instruction(read);
					if (previous.TokenType == Token::Type::End)
					{
						ErrorAt(doStart, "Unterminated do loop");
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
		}
		break;

	case Token::Type::Identifier:
		Error("Invalid use of an identifier");
		break;

	case Token::Type::If:
	{
		Token ifStart = previous;
		uint16_t ifOffset = EmitJump(OpCode::JumpIfFalse);
		uint16_t elseOffset = 0;
		Advance(read);
		bool ifPatched = false;
		while (previous.TokenType != Token::Type::EndIf)
		{
			Instruction(read);
			if (previous.TokenType == Token::Type::Else)
			{
				if (ifPatched)
				{
					Error("Invalid else statement");
					break;
				}
				else
				{
					elseOffset = EmitJump(OpCode::Jump);
					PatchJump(ifOffset);
					ifPatched = true;
				}
				Advance(read);
			}
			if (previous.TokenType == Token::Type::End)
			{
				ErrorAt(ifStart, "Unterminated if statement");
				break;
			}
		}
		PatchJump(ifPatched ? elseOffset : ifOffset);
		break;
	}

	// workaround to delay call to Advance for above case to behave properly
	case Token::Type::Else:
		return;

	case Token::Type::FunctionCall:
		Consume(Token::Type::Identifier, "Expected an identifier", read);
		if (previous.HadWhitespace)
		{
			ErrorAt(previousPrevious, "Invalid trailing whitespace");
			break;
		}
		else
		{
			if (functions.find(previous.Lexeme) == functions.end())
			{
				Error("Undefined function");
				break;
			}
			else
			{
				Token _previousPrevious = previousPrevious;
				Token _previous = previous;
				Token _current = current;
				for (size_t i = 1; i < functions[_previous.Lexeme].size() - 1; i++)
				{
					previousPrevious = functions[_previous.Lexeme][i - 1];
					previous = functions[_previous.Lexeme][i];
					current = functions[_previous.Lexeme][i + 1];
					Instruction(false);
				}
				previousPrevious = _previousPrevious;
				previous = _previous;
				current = _current;
			}
		}
		break;

	case Token::Type::FunctionHeader:
		Consume(Token::Type::Identifier, "Expected an identifier", read);
		if (previous.HadWhitespace)
		{
			ErrorAt(previousPrevious, "Invalid trailing whitespace");
			break;
		}
		else
		{
			Token func = previous;
			functions.insert({ func.Lexeme, { } });
			functions[func.Lexeme].push_back(func);
			while (true)
			{
				Advance(read);
				if (previous.TokenType == Token::Type::FunctionFooter)
				{
					functions[func.Lexeme].push_back(previous);
					break;
				}
				else if (previous.TokenType == Token::Type::FunctionHeader || previous.TokenType == Token::Type::End)
				{
					ErrorAt(func, "Unterminated function");
					break;
				}
				else
				{
					functions[func.Lexeme].push_back(previous);
				}
			}
			return;
		}
	}
	Advance(read);
}

size_t Compiler::EmitByte(uint8_t byte)
{
	return CurrentChunk()->Write(byte, previous.Line);
}

size_t Compiler::EmitByte(OpCode op)
{
	return CurrentChunk()->Write(op, previous.Line);
}

size_t Compiler::EmitConstant(const Value& value, OpCode ifShort, OpCode ifLong)
{
	bool success;
	size_t ret = CurrentChunk()->AddConstant(value, previous.Line, ifShort, ifLong, success);
	if (!success) { Error("Too many constants in one chunk"); }
	return ret;
}

uint16_t Compiler::EmitJump(OpCode op)
{
	EmitByte(op);
	EmitByte(0xFF);
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

void Compiler::EndCompile()
{
	EmitByte(OpCode::Return);
#ifdef _DEBUG
	CurrentChunk()->Disassemble("code");
	std::cout << '\n';
#endif
	scanner = Scanner(source);
	previousPrevious = Token();
	previous = Token();
	current = Token();
	hadError = false;
	panicMode = false;
	functions = { };
	compilingChunk = nullptr;
}

void Compiler::Error(const std::string& message)
{
	ErrorAt(previous, message);
}

void Compiler::ErrorAt(const Token& token, const std::string& message)
{
	if (panicMode) { return; }
	//panicMode = true;

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
	hadError = true;
}
