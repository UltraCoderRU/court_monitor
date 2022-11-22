#ifndef COURT_MONITOR_DIALOG_HELPERS_H
#define COURT_MONITOR_DIALOG_HELPERS_H

#include "Logger.h"
#include "Storage.h"

#include <banana/types.hpp>

#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>

namespace statechart = boost::statechart;

template <class MostDerived, class InitialState>
struct StateMachine : public statechart::state_machine<MostDerived, InitialState>
{
	explicit StateMachine(banana::agent::beast_async_monadic& agent, banana::integer_t userId)
	    : agent(agent), userId(userId)
	{
	}

	banana::agent::beast_async_monadic& agent;
	banana::integer_t userId;
};

struct BasicState
{
	[[nodiscard]] virtual bool isFinal() const noexcept = 0;
};

template <class MostDerived, class Context, bool IsFinal = false>
class State : public statechart::state<MostDerived, Context>, public BasicState
{
public:
	State(typename statechart::state<MostDerived, Context>::my_context ctx, const char* name)
	    : statechart::state<MostDerived, Context>(ctx), name_(name)
	{
		LOG(dialog, "entering state {}", name_);
	}

	~State() { LOG(dialog, "leaving state {}", name_); }

	[[nodiscard]] bool isFinal() const noexcept override { return IsFinal; }

private:
	const char* name_;
};

struct NewMessageEvent : statechart::event<NewMessageEvent>
{
	explicit NewMessageEvent(banana::api::message_t message) : message(std::move(message)) {}
	banana::api::message_t message;
};

struct NewCallbackQueryEvent : statechart::event<NewCallbackQueryEvent>
{
	explicit NewCallbackQueryEvent(banana::api::callback_query_t query) : query(std::move(query)) {}
	banana::api::callback_query_t query;
};

#endif // COURT_MONITOR_DIALOG_HELPERS_H
