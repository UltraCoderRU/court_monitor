#ifndef COURT_MONITOR_STORAGE_H
#define COURT_MONITOR_STORAGE_H

#include <cstddef>
#include <map>
#include <string>
#include <vector>

using UserId = std::int64_t;

struct UserData
{
	struct CaseSubscription
	{
		std::size_t counter = 0;
	};

	std::map<std::string, CaseSubscription> caseSubscriptions;
};

struct LocalStorage
{
	std::string token;
	std::map<UserId, UserData> userData;
	std::uint32_t checkTime; // секунды с 00:00
};

void loadStorage(LocalStorage& storage);
void saveStorage(const LocalStorage& storage);

#endif // COURT_MONITOR_STORAGE_H
