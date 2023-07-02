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

# https://stackoverflow.com/questions/13429656/how-to-copy-contents-of-a-directory-into-build-directory-after-make-with-cmake
# copy res
add_custom_target(res)
add_custom_command(TARGET res POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)
# copy scripts
add_custom_target(scripts)
add_custom_command(TARGET scripts POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/scripts/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/res/scripts)

include_directories(${PROJECT_DIRECTORIES})

add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})

add_dependencies(${PROJECT_NAME} shaders)
add_dependencies(${PROJECT_NAME} res)
add_dependencies(${PROJECT_NAME} scripts)

#https://github.com/holy-shit/clion-directx-example
set(LIBS d3d11 d3dcompiler winmm shlwapi)

target_link_libraries(ProcessEngine ${LIBS})