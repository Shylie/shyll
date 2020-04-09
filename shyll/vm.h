#pragma once

#include <map>

#include "chunk.h"

enum class InterpretResult
{
	Ok,
	LinkerError,
	CompileError,
	RuntimeError
};

class VM
{
public:
	VM();
	~VM();

	InterpretResult Interpret(const std::string& source);
	InterpretResult Interpret(const std::map<std::string, std::string>& sources);
	std::string ErrorMessage() const;
	void Cleanup(bool clearGlobals);

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
#ifndef EXCLUDE_RAYLIB
	bool windowActive;
	bool isDrawing;
#endif

	bool Push(const Value& value);
	Value Pop();
};