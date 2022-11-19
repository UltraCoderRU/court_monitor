#include "Storage.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::ordered_json;

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
		auto jsonStorage = json::parse(ifs);
		storage.token = jsonStorage.at("token").get<std::string>();
		storage.checkTime = parseTime(jsonStorage.at("check_time").get<std::string>());

		for (const auto& jsonUserData : jsonStorage.at("user_data"))
		{
			UserData userData;
			auto userId = jsonUserData.at("user_id").get<UserId>();
			for (const auto& jsonCaseSubscription : jsonUserData.at("case_subscriptions"))
			{
				UserData::CaseSubscription caseSubscription;
				auto caseNumber = jsonCaseSubscription.at("case").get<std::string>();
				caseSubscription.counter = jsonCaseSubscription.at("counter").get<std::size_t>();
				userData.caseSubscriptions[caseNumber] = caseSubscription;
			}
			storage.userData[userId] = std::move(userData);
		}
	}
	else
		throw std::runtime_error("failed to load storage");
}

void saveStorage(const LocalStorage& storage)
{
	json jsonStorage;
	jsonStorage["token"] = storage.token;
	jsonStorage["check_time"] = timeToString(storage.checkTime);

	jsonStorage["user_data"] = json::array();
	for (const auto& subscription : storage.userData)
	{
		json jsonUserData;
		jsonUserData["user_id"] = subscription.first;
		jsonUserData["case_subscriptions"] = json::array();
		for (const auto& caseSubscription : subscription.second.caseSubscriptions)
		{
			json jsonCaseSubscription;
			jsonCaseSubscription["case"] = caseSubscription.first;
			jsonCaseSubscription["counter"] = caseSubscription.second.counter;
			jsonUserData["case_subscriptions"].push_back(std::move(jsonCaseSubscription));
		}
		jsonStorage["user_data"].push_back(std::move(jsonUserData));
	}

	std::ofstream ofs("storage.json");
	if (ofs.is_open())
		ofs << std::setw(2) << jsonStorage;
	else
		throw std::runtime_error("failed to save storage");
}
