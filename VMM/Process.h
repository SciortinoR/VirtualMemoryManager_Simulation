#pragma once

#include <string>

class Process
{
public:

	// Constructors/Destructors
	Process();
	Process(int Id, int start_time, int service_time);
	~Process();

	// Getters
	int getId() const;
	int getEndTime() const;
	int getDuration() const;
	int getStartTime() const;
	std::string getStatus() const;

	// Setters
	void setStatus(std::string state);

	// Class Variables
	inline static const std::string pending = "Pending Start";
	inline static const std::string started = "Started";
	inline static const std::string finished = "Finished";

private:

	// Member Variables
	int id;
	int end;
	int start;
	int duration;
	std::string status;
};

