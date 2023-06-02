cmake_minimum_required(VERSION 3.22)

project(ProcessEngine)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
endif()

macro(recursive_add_all)
    #include all source files into main list
    file(GLOB_RECURSE LOCAL_PROJECT_SOURCES CONFIGURE_DEPENDS *.h *.cpp)
    set(PROJECT_SOURCES ${PROJECT_SOURCES}${LOCAL_PROJECT_SOURCES}\; PARENT_SCOPE)

    #include all subdirectories
    file(GLOB_RECURSE FILE_LIST LIST_DIRECTORIES true CONFIGURE_DEPENDS *)
    foreach(DIR ${FILE_LIST})
        if(IS_DIRECTORY ${DIR})
            set(LOCAL_DIRECTORIES ${LOCAL_DIRECTORIES}${DIR}\;)
        endif()
    endforeach()

    #include this directory
    set(LOCAL_DIRECTORIES ${LOCAL_DIRECTORIES}${CMAKE_CURRENT_LIST_DIR}\;)

    set(PROJECT_DIRECTORIES ${PROJECT_DIRECTORIES}\;${LOCAL_DIRECTORIES} PARENT_SCOPE)
endmacro()

add_subdirectory(./_header)
add_subdirectory(./_source)
add_subdirectory(./wasp)
add_subdirectory(./darkness)
add_subdirectory(./_shaders)

include_directories(${PROJECT_DIRECTORIES})

add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})

add_dependencies(${PROJECT_NAME} shaders)

#https://github.com/holy-shit/clion-directx-example
set(LIBS d3d11 d3dcompiler winmm shlwapi)

target_link_libraries(ProcessEngine ${LIBS})