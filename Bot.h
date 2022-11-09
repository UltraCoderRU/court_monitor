#ifndef COURT_MONITOR_BOT_H
#define COURT_MONITOR_BOT_H

#include "CourtApi.h"
#include "Storage.h"

#include <banana/agent/beast.hpp>
#include <banana/types_fwd.hpp>

class Bot
{
public:
	explicit Bot(boost::asio::io_context& asioContext, LocalStorage& storage, bool& terminationFlag);

	void setupCommands();

	void notifyUser(int userId,
	                const std::string& caseNumber,
	                std::string caseUrl,
	                const CaseHistoryItem& item);

private:
	void getUpdates();
	void processUpdate(const banana::api::update_t& update);

	void processStartCommand(const banana::api::message_t& message);
	void processSubscribeCaseCommand(const banana::api::message_t& message);

	LocalStorage& storage_;
	bool& terminationFlag_;
	banana::agent::beast_callback agent_;
	std::int64_t updatesOffset_ = 0;
};

#endif // COURT_MONITOR_BOT_H
