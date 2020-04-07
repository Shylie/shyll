#pragma once

#include <map>
#include <string>
#include <vector>

#include "value.h"

enum class OpCode : uint8_t
{
	None,
	AsDouble,
	AsLong,
	AsString,
	Constant,
	ConstantLong,
	Add,
	Subtract,
	Multiply,
	Divide,
	Exponent,
	LessThan,
	LessThanEqual,
	GreaterThan,
	GreaterThanEqual,
	Equal,
	NotEqual,
	LogicalAnd,
	LogicalOr,
	LogicalNot,
	Duplicate,
	Pop,
	Print,
	PrintLn,
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
	JumpToCallStackAddress,
	PushJumpAddress,
	Return,
#pragma region RAYLIB OPCODES
#pragma region CORE MODULE
	InitWindow,
	WindowShouldClose,
	CloseWindow,
	ShowCursor,
	HideCursor,
	ClearBackground,
	BeginDrawing,
	EndDrawing,
	SetTargetFPS,
	GetTime,
	GetRandomValue, // TODO: Implement
	LoadStorageValue, // TODO: Implement
	SaveStorageValue,
	IsKeyPressed,
	IsKeyDown,
	IsKeyReleased,
	IsKeyUp,
	GetKeyPressed,
	SetExitKey,
	IsMouseButtonPressed,
	IsMouseButtonDown,
	IsMouseButtonReleased,
	IsMouseButtonUp,
	GetMouseX,
	GetMouseY,
	GetMousePosition,
	SetMousePosition,
	SetMouseOffset,
	SetMouseScale,
	GetMouseWheelMove,
#pragma endregion
#pragma region SHAPES MODULE
	DrawPixel,
	DrawLine,
	DrawLineStrip, // TODO: Implement
	DrawCircle,
	DrawCircleSector, // TODO: Implement
	DrawCircleSectorLines, // TODO: Implement
	DrawCircleGradient, // TODO: Implement
	DrawCircleLines,
	DrawEllipse,
	DrawEllipseLines,
	DrawRing, // TODO: Implement
	DrawRingLines, // TODO: Implement
	DrawRectangle,
	DrawRectangleGradient, // TODO: Implement
	DrawRectangleLines,
	DrawRectangleRounded, // TODO: Implement
	DrawRectangleRoundedLines, // TODO: Implement
	DrawTriangle,
	DrawTriangleLines,
	DrawTriangleFan, // TODO: Implement
	DrawTriangleStrip, // TODO: Implement
	DrawPolygon, // TODO: Implement
	DrawPolygonLines, // TODO: Implement
	CheckCollisionRectangle, // TODO: Implement
	CheckCollisionCircle, // TODO: Implement
	CheckCollisionCircleRectangle, // TODO: Implement
	GetCollisionRectangle, // TODO: Implement
	CheckCollisionPointRectangle, // TODO: Implement
	CheckCollisionPointCircle, // TODO: Implement
	CheckCollisionPointTriangle, // TODO: Implement
#pragma endregion
#pragma region TEXT MODULE
	DrawFPS, // TODO: Implement
	DrawText, // TODO: Implement
	MeasureText // TODO: Implement
#pragma endregion
#pragma endregion
};

class Chunk
{
public:
	size_t Write(uint8_t instruction, int line);
	size_t Write(OpCode instruction, int line);
	size_t WriteLong(uint16_t instruction, int line);
	size_t AddConstant(const Value& value, int line, OpCode ifShort, OpCode ifLong, bool& success);
	size_t AddConstant(const Value& value, int line, OpCode ifShort, OpCode ifLong);

	void AddMeta(size_t offset, const Value& metadata);
	const Value* GetMeta(size_t offset) const;

	void Modify(size_t offset, uint8_t nval);
	void ModifyLong(size_t offset, uint16_t nval);
	void ModifyConstant(size_t offset, const Value& value);

	uint8_t Read(size_t offset) const;
	uint16_t ReadLong(size_t offset) const;
	Value ReadConstant(uint16_t index) const;
	int ReadLine(size_t offset) const;

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

	size_t SimpleInstruction(const std::string& name, size_t offset) const;
	size_t ConstantInstruction(const std::string& name, size_t offset) const;
	size_t ConstantInstructionLong(const std::string& name, size_t offset) const;
	size_t JumpInstruction(const std::string& name, size_t offset) const;

	std::vector<uint8_t> instructions;
	std::vector<Value> values;
	std::vector<LineInfo> lines;
	std::map<size_t, Value> meta;
};