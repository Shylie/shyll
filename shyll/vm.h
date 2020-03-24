#pragma once

#include <map>
#include "chunk.h"

enum class InterpretResult
{
	Ok,
	CompileError,
	RuntimeError
};

class VM
{
public:
	VM();
	~VM();

	InterpretResult Interpret(const std::string& source);
	const std::string& ErrorMessage() const;

	static constexpr size_t STACK_MAX = 512;

private:
	Chunk chunk;
	size_t ip;
	Value stack[STACK_MAX];
	Value* stackTop;
	Value traceLog;
	Value error;
	std::vector<size_t> callStack;
	std::map<std::string, Value> globals;
	std::string filename;

	bool Push(const Value& value);
	Value Pop();
};