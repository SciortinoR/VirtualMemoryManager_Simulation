#pragma once

#include <string>

class Variable
{
public:

	// Constructors/Destructors
	Variable();
	Variable(std::string variableId, unsigned int value, int time);
	~Variable();

	// Getters
	std::string getId() const;
	unsigned int getValue() const;
	int getLastAccessTime() const;

	//Setters
	void setId(std::string variableId);
	void setValue(unsigned int value);
	void setLastAccessTime(int time);
private:

	std::string id;
	unsigned int val;
	int last_access_time;
};

