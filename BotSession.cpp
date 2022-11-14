#include "BotSession.h"

#include "Logger.h"
#include "SubscribeCaseDialog.h"

#include <banana/api.hpp>
#include <fmt/core.h>

BotSession::BotSession(banana::agent::beast_callback& agent, banana::integer_t userId)
    : agent_(agent), userId_(userId)
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
			processStartCommand(message);
		else if (text == "/stop")
			processStopCommand(message);
		else if (text == "/subscribe_case")
			activeDialog_ = makeDialog<SubscribeCaseDialog>();
		else if (activeDialog_)
		{
			if (activeDialog_->processMessage(message))
			{
				// TODO
				processStartCommand(message);
			}
		}
		else
		{
			// TODO
			processStartCommand(message);
		}
	}
	else
		LOGE(session, "skip message without text"); // TODO ответить
}

void BotSession::processCallbackQuery(const banana::api::callback_query_t& query)
{
}

void BotSession::processStartCommand(const banana::api::message_t& message)
{
	activeDialog_.reset();

	std::string text =
	    "Вас приветствует неофициальный бот Мирового Суда г. Санкт-Петербурга.\n\n"
	    "Выберите в меню желаемое действие.";

	banana::api::send_message(agent_, {.chat_id = userId_, .text = std::move(text)},
	                          [](const banana::expected<banana::api::message_t>& message) {});
}

void BotSession::processStopCommand(const banana::api::message_t& message)
{
	activeDialog_.reset();
}
