#include "Asio.h"
#include "CourtApi.h"
#include "Storage.h"

#include <banana/agent/beast.hpp>
#include <banana/api.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <boost/asio/system_timer.hpp>

#include <chrono>
#include <csignal>
#include <ctime>
#include <string>

struct CaseHistoryItem
{
	std::string date;
	std::string time;
	std::string status;
	std::string publishDate;
	std::string publishTime;
};

std::vector<CaseHistoryItem> parseHistory(const nlohmann::json& details)
{
	std::vector<CaseHistoryItem> items;
	const auto& history = details.at("history");
	for (const auto& obj : history)
	{
		CaseHistoryItem item;
		item.date = obj.at("date").get<std::string>();
		item.time = obj.at("time").get<std::string>();
		item.status = obj.at("status").get<std::string>();
		item.publishDate = obj.at("publish_date").get<std::string>();
		item.publishTime = obj.at("publish_time").get<std::string>();
		items.push_back(std::move(item));
	}
	return items;
}

void notifyUser(banana::agent::beast_callback& bot,
                int userId,
                const std::string& caseNumber,
                std::string caseUrl,
                const CaseHistoryItem& item)
{
	caseUrl = fmt::format("https://mirsud.spb.ru{}", caseUrl);
	std::string message = fmt::format(
	    "Новое событие по делу [№{}]({}):\n"
	    "{}\n"
	    "Дата: {} {}\n",
	    caseNumber, caseUrl, item.status, item.date, item.time);
	banana::api::send_message(bot, {.chat_id = userId, .text = message, .parse_mode = "markdown"},
	                          [](const auto&) {});
}

void processAllSubscriptions(LocalStorage& storage, banana::agent::beast_callback& bot)
{
	for (auto& subscription : storage.subscriptions)
	{
		try
		{
			fmt::print("* Processing subscriptions for user {}\n", subscription.userId);
			for (auto& counter : subscription.counters)
			{
				fmt::print("** Processing case {}\n", counter.caseNumber);
				auto details = getCaseDetails(counter.courtId, counter.caseNumber);
				fmt::print("{}\n", details.dump());
				auto url = details["url"].get<std::string>();

				auto history = parseHistory(details);
				for (std::size_t i = counter.value; i < history.size(); i++)
					notifyUser(bot, subscription.userId, counter.caseNumber, url, history[i]);
				counter.value = history.size();
			}
		}
		catch (const std::exception& e)
		{
			fmt::print(stderr, "{}\n", e.what());
			continue;
		}
	}
}

void processUpdate(const banana::api::update_t& update)
{
	if (update.message)
	{
		if (update.message->text)
		{
			fmt::print("rx: {}\n", *update.message->text);
		}
	}
}

std::chrono::system_clock::time_point getNextCheckTime(std::uint32_t checkTime)
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm tm = *std::localtime(&now);
	std::uint32_t secsOfDay = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
	auto dayBegin = std::chrono::system_clock::from_time_t(now - secsOfDay);
	auto t = dayBegin + std::chrono::seconds(checkTime);
	return (secsOfDay < checkTime) ? t : (t + std::chrono::days(1));
}

auto asioWork = boost::asio::make_work_guard(asioContext);
bool terminate = false;

void handleSignal(int)
{
	fmt::print("signal!\n");
	terminate = true;
	asioWork.reset();
	asioContext.stop();
}

int64_t offset = 0;

void getUpdates(banana::agent::beast_callback& bot);

void processUpdates(banana::agent::beast_callback bot,
                    banana::expected<banana::array_t<banana::api::update_t>> updates)
{
	if (terminate)
	{
		fmt::print("exit\n");
		return;
	}

	if (updates)
	{
		for (const auto& update : *updates)
		{
			processUpdate(update);
			offset = update.update_id + 1;
		}
	}
	else
		fmt::print(stderr, "failed to get updates: {}\n", updates.error());

	getUpdates(bot);
}

void getUpdates(banana::agent::beast_callback& bot)
{
	banana::api::get_updates(bot, {.offset = offset, .timeout = 50},
	                         [bot](auto updates) { processUpdates(bot, std::move(updates)); });
}

int main()
{
	std::signal(SIGTERM, handleSignal);

	try
	{
		// Загрузить данные из локального хранилища
		LocalStorage storage;
		loadStorage(storage);
		fmt::print("Storage loaded\n");

		// Инициализировать SSL
		initSSL();

		// Создать бота
		banana::agent::beast_callback bot(storage.token, asioContext, sslContext);

		// Создать таймер ежедневной проверки
		boost::asio::system_timer checkTimer(asioContext);
		std::function<void(const boost::system::error_code&)> onTimer =
		    [&](const boost::system::error_code& error)
		{
			if (!error)
			{
				processAllSubscriptions(storage, bot);
				checkTimer.expires_at(getNextCheckTime(storage.checkTime));
				checkTimer.async_wait(onTimer);
			}
		};
		checkTimer.expires_at(getNextCheckTime(storage.checkTime));
		checkTimer.async_wait(onTimer);

		// Запустить асинхронное получение обновлений
		getUpdates(bot);

		// Запустить цикл обработки событий
		asioContext.run();

		// Сохранить данные в локальное хранилище
		fmt::print("Saving storage\n");
		saveStorage(storage);
		return 0;
	}
	catch (const std::exception& e)
	{
		fmt::print(stderr, "{}\n", e.what());
		return 1;
	}
}
