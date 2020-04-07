#pragma once

#include <map>

#include "chunk.h"
#include "compiler.h"

enum class BuildResult
{
	Ok,
	CompilerError,
	LinkerError
};

class Linker
{
public:
	Linker(const std::string& source);

	BuildResult Link(Chunk& chunk);

private:
	Compiler compiler;
	std::map<std::string, size_t> locs;

	bool MakeSymbol(std::map<std::string, Chunk>& symbols, const std::string& name, OpCode opcode);
	bool MakeSymbol(std::map<std::string, Chunk>& symbols, const std::string& name, Value value);
};