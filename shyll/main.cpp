#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "vm.h"

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		std::ifstream file(argv[1]);

		if (file.good())
		{
			std::stringstream out;
			out << file.rdbuf();

			VM* vm = new VM();

			InterpretResult result = vm->Interpret(out.str());

			switch (result)
			{
			case InterpretResult::Ok:

				delete vm;
				return 0;

			case InterpretResult::CompileError:
				std::cout << "Compilation error";

				delete vm;
				return 1;

			case InterpretResult::RuntimeError:
				std::cout << "Runtime error: " << vm->ErrorMessage();
				delete vm;
				return 2;
			}
		}
		else
		{
			std::cout << "Unable to open file '" << argv[1] << '\'';
			return -1;
		}
	}
	else
	{
		VM* vm = new VM();
		for (;;)
		{
			std::string line;

			std::cout << ">> ";
			std::getline(std::cin, line);
			std::cout << '\n';

			if (line == "quit") { break; }

			switch (vm->Interpret(line))
			{
			case InterpretResult::Ok:
				break;

			case InterpretResult::CompileError:
				std::cout << "\nCompilation Error.\n";
				break;

			case InterpretResult::RuntimeError:
				std::cout << "\nRuntime Error: " << vm->ErrorMessage() << '\n';
				break;
			}
			std::cout << '\n';
		}
		delete vm;

		return 0;
	}
}