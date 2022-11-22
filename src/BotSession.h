#ifndef COURT_MONITOR_BOT_SESSION_H
#define COURT_MONITOR_BOT_SESSION_H

#include "Storage.h"

#include <banana/agent/beast.hpp>
#include <banana/types_fwd.hpp>
#include <banana/utils/basic_types.hpp>

#include <memory>

class Dialog;

class BotSession final
{
public:
	BotSession(banana::agent::beast_async_monadic& agent,
	           banana::integer_t userId,
	           LocalStorage& storage);
	~BotSession();

	void processMessage(const banana::api::message_t& message);
	void processCallbackQuery(const banana::api::callback_query_t& query);

private:
	template <class T>
	std::unique_ptr<Dialog> makeDialog()
	{
		return std::make_unique<T>(agent_, userId_, storage_);
	}

	void processStartCommand();
	void processStopCommand();

	banana::agent::beast_async_monadic& agent_;
	banana::integer_t userId_;
	LocalStorage& storage_;
	std::unique_ptr<Dialog> activeDialog_;
};

#endif // COURT_MONITOR_BOT_SESSION_H
