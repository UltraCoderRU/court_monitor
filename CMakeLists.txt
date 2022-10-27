cmake_minimum_required(VERSION 3.5)
project(court_monitor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(OpenSSL REQUIRED)

find_package(Boost COMPONENTS system REQUIRED)

add_subdirectory(external)

add_executable(court_monitor
	Asio.cpp
	CourtApi.cpp
	Storage.cpp
	main.cpp
	)
target_link_libraries(court_monitor
	banana-beast
	fmt::fmt
	nlohmann_json::nlohmann_json
	certify::core
	Boost::system
	${OPENSSL_LIBRARIES}
	)
