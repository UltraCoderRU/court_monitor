#include "SubscribeCaseDialog.h"

#include "CourtApi.h"
#include "DialogHelpers.h"
#include "Logger.h"

#include <banana/api.hpp>

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>

namespace {

// clang-format off

// Состояния
struct WaitingForInput;
struct GettingCaseDetails;
struct WaitingForConfirmation;

// События
struct CaseDetailsFetched : statechart::event<CaseDetailsFetched> { };
struct SubscriptionConfirmed : statechart::event<SubscriptionConfirmed> { };

// clang-format on

} // namespace

struct SubscribeCaseStateMachine : StateMachine<SubscribeCaseStateMachine, WaitingForInput>
{
	using StateMachine::StateMachine;
};

namespace {

struct WaitingForInput : State<WaitingForInput, SubscribeCaseStateMachine>
{
	using reactions = statechart::custom_reaction<NewMessageEvent>;

	explicit WaitingForInput(const my_context& ctx) : State(ctx, "WaitingForInput")
	{
		auto& dialog = context<SubscribeCaseStateMachine>().dialog;
		std::string text = "Введите номер дела";
		banana::api::send_message(dialog.getAgent(),
		                          {.chat_id = dialog.getUserId(), .text = std::move(text)},
		                          [](auto) {});
	}

	statechart::result react(const NewMessageEvent& event) { return transit<GettingCaseDetails>(); }
};

struct GettingCaseDetails : State<GettingCaseDetails, SubscribeCaseStateMachine, true>
{
	explicit GettingCaseDetails(const my_context& ctx) : State(ctx, "GettingCaseDetails") {}
};

struct WaitingForConfirmation : State<WaitingForConfirmation, SubscribeCaseStateMachine>
{
	explicit WaitingForConfirmation(const my_context& ctx) : State(ctx, "WaitingForConfirmation") {}
};

} // namespace

/////////////////////////////////////////////////////////////////////////////

SubscribeCaseDialog::SubscribeCaseDialog(banana::agent::beast_callback& agent,
                                         banana::integer_t userId)
    : Dialog(agent, userId, "SubscribeCase"),
      machine_(std::make_unique<SubscribeCaseStateMachine>(*this))
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
