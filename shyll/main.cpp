#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "vm.h"

int main(int argc, char** argv)
{
	if (argc >= 2)
	{
		std::map<std::string, std::string> sources;
		for (size_t i = 1; i < argc; i++)
		{
			std::ifstream file(argv[i]);
			if (file.good())
			{
				std::stringstream out;
				out << file.rdbuf();
				sources[argv[i]] = out.str();
			}
			else
			{
				std::cerr << "Unable to open file '" << argv[1] << '\'';
				return -1;
			}
		}
		VM* vm = new VM();

		std::cerr << '\n';

		InterpretResult result = vm->Interpret(sources);

		switch (result)
		{
		case InterpretResult::Ok:

			delete vm;
			return 0;

		case InterpretResult::CompileError:
			std::cerr << "Compilation error:";
			delete vm;
			return 1;

		case InterpretResult::LinkerError:
			std::cerr << "Linking error:";
			delete vm;
			return 2;

		case InterpretResult::RuntimeError:
			std::cerr << "Runtime error:\n" << vm->ErrorMessage();
			delete vm;
			return 3;
		}
	}
	else
	{
		VM* vm = new VM();
		for (;;)
		{
			std::string line;
			std::string code;
			bool confirm = false;

			std::cerr << "\n>| ";
			for (;;)
			{
				std::getline(std::cin, line);
				if (line.empty())
				{
					if (confirm || !code.empty())
					{
						break;
					}
					else
					{
						confirm = true;
						std::cerr << " | ";
						continue;
					}
				}
				std::cerr << " | ";
				code += line + '\n';
			}

			std::cerr << '\n';

			if (code.empty()) { break; }

			switch (vm->Interpret(code))
			{
			case InterpretResult::Ok:
				break;

			case InterpretResult::CompileError:
				std::cerr << "\nCompilation Error.\n";
				break;

			case InterpretResult::LinkerError:
				std::cerr << "\nLinking Error.\n";
				break;

			case InterpretResult::RuntimeError:
				std::cerr << "\nRuntime Error:\n" << vm->ErrorMessage() << '\n';
				vm->Cleanup(false);
				break;
			}
			std::cerr << '\n';
		}
		delete vm;

		return 0;
	}
}