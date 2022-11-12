#include "Bot.h"

#include "Logger.h"

#include <banana/api.hpp>
#include <fmt/format.h>

#include <boost/asio/ssl/context.hpp>

namespace {
boost::asio::ssl::context sslContext(boost::asio::ssl::context::tlsv13_client);
}

Bot::Bot(boost::asio::io_context& asioContext, LocalStorage& storage, bool& terminationFlag)
    : storage_(storage),
      terminationFlag_(terminationFlag),
      agent_(storage.token, asioContext, sslContext)
{
	getUpdates();
}

void Bot::setupCommands()
{
	banana::api::set_my_commands_args_t args;
	args.commands.push_back(banana::api::bot_command_t{
	    "/subscribe_case", "Подписаться на обновления дела по его номеру"});
	args.commands.push_back(banana::api::bot_command_t{"/find_case", "Найти дело по параметрам"});
	args.scope = banana::api::bot_command_scope_all_private_chats_t{"all_private_chats"};
	banana::api::set_my_commands(agent_, std::move(args),
	                             [](banana::expected<banana::boolean_t> result)
	                             {
		                             if (result)
			                             LOG(bot, "commands set up successfully");
		                             else
			                             LOGE(bot, "failed to set up commands");
	                             });
}

void Bot::notifyUser(int userId,
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
	banana::api::send_message(agent_, {.chat_id = userId, .text = message, .parse_mode = "markdown"},
	                          [](const auto&) {});
}

void Bot::getUpdates()
{
	auto handler = [this](banana::expected<banana::array_t<banana::api::update_t>> updates)
	{
		if (terminationFlag_)
			return;

		if (updates)
		{
			for (const auto& update : *updates)
			{
				processUpdate(update);
				updatesOffset_ = update.update_id + 1;
			}
			getUpdates();
		}
		else
			LOG(bot, "failed to get updates: {}", updates.error());
	};

	banana::api::get_updates(agent_, {.offset = updatesOffset_, .timeout = 50}, std::move(handler));
}

void Bot::processUpdate(const banana::api::update_t& update)
{
	if (update.message)
	{
		if (update.message->text)
		{
			LOG(bot, "rx: {}\n", *update.message->text);
			if (*update.message->text == "/start")
				processStartCommand(*update.message);
			else if (*update.message->text == "/subscribe_case")
				processSubscribeCaseCommand(*update.message);
			else
			{
				// TODO
			}
		}
		else
			LOG(bot, "skip message without text"); // TODO ответить
	}
	else
		LOGE(bot, "skip unknown update type");
}

void Bot::processStartCommand(const banana::api::message_t& message)
{
}

void Bot::processSubscribeCaseCommand(const banana::api::message_t& message)
{
}
