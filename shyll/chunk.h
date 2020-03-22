#pragma once

#include <string>
#include <vector>

#include "value.h"

enum class OpCode : uint8_t
{
	Constant,
	ConstantLong,
	Add,
	Subtract,
	Multiply,
	Divide,
	LessThan,
	LessThanEqual,
	GreaterThan,
	GreaterThanEqual,
	Equal,
	NotEqual,
	LogicalAnd,
	LogicalOr,
	Duplicate,
	Pop,
	Trace,
	ShowTraceLog,
	ClearTraceLog,
	Store,
	StoreLong,
	Load,
	LoadLong,
	Del,
	DelLong,
	Create,
	CreateLong,
	Jump,
	JumpIfFalse,
	Return
};

class Chunk
{
public:
	size_t Write(uint8_t instruction, int line);
	size_t Write(OpCode instruction, int line);
	size_t WriteLong(uint16_t instruction, int line);
	size_t AddConstant(const Value& value, int line, OpCode ifShort, OpCode ifLong, bool& success);
	size_t AddConstant(const Value& value, int line, OpCode ifShort, OpCode ifLong);

	void Modify(size_t offset, uint8_t nval);
	void ModifyLong(size_t offset, uint16_t nval);

	uint8_t Read(size_t offset) const;
	uint16_t ReadLong(size_t offset) const;
	Value ReadConstant(uint16_t index) const;

	void Disassemble(const std::string& name) const;
	size_t DisassembleInstruction(size_t offset, size_t dif) const;

	size_t Size() const;

private:
	struct LineInfo
	{
		size_t len;
		int line;
	};

	void WriteLine(int line);
	int ReadLine(size_t offset) const;

	size_t SimpleInstruction(const std::string& name, size_t offset) const;
	size_t ConstantInstruction(const std::string& name, size_t offset) const;
	size_t ConstantInstructionLong(const std::string& name, size_t offset) const;
	size_t JumpInstruction(const std::string& name, size_t offset) const;

	std::vector<uint8_t> instructions;
	std::vector<Value> values;
	std::vector<LineInfo> lines;
};