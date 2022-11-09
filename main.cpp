#include "Bot.h"
#include "CourtApi.h"
#include "Storage.h"

#include <banana/agent/beast.hpp>
#include <banana/api.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/system_timer.hpp>

#include <chrono>
#include <csignal>
#include <ctime>
#include <string>

boost::asio::io_context asioContext;

void processAllSubscriptions(LocalStorage& storage, Bot& bot)
{
	for (auto& subscription : storage.subscriptions)
	{
		try
		{
			fmt::print("* Processing subscriptions for user {}\n", subscription.userId);
			for (auto& counter : subscription.counters)
			{
				fmt::print("** Processing case {}\n", counter.caseNumber);
				auto details = getCaseDetails(asioContext, counter.courtId, counter.caseNumber);
				fmt::print("{}\n", details.dump());
				auto url = details["url"].get<std::string>();

				auto history = parseHistory(details);
				for (std::size_t i = counter.value; i < history.size(); i++)
					bot.notifyUser(subscription.userId, counter.caseNumber, url, history[i]);
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

int main()
{
	std::signal(SIGTERM, handleSignal);

	try
	{
		// Загрузить данные из локального хранилища
		LocalStorage storage;
		loadStorage(storage);
		fmt::print("Storage loaded\n");

		// Создать бота
		Bot bot(asioContext, storage, terminate);
		bot.setupCommands();

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
