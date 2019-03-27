#pragma once

#include <vector>
#include <fstream>
#include <string>

#include "Variable.h"

// Assertion for accessing Main/Disk Memory
#ifndef NDEBUG
#	define ASSERT(condition, message) \
	do { \
		if (condition) { \
			std::cerr << "Assertion '" #condition "' failed in " << __FILE__ \
				<< " line " << __LINE__ << ": " << message << std::endl; \
			std::terminate(); \
		} \
	} while (false) 
#else 
#	define ASSERT(condition, message) do { } while (false)
#endif

// Current system time accessible to main and vmm
extern int current_system_time;

class VirtualMemoryManager
{
public:

	// Constructors/Destructors
	VirtualMemoryManager(int pages, std::string filepath);
	~VirtualMemoryManager();

	// Getters
	int getMemoryPages() const;

	// Process API functions
	void store(std::string variableId, unsigned int value);
	void release(std::string variableId);
	long lookup(std::string variableId);

	// Memory Management functions
	long searchAllMemory(std::string variableId, int functionId = 0);
	void swap(const Variable& variable, const std::vector<std::string>& disk_buffer = std::vector<std::string>());
	void writeDisk(const std::vector<std::string>& disk_buffer, const Variable& variable = Variable());

private:
	
	// Member Variables
	const int memory_pages;
	std::string vm_path;
	std::fstream disk_memory;
	std::vector<Variable> main_memory;
};

