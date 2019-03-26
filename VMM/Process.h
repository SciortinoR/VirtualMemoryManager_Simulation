#pragma once

#include <string>

class Process
{
public:

	// Constructors/Destructors
	Process(int Id, int start_time, int service_time);
	~Process();

	// Getters
	int getId() const;
	int getDuration() const;
	int getStartTime() const;
	int getLastAccessTime() const;
	std::string getStatus() const;

	// Setters
	void setStatus(std::string state);
	void setLastAccessTime(int last_time);

private:

	// Member Variables
	const int id;
	const int start;
	const int duration;
	int last_access_time;
	std::string status;
};

