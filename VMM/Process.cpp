#include "Process.h"

Process::Process(int Id, int start_time, int service_time) :
	id(Id), start(start_time), duration(service_time), last_access_time(0), status("Pending start")	
{
}

Process::~Process()
{
}

int Process::getId() const
{
	return id;
}

int Process::getDuration() const
{
	return duration;
}

int Process::getStartTime() const
{
	return start;
}

int Process::getLastAccessTime() const
{
	return last_access_time;
}

std::string Process::getStatus() const
{
	return status;
}

void Process::setStatus(std::string state)
{
	status = state;
}

void Process::setLastAccessTime(int last_time)
{
	last_access_time = last_time;
}
