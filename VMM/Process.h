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
	std::string getStatus() const;

	// Setters
	void setStatus(std::string state);

private:

	// Member Variables
	int id;
	int start;
	int duration;
	std::string status;
};

