#ifndef COURT_MONITOR_LOGGER_H
#define COURT_MONITOR_LOGGER_H

#include <fmt/core.h>

// clang-format off

#define _CAT(a, b) a ## b
#define _SELECT(PREFIX, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, SUFFIX, ...) PREFIX##_##SUFFIX

#define _LOG_1(stream, category, text) log(stream, category, text)
#define _LOG_N(stream, category, format, ...) log(stream, category, format, __VA_ARGS__)

#define _LOG(stream, category, ...) _CAT(_SELECT(_LOG, __VA_ARGS__, N, N, N, N, N, N, N, N, N, 1,)(stream, category, __VA_ARGS__),)

#define LOG(category, ...) _LOG(stdout, #category, __VA_ARGS__)
#define LOGE(category, ...) _LOG(stderr, #category, __VA_ARGS__)

// clang-format on

void log(std::FILE* stream, const char* category, const std::string_view& message);

void vlog(std::FILE* stream, const char* category, fmt::string_view format, fmt::format_args args);

template <typename... T>
void log(std::FILE* stream, const char* category, fmt::format_string<T...> format, T&&... args)
{
	vlog(stream, category, format, fmt::make_format_args(args...));
}

#endif // COURT_MONITOR_LOGGER_H
