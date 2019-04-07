#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <future>
#include <random>

#include "Process.h"
#include "VirtualMemoryManager.h"
#include "Variable.h"
#include "Clock.h"

// System Variables
Clock system_clock;
std::mutex m;
std::shared_timed_mutex log_m;
std::condition_variable cv;
std::stringstream system_log;

// Random Number generator
std::random_device rd;									// Get random seed from the OS entropy device
std::mt19937_64 eng(rd());								// Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy
std::uniform_int_distribution<int> dist(750, 2000);		// Define distribution of random numbers

int randomTime()
{
	return dist(eng);
}

void logVMM(const std::stringstream& swap_log)
{
	//	Sync VMM logger with System logger
	std::unique_lock<std::shared_timed_mutex> log_lk(log_m);

	if (!swap_log.str().empty())
	{
		system_log << swap_log.rdbuf();
	}
	system_log.clear();
}

void logProcess(const Process& process, int time = 0, const std::string& task = std::string(), const Variable& variable = Variable())
{
	// Sync system logger with VMM logger
	std::unique_lock<std::shared_timed_mutex> log_lk(log_m);

	system_log << "Clock: ";
	if (task.empty() && (process.getStatus() == Process::started))
	{
		system_log << system_clock.getTime() << ", Process " << process.getId() << ": " << process.getStatus() << "." << std::endl;
	}
	else if (process.getStatus() == Process::finished)
	{
		if (system_clock.getTime() < process.getEndTime())
		{
			system_log << system_clock.getTime();
		}
		else 
		{
			system_log << process.getEndTime();
		}
		system_log << ", Process " << process.getId() << ": " << process.getStatus() << "." << std::endl;
	}
	else 
	{
		system_log << time << ", Process " << process.getId() << ", " << task << ": Variable " << variable.getId();
		if (task != std::string("Release"))
		{
			system_log << ", Value: " << variable.getValue() << std::endl;
		}
		else
		{
			system_log << std::endl;
		}
	}
	system_log.clear();
}

void runProcessTasks(Process& process, VirtualMemoryManager& vmm, std::ifstream& commands)
{
	// Prepare variables
	unsigned int value;
	std::string line, task, variableId;
	std::stringstream ss;

	// Keep running Process as long as not finished
	while (process.getEndTime() > system_clock.getTime())
	{
		{
			// Process wait until start time or clock finished status
			std::unique_lock<std::mutex> time_lk(m);
			cv.wait(time_lk, [&process] { return ((process.getStartTime() <= system_clock.getTime()) || (system_clock.getFinishedStatus())); });
		}

		{
			// Access commands one at a time
			std::scoped_lock<std::mutex> read_lk(m);
			if (!std::getline(commands, line))
			{
				// Tasks all completed
				system_clock.setFinishedStatus(true);
				break;
			}
		}

		// Log process if just started
		if (process.getStatus() == Process::pending)
		{
			// Start & notify other threads of new system time
			process.setStatus(Process::started);
			logProcess(process);
		}

		// Seperate stream tokens
		ss.str(line);
		ss.clear();
		ss >> task >> variableId;
		std::transform(task.begin(), task.end(), task.begin(), ::tolower);
		task[0] = toupper(task[0]);

		{
			// Determine which task to run & log Process activity
			std::unique_lock<std::mutex> run_lk(m);
			std::thread vmm_thread;
			std::future<long> lookup_value;
			int start_time = system_clock.getTime();
			if (task == std::string("Store"))
			{
				ss >> value;
				vmm_thread = std::thread(&VirtualMemoryManager::store, &vmm, variableId, value);
				vmm_thread.join();
				run_lk.unlock();
				logProcess(process, start_time, task, Variable(variableId, value, 0));
			}
			else if (task == std::string("Release"))
			{
				vmm_thread = std::thread(&VirtualMemoryManager::release, &vmm, variableId);
				vmm_thread.join();
				run_lk.unlock();
				logProcess(process, start_time, task, Variable(variableId, 0, 0));
			}
			else if (task == std::string("Lookup"))
			{
				lookup_value = std::async(&VirtualMemoryManager::lookup, &vmm, variableId);
				long val = lookup_value.get();
				run_lk.unlock();
				logProcess(process, start_time, task, Variable(variableId, val, 0));
			}
			else
			{
				std::clog << "WARNING (Process " << process.getId() << "): Could not determine task to run.";
				continue;
			}
		}

		// Log VMM
		logVMM(vmm.getSwapLog());

		// Sleep for random time until next command
		std::this_thread::sleep_for(std::chrono::milliseconds(randomTime()));
	}

	// Finished process
	if (process.getStatus() != Process::pending)
	{
		process.setStatus(Process::finished);
		logProcess(process);
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

	return process_list;
}
int main(int argc, char* argv[])
{
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

	// Initialize Processes and threads list
	std::vector<Process> process_list = initProcesses(processes);
	std::vector<std::thread> process_threads;
	process_threads.reserve(process_list.size());

	// Start clock & run Process commands in seperate threads
	system_clock.startClock();
	for (auto& process : process_list)
	{
		process_threads.emplace_back(runProcessTasks, std::ref(process), std::ref(vmm), std::ref(commands));
	}

	// Wait for all threads to complete before termnating
	for (auto& thread : process_threads)
	{
		thread.join();
	}

	// Write contents of system log to output file
	output << system_log.rdbuf();

	return 0;
}
