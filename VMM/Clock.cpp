#include "Clock.h"

Clock::Clock() : counter(0), finished(false)
{
}

Clock::~Clock()
{
	clock_thread.join();
}

int Clock::getTime() const
{
	return counter;
}

bool Clock::getFinishedStatus() const
{
	return finished;
}

void Clock::setFinishedStatus(bool status)
{
	finished = status;
}

void Clock::incrementCounter()
{
	while (!finished)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		counter++;
		cv.notify_all();
	}
}

void Clock::startClock()
{
	clock_thread = std::thread(&Clock::incrementCounter, this);
}
