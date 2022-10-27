#include "CourtApi.h"

#include "Asio.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <boost/asio/ssl/stream.hpp>
#include <boost/beast.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <iostream>
#include <thread>

const char* serverDomain = "mirsud.spb.ru";

using ssl_stream = boost::asio::ssl::stream<boost::beast::tcp_stream>;

ssl_stream connect(const std::string& hostname)
{
	ssl_stream stream(asioContext, sslContext);

	static boost::asio::ip::tcp::resolver resolver(asioContext);
	auto const results = resolver.resolve(hostname, "https");

	boost::certify::set_server_hostname(stream, hostname);
	boost::certify::sni_hostname(stream, hostname);
	boost::beast::get_lowest_layer(stream).connect(results);
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

	std::cout << "tx: " << request << std::endl;

	boost::beast::http::write(stream, request);

	boost::beast::http::response<boost::beast::http::string_body> response;
	boost::beast::flat_buffer buffer;
	boost::beast::http::read(stream, buffer, response);
	std::cout << "rx: " << response << std::endl;

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

nlohmann::json getCaseDetails(int courtId, const std::string_view& caseNumber)
{
	ssl_stream stream = connect(serverDomain);

	int status;
	std::string result;
	std::tie(status, result) =
	    get(stream, serverDomain,
	        fmt::format("/cases/api/detail/?id={}&court_site_id={}", caseNumber, courtId));
	if (status == 200)
	{
		auto uuid = nlohmann::json::parse(result).at("id").get<std::string>();

		for (int i = 0; i < 10; i++)
		{
			auto results = getResults(stream, uuid);
			bool finished = results.at("finished").get<bool>();
			if (finished)
				return results.at("result");
			else
				std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		throw std::runtime_error("failed to get results in time");
	}
	else
		throw std::runtime_error(
		    fmt::format("failed to retrieve JSON (server returned code {})", status));
}
