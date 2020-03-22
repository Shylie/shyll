#pragma once

#include <map>
#include "chunk.h"
#include "scanner.h"

class Compiler
{
public:
	Compiler(const std::string& source);

	bool Compile(Chunk* chunk);

private:
	std::string source;
	Scanner scanner;
	Token previousPrevious;
	Token previous;
	Token current;
	bool hadError;
	bool panicMode;
	std::map<std::string, std::vector<Token>> functions;
	Chunk* compilingChunk;

	Chunk* CurrentChunk();

	void Advance(bool read = true);
	void Consume(Token::Type type, const std::string& message, bool read = true);

	void Instruction(bool read = true);

	size_t EmitByte(uint8_t byte);
	size_t EmitByte(OpCode op);
	size_t EmitConstant(const Value& value, OpCode ifShort, OpCode ifLong);
	uint16_t EmitJump(OpCode op);
	void PatchJump(uint16_t address);
	void PatchJump(uint16_t address, uint16_t jumpAddress);

	void EndCompile();

	void Error(const std::string& message);
	void ErrorAt(const Token& token, const std::string& message);
};