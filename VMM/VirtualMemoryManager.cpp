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
		main_memory.emplace_back(variableId, value, current_system_time);
	}
	else
	{
		Variable variable(variableId, value, current_system_time);
		swap(variable);
	}
}

void VirtualMemoryManager::release(std::string variableId)
{
	// Check through Main Memory first
	ASSERT((main_memory.size() <= memory_pages), "RELEASE ERROR: Main Memory page limit exceeded.");
	for (auto& variable : main_memory)
	{
		if (variable.getId() == variableId)
		{
			// Remove variable and end function
			main_memory.erase(std::remove(main_memory.begin(), main_memory.end(), variable), main_memory.end());
			return;
		}
	}

	// Check through Disk Memory second
	disk_memory.open(vm_path, std::ios::in);
	ASSERT(disk_memory, "RELEASE ERROR: Could not access Disk Memory.");

	// Set up necessities
	int line_counter;
	std::stringstream ss;
	std::string token, line;
	std::vector<std::string> disk_buffer;

	// Loop through Disk Memory
	while (std::getline(disk_memory, line))
	{
		ss << line;
		ss >> token;
		if (token != variableId)
		{
			disk_buffer.push_back(line);
			line_counter++;
		}
	}
	disk_memory.close();

	// Check if no variable found
	if (line_counter == disk_buffer.size())
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
		for (auto& line : disk_buffer)
		{
			disk_memory << line << std::endl;
		}
	}
}

unsigned int VirtualMemoryManager::lookup(std::string variableId)
{
	// Check through Main Memory first
	ASSERT((main_memory.size() <= memory_pages), "LOOKUP ERROR: Main Memory page limit exceeded.");
	for (auto& variable : main_memory)
	{
		if (variable.getId() == variableId)
		{
			return variable.getValue();
		}
	}

	// Check Disk Memory second
	disk_memory.open(vm_path, std::ios::in);
	ASSERT(disk_memory, "LOOKUP ERROR: Could not access Disk Memory.");

	// Set up necessities
	int line_counter;
	Variable temp;
	std::stringstream ss;
	std::string id, value, time, line;
	std::vector<std::string> disk_buffer;

	// Loop through Disk Memory
	while (std::getline(disk_memory, line))
	{
		ss << line;
		ss >> id;
		ss >> value;
		ss >> time;
		if (id != variableId)
		{
			disk_buffer.push_back(line);
			line_counter++;
		}
		else
		{
			temp.setId(id);
			temp.setValue(std::stoul(value));
			temp.setLastAccessTime(std::stoi(time));
		}
	}
	disk_memory.close();

	// Check if no variable found
	if (line_counter == disk_buffer.size())
	{
		std::clog << "WARNING (Release): Variable not found in Main or Disk Memory" << std::endl;
		return -1;
	}
	else
	{
		// Check if main memory is not full
		ASSERT((main_memory.size() <= memory_pages), "LOOKUP ERROR: Main Memory page limit exceeded.");
		if (main_memory.size() != memory_pages)
		{
			main_memory.push_back(temp);

			// Re-open and clear contents of file
			disk_memory.open(vm_path, std::ios::trunc | std::ios::out);
			ASSERT(disk_memory, "LOOKUP ERROR: Could not access Disk Memory.");

			// Overwrite Disk Memory with new buffer
			for (auto& line : disk_buffer)
			{
				disk_memory << line << std::endl;
			}
		}
		else
		{
			swap(temp, disk_buffer);
		}
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

	// Write to end of Disk Memory if STORE, otherwise rewrite Disk Memory if LOOKUP
	if (disk_buffer.empty())
	{
		// Access(Open) Disk Memory at end of file
		disk_memory.open(vm_path, std::ios::ate | std::ios::out);
		ASSERT(disk_memory, "SWAP ERROR: Could not access Disk Memory.");

		// Write previously removed variable to Disk Memory
		disk_memory << temp.getId() << " " << temp.getValue() << " " << temp.getLastAccessTime() << std::endl;
		disk_memory.close();
	}
	else
	{
		// Re-open and clear contents of file
		disk_memory.open(vm_path, std::ios::trunc | std::ios::out);
		ASSERT(disk_memory, "SWAP ERROR: Could not access Disk Memory.");

		// Overwrite Disk Memory with new buffer
		for (auto& line : disk_buffer)
		{
			disk_memory << line << std::endl;
		}
	}
}