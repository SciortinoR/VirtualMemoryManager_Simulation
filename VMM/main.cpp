#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <future>

#include "Process.h"
#include "VirtualMemoryManager.h"
#include "Variable.h"

int system_clock;
std::stringstream system_log;
std::mutex m, read, run, start;
std::condition_variable cv;

int randomTime()
{
	return rand() % 1000 + 1;
}

void log(const VirtualMemoryManager& vmm = VirtualMemoryManager(), const Process& process = Process(),
	const std::string& task = std::string(), const Variable& variable = Variable())
{
	if (!process.getStatus().empty())
	{
		system_log << "Clock: ";
		if (task.empty() && (process.getStatus() == Process::started))
		{
			system_log << process.getStartTime() << ", Process " << process.getId() << ": " << process.getStatus() << "." << std::endl;
		}
		else if (process.getStatus() == Process::finished)
		{
			system_log << process.getEndTime() << ", Process " << process.getId() << ": " << process.getStatus() << "." << std::endl;
		}
		else 
		{
			system_log << system_clock << ", Process " << process.getId() << ", " << task << ": Variable " << variable.getId();
			if (task != std::string("Release"))
			{
				system_log << ", Value: " << variable.getValue() << std::endl;
			}
			else
			{
				system_log << std::endl;
			}
		}
	}
	else
	{
		system_log << vmm.getSwapLog().rdbuf();
	}
}

void runProcessTasks(Process& process, VirtualMemoryManager& vmm, std::ifstream& commands)
{
	// Prepare variables
	unsigned int value;
	std::string line, task, variableId;
	std::stringstream ss;

	while (1)
	{
		{
			// Will wait until process is notified of new system clock
			std::unique_lock<std::mutex> lk(m);
			while (process.getStartTime() > system_clock)
			{
				cv.wait(lk);
			}
		}

		// Access commands one at a time
		read.lock();
		if (!std::getline(commands, line))
		{
			// Checks for EOF (no more Process tasks)
			process.setStatus(Process::finished);
			log(vmm, process);
			read.unlock();
			return;
		}
		read.unlock();

		// Seperate stream tokens
		ss.str(line);
		ss.clear();
		ss >> task >> variableId;
		std::transform(task.begin(), task.end(), task.begin(), ::tolower);
		task[0] = toupper(task[0]);


		// Log process if just started
		if (process.getStatus() == Process::pending)
		{
			process.setStatus(Process::started);
			log(vmm, process);

			// Sleep 10 milliseconds to simulate start & notify other threads of new system time
			std::lock_guard<std::mutex> lk(m);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			system_clock += 10;
			cv.notify_all();
		}

		// Determine which task to run & log Process activity
		run.lock();
		std::thread vmm_thread;
		std::future<long> lookup_value;
		if (task == std::string("Store"))
		{
			ss >> value;
			vmm_thread = std::thread(&VirtualMemoryManager::store, &vmm, variableId, value);
			vmm_thread.join();
			log(vmm, process, task, Variable(variableId, value, 0));
		}
		else if (task == std::string("Release"))
		{
			vmm_thread = std::thread(&VirtualMemoryManager::release, &vmm, variableId);
			vmm_thread.join();
			log(vmm, process, task, Variable(variableId, 0, 0));
		}
		else if (task == std::string("Lookup"))
		{
			lookup_value = std::async(&VirtualMemoryManager::lookup, &vmm, variableId);
			long val = lookup_value.get();
			log(vmm, process, task, Variable(variableId, val, 0));
		}
		else
		{
			std::clog << "WARNING (Process " << process.getId() << "): Could not determine task to run.";
			continue;
		}
		run.unlock();

		{
			// Log VMM activity if Swap occured and notify other threads of new system time
			std::lock_guard<std::mutex> lk(m);
			int wait_time = randomTime();
			system_clock += wait_time;
			cv.notify_all();
			std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
			log(vmm);
		}

		// Log process if ended
		if (process.getEndTime() <= system_clock)
		{
			process.setStatus(Process::finished);
			log(vmm, process);
			return;
		}
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
		ss >> start >> duration;
		process_list.emplace_back(i + 1, std::stoi(start), std::stoi(duration));
	}

	//// Sort Process list in ascending start time order
	//std::sort(process_list.begin(), process_list.end(), [](const auto& lhs, const auto& rhs)
	//{
	//	return lhs.getStartTime() < rhs.getStartTime();
	//});

	return process_list;
}
int main(int argc, char* argv[])
{
	// Initialize system clock to 1ms & seed random number generator
	system_clock = 1000;
	srand(1);

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

	// Wait for all threads to complete before termnating
	for (auto& thread : process_threads)
	{
		thread.join();
	}

	return 0;
}
