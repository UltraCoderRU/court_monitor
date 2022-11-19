#ifndef COURT_MONITOR_COURT_API_H
#define COURT_MONITOR_COURT_API_H

#include <nlohmann/json_fwd.hpp>

#include <boost/asio/io_context.hpp>

#include <string>
#include <string_view>
#include <vector>

struct CaseParticipant
{
	std::string title;
	std::string name;
};

struct CaseHistoryItem
{
	std::string date;
	std::string time;
	std::string status;
	std::string publishDate;
	std::string publishTime;
};

struct CaseDetails
{
	std::string id;
	std::string courtNumber;
	std::string name;
	std::string description;
	std::string url;

	std::string districtName;
	std::string judgeName;
	std::vector<CaseParticipant> participants;

	std::string status;
	std::vector<CaseHistoryItem> history;

	std::string createdDate;
	std::string acceptedDate;
	std::string judicialUid;
};

std::vector<CaseDetails> findCases(boost::asio::io_context& asioContext,
                                   const std::string_view& name);

CaseDetails getCaseDetails(boost::asio::io_context& asioContext, const std::string_view& caseNumber);

#endif // COURT_MONITOR_COURT_API_H
