#include "Dialog.h"

#include "Logger.h"

Dialog::Dialog(banana::integer_t userId, const char* name) : userId_(userId), name_(name)
{
	LOG(dialog, "{} dialog created for user {}", name_, userId_);
}

Dialog::~Dialog()
{
	LOG(dialog, "{} dialog for user {} destroyed", name_, userId_);
}
