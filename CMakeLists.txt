cmake_minimum_required(VERSION 3.22)

project(ProcessEngine)

set(CMAKE_CXX_STANDARD 17)

include_directories(./_header)
include_directories(./_source)
include_directories(./wasp/_debug)
include_directories(./wasp/_header)
include_directories(./wasp/_source)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
endif()

add_executable(${PROJECT_NAME} _source/main.cpp wasp/_header/windowsInclude.h wasp/_header/targetver.h wasp/_debug/ConsoleOutput.h wasp/_debug/Logging.h)