#include <iostream>
#include <algorithm>
#include <sstream>

#include "VirtualMemoryManager.h"

VirtualMemoryManager::VirtualMemoryManager() : memory_pages(0)
{
}

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

const std::stringstream& VirtualMemoryManager::getSwapLog() const 
{
	return swap_log;
}	

void VirtualMemoryManager::store(std::string variableId, unsigned int value)
{
	// Check if Main Memory is not full
	if (main_memory.size() < memory_pages)
	{
		main_memory.emplace_back(variableId, value, system_clock);
	}
	else
	{
		ASSERT((main_memory.size() > memory_pages), "STORE ERROR: Main Memory page limit exceeded.");
		Variable variable(variableId, value, system_clock);
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
				variable.setLastAccessTime(system_clock);
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
		ss.clear();
		ss >> id >> value >> time;
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
		std::clog << "WARNING (Search): Variable " << variableId << "not found in Main or Disk Memory" << std::endl;
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
		// Change access time of variable
		temp.setLastAccessTime(system_clock);
		
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
	temp.setLastAccessTime(system_clock);
	writeDisk(disk_buffer, temp);

	// Log Swap
	log_swap(variable, temp);
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

void VirtualMemoryManager::log_swap(const Variable& new_var, const Variable& old_var)
{
	swap_log.str("Clock: " + std::to_string(system_clock) + ", Memory Manager, SWAP: Variable " 
		+ new_var.getId() + " with Variable " + old_var.getId());

	swap_log.clear();
}

bool operator ==(const Variable& v1, const Variable& v2)
{
	return v1.getId() == v2.getId();
}