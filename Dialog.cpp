#include "Dialog.h"

#include "Logger.h"

Dialog::Dialog(banana::agent::beast_callback& agent, banana::integer_t userId, const char* name)
    : agent_(agent), userId_(userId), name_(name)
{
	LOG(dialog, "{} dialog created for user {}", name_, userId_);
}

Dialog::~Dialog()
{
	LOG(dialog, "{} dialog for user {} destroyed", name_, userId_);
}

banana::agent::beast_callback& Dialog::getAgent() const
{
	return agent_;
}

banana::integer_t Dialog::getUserId() const
{
	return userId_;
}
