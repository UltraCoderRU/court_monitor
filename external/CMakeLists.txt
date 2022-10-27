add_subdirectory(fmt EXCLUDE_FROM_ALL)

add_subdirectory(nlohmann_json EXCLUDE_FROM_ALL)

add_subdirectory(banana EXCLUDE_FROM_ALL)

set(BUILD_TESTING OFF CACHE INTERNAL "")
add_subdirectory(certify EXCLUDE_FROM_ALL)
