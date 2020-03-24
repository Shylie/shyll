#include "linker.h"

Linker::Linker(const std::string& source) : compiler(source)
{

}

bool Linker::Link(Chunk& chunk)
{
	bool success;

	std::map<std::string, Chunk>& symbols = compiler.Compile(success);
	if (!success) { return false; }

	locs.clear();

	chunk = symbols["main"];

	for (auto& [symbol, _chunk] : symbols)
	{
		if (symbol == "main") { continue; }
		locs[symbol] = chunk.Size();

		for (size_t i = 0; i < _chunk.Size(); i++)
		{
			chunk.Write(_chunk.Read(i), _chunk.ReadLine(i));
			if (_chunk.GetMeta(i)) { chunk.AddMeta(locs[symbol] + i, *_chunk.GetMeta(i)); }
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
				}
			}
			else if (locs.find(*meta) != locs.end())
			{
				chunk.ModifyLong(i, locs[*meta] - i - 2);
			}
		}
	}

#ifdef _DEBUG
	chunk.Disassemble("code");
#endif

	return true;
}