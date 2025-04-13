include_directories(/usr/include/readline)
link_directories(/usr/lib)

cmake_minimum_required(VERSION 3.10)
project(pgshell)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
# Find and link Readline manually
find_library(Readline_LIBRARY readline)


# Add nlohmann/json as a dependency
include(FetchContent)
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Source files
set(SOURCES
    main.cpp
    helper.cpp
    index.cpp
    memory.cpp
    python_bridge.cpp
)

# Create the executable
add_executable(pgshell ${SOURCES})

# Include directories
target_include_directories(pgshell PRIVATE
    ${PQXX_INCLUDE_DIRS}
    ${Readline_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(pgshell PRIVATE
    ${PQXX_LIBRARIES}
    ${Readline_LIBRARY}
    nlohmann_json::nlohmann_json
)


# Copy the Python parser to the build directory
configure_file(query_parser.py ${CMAKE_BINARY_DIR}/query_parser.py COPYONLY)

# Install target
install(TARGETS pgshell DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/query_parser.py DESTINATION bin)