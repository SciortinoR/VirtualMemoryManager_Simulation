#include "Process.h"

Process::Process()
{
}

Process::Process(int Id, int start_time, int service_time) :
	id(Id), start(start_time), duration(service_time), status("Pending Start")	
{
	end = start + duration;
}

Process::~Process()
{
}

int Process::getId() const
{
	return id;
}

int Process::getEndTime() const
{
	return end;
}

int Process::getDuration() const
{
	return duration;
}

int Process::getStartTime() const
{
	return start;
}

std::string Process::getStatus() const
{
	return status;
}

void Process::setStatus(std::string state)
{
	status = state;
}
