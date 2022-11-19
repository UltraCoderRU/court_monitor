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
	setupCommands();
	getUpdates();
}

void Bot::setupCommands()
{
	banana::api::set_my_commands_args_t args;
	args.commands.push_back(
	    banana::api::bot_command_t{"/subscribe_case", "Подписаться на обновления дела"});
	args.commands.push_back(
	    banana::api::bot_command_t{"/unsubscribe_case", "Отписаться от обновлений дела"});
	args.commands.push_back(banana::api::bot_command_t{"/check_case", "Проверить дело"});
	args.commands.push_back(banana::api::bot_command_t{"/find_case", "Найти дело"});
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

void Bot::notifyUser(UserId userId,
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
			LOGE(bot, "failed to get updates: {}", updates.error());
	};

	banana::api::get_updates(
	    agent_,
	    {.offset = updatesOffset_,
	     .timeout = 50,
	     .allowed_updates = banana::array_t<banana::string_t>{"message", "callback_query"}},
	    std::move(handler));
}

void Bot::processUpdate(const banana::api::update_t& update)
{
	if (update.message)
	{
		banana::integer_t userId = update.message->from->id;

		if (update.message->text)
			LOG(bot, "incoming message: user={} text='{}'", userId, *update.message->text);
		else
			LOG(bot, "incoming message: user={} (not text)", userId);

		auto& session = getOrCreateSession(userId);
		session.processMessage(*update.message);
	}
	else if (update.callback_query)
	{
		banana::integer_t userId = update.callback_query->from.id;
		LOG(bot, "incoming callback query: user={} data='{}'", userId, *update.callback_query->data);

		auto& session = getOrCreateSession(userId);
		session.processCallbackQuery(*update.callback_query);
	}
	else
		LOG(bot, "skip unknown update type");
}

BotSession& Bot::getOrCreateSession(banana::integer_t userId)
{
	auto sessionIt = sessions_.find(userId);
	if (sessionIt == sessions_.end())
	{
		bool ok;
		std::tie(sessionIt, ok) =
		    sessions_.emplace(std::piecewise_construct, std::forward_as_tuple(userId),
		                      std::forward_as_tuple(agent_, userId, storage_));
	}
	return sessionIt->second;
}
