cmake_minimum_required(VERSION 3.10)
project(pgshell)

set(CMAKE_CXX_STANDARD 17)

# Manually set include and library paths like in Makefile
include_directories(/opt/homebrew/include /usr/include/readline)
link_directories(/opt/homebrew/lib /usr/lib)

# Source files
set(SOURCES
    main.cpp
    helper.cpp
    index.cpp
    memory.cpp
)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Find and link Readline manually
find_library(Readline_LIBRARY readline)

# Create the executable
add_executable(pgshell ${SOURCES})

# Include directories
target_include_directories(pgshell PRIVATE
    ${PQXX_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(pgshell PRIVATE
    ${PQXX_LIBRARIES}
    ${Readline_LIBRARY}
)
