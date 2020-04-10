#include <iostream>

#include "vm.h"
#include "linker.h"

#ifndef EXCLUDE_RAYLIB
#include "raylib.h"
#endif

#ifndef EXCLUDE_RAYLIB
VM::VM() : ip(0), stack{ }, stackTop(stack), traceLog(""), error(""), windowActive(false), isDrawing(false)
#else
VM::VM() : ip(0), stack{ }, stackTop(stack), traceLog(""), error("")
#endif
{
}

VM::~VM()
{
	while (stackTop > stack) { (--stackTop)->~Value(); }
#ifndef EXCLUDE_RAYLIB
	if (windowActive)
	{
		CloseWindow();
	}
#endif
}

InterpretResult VM::Interpret(const std::string& source)
{
	return Interpret(std::map<std::string, std::string>{ std::pair<std::string, std::string>{ "REPL", source } });
}

InterpretResult VM::Interpret(const std::map<std::string, std::string>& sources)
{
	using namespace std::string_literals;

	while (stackTop > stack) { (--stackTop)->~Value(); }
	callStack.clear();

	ip = 0;
	traceLog = ""s;
	error = ""s;

	switch (Linker(sources).Link(chunk))
	{
	case BuildResult::CompilerError:
		return InterpretResult::CompileError;

	case BuildResult::LinkerError:
		return InterpretResult::LinkerError;
	}

#ifndef EXCLUDE_RAYLIB
	SetTraceLogLevel(LOG_NONE);
#endif

	uint8_t instruction;
	for (;;)
	{
		if (ip >= chunk.Size())
		{
			return InterpretResult::RuntimeError;
		}

		instruction = chunk.Read(ip);
#ifndef NDEBUG
		std::cerr << "\n          ";
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
#ifndef EXCLUDE_RAYLIB
			if (windowActive)
			{
				CloseWindow();
			}
#endif
			return InterpretResult::Ok;

		case OpCode::None:
			break;

		case OpCode::AsDouble:
		{
			if (stackTop - stack < 1)
			{
				error = "No value on stack to convert to double"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<double>())
			{
				Value val = Pop();
				if (val.Get<long>())
				{
					Push(static_cast<double>(*val.Get<long>()));
				}
				else
				{
					error = "Invalid conversion to double"s;
					return InterpretResult::RuntimeError;
				}
			}
			break;
		}

		case OpCode::AsLong:
		{
			if (stackTop - stack < 1)
			{
				error = "No value on stack to convert to long"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				Value val = Pop();
				if (val.Get<double>())
				{
					Push(static_cast<long>(*val.Get<double>()));
				}
				else
				{
					error = "Invalid conversion to long"s;
					return InterpretResult::RuntimeError;
				}
			}
			break;
		}

		case OpCode::AsString:
		{
			if (stackTop - stack < 1)
			{
				error = "No value on stack to convert to double"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<std::string>())
			{
				Push(Pop() + Value(""s));
			}
			break;
		}

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
					error = "Invalid value in variable '" + loc + '\'';
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
					error = "Invalid value in variable '" + loc + '\'';
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

		case OpCode::Exponent:
		{
			if (stackTop - stack < 2)
			{
				error = "Not enough values on stack to perform operation 'exponent'"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<double>() || !stackTop[-2].Get<double>())
			{
				error = "Invalid arguments for operation 'exponent'"s;
				return InterpretResult::RuntimeError;
			}
			double exponent = *Pop().Get<double>();
			double base = *Pop().Get<double>();
			Push(std::pow(base, exponent));
			break;
		}

		case OpCode::Negate:
			if (stackTop - stack < 1)
			{
				error = "No value on stack to numerically negate"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() && !stackTop[-1].Get<double>())
			{
				error = "Invalid argument for numerical negation"s;
				return InterpretResult::RuntimeError;
			}
			Push(-Pop());
			break;

		case OpCode::LogicalNot:
			if (stackTop - stack < 1)
			{
				error = "No value on stack to logically negate"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<bool>())
			{
				error = "Invalid argument for logical negation"s;
				return InterpretResult::RuntimeError;
			}
			Push(!Pop());
			break;

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
			std::cout << Pop();
			break;

		case OpCode::PrintLn:
			if (stackTop - stack < 1)
			{
				error = "No value on the stack to println"s;
				return InterpretResult::RuntimeError;
			}
			std::cout << Pop() << '\n';
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

#ifndef EXCLUDE_RAYLIB
		case OpCode::InitWindow:
		{
			if (stackTop - stack < 3)
			{
				error = "Not enough values on the stack to init window"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<std::string>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>())
			{
				error = "Invalid arguments for init window"s;
				return InterpretResult::RuntimeError;
			}
			std::string title = *Pop().Get<std::string>();
			long height = *Pop().Get<long>();
			long width = *Pop().Get<long>();
			InitWindow(width, height, title.c_str());
			windowActive = true;
			break;
		}

		case OpCode::WindowShouldClose:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(WindowShouldClose());
			break;

		case OpCode::CloseWindow:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			CloseWindow();
			windowActive = false;
			break;

		case OpCode::ShowCursor:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			ShowCursor();
			break;

		case OpCode::HideCursor:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			HideCursor();
			break;

		case OpCode::ClearBackground:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 4)
			{
				error = "Not enough values on stack to clear background"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>())
			{
				error = "Invalid arguments for clear background"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			ClearBackground(Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::BeginDrawing:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (isDrawing)
			{
				error = "Already drawing"s;
				return InterpretResult::RuntimeError;
			}
			BeginDrawing();
			isDrawing = true;
			break;

		case OpCode::EndDrawing:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			EndDrawing();
			isDrawing = false;
			break;

		case OpCode::SetTargetFPS:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to set target fps"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for set target fps"s;
				return InterpretResult::RuntimeError;
			}
			SetTargetFPS(*Pop().Get<long>());
			break;

		case OpCode::GetTime:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(GetTime());
			break;

		case OpCode::GetRandomValue:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 2)
			{
				error = "Not enough values on stack to get random value"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>())
			{
				error = "Invalid arguments for get random value"s;
				return InterpretResult::RuntimeError;
			}
			long max = *Pop().Get<long>();
			long min = *Pop().Get<long>();
			Push(static_cast<long>(GetRandomValue(min, max)));
			break;
		}

		case OpCode::IsKeyPressed:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if key is pressed"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is key pressed"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsKeyPressed(*Pop().Get<long>()));
			break;

		case OpCode::IsKeyDown:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if key is down"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is key down"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsKeyDown(*Pop().Get<long>()));
			break;

		case OpCode::IsKeyReleased:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if key is released"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is key released"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsKeyReleased(*Pop().Get<long>()));
			break;

		case OpCode::IsKeyUp:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if key is up"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is key up"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsKeyUp(*Pop().Get<long>()));
			break;

		case OpCode::GetKeyPressed:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(static_cast<long>(GetKeyPressed()));
			break;

		case OpCode::SetExitKey:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to set as exit key"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for set exit key"s;
				return InterpretResult::RuntimeError;
			}
			SetExitKey(*Pop().Get<long>());
			break;

		case OpCode::IsMouseButtonPressed:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if mouse button is pressed"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is mouse button pressed"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsMouseButtonPressed(*Pop().Get<long>()));
			break;

		case OpCode::IsMouseButtonDown:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if mouse button is down"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is mouse button down"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsMouseButtonDown(*Pop().Get<long>()));
			break;

		case OpCode::IsMouseButtonReleased:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if mouse button is released"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is mouse button released"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsMouseButtonReleased(*Pop().Get<long>()));
			break;

		case OpCode::IsMouseButtonUp:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 1)
			{
				error = "No value on stack to check if mouse button is up"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>())
			{
				error = "Invalid arguments for is mouse button up"s;
				return InterpretResult::RuntimeError;
			}
			Push(IsMouseButtonUp(*Pop().Get<long>()));
			break;

		case OpCode::GetMouseX:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(static_cast<long>(GetMouseX()));
			break;

		case OpCode::GetMouseY:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(static_cast<long>(GetMouseY()));
			break;

		case OpCode::GetMousePosition:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(static_cast<long>(GetMouseX()));
			Push(static_cast<long>(GetMouseY()));
			break;

		case OpCode::SetMousePosition:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 2)
			{
				error = "Not enough values on stack to set mouse position"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>())
			{
				error = "Invalid arguments for set mouse position"s;
				return InterpretResult::RuntimeError;
			}
			long x = *Pop().Get<long>();
			long y = *Pop().Get<long>();
			SetMousePosition(x, y);
			break;
		}

		case OpCode::SetMouseOffset:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 2)
			{
				error = "Not enough values on stack to set mouse offset"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>())
			{
				error = "Invalid arguments for set mouse offset"s;
				return InterpretResult::RuntimeError;
			}
			long x = *Pop().Get<long>();
			long y = *Pop().Get<long>();
			SetMouseOffset(x, y);
			break;
		}

		case OpCode::SetMouseScale:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 2)
			{
				error = "Not enough values on stack to set mouse scale"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<double>() || !stackTop[-2].Get<double>())
			{
				error = "Invalid arguments for set mouse scale"s;
				return InterpretResult::RuntimeError;
			}
			double x = *Pop().Get<double>();
			double y = *Pop().Get<double>();
			SetMouseScale(x, y);
			break;
		}

		case OpCode::GetMouseWheelMove:
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			Push(static_cast<long>(GetMouseWheelMove()));
			break;

		case OpCode::DrawPixel:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 6)
			{
				error = "Not enough values on stack to draw pixel"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<long>() || !stackTop[-6].Get<long>())
			{
				error = "Invalid arguments for draw pixel"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawPixel(x, y, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawLine:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 9)
			{
				error = "Not enough values on stack to draw line"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<double>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>() || !stackTop[-9].Get<long>())
			{
				error = "Invalid arguments for draw line"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			double thick = *Pop().Get<double>();
			long y2 = *Pop().Get<long>();
			long x2 = *Pop().Get<long>();
			long y1 = *Pop().Get<long>();
			long x1 = *Pop().Get<long>();
			DrawLineEx(Vector2{ static_cast<float>(x1), static_cast<float>(y1) }, Vector2{ static_cast<float>(x2), static_cast<float>(y2) }, thick, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawCircle:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 7)
			{
				error = "Not enough values on stack to draw circle"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<double>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>())
			{
				error = "Invalid arguments for draw circle"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			double rad = *Pop().Get<double>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawCircle(x, y, rad, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawCircleLines:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 7)
			{
				error = "Not enough values on stack to draw circle lines"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<double>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>())
			{
				error = "Invalid arguments for draw circle lines"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			double rad = *Pop().Get<double>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawCircleLines(x, y, rad, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawEllipse:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 8)
			{
				error = "Not enough values on stack to draw ellipse"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<double>() || !stackTop[-6].Get<double>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>())
			{
				error = "Invalid arguments for draw ellipse"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			double radY = *Pop().Get<double>();
			double radX = *Pop().Get<double>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawEllipse(x, y, radY, radX, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawEllipseLines:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 8)
			{
				error = "Not enough values on stack to draw ellipse lines"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<double>() || !stackTop[-6].Get<double>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>())
			{
				error = "Invalid arguments for draw ellipse lines"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			double radY = *Pop().Get<double>();
			double radX = *Pop().Get<double>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawEllipseLines(x, y, radY, radX, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawRectangle:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 8)
			{
				error = "Not enough values on stack to draw rectangle"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<long>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>())
			{
				error = "Invalid arguments for draw rectangle"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			long h = *Pop().Get<long>();
			long w = *Pop().Get<long>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawRectangle(x, y, w, h, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawRectangleLines:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 8)
			{
				error = "Not enough values on stack to draw rectangle lines"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<long>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>())
			{
				error = "Invalid arguments for draw rectangle lines"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			long h = *Pop().Get<long>();
			long w = *Pop().Get<long>();
			long y = *Pop().Get<long>();
			long x = *Pop().Get<long>();
			DrawRectangleLines(x, y, w, h, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawTriangle:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 10)
			{
				error = "Not enough values on stack to draw triangle"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<long>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>() || !stackTop[-9].Get<long>() || !stackTop[-10].Get<long>())
			{
				error = "Invalid arguments for draw triangle"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			float y3 = static_cast<float>(*Pop().Get<long>());
			float x3 = static_cast<float>(*Pop().Get<long>());
			float y2 = static_cast<float>(*Pop().Get<long>());
			float x2 = static_cast<float>(*Pop().Get<long>());
			float y1 = static_cast<float>(*Pop().Get<long>());
			float x1 = static_cast<float>(*Pop().Get<long>());
			DrawTriangle(Vector2{ x1, y1 }, Vector2{ x2, y2 }, Vector2{ x3, y3 }, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}

		case OpCode::DrawTriangleLines:
		{
			if (!windowActive)
			{
				error = "Window not active"s;
				return InterpretResult::RuntimeError;
			}
			if (!isDrawing)
			{
				error = "Not drawing"s;
				return InterpretResult::RuntimeError;
			}
			if (stackTop - stack < 10)
			{
				error = "Not enough values on stack to draw triangle lines"s;
				return InterpretResult::RuntimeError;
			}
			if (!stackTop[-1].Get<long>() || !stackTop[-2].Get<long>() || !stackTop[-3].Get<long>() || !stackTop[-4].Get<long>() || !stackTop[-5].Get<long>() || !stackTop[-6].Get<long>() || !stackTop[-7].Get<long>() || !stackTop[-8].Get<long>() || !stackTop[-9].Get<long>() || !stackTop[-10].Get<long>())
			{
				error = "Invalid arguments for draw triangle lines"s;
				return InterpretResult::RuntimeError;
			}
			long a = *Pop().Get<long>();
			long b = *Pop().Get<long>();
			long g = *Pop().Get<long>();
			long r = *Pop().Get<long>();
			float y3 = static_cast<float>(*Pop().Get<long>());
			float x3 = static_cast<float>(*Pop().Get<long>());
			float y2 = static_cast<float>(*Pop().Get<long>());
			float x2 = static_cast<float>(*Pop().Get<long>());
			float y1 = static_cast<float>(*Pop().Get<long>());
			float x1 = static_cast<float>(*Pop().Get<long>());
			DrawTriangleLines(Vector2{ x1, y1 }, Vector2{ x2, y2 }, Vector2{ x3, y3 }, Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) });
			break;
		}
#endif
		}
	}
}

std::string VM::ErrorMessage() const
{
	using namespace std::string_literals;
	std::string msg = ""s;
	for (size_t i = 0; i < callStack.size(); i++)
	{
		msg += "[Line "s + std::to_string(chunk.ReadLine(callStack[i])) + "]"s;
		if (chunk.GetMeta(callStack[i] - 2) && chunk.GetMeta(callStack[i] - 2)->Get<std::string>())
		{
			msg += " @"s + *chunk.GetMeta(callStack[i] - 2)->Get<std::string>();
		}
		msg += "\n"s;
	}
	return msg + "[Line "s + std::to_string(chunk.ReadLine(ip)) + "] "s + *error.Get<std::string>();
}

void VM::Cleanup(bool clearGlobals)
{
	while (stackTop > stack) { (--stackTop)->~Value(); }
	if (clearGlobals) { globals.clear(); }
#ifndef EXCLUDE_RAYLIB
	if (windowActive)
	{
		CloseWindow();
	}
#endif
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