#include "BotSession.h"

#include "Logger.h"
#include "Storage.h"
#include "SubscribeCaseDialog.h"

#include <banana/api.hpp>
#include <fmt/core.h>

BotSession::BotSession(banana::agent::beast_async_monadic& agent,
                       banana::integer_t userId,
                       LocalStorage& storage)
    : agent_(agent), userId_(userId), storage_(storage)
{
	LOG(session, "new session created for user {}", userId_);
}

BotSession::~BotSession() = default;

void BotSession::processMessage(const banana::api::message_t& message)
{
	if (message.text)
	{
		auto text = *message.text;
		if (text == "/start")
			processStartCommand();
		else if (text == "/stop")
			processStopCommand();
		else if (text == "/subscribe_case")
			activeDialog_ = makeDialog<SubscribeCaseDialog>();
		else if (activeDialog_)
		{
			if (activeDialog_->processMessage(message))
			{
				// TODO
				processStartCommand();
			}
		}
		else
		{
			// TODO
			processStartCommand();
		}
	}
	else
		LOGE(session, "skip message without text"); // TODO ответить
}

void BotSession::processCallbackQuery(const banana::api::callback_query_t& query)
{
	if (query.data)
	{
		if (activeDialog_)
		{
			if (activeDialog_->processCallbackQuery(query))
				processStartCommand();
		}
		else
			LOGE(session, "skip callback query, because there is no active dialog");
	}
	else
		LOGE(session, "skip callback query without data");
}

void BotSession::processStartCommand()
{
	activeDialog_.reset();

	std::string text =
	    "Вас приветствует неофициальный бот Мирового Суда г. Санкт-Петербурга.\n\n"
	    "Выберите в меню желаемое действие.";

	banana::api::send_message(agent_, {.chat_id = userId_, .text = std::move(text)},
	                          [](const banana::expected<banana::api::message_t>& message) {});
}

void BotSession::processStopCommand()
{
	activeDialog_.reset();
}
