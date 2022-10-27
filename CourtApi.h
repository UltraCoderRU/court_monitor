#ifndef COURT_MONITOR_COURT_API_H
#define COURT_MONITOR_COURT_API_H

#include <nlohmann/json_fwd.hpp>

#include <string_view>

nlohmann::json findCases(const std::string_view& name);

nlohmann::json getCaseDetails(int courtId, const std::string_view& caseNumber);

#endif // COURT_MONITOR_COURT_API_H
