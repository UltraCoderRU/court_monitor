#ifndef COURT_MONITOR_SUBSCRIBE_CASE_DIALOG_H
#define COURT_MONITOR_SUBSCRIBE_CASE_DIALOG_H

#include "Dialog.h"

struct SubscribeCaseStateMachine;

class SubscribeCaseDialog : public Dialog
{
public:
	SubscribeCaseDialog(banana::agent::beast_callback& agent, banana::integer_t userId);
	~SubscribeCaseDialog() override;

	bool processMessage(const banana::api::message_t& message) override;
	bool processCallbackQuery(const banana::api::callback_query_t& query) override;

private:
	std::unique_ptr<SubscribeCaseStateMachine> machine_;
};

#endif // COURT_MONITOR_SUBSCRIBE_CASE_DIALOG_H
