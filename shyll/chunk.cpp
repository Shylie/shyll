#include <iomanip>
#include <iostream>

#include "chunk.h"

size_t Chunk::Write(uint8_t instruction, int line)
{
	instructions.push_back(instruction);
	WriteLine(line);
	return instructions.size() - 1;
}

size_t Chunk::Write(OpCode instruction, int line)
{
	instructions.push_back(static_cast<uint8_t>(instruction));
	WriteLine(line);
	return instructions.size() - 1;
}

size_t Chunk::WriteLong(uint16_t instruction, int line)
{
	instructions.push_back(static_cast<uint8_t>(UINT8_MAX & (instruction >> 8)));
	instructions.push_back(static_cast<uint8_t>(UINT8_MAX & instruction));
	WriteLine(line);
	WriteLine(line);
	return instructions.size() - 2;
}

size_t Chunk::AddConstant(const Value& value, int line, OpCode ifShort, OpCode ifLong, bool& success)
{
	size_t ret = instructions.size();
	for (size_t i = 0; i < values.size(); i++)
	{
		if ((value == values[i]).Get<bool>() && *(value == values[i]).Get<bool>())
		{
			success = true;
			if (i > UINT8_MAX)
			{
				Write(ifLong, line);
				WriteLong(i, line);
			}
			else
			{
				Write(ifShort, line);
				Write(i, line);
			}
			return ret;
		}
	}
	if (values.size() + 1 >= UINT16_MAX)
	{
		success = false;
		return ret;
	}
	else if (values.size() + 1 >= UINT8_MAX)
	{
		Write(ifLong, line);
		WriteLong(values.size(), line);
	}
	else
	{
		Write(ifShort, line);
		Write(values.size(), line);
	}
	success = true;
	values.push_back(value);
	return ret;
}

size_t Chunk::AddConstant(const Value& value, int line, OpCode ifShort, OpCode ifLong)
{
	bool throwaway;
	return AddConstant(value, line, ifShort, ifLong, throwaway);
}

void Chunk::AddMeta(size_t offset, const Value& metadata)
{
	meta[offset] = metadata;
}

const Value* Chunk::GetMeta(size_t offset) const
{
	if (meta.find(offset) != meta.end())
	{
		return &meta.at(offset);
	}
	else
	{
		return nullptr;
	}
}

void Chunk::Modify(size_t offset, uint8_t nval)
{
	instructions[offset] = nval;
}

void Chunk::ModifyLong(size_t offset, uint16_t nval)
{
	instructions[offset] = static_cast<uint8_t>(UINT8_MAX & (nval >> 8));
	instructions[offset + 1] = static_cast<uint8_t>(UINT8_MAX & nval);
}

void Chunk::ModifyConstant(size_t offset, const Value& value)
{
	for (size_t i = 0; i < values.size(); i++)
	{
		if ((value == values[i]).Get<bool>() && *(value == values[i]).Get<bool>())
		{
			if (i > UINT8_MAX)
			{
				ModifyLong(offset, i);
			}
			else
			{
				Modify(offset, i);
			}
			return;
		}
	}
	if (values.size() + 1 >= UINT8_MAX)
	{
		ModifyLong(offset, values.size());
	}
	else
	{
		Modify(offset, values.size());
	}
	values.push_back(value);
}

uint8_t Chunk::Read(size_t offset) const
{
	return instructions[offset];
}

uint16_t Chunk::ReadLong(size_t offset) const
{
	return static_cast<uint16_t>(((instructions[offset] << 8) & 0xFF00) + (instructions[offset + 1] & 0x00FF));
}

Value Chunk::ReadConstant(uint16_t index) const
{
	return values[index];
}

int Chunk::ReadLine(size_t offset) const
{
	size_t curlen = 0;
	size_t total = 0;
	for (size_t i = 0; i < lines.size(); i++)
	{
		while (curlen < lines[i].len)
		{
			if (total == offset) { return lines[i].line; }
			total++;
			curlen++;
		}
		curlen = 0;
	}
	return -1;
}

void Chunk::Disassemble(const std::string& name) const
{
	std::cerr << "== " << name << " ==\n";

	size_t oldOffset = 0;
	size_t newOffset = 0;
	size_t dif = 0;
	for (size_t offset = 0; offset < instructions.size();)
	{
		dif = newOffset - oldOffset;
		oldOffset = offset;
		offset = DisassembleInstruction(offset, dif);
		newOffset = offset;
	}
}

size_t Chunk::DisassembleInstruction(size_t offset, size_t dif) const
{
	std::cerr << std::setfill('0') << std::setw(4) << std::right << offset << ' ';
	if (dif > 0 && offset > 0 && ReadLine(offset - dif) == ReadLine(offset))
	{
		std::cerr << ("   | ");
	}
	else
	{
		std::cerr << std::setfill(' ') << std::setw(4) << std::right << ReadLine(offset) << ' ';
	}

	uint8_t instruction = instructions[offset];
	switch (static_cast<OpCode>(instruction))
	{
	case OpCode::Return:
		return SimpleInstruction("OP_RETURN", offset);

	case OpCode::Constant:
		return ConstantInstruction("OP_CONSTANT", offset);

	case OpCode::ConstantLong:
		return ConstantInstructionLong("OP_CONSTANT_LONG", offset);

	case OpCode::Store:
		return ConstantInstruction("OP_STORE", offset);

	case OpCode::StoreLong:
		return ConstantInstructionLong("OP_STORE_LONG", offset);

	case OpCode::Load:
		return ConstantInstruction("OP_LOAD", offset);

	case OpCode::LoadLong:
		return ConstantInstructionLong("OP_LOAD_LONG", offset);

	case OpCode::Del:
		return ConstantInstruction("OP_DEL", offset);

	case OpCode::DelLong:
		return ConstantInstructionLong("OP_DEL_LONG", offset);

	case OpCode::Create:
		return ConstantInstruction("OP_CREATE", offset);

	case OpCode::CreateLong:
		return ConstantInstructionLong("OP_CREATE_LONG", offset);

	case OpCode::Add:
		return SimpleInstruction("OP_ADD", offset);

	case OpCode::Subtract:
		return SimpleInstruction("OP_SUBTRACT", offset);

	case OpCode::Multiply:
		return SimpleInstruction("OP_MULTIPLY", offset);

	case OpCode::Divide:
		return SimpleInstruction("OP_DIVIDE", offset);

	case OpCode::LessThan:
		return SimpleInstruction("OP_LESS", offset);

	case OpCode::LessThanEqual:
		return SimpleInstruction("OP_LESS_EQUAL", offset);

	case OpCode::GreaterThan:
		return SimpleInstruction("OP_GREATER", offset);

	case OpCode::GreaterThanEqual:
		return SimpleInstruction("OP_GREATER_EQUAL", offset);

	case OpCode::Equal:
		return SimpleInstruction("OP_EQUAL", offset);

	case OpCode::NotEqual:
		return SimpleInstruction("OP_NOT_EQUAL", offset);

	case OpCode::LogicalAnd:
		return SimpleInstruction("OP_LOGICAL_AND", offset);

	case OpCode::LogicalOr:
		return SimpleInstruction("OP_LOGICAL_OR", offset);

	case OpCode::Duplicate:
		return SimpleInstruction("OP_DUPLICATE", offset);

	case OpCode::Pop:
		return SimpleInstruction("OP_POP", offset);

	case OpCode::Print:
		return SimpleInstruction("OP_PRINT", offset);

	case OpCode::PrintLn:
		return SimpleInstruction("OP_PRINT_LN", offset);

	case OpCode::Trace:
		return SimpleInstruction("OP_TRACE", offset);

	case OpCode::ShowTraceLog:
		return SimpleInstruction("OP_SHOW_TRACELOG", offset);

	case OpCode::ClearTraceLog:
		return SimpleInstruction("OP_CLEAR_TRACELOG", offset);

	case OpCode::Jump:
		return JumpInstruction("OP_JUMP", offset);

	case OpCode::JumpIfFalse:
		return JumpInstruction("OP_JUMP_IF_FALSE", offset);

	case OpCode::JumpToCallStackAddress:
		return SimpleInstruction("OP_JUMP_TO_CALL_STACK_ADDRESS", offset);

	case OpCode::PushJumpAddress:
		return SimpleInstruction("OP_PUSH_JUMP_ADDRESS", offset);

	default:
		std::cerr << "Unknown opcode " << instruction << '\n';
		return offset + 1;
	}
}

size_t Chunk::Size() const
{
	return instructions.size();
}

void Chunk::WriteLine(int line)
{
	if (lines.size() > 0 && line == lines.back().line)
	{
		lines.back().len++;
	}
	else
	{
		lines.push_back(LineInfo{ 1, line });
	}
}

size_t Chunk::SimpleInstruction(const std::string& name, size_t offset) const
{
	std::cerr << std::setfill(' ') << std::left << std::setw(16) << name << '\n';
	return offset + 1;
}

size_t Chunk::ConstantInstruction(const std::string& name, size_t offset) const
{
	uint8_t constant = instructions[offset + 1];
	std::cerr << std::setfill(' ') << std::left << std::setw(16) << name << std::right << std::setw(6) << static_cast<uint16_t>(constant) << " '" << values[constant] << "'\n";
	return offset + 2;
}

size_t Chunk::ConstantInstructionLong(const std::string& name, size_t offset) const
{
	uint16_t constant = ReadLong(offset + 1);
	std::cerr << std::setfill(' ') << std::left << std::setw(16) << name << std::right << std::setw(6) << constant << " '" << values[constant] << "'\n";
	return offset + 3;
}

size_t Chunk::JumpInstruction(const std::string& name, size_t offset) const
{
	uint16_t ip = offset + 3 + static_cast<int16_t>(ReadLong(offset + 1));
	std::cerr << std::setfill(' ') << std::left << std::setw(16) << name << "  " << std::right << std::setfill('0') << std::setw(4) << ip << '\n';
	return offset + 3;
}