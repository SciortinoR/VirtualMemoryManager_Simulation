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
	if (main_memory.size() < memory_pages)
	{
		main_memory.emplace_back(variableId, value, current_system_time);
	}
	else
	{
		ASSERT((main_memory.size() > memory_pages), "STORE ERROR: Main Memory page limit exceeded.");
		Variable variable(variableId, value, current_system_time);
		swap(variable);
	}
}

void VirtualMemoryManager::release(std::string variableId)
{
	// Search for variable and remove it
	searchAllMemory(variableId);
}

long VirtualMemoryManager::lookup(std::string variableId)
{
	// Search for variable and return it
	return searchAllMemory(variableId, 1);
}

long VirtualMemoryManager::searchAllMemory(std::string variableId, int functionId)
{
	// Check through Main Memory first
	for (auto& variable : main_memory)
	{
		if (variable.getId() == variableId)
		{
			if (functionId == 0)
			{
				// RELEASE: Remove variable and end function
				main_memory.erase(std::remove(main_memory.begin(), main_memory.end(), variable), main_memory.end());
				return 0;
			}
			else
			{
				// LOOKUP: Return variable value
				return variable.getValue();
			}
		}
	}

	// Check Disk Memory second
	disk_memory.open(vm_path, std::ios::in);
	ASSERT(!disk_memory, "DISK ERROR: Could not access Disk Memory.");

	// Prepare variables
	Variable temp;
	std::stringstream ss;
	std::string id, value, time, line;
	std::vector<std::string> disk_buffer;

	// Find variable in Disk Memory
	while (std::getline(disk_memory, line))
	{
		ss.str(line);
		ss >> id;
		ss >> value;
		ss >> time;
		if (id != variableId)
		{
			disk_buffer.push_back(line);
		}
		else
		{
			temp.setId(id);
			temp.setValue(std::stoul(value));
			temp.setLastAccessTime(std::stoi(time));
		}
	}
	disk_memory.close();

	// Check if variable found
	if (temp.getId().empty())
	{
		std::clog << "WARNING (Search): Variable not found in Main or Disk Memory" << std::endl;
		return -1;
	}
	else if (functionId == 0)
	{
		// RELEASE: Simple write to Disk
		writeDisk(disk_buffer);
		return 0;
	}
	else
	{
		// LOOKUP: Move variable to main memory (swap if needed)
		if (main_memory.size() < memory_pages)
		{
			main_memory.push_back(temp);
			writeDisk(disk_buffer);
		}
		else
		{
			ASSERT((main_memory.size() > memory_pages), "LOOKUP ERROR: Main Memory page limit exceeded.");
			swap(temp, disk_buffer);
		}

		return temp.getValue();
	}
}

void VirtualMemoryManager::swap(const Variable& variable, const std::vector<std::string>& disk_buffer)
{
	// Find last accessed Variable and remove it from Main Memory
	Variable temp = main_memory[0];
	for (auto& var : main_memory)
	{
		if (var.getLastAccessTime() < temp.getLastAccessTime())
		{
			temp = var;
		}
	}
	main_memory.erase(std::remove(main_memory.begin(), main_memory.end(), temp), main_memory.end());

	// Write new variable to Main Memory
	main_memory.push_back(variable);

	// Write swapped variable to Disk Memory
	writeDisk(disk_buffer, temp);
}

void VirtualMemoryManager::writeDisk(const std::vector<std::string>& disk_buffer, const Variable& variable)
{
	// Check if both Disk and Variable buffers are empty
	ASSERT((disk_buffer.empty() && variable.getId().empty()), "DISK WRITE ERROR: Variable & Disk Buffer are both empty.");
	
	// Write to Disk Memory depending on arguments
	if (disk_buffer.empty())
	{
		// Access(Open) Disk Memory at end of file
		disk_memory.open(vm_path, std::ios::ate | std::ios::out);
		ASSERT(!disk_memory, "DISK ERROR: Could not access Disk Memory.");
	}
	else
	{
		// Open and clear contents of Disk Memory
		disk_memory.open(vm_path, std::ios::trunc | std::ios::out);
		ASSERT(!disk_memory, "DISK ERROR: Could not access Disk Memory.");

		// Overwrite Disk Memory with new buffer
		for (auto& line : disk_buffer)
		{
			disk_memory << line << std::endl;
		}
	}
	
	// Check if Variable found
	if (variable.getId().empty())
	{
		disk_memory.close();
	}
	else
	{
		// Write swapped out variable to Disk Memory 
		disk_memory << variable.getId() << " " << variable.getValue() << " " << variable.getLastAccessTime() << std::endl;
		disk_memory.close();
	}
}
