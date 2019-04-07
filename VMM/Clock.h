#pragma once

#include <thread>
#include <condition_variable>	

extern std::condition_variable cv;

class Clock
{
public:

	// Constructors/Destructors
	Clock();
	~Clock();

	// Getters
	int getTime() const;
	bool getFinishedStatus() const;

	// Setters
	void setFinishedStatus(bool status);

	// Clock Manipulation
	void incrementCounter();
	void startClock();

private:

	// Member Variables
	int counter;
	bool finished;
	std::thread clock_thread;
};

