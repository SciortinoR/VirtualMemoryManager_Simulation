#include <iostream>
#include <algorithm>
#include <sstream>

#include "VirtualMemoryManager.h"

VirtualMemoryManager::VirtualMemoryManager(int pages, std::string filepath) :
	memory_pages(pages), main_memory(memory_pages), vm_path(filepath)
{	
	// Initial clear of Disk Memory contents 
	disk_memory.open(vm_path, std::ios::trunc);
	disk_memory.close();
}

VirtualMemoryManager::~VirtualMemoryManager()
{
}

int VirtualMemoryManager::getMemoryPages() const
{
	return memory_pages;
}

void VirtualMemoryManager::store(std::string variableId, unsigned int value)
{
	// Check if Main Memory is not full
	ASSERT((main_memory.size() <= memory_pages), "STORE ERROR: Main Memory page limit exceeded.");
	if (main_memory.size() != memory_pages)
	{
		main_memory.push_back(std::make_pair(variableId, value));
	}
	else
	{
		// Access(Open) Disk Memory at end of file
		disk_memory.open(vm_path, std::ios::ate | std::ios::out);
		ASSERT(disk_memory, "STORE ERROR: Could not access Disk Memory.");

		// Add to Disk Memory
		disk_memory << variableId << " " << value << std::endl;
		disk_memory.close();
	}
}

void VirtualMemoryManager::release(std::string variableId)
{
	// Check Main Memory first
	for (auto& variable : main_memory)
	{
		if (variable.first == variableId)
		{
			// Remove variable and end function
			main_memory.erase(std::remove(main_memory.begin(), main_memory.end(), variable), main_memory.end());
			return;
		}
	}

	// Check Disk Memory second
	disk_memory.open(vm_path, std::ios::in | std::ios::out);
	ASSERT(disk_memory, "RELEASE ERROR: Could not access Disk Memory.");

	// Set up necessities
	int line_counter;
	std::stringstream ss;
	std::string token, line;
	std::vector<std::string> file_buffer;

	// Loop through Disk Memory
	while (std::getline(disk_memory, line))
	{
		ss << line;
		ss >> token;
		if (token != variableId)
		{
			file_buffer.push_back(line);
			line_counter++;
		}
	}
	disk_memory.close();

	// Check if no variable found
	if (line_counter == file_buffer.size())
	{
		std::clog << "WARNING (Release): Variable not found in Main or Disk Memory" << std::endl;
		return;
	}
	else
	{
		// Re-open and clear contents of file
		disk_memory.open(vm_path, std::ios::trunc | std::ios::out);
		ASSERT(disk_memory, "RELEASE ERROR: Could not access Disk Memory.");
		
		// Overwrite Disk Memory with new buffer
		for (auto& line : file_buffer)
		{
			disk_memory << line << std::endl;
		}
	}
}

unsigned int VirtualMemoryManager::lookup(std::string variableId)
{
	return 0;
}
