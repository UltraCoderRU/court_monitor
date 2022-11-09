#ifndef COURT_MONITOR_COURT_API_H
#define COURT_MONITOR_COURT_API_H

#include <nlohmann/json_fwd.hpp>

#include <boost/asio/io_context.hpp>

#include <string>
#include <string_view>
#include <vector>

struct CaseHistoryItem
{
	std::string date;
	std::string time;
	std::string status;
	std::string publishDate;
	std::string publishTime;
};

nlohmann::json findCases(boost::asio::io_context& asioContext, const std::string_view& name);

nlohmann::json getCaseDetails(boost::asio::io_context& asioContext,
                              int courtId,
                              const std::string_view& caseNumber);

std::vector<CaseHistoryItem> parseHistory(const nlohmann::json& details);

#endif // COURT_MONITOR_COURT_API_H
