#ifndef COURT_MONITOR_ASIO_H
#define COURT_MONITOR_ASIO_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

static boost::asio::io_context asioContext;

static boost::asio::ssl::context sslContext(boost::asio::ssl::context::tls_client);

void initSSL();

#endif // COURT_MONITOR_ASIO_H
