#include "CourtApi.h"

#include "Logger.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <thread>

const char* serverDomain = "mirsud.spb.ru";

namespace {
boost::asio::ssl::context sslContext(boost::asio::ssl::context::tlsv13_client);
using ssl_stream = boost::asio::ssl::stream<boost::beast::tcp_stream>;
} // namespace

ssl_stream connect(boost::asio::io_context& asioContext, const std::string& hostname)
{
	sslContext.set_verify_mode(boost::asio::ssl::verify_peer |
	                           boost::asio::ssl::verify_fail_if_no_peer_cert);
	sslContext.set_default_verify_paths();
	sslContext.load_verify_file("ISRG_X1.pem");
	boost::certify::enable_native_https_server_verification(sslContext);
	ssl_stream stream(asioContext, sslContext);

	static boost::asio::ip::tcp::resolver resolver(asioContext);
	auto const results = resolver.resolve(hostname, "https");
	boost::beast::get_lowest_layer(stream).connect(results);

	boost::certify::set_server_hostname(stream, hostname);
	boost::certify::sni_hostname(stream, hostname);
	stream.handshake(boost::asio::ssl::stream_base::client);

	return stream;
}

std::pair<int, std::string> get(ssl_stream& stream,
                                const std::string& hostname,
                                const std::string& url,
                                std::optional<std::string> payload = {})
{
	// Создать HTTP-запрос
	boost::beast::http::request<boost::beast::http::string_body> request;
	request.method(boost::beast::http::verb::get);
	request.target(url);
	request.keep_alive(true);
	request.set(boost::beast::http::field::host, hostname);

	if (payload)
	{
		request.set(boost::beast::http::field::content_type, "application/json");
		request.body() = std::move(*payload);
	}

	boost::beast::http::write(stream, request);

	boost::beast::http::response<boost::beast::http::string_body> response;
	boost::beast::flat_buffer buffer;
	boost::beast::http::read(stream, buffer, response);

	return {response.base().result_int(), response.body()};
}

nlohmann::json getResults(ssl_stream& stream, const std::string_view& uuid)
{
	int status;
	std::string result;
	std::tie(status, result) =
	    get(stream, serverDomain, fmt::format("/cases/api/results/?id={}", uuid));
	if (status == 200)
	{
		return nlohmann::json::parse(result);
	}
	else
		throw std::runtime_error(
		    fmt::format("failed to retrieve JSON (server returned code {})", status));
}

CaseDetails getCaseDetails(boost::asio::io_context& asioContext, const std::string_view& caseNumber)
{
	ssl_stream stream = connect(asioContext, serverDomain);

	int status;
	std::string result;
	std::tie(status, result) =
	    get(stream, serverDomain, fmt::format("/cases/api/detail/?id={}", caseNumber));
	if (status == 200)
	{
		auto uuid = nlohmann::json::parse(result).at("id").get<std::string>();

		for (int i = 0; i < 5; i++)
		{
			auto response = getResults(stream, uuid);
			if (response.at("finished").get<bool>())
			{
				auto& results = response.at("result");
				LOG(court, results.dump());

				CaseDetails details;
				details.id = results["id"].get<std::string>();
				details.courtNumber = results["court_number"].get<std::string>();
				details.name = results["name"].get<std::string>();
				details.description = results["description"].get<std::string>();
				details.url =
				    fmt::format("https://{}{}", serverDomain, results["url"].get<std::string>());

				details.districtName = results["district_name"].get<std::string>();
				details.judgeName = results["judge"].get<std::string>();

				for (const auto& participant : results["participants"])
				{
					CaseParticipant p;
					p.title = participant["title"].get<std::string>();
					p.name = participant["name"].get<std::string>();
					details.participants.push_back(std::move(p));
				}

				for (const auto& obj : results["history"])
				{
					CaseHistoryItem item;
					item.date = obj.at("date").get<std::string>();
					item.time = obj.at("time").get<std::string>();
					item.status = obj.at("status").get<std::string>();
					item.publishDate = obj.at("publish_date").get<std::string>();
					item.publishTime = obj.at("publish_time").get<std::string>();
					details.history.push_back(std::move(item));
				}

				return details;
			}
			else
				std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		throw std::runtime_error("failed to get results in time");
	}
	else
		throw std::runtime_error(
		    fmt::format("failed to retrieve JSON (server returned code {})", status));
}
