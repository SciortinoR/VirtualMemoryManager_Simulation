#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "Process.h"
#include "VirtualMemoryManager.h"

void runProcessTasks(Process& process, VirtualMemoryManager& vmm, std::fstream& commands)
{
	// Prepare variables
	unsigned int value;
	std::string line;
	std::string task;
	std::stringstream ss;
	std::string variableId;
	std::getline(commands, line);

	// Determine task to run
	ss.str(line);
	ss >> task;
	std::transform(task.begin(), task.end(), task.begin(), ::tolower);
	if (task == std::string("store"))
	{
		ss >> variableId;
		ss >> value;
		std::thread vmm_thread(&VirtualMemoryManager::store, &vmm, variableId, value);
	}
	else if (task == std::string("release"))
	{
		ss >> variableId;
		std::thread vmm_thread(&VirtualMemoryManager::release, &vmm, variableId);
	}
	else
	{
		ss >> variableId;
		std::thread vmm_thread(&VirtualMemoryManager::lookup, &vmm, variableId);
	}
}

int main(int argc, char* argv[])
{
	// Create input/output streams
	std::ifstream commands(argv[1] + std::string("\\commands.txt"), std::ios::in);
	std::ifstream memconfig(argv[1] + std::string("\\memconfig.txt"), std::ios::in);
	std::ifstream processes(argv[1] + std::string("\\processes.txt"), std::ios::in);
	std::fstream output(argv[2] + std::string("\\output.txt"), std::ios::trunc | std::ios::out);

	// Check if all files opened properly
	ASSERT((commands && memconfig && processes && output), "I/O ERROR: Could not access I/O files");

	// Initializing VMM
	std::string line;
	std::getline(memconfig, line);
	VirtualMemoryManager vmm(std::stoi(line), argv[1] + std::string("\\vm.txt"));

	// Initialize Processes
	std::stringstream ss;
	std::vector<int> tokens(2);
	std::getline(processes, line);
	int num_processes = std::stoi(line);
	std::vector<Process> process_list(num_processes);
	for (int i = 0; i < num_processes; ++i)
	{
		std::getline(processes, line);
		ss.str(line);
		while (ss >> line)
		{
			tokens.push_back(std::stoi(line));
		}
		process_list.emplace_back(i, tokens[0], tokens[1]);
	}

	// Sort Process list in ascending start time order
	std::sort(process_list.begin(), process_list.end(), [](const auto& lhs, const auto& rhs)
	{
		return lhs.getStartTime() < rhs.getStartTime();
	});

	// Run Process Commands in seperate threads
	std::vector<std::thread> process_threads(num_processes);
	for (auto& process : process_list)
	{
		process_threads.emplace_back(runProcessTasks, std::ref(process), std::ref(vmm), std::ref(commands));
	}

	return 0;
}
