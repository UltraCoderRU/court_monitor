#include "SubscribeCaseDialog.h"

#include "CourtApi.h"
#include "DialogHelpers.h"
#include "Logger.h"

#include <banana/api.hpp>
#include <nlohmann/json.hpp>

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>

#include <regex>

namespace {

// clang-format off

// Состояния
struct WaitingForInput;
struct GettingCaseDetails;
struct WaitingForConfirmation;
struct Subscribed;

// События
struct CaseDetailsFetched : statechart::event<CaseDetailsFetched> { };
struct SubscriptionConfirmed : statechart::event<SubscriptionConfirmed> { };

// clang-format on

} // namespace

struct SubscribeCaseStateMachine : StateMachine<SubscribeCaseStateMachine, WaitingForInput>
{
	SubscribeCaseStateMachine(banana::agent::beast_async_monadic& agent,
	                          long userId,
	                          LocalStorage& storage)
	    : StateMachine(agent, userId), storage(storage)
	{
	}
	LocalStorage& storage;
	std::string caseNumber;
	CaseDetails caseDetails;
};

namespace {

struct WaitingForInput : State<WaitingForInput, SubscribeCaseStateMachine>
{
	using reactions = statechart::custom_reaction<NewMessageEvent>;

	explicit WaitingForInput(const my_context& ctx) : State(ctx, "WaitingForInput")
	{
		auto& machine = context<SubscribeCaseStateMachine>();
		std::string text = "Введите номер дела...";
		banana::api::send_message(machine.agent,
		                          {.chat_id = machine.userId, .text = std::move(text)});
	}

	statechart::result react(const NewMessageEvent& event)
	{
		auto& machine = context<SubscribeCaseStateMachine>();
		const std::regex rex(R"(\d-(?:\d+)/(?:\d){4}-(?:\d+))");
		std::smatch captures;
		if (std::regex_match(*event.message.text, captures, rex))
		{
			machine.caseNumber = *event.message.text;
			return transit<GettingCaseDetails>();
		}
		else
		{
			std::string text =
			    "Некорректный формат номера дела!\n"
			    "Попробуйте еще раз.";
			banana::api::send_message(machine.agent,
			                          {.chat_id = machine.userId, .text = std::move(text)});
			return discard_event();
		}
	}
};

struct GettingCaseDetails : State<GettingCaseDetails, SubscribeCaseStateMachine>
{
	using reactions = statechart::custom_reaction<CaseDetailsFetched>;

	explicit GettingCaseDetails(const my_context& ctx) : State(ctx, "GettingCaseDetails")
	{
		auto& machine = context<SubscribeCaseStateMachine>();
		boost::asio::io_context ioContext;

		try
		{
			machine.caseDetails = getCaseDetails(ioContext, machine.caseNumber);
			std::string text;
			fmt::format_to(std::back_inserter(text), "Проверьте информацию:\n{}\n",
			               machine.caseDetails.name);
			for (const auto& participant : machine.caseDetails.participants)
				fmt::format_to(std::back_inserter(text), "{}: {}\n", participant.title,
				               participant.name);
			fmt::format_to(std::back_inserter(text), "Судья: {}", machine.caseDetails.judgeName);

			banana::api::inline_keyboard_markup_t keyboard;
			keyboard.inline_keyboard.resize(1);
			keyboard.inline_keyboard[0].resize(2);

			keyboard.inline_keyboard[0][0].text = "Верно";
			keyboard.inline_keyboard[0][0].callback_data = "yes";
			keyboard.inline_keyboard[0][1].text = "Отмена";
			keyboard.inline_keyboard[0][1].callback_data = "no";

			banana::api::send_message(
			    machine.agent,
			    {.chat_id = machine.userId, .text = std::move(text), .reply_markup = keyboard});

			post_event(CaseDetailsFetched());
		}
		catch (const std::exception& e)
		{
			LOGE(dialog, e.what());
			// TODO ???
		}
	}

	statechart::result react(const CaseDetailsFetched& event)
	{
		return transit<WaitingForConfirmation>();
	}
};

struct WaitingForConfirmation : State<WaitingForConfirmation, SubscribeCaseStateMachine>
{
	using reactions = statechart::custom_reaction<NewCallbackQueryEvent>;

	explicit WaitingForConfirmation(const my_context& ctx) : State(ctx, "WaitingForConfirmation") {}

	statechart::result react(const NewCallbackQueryEvent& event)
	{
		auto& machine = context<SubscribeCaseStateMachine>();
		if (event.query.data)
		{
			if (event.query.message)
				banana::api::edit_message_reply_markup(
				    machine.agent, {.chat_id = event.query.message->chat.id,
				                    .message_id = event.query.message->message_id});

			if (*event.query.data == "yes")
			{
				auto& userData = machine.storage.userData[machine.userId];
				userData.caseSubscriptions[machine.caseNumber].counter =
				    machine.caseDetails.history.size();
				saveStorage(machine.storage);
				return transit<Subscribed>();
			}
			else if (*event.query.data == "no")
			{
				return transit<WaitingForInput>();
			}
		}
		return discard_event();
	}
};

struct Subscribed : State<Subscribed, SubscribeCaseStateMachine, true>
{
	explicit Subscribed(const my_context& ctx) : State(ctx, "Subscribed") {}
};

} // namespace

/////////////////////////////////////////////////////////////////////////////

SubscribeCaseDialog::SubscribeCaseDialog(banana::agent::beast_async_monadic& agent,
                                         banana::integer_t userId,
                                         LocalStorage& storage)
    : Dialog(userId, "SubscribeCase"),
      machine_(std::make_unique<SubscribeCaseStateMachine>(agent, userId, storage))
{
	machine_->initiate();
}

SubscribeCaseDialog::~SubscribeCaseDialog() = default;

bool SubscribeCaseDialog::processMessage(const banana::api::message_t& message)
{
	machine_->process_event(NewMessageEvent{message});
	return machine_->state_cast<const BasicState&>().isFinal();
}

bool SubscribeCaseDialog::processCallbackQuery(const banana::api::callback_query_t& query)
{
	machine_->process_event(NewCallbackQueryEvent{query});
	return machine_->state_cast<const BasicState&>().isFinal();
}
