cmake_minimum_required(VERSION 3.22)

project(ProcessEngine)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
endif()

add_subdirectory(./_header)
add_subdirectory(./_source)
add_subdirectory(./wasp)

string(REPLACE ${PROJECT_SOURCE_DIR}/ "" PROJECT_SOURCES "${PROJECT_SOURCES}")
message(WARNING "${PROJECT_SOURCES}")

message(WARNING "${PROJECT_DIRECTORIES}")
include_directories(${PROJECT_DIRECTORIES})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})