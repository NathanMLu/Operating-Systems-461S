cmake_minimum_required(VERSION 3.10)

# Set the project name
project(yash LANGUAGES C)

# Add the executable
add_executable(yash
        src/main.c
        src/job_control.c
        src/parser.c
        src/process.c
        src/signals.c
)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Link the readline library
find_library(READLINE_LIB readline REQUIRED)

# Add platform-specific flags
if(APPLE)
    target_link_libraries(yash ${READLINE_LIB})
else()
    target_link_libraries(yash ${READLINE_LIB})
endif()

# Enable stricter warnings
target_compile_options(yash PRIVATE -Wall -Wextra -pedantic)