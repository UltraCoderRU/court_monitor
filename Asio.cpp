#include "Asio.h"

#include <boost/certify/https_verification.hpp>

void initSSL()
{
	sslContext.set_verify_mode(boost::asio::ssl::verify_peer |
	                           boost::asio::ssl::verify_fail_if_no_peer_cert);
	sslContext.set_default_verify_paths();
	boost::certify::enable_native_https_server_verification(sslContext);
}
