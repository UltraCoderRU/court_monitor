#include "Storage.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::json;

std::uint32_t parseTime(const std::string& str)
{
	std::istringstream ss(str);
	std::tm tm = {};
	ss >> std::get_time(&tm, "%H:%M:%S");
	if (ss.fail())
		throw std::invalid_argument("invalid time string");
	return tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
}

std::string timeToString(std::uint32_t time)
{
	auto hours = time / 3600;
	auto minutes = (time % 3600) / 60;
	auto seconds = (time % 3600) % 60;
	return fmt::format("{:0>2}:{:0>2}:{:0>2}", hours, minutes, seconds);
}

void loadStorage(LocalStorage& storage)
{
	std::ifstream ifs("storage.json");
	if (ifs.is_open())
	{
		auto data = json::parse(ifs);
		storage.token = data.at("token").get<std::string>();
		storage.checkTime = parseTime(data.at("check_time").get<std::string>());

		for (const auto& subscription : data.at("subscriptions"))
		{
			Subscription s;
			s.userId = subscription.at("user_id");
			for (const auto& counter : subscription.at("counters"))
			{
				Counter c;
				c.courtId = counter.at("court").get<int>();
				c.caseNumber = counter.at("case").get<std::string>();
				c.value = counter.at("value").get<std::size_t>();
				s.counters.push_back(std::move(c));
			}
			storage.subscriptions.push_back(std::move(s));
		}
	}
	else
		throw std::runtime_error("failed to load storage");
}

void saveStorage(const LocalStorage& storage)
{
	json data;
	data["token"] = storage.token;
	data["check_time"] = timeToString(storage.checkTime);

	data["subscriptions"] = json::array();
	for (const auto& subscription : storage.subscriptions)
	{
		json jsonSubscription;
		jsonSubscription["user_id"] = subscription.userId;
		jsonSubscription["counters"] = json::array();
		for (const auto& counter : subscription.counters)
		{
			json jsonCounter;
			jsonCounter["court"] = counter.courtId;
			jsonCounter["case"] = counter.caseNumber;
			jsonCounter["value"] = counter.value;
			jsonSubscription["counters"].push_back(std::move(jsonCounter));
		}
		data["subscriptions"].push_back(std::move(jsonSubscription));
	}

	std::ofstream ofs("storage.json");
	if (ofs.is_open())
		ofs << std::setw(2) << data;
	else
		throw std::runtime_error("failed to save storage");
}
