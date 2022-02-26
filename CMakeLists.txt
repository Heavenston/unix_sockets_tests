cmake_minimum_required(VERSION 3.10)

# set the project name
project(
    DataGrams
    LANGUAGES C
)

file(GLOB_RECURSE UTILS_SOURCES "utils/*.c")
add_library(Utils STATIC ${UTILS_SOURCES})

set(CMAKE_C_FLAGS "-Werror=implicit-function-declaration")

function(create_exec name folder)
    file(GLOB_RECURSE srcs "${folder}/*.c")
    add_executable(${name} ${srcs})

    target_include_directories(${name} PUBLIC "${folder}")

    target_link_libraries(${name} PUBLIC Utils)
    target_include_directories(${name} PUBLIC "utils")
endfunction()

create_exec("DgramPaired" "dgram_paired")
create_exec("DgramConnected" "dgram_connected")
create_exec("StreamConnected" "stream_connected")
create_exec("StreamEpoll" "stream_epoll")
