#ifndef COURT_MONITOR_BOT_SESSION_H
#define COURT_MONITOR_BOT_SESSION_H

#include <banana/agent/beast.hpp>
#include <banana/types_fwd.hpp>
#include <banana/utils/basic_types.hpp>

#include <memory>

class Dialog;

class BotSession final
{
public:
	BotSession(banana::agent::beast_callback& agent, banana::integer_t userId);
	~BotSession();

	void processMessage(const banana::api::message_t& message);
	void processCallbackQuery(const banana::api::callback_query_t& query);

private:
	template <class T>
	std::unique_ptr<Dialog> makeDialog()
	{
		return std::make_unique<T>(agent_, userId_);
	}

	void processStartCommand(const banana::api::message_t& message);
	void processStopCommand(const banana::api::message_t& message);

	banana::agent::beast_callback& agent_;
	banana::integer_t userId_;
	std::unique_ptr<Dialog> activeDialog_;
};

#endif // COURT_MONITOR_BOT_SESSION_H
