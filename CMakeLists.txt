cmake_minimum_required (VERSION 2.8)
project (JsonParser)

set (CMAKE_C_STANDARD 99)

file(GLOB SOURCES
    src/*.h
    src/*.c
)
add_definitions(-DBUILD_TEST)

add_executable(JsonParserTest ${SOURCES})