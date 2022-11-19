#ifndef COURT_MONITOR_STORAGE_H
#define COURT_MONITOR_STORAGE_H

#include <cstddef>
#include <string>
#include <vector>

struct Counter
{
	int courtId = 0;
	std::string caseNumber;
	std::size_t value = 0;
};

struct Subscription
{
	int userId = 0;
	std::vector<Counter> counters;
};

struct LocalStorage
{
	std::string token;
	std::vector<Subscription> subscriptions;
	std::uint32_t checkTime; // секунды с 00:00
};

void loadStorage(LocalStorage& storage);
void saveStorage(const LocalStorage& storage);

#endif // COURT_MONITOR_STORAGE_H
