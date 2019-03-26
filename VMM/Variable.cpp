#include "Variable.h"

Variable::Variable()
{
}

Variable::Variable(std::string variableId, unsigned int value, int time) :
	id(variableId), val(value), last_access_time(time)
{
}

Variable::~Variable()
{
}

std::string Variable::getId() const
{
	return id;
}

unsigned int Variable::getValue() const
{
	return val;
}

int Variable::getLastAccessTime() const
{
	return last_access_time;
}

void Variable::setId(std::string variableId)
{
	id = variableId;
}

void Variable::setValue(unsigned int value)
{
	val = value;
}

void Variable::setLastAccessTime(int time)
{
	last_access_time = time;
}
