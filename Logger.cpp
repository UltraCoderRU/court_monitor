#include "Logger.h"

void vlog(std::FILE* stream, const char* category, fmt::string_view format, fmt::format_args args)
{
	log(stream, category, fmt::vformat(format, args));
}

void log(std::FILE* stream, const char* category, const std::string_view& message)
{
	auto formattedCategory = fmt::format("({})", category);
	fmt::print(stream, "{: >16} {}\n", formattedCategory, message);
}
