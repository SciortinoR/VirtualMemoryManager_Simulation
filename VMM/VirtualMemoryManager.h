#pragma once

#include <vector>
#include <fstream>
#include <string>

#ifndef NDEBUG
#	define ASSERT(condition, message) \
	do { \
		if (!condition) { \
			std::cerr << "Assertion '" #condition "' failed in " << __FILE__ \
				<< " line " << __LINE__ << ": " << message << std::endl; \
			std::terminate(); \
		} \
	} while (false) 
#else 
#	define ASSERT(condition, message) do { } while (false)
#endif

class VirtualMemoryManager
{
public:

	// Constructors/Destructors
	VirtualMemoryManager(int pages, std::string filepath);
	~VirtualMemoryManager();

	// Getters
	int getMemoryPages() const;

	// Memory Management
	void store(std::string variableId, unsigned int value);
	void release(std::string variableId);
	unsigned int lookup(std::string variableId);

private:
	
	// Member Variables
	const int memory_pages;
	std::string vm_path;
	std::fstream disk_memory;
	std::vector<std::pair<std::string, unsigned int>> main_memory;
};

