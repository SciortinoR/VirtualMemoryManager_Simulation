#include <iostream>
#include <thread>
#include <fstream>
#include <string>

#include "Process.h"
#include "VirtualMemoryManager.h"

int main(int argc, char* argv[])
{
	// Create input/output streams
	std::fstream commands(argv[1] + std::string("\\commands.txt"), std::ios::in);
	std::fstream memconfig(argv[1] + std::string("\\memconfig.txt"), std::ios::in);
	std::fstream processes(argv[1] + std::string("\\processes.txt"), std::ios::in);
	std::fstream output(argv[2] + std::string("\\output.txt"), std::ios::trunc | std::ios::out);
	std::fstream vm(argv[1] + std::string("\\vm.txt"), std::ios::trunc | std::ios::in | std::ios::out);

	Process(2, 2, 2);
	VirtualMemoryManager(2, vm);

	return 0;
}
