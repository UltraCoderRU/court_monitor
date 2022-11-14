#ifndef COURT_MONITOR_DIALOG_H
#define COURT_MONITOR_DIALOG_H

#include <banana/agent/beast.hpp>
#include <banana/types_fwd.hpp>
#include <banana/utils/basic_types.hpp>

class Dialog
{
public:
	Dialog(banana::agent::beast_callback& agent, banana::integer_t userId, const char* name);
	virtual ~Dialog();

	[[nodiscard]] banana::agent::beast_callback& getAgent() const;
	[[nodiscard]] banana::integer_t getUserId() const;

	// Возвращают true, если диалог завершен.
	virtual bool processMessage(const banana::api::message_t& message) = 0;
	virtual bool processCallbackQuery(const banana::api::callback_query_t& query) = 0;

private:
	banana::agent::beast_callback& agent_;
	banana::integer_t userId_;
	const char* name_;
};

#endif // COURT_MONITOR_DIALOG_H
