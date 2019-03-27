#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "Process.h"
#include "VirtualMemoryManager.h"

int current_system_time;

void runProcessTasks(Process& process, VirtualMemoryManager& vmm, std::ifstream& commands)
{
	// Prepare variables
	unsigned int value;
	std::string line, task;
	std::stringstream ss;
	std::string variableId;
	std::getline(commands, line);

	// Seperate stream tokens
	ss.str(line);
	ss >> task;
	ss >> variableId;
	std::transform(task.begin(), task.end(), task.begin(), ::tolower);

	// Determine which task to run
	if (task == std::string("store"))
	{
		ss >> value;
		std::thread vmm_thread(&VirtualMemoryManager::store, &vmm, variableId, value);
	}
	else if (task == std::string("release"))
	{
		std::thread vmm_thread(&VirtualMemoryManager::release, &vmm, variableId);
	}
	else if (task == std::string("lookup"))
	{
		std::thread vmm_thread(&VirtualMemoryManager::lookup, &vmm, variableId);
	}
	else
	{
		std::clog << "WARNING (Process): Could not determine task to run.";
	}
}

std::vector<Process> initProcesses(std::ifstream& processes)
{
	// Prepare variables
	std::string line, start, duration;
	std::stringstream ss;
	std::getline(processes, line);
	int num_processes = std::stoi(line);

	// Vectors
	std::vector<Process> process_list;
	process_list.reserve(num_processes);

	// Splits lines into tokens and generates Process list
	for (int i = 0; i < num_processes; ++i)
	{
		std::getline(processes, line);
		ss.str(line);
		ss.clear();
		ss >> start;
		ss >> duration;
		process_list.emplace_back(i, std::stoi(start), std::stoi(duration));
	}

	// Sort Process list in ascending start time order
	std::sort(process_list.begin(), process_list.end(), [](const auto& lhs, const auto& rhs)
	{
		return lhs.getStartTime() < rhs.getStartTime();
	});

	return process_list;
}
int main(int argc, char* argv[])
{
	// Initialize system clock to 1ms
	current_system_time = 1000;

	// Define input/output streams
	std::ifstream commands(argv[1] + std::string("\\commands.txt"), std::ios::in);
	std::ifstream memconfig(argv[1] + std::string("\\memconfig.txt"), std::ios::in);
	std::ifstream processes(argv[1] + std::string("\\processes.txt"), std::ios::in);
	std::fstream output(argv[2] + std::string("\\output.txt"), std::ios::trunc | std::ios::out);

	// Check if all files opened properly
	ASSERT((!commands || !memconfig || !processes || !output), "I/O ERROR: Could not access I/O files");

	// Initializing VMM
	std::string line;
	std::getline(memconfig, line);
	VirtualMemoryManager vmm(std::stoi(line), argv[1] + std::string("\\vm.txt"));

	// Initialize Processes
	std::vector<Process> process_list = initProcesses(processes);

	// Run Process commands in seperate threads
	std::vector<std::thread> process_threads;
	process_threads.reserve(process_list.size());
	for (auto& process : process_list)
	{
		process_threads.emplace_back(runProcessTasks, std::ref(process), std::ref(vmm), std::ref(commands));
	}

	return 0;
}
