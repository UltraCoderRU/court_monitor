#ifndef COURT_MONITOR_BOT_H
#define COURT_MONITOR_BOT_H

#include "BotSession.h"
#include "CourtApi.h"
#include "Storage.h"

#include <banana/agent/beast.hpp>
#include <banana/types_fwd.hpp>

#include <map>

class Bot
{
public:
	explicit Bot(boost::asio::io_context& asioContext, LocalStorage& storage, bool& terminationFlag);

	void notifyUser(int userId,
	                const std::string& caseNumber,
	                std::string caseUrl,
	                const CaseHistoryItem& item);

private:
	void setupCommands();

	void getUpdates();
	void processUpdate(const banana::api::update_t& update);

	LocalStorage& storage_;
	bool& terminationFlag_;
	banana::agent::beast_callback agent_;
	std::int64_t updatesOffset_ = 0;
	std::map<banana::integer_t, BotSession> sessions_;
};

#endif // COURT_MONITOR_BOT_H
