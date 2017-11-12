cmake_minimum_required (VERSION 3.8)
project (JsonParser)

file(GLOB SOURCES
    src/*.h
    src/*.c
)
add_definitions(-DBUILD_TEST)

add_executable(JsonParserTest ${SOURCES})