#include "linker.h"

#ifndef EXCLUDE_RAYLIB
#include "raylib.h"
#endif

Linker::Linker(const std::string& source) : compiler(source)
{

}

BuildResult Linker::Link(Chunk& chunk)
{
	bool success;
	std::map<std::string, Chunk>& symbols = compiler.Compile(success);
	if (!success) { return BuildResult::CompilerError; }

	locs.clear();

	chunk = symbols["!main"];

	std::map<std::string, Chunk> raylibSymbols;

#ifndef EXCLUDE_RAYLIB
#pragma region CORE MODULE
	MakeSymbol(raylibSymbols, "initwindow", OpCode::InitWindow);
	MakeSymbol(raylibSymbols, "windowshouldclose", OpCode::WindowShouldClose);
	MakeSymbol(raylibSymbols, "closewindow", OpCode::CloseWindow);
	MakeSymbol(raylibSymbols, "showcursor", OpCode::ShowCursor);
	MakeSymbol(raylibSymbols, "hidecursor", OpCode::HideCursor);
	MakeSymbol(raylibSymbols, "clearbackground", OpCode::ClearBackground);
	MakeSymbol(raylibSymbols, "begindrawing", OpCode::BeginDrawing);
	MakeSymbol(raylibSymbols, "enddrawing", OpCode::EndDrawing);
	MakeSymbol(raylibSymbols, "settargetfps", OpCode::SetTargetFPS);
	MakeSymbol(raylibSymbols, "gettime", OpCode::GetTime);
	MakeSymbol(raylibSymbols, "getrandomvalue", OpCode::GetRandomValue);
	// TODO: save in loaded file's directory when implemented. save next to exe when in REPL.
	// MakeSymbol(raylibSymbols, "loadstoragevalue", OpCode::LoadStorageValue);
	// MakeSymbol(raylibSymbols, "savestoragevalue", OpCode::SaveStorageValue);
	MakeSymbol(raylibSymbols, "iskeypressed", OpCode::IsKeyPressed);
	MakeSymbol(raylibSymbols, "iskeydown", OpCode::IsKeyDown);
	MakeSymbol(raylibSymbols, "iskeyreleased", OpCode::IsKeyReleased);
	MakeSymbol(raylibSymbols, "iskeyup", OpCode::IsKeyUp);
	MakeSymbol(raylibSymbols, "getkeypressed", OpCode::GetKeyPressed);
	MakeSymbol(raylibSymbols, "setexitkey", OpCode::SetExitKey);
	MakeSymbol(raylibSymbols, "ismousebuttonpressed", OpCode::IsMouseButtonPressed);
	MakeSymbol(raylibSymbols, "ismousebuttondown", OpCode::IsMouseButtonDown);
	MakeSymbol(raylibSymbols, "ismousebuttonreleased", OpCode::IsMouseButtonReleased);
	MakeSymbol(raylibSymbols, "ismousebuttonup", OpCode::IsMouseButtonUp);
	MakeSymbol(raylibSymbols, "getmousex", OpCode::GetMouseX);
	MakeSymbol(raylibSymbols, "getmousey", OpCode::GetMouseY);
	MakeSymbol(raylibSymbols, "getmousepos", OpCode::GetMousePosition);
	MakeSymbol(raylibSymbols, "setmousepos", OpCode::SetMousePosition);
	MakeSymbol(raylibSymbols, "setmouseoffset", OpCode::SetMouseOffset);
	MakeSymbol(raylibSymbols, "setmousescale", OpCode::SetMouseScale);
	MakeSymbol(raylibSymbols, "getmousewheel", OpCode::GetMouseWheelMove);
#pragma endregion

#pragma region SHAPES MODULE
	MakeSymbol(raylibSymbols, "drawpixel", OpCode::DrawPixel);
	MakeSymbol(raylibSymbols, "drawline", OpCode::DrawLine);
	// MakeSymbol(raylibSymbols, "drawlinestrip", OpCode::DrawLineStrip);
	MakeSymbol(raylibSymbols, "drawcircle", OpCode::DrawCircle);
	// MakeSymbol(raylibSymbols, "drawcirclesector", OpCode::DrawCircleSector);
	// MakeSymbol(raylibSymbols, "drawcirclesectorlines", OpCode::DrawCircleSectorLines);
	MakeSymbol(raylibSymbols, "drawcirclegradient", OpCode::DrawCircleGradient);
	MakeSymbol(raylibSymbols, "drawcirclelines", OpCode::DrawCircleLines);
	MakeSymbol(raylibSymbols, "drawellipse", OpCode::DrawEllipse);
	MakeSymbol(raylibSymbols, "drawellipselines", OpCode::DrawEllipseLines);
	// MakeSymbol(raylibSymbols, "drawring", OpCode::DrawRing);
	// MakeSymbol(raylibSymbols, "drawringlines", OpCode::DrawRingLines);
	MakeSymbol(raylibSymbols, "drawrectangle", OpCode::DrawRectangle);
	// MakeSymbol(raylibSymbols, "drawrectanglegradient", OpCode::DrawRectangleGradient);
	MakeSymbol(raylibSymbols, "drawrectanglelines", OpCode::DrawRectangleLines);
	// MakeSymbol(raylibSymbols, "drawrectanglerounded", OpCode::DrawRectangleRounded);
	// MakeSymbol(raylibSymbols, "drawrectangleroundedlines", OpCode::DrawRectangleRoundedLines);
	MakeSymbol(raylibSymbols, "drawtriangle", OpCode::DrawTriangle);
	MakeSymbol(raylibSymbols, "drawtrianglelines", OpCode::DrawTriangleLines);
	// MakeSymbol(raylibSymbols, "drawtrianglefan", OpCode::DrawTriangleFan);
	// MakeSymbol(raylibSymbols, "drawtrianglestrip", OpCode::DrawTriangleStrip);
	// MakeSymbol(raylibSymbols, "drawpolygon", OpCode::DrawPolygon);
	// MakeSymbol(raylibSymbols, "drawpolygonlines", OpCode::DrawPolygonLines);
	// MakeSymbol(raylibSymbols, "checkcollisionrectangle", OpCode::CheckCollisionRectangle);
	// MakeSymbol(raylibSymbols, "checkcollisioncircle", OpCode::CheckCollisionCircle);
	// MakeSymbol(raylibSymbols, "checkcollisioncirclerectangle", OpCode::CheckCollisionCircleRectangle);
	// MakeSymbol(raylibSymbols, "getcollisionrectangle", OpCode::GetCollisionRectangle);
	// MakeSymbol(raylibSymbols, "checkcollisionpointrectangle", OpCode::CheckCollisionPointRectangle);
	// MakeSymbol(raylibSymbols, "checkcollisionpointcircle", OpCode::CheckCollisionPointCircle);
	// MakeSymbol(raylibSymbols, "checkcollisionpointtriangle", OpCode::CheckCollisionPointTriangle);
#pragma endregion

#pragma region TEXT MODULE
	MakeSymbol(raylibSymbols, "drawfps", OpCode::DrawFPS);
	MakeSymbol(raylibSymbols, "drawtext", OpCode::DrawText);
	MakeSymbol(raylibSymbols, "measuretext", OpCode::MeasureText);
#pragma endregion

#pragma region CONSTANTS
	MakeSymbol(raylibSymbols, "key_a", static_cast<long>(KEY_A));
	MakeSymbol(raylibSymbols, "key_b", static_cast<long>(KEY_B));
	MakeSymbol(raylibSymbols, "key_c", static_cast<long>(KEY_C));
	MakeSymbol(raylibSymbols, "key_d", static_cast<long>(KEY_D));
	MakeSymbol(raylibSymbols, "key_e", static_cast<long>(KEY_E));
	MakeSymbol(raylibSymbols, "key_f", static_cast<long>(KEY_F));
	MakeSymbol(raylibSymbols, "key_g", static_cast<long>(KEY_G));
	MakeSymbol(raylibSymbols, "key_h", static_cast<long>(KEY_H));
	MakeSymbol(raylibSymbols, "key_i", static_cast<long>(KEY_I));
	MakeSymbol(raylibSymbols, "key_j", static_cast<long>(KEY_J));
	MakeSymbol(raylibSymbols, "key_k", static_cast<long>(KEY_K));
	MakeSymbol(raylibSymbols, "key_l", static_cast<long>(KEY_L));
	MakeSymbol(raylibSymbols, "key_m", static_cast<long>(KEY_M));
	MakeSymbol(raylibSymbols, "key_n", static_cast<long>(KEY_N));
	MakeSymbol(raylibSymbols, "key_o", static_cast<long>(KEY_O));
	MakeSymbol(raylibSymbols, "key_p", static_cast<long>(KEY_P));
	MakeSymbol(raylibSymbols, "key_q", static_cast<long>(KEY_Q));
	MakeSymbol(raylibSymbols, "key_r", static_cast<long>(KEY_R));
	MakeSymbol(raylibSymbols, "key_s", static_cast<long>(KEY_S));
	MakeSymbol(raylibSymbols, "key_t", static_cast<long>(KEY_T));
	MakeSymbol(raylibSymbols, "key_u", static_cast<long>(KEY_U));
	MakeSymbol(raylibSymbols, "key_v", static_cast<long>(KEY_V));
	MakeSymbol(raylibSymbols, "key_w", static_cast<long>(KEY_W));
	MakeSymbol(raylibSymbols, "key_x", static_cast<long>(KEY_X));
	MakeSymbol(raylibSymbols, "key_y", static_cast<long>(KEY_Y));
	MakeSymbol(raylibSymbols, "key_z", static_cast<long>(KEY_Z));
	MakeSymbol(raylibSymbols, "mouse_left", static_cast<long>(MOUSE_LEFT_BUTTON));
	MakeSymbol(raylibSymbols, "mouse_middle", static_cast<long>(MOUSE_MIDDLE_BUTTON));
	MakeSymbol(raylibSymbols, "mouse_right", static_cast<long>(MOUSE_RIGHT_BUTTON));
#pragma endregion
#endif

	for (auto& [symbol, _chunk] : symbols)
	{
		if (symbol == "!main") { continue; }
#ifndef EXCLUDE_RAYLIB
		if (locs.find(symbol) != locs.end() || raylibSymbols.find(symbol) != raylibSymbols.end())
#else
		if (locs.find(symbol) != locs.end())
#endif
		{
			std::cerr << "Function '" << symbol << "' already exists\n";
			success = false;
		}
		else
		{
			locs[symbol] = chunk.Size();

			for (size_t i = 0; i < _chunk.Size(); i++)
			{
				chunk.Write(_chunk.Read(i), _chunk.ReadLine(i));
				if (_chunk.GetMeta(i)) { chunk.AddMeta(locs[symbol] + i, *_chunk.GetMeta(i)); }
			}
		}
	}

	for (size_t i = 0; i < chunk.Size(); i++)
	{
		if (chunk.GetMeta(i) && chunk.GetMeta(i)->Get<std::string>())
		{
			const std::string* meta = chunk.GetMeta(i)->Get<std::string>();
			if (*meta == "!constant")
			{
				if (chunk.GetMeta(i + 1))
				{
					chunk.ModifyConstant(i + 1, *chunk.GetMeta(i + 1));
					i++;
				}
			}
			else if (locs.find(*meta) != locs.end())
			{
				chunk.ModifyLong(i, locs[*meta] - i - 2);
			}
#ifndef EXCLUDE_RAYLIB
			else if (raylibSymbols.find(*meta) != raylibSymbols.end())
			{
				locs[*meta] = chunk.Size();
				for (size_t j = 0; j < raylibSymbols[*meta].Size(); j++)
				{
					chunk.Write(raylibSymbols[*meta].Read(j), raylibSymbols[*meta].ReadLine(j));
					if (raylibSymbols[*meta].GetMeta(j)) { chunk.AddMeta(locs[*meta] + j, *raylibSymbols[*meta].GetMeta(j)); }
				}
				chunk.ModifyLong(i, locs[*meta] - i - 2);
			}
#endif
			else
			{
				std::cerr << "Undefined function '" << *meta << "'\n";
				success = false;
			}
		}
	}

#ifndef NDEBUG
	std::cerr << '\n';
	chunk.Disassemble("code");
#endif

	return success ? BuildResult::Ok : BuildResult::LinkerError;
}

bool Linker::MakeSymbol(std::map<std::string, Chunk>& symbols, const std::string& name, OpCode opcode)
{
	if (opcode <= OpCode::Return)
	{
		std::cerr << "Non-raylib opcodes cannot be made into a symbol\n";
		return false;
	}
	else if (symbols.find(name) != symbols.end())
	{
		std::cerr << "Symbol '" << name << "' already exists\n";
		return false;
	}
	else
	{
		symbols[name].Write(opcode, -1);
		symbols[name].Write(OpCode::JumpToCallStackAddress, -1);
		return true;
	}
}

bool Linker::MakeSymbol(std::map<std::string, Chunk>& symbols, const std::string& name, Value value)
{
	if (symbols.find(name) != symbols.end())
	{
		std::cerr << "Symbol '" << name << "' already exists\n";
		return false;
	}
	else
	{
		using namespace std::string_literals;

		size_t ret = symbols[name].AddConstant(value, -1, OpCode::Constant, OpCode::ConstantLong);
		symbols[name].AddMeta(ret, "!constant"s);
		symbols[name].AddMeta(ret + 1, value);
		symbols[name].Write(OpCode::JumpToCallStackAddress, -1);
		return true;
	}
}