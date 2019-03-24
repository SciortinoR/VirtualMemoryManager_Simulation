#include "VirtualMemoryManager.h"

VirtualMemoryManager::VirtualMemoryManager(int pages, std::fstream& vm) :
	memory_pages(pages), main_memory(memory_pages), disk_memory(vm)
{	
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
}

void VirtualMemoryManager::release(std::string variableId)
{
}

unsigned int VirtualMemoryManager::lookup(std::string variableId)
{
	return 0;
}
