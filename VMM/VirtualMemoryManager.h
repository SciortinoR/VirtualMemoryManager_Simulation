#pragma once

#include <vector>
#include <fstream>
#include <string>

class VirtualMemoryManager
{
public:

	// Constructors/Destructors
	VirtualMemoryManager(int pages, std::fstream& vm);
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
	std::fstream& disk_memory;
	std::vector<std::pair<std::string, unsigned int>> main_memory;
};

