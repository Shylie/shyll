#pragma once

#include <map>
#include "chunk.h"
#include "compiler.h"

class Linker
{
public:
	Linker(const std::string& source);

	bool Link(Chunk& chunk);

private:
	Compiler compiler;
	std::map<std::string, size_t> locs;
};