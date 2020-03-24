#pragma once

#include <map>
#include "chunk.h"
#include "scanner.h"

class Compiler
{
public:
	Compiler(const std::string& source);

	std::map<std::string, Chunk>& Compile(bool& success);

private:
	std::vector<Token> tokens;
	size_t currentToken;
	std::map<std::string, Chunk> symbols;
	std::string currentSymbol;
	bool hadError;
	bool panicMode;
	bool compiled;

	Chunk* CurrentChunk();
	const Token* PreviousToken();
	const Token* CurrentToken();
	const Token* NextToken();
	const Token* TokenRelative(int offset);

	bool Instruction();

	size_t EmitByte(uint8_t byte);
	size_t EmitByte(OpCode op);
	size_t EmitConstant(const Value& value, OpCode ifShort, OpCode ifLong);
	uint16_t EmitJump(OpCode op);
	void PatchJump(uint16_t address);
	void PatchJump(uint16_t address, uint16_t jumpAddress);

	void EndSymbol();

	void WarnAt(const Token& token, const std::string& message);
	void ErrorAt(const Token& token, const std::string& message);
};