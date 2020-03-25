#include <iostream>

#include "vm.h"
#include "linker.h"

VM::VM() : ip(0), stack{ }, stackTop(stack), traceLog(""), error("")
{
}

VM::~VM()
{
	while (stackTop > stack) { (--stackTop)->~Value(); }
}

InterpretResult VM::Interpret(const std::string& source)
{
	using namespace std::string_literals;

	while (stackTop > stack) { (--stackTop)->~Value(); }

	ip = 0;
	traceLog = ""s;
	error = ""s;

	if (!Linker(source).Link(chunk)) { return InterpretResult::CompileError; }

	uint8_t instruction;
	for (;;)
	{
		if (ip >= chunk.Size()) { return InterpretResult::RuntimeError; }

		instruction = chunk.Read(ip);
#ifdef _DEBUG
		std::cerr << "          ";
		for (Value* slot = stack; slot < stackTop; slot++)
		{
			std::cerr << "[ " << *slot << " ]";
		}
		std::cerr << '\n';
		chunk.DisassembleInstruction(ip, 0);
		std::cerr << '\n';
#endif
		ip++;
		switch (static_cast<OpCode>(instruction))
		{
		case OpCode::Return:
			return InterpretResult::Ok;

		case OpCode::Constant:
		{
			Value constant = chunk.ReadConstant(chunk.Read(ip++));
			if (!Push(constant))
			{
				error = Value("Invalid constant pushed to stack: '"s) + constant + "'"s;
				return InterpretResult::RuntimeError;
			}
			break;
		}

		case OpCode::ConstantLong:
		{
			Value constant = chunk.ReadConstant(chunk.ReadLong(ip)); ip += 2;
			if (!Push(constant))
			{
				error = Value("Invalid constant pushed to stack: '"s) + constant + "'"s;
				return InterpretResult::RuntimeError;
			}
			break;
		}

		case OpCode::Store:
		{
			std::string loc = *chunk.ReadConstant(chunk.Read(ip++)).Get<std::string>();
			if (stackTop - stack < 1)
			{
				error = "Not enough values on stack to store into variable '" + loc + '\'';
				return InterpretResult::RuntimeError;
			}
			if (globals.find(loc) != globals.end())
			{
				globals[loc] = Pop();
			}
			else
			{
				error = "Undeclared variable '" + loc + '\'';
				return InterpretResult::RuntimeError;
			}
			break;
		}

		case OpCode::StoreLong:
		{
			std::string loc = *chunk.ReadConstant(chunk.ReadLong(ip)).Get<std::string>(); ip += 2;
			if (stackTop - stack < 1)
			{
				error = "Not enough values on stack to store into variable '" + loc + '\'';
				return InterpretResult::RuntimeError;
			}
			if (globals.find(loc) != globals.end())
			{
				globals[loc] = Pop();
			}
			else
			{
				error = "Undeclared variable '" + loc + '\'';
				return InterpretResult::RuntimeError;
			}
			break;
		}

		case OpCode::Load:
		{
			std::string loc = *chunk.ReadConstant(chunk.Read(ip++)).Get<std::string>();
			if (globals.find(loc) != globals.end())
			{
				if (!Push(Value(globals[loc])))
				{
					error = "shit"s;
					return InterpretResult::RuntimeError;
				}
			}
			else
			{
				error = "Undeclared variable '" + loc + '\'';
				return InterpretResult::RuntimeError;
			}
			break;
		}

		case OpCode::LoadLong:
		{
			std::string loc = *chunk.ReadConstant(chunk.ReadLong(ip)).Get<std::string>(); ip += 2;
			if (globals.find(loc) != globals.end())
			{
				if (!Push(Value(globals[loc])))
				{
					error = "shit"s;
					return InterpretResult::RuntimeError;
				}
			}
			else
			{
				error = "Undeclared variable '" + loc + '\'';
				return InterpretResult::RuntimeError;
			}
			break;
		}

		case OpCode::Del:
		{
			std::string loc = *chunk.ReadConstant(chunk.Read(ip++)).Get<std::string>();
			if (globals.find(loc) != globals.end())
			{
				globals.erase(loc);
			}
			break;
		}

		case OpCode::DelLong:
		{
			std::string loc = *chunk.ReadConstant(chunk.ReadLong(ip)).Get<std::string>(); ip += 2;
			if (globals.find(loc) == globals.end())
			{
				globals.erase(loc);
			}
			break;
		}

		case OpCode::Create:
		{
			std::string loc = *chunk.ReadConstant(chunk.Read(ip++)).Get<std::string>();
			if (globals.find(loc) == globals.end())
			{
				globals.insert(std::pair<std::string, Value>(loc, Value()));
			}
			break;
		}

		case OpCode::CreateLong:
		{
			std::string loc = *chunk.ReadConstant(chunk.ReadLong(ip)).Get<std::string>(); ip += 2;
			if (globals.find(loc) != globals.end())
			{
				globals.insert(std::pair<std::string, Value>(loc, Value()));
			}
			break;
		}

		case OpCode::Duplicate:
			if (stackTop - stack < 1)
			{
				error = "No value on stack to duplicate"s;
				return InterpretResult::RuntimeError;
			}
			// since stackTop refers to the next open stack slot, stackTop[-1] refers to the top stack item.
			Push(Value(stackTop[-1]));
			break;

		case OpCode::Pop:
			if (stackTop - stack < 1)
			{
				error = "No value on stack to pop"s;
				return InterpretResult::RuntimeError;
			}
			stackTop[-1].~Value();
			stackTop--;
			break;

#define BINARY_OP(op, opname) \
do \
{ \
	if (stackTop - stack < 2) \
	{ \
		error = "Not enough values on stack to perform operation '"s + opname + "'"s; \
		return InterpretResult::RuntimeError; \
	} \
	Value b = Pop(); \
	Value a = Pop(); \
	if (!Push(a op b)) \
	{ \
		error = "Invalid arguments for operation '"s + opname + "'"s; \
		return InterpretResult::RuntimeError; \
	} \
} while (false)

		case OpCode::Add:
			BINARY_OP(+, "add");
			break;

		case OpCode::Subtract:
			BINARY_OP(-, "sub");
			break;

		case OpCode::Multiply:
			BINARY_OP(*, "mul");
			break;

		case OpCode::Divide:
			BINARY_OP(/, "div");
			break;

		case OpCode::LessThan:
			BINARY_OP(<, "lt");
			break;

		case OpCode::LessThanEqual:
			BINARY_OP(<=, "lte");
			break;

		case OpCode::GreaterThan:
			BINARY_OP(>, "gt");
			break;

		case OpCode::GreaterThanEqual:
			BINARY_OP(>=, "gte");
			break;

		case OpCode::Equal:
			BINARY_OP(==, "eq");
			break;

		case OpCode::NotEqual:
			BINARY_OP(!=, "neq");
			break;

		case OpCode::LogicalAnd:
			BINARY_OP(&&, "and");
			break;

		case OpCode::LogicalOr:
			BINARY_OP(||, "or");
			break;

#undef BINARY_OP

		case OpCode::Print:
			if (stackTop - stack < 1)
			{
				error = "No value on the stack to print"s;
				return InterpretResult::RuntimeError;
			}
			std::cout << stackTop[-1];
			break;

		case OpCode::PrintLn:
			if (stackTop - stack >= 1)
			{
				std::cout << stackTop[-1];
			}
			std::cout << '\n';
			break;

		case OpCode::Trace:
			if (stackTop - stack < 1)
			{
				error = "No value on the stack to trace"s;
				return InterpretResult::RuntimeError;
			}
			traceLog = traceLog + stackTop[-1] + "\n"s;
			break;

		case OpCode::ShowTraceLog:
			std::cout << traceLog;
			break;

		case OpCode::ClearTraceLog:
			traceLog = ""s;
			break;

		case OpCode::Jump:
		{
			int16_t offset = static_cast<int16_t>(chunk.ReadLong(ip)); ip += 2;
			ip += offset;
			break;
		}

		case OpCode::JumpIfFalse:
		{
			if (stackTop - stack < 1)
			{
				error = "No value on the stack for a conditional statement"s;
				return InterpretResult::RuntimeError;
			}
			int16_t offset = static_cast<int16_t>(chunk.ReadLong(ip)); ip += 2;
			Value condition = Pop();
			if (condition.Get<bool>())
			{
				if (!*condition.Get<bool>()) { ip += offset; }
			}
			else
			{
				error = "Invalid arguments for conditional"s;
				return InterpretResult::RuntimeError;
			}
			Value tmp, tmp2, tmp3;
			break;
		}

		case OpCode::JumpToCallStackAddress:
			if (callStack.empty())
			{
				error = "Call stack is empty, cannot jump"s;
				return InterpretResult::RuntimeError;
			}
			ip = callStack.back();
			callStack.pop_back();
			break;

		case OpCode::PushJumpAddress:
			callStack.push_back(ip + 3);
			break;
		}
	}
}

const std::string& VM::ErrorMessage() const
{
	return *error.Get<std::string>();
}

bool VM::Push(const Value& value)
{
	*stackTop = value;
	stackTop++;
	return value.Valid();
}

Value VM::Pop()
{
	Value val = stackTop[-1];
	stackTop[-1].~Value();
	stackTop--;
	return val;
}