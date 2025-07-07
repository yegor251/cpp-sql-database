cmake_minimum_required(VERSION 3.29)
project(sql_db_engine)

set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/opt/homebrew/include)

add_executable(sql_db_engine 
    src/main.cpp
    src/sql/Parser.cpp
    src/sql/parsers/Utils.cpp
    src/sql/parsers/CreateParser.cpp
    src/sql/parsers/DropParser.cpp
    src/sql/parsers/InsertParser.cpp
    src/sql/parsers/SelectParser.cpp
    src/sql/parsers/OtherParsers.cpp
    src/sql/parsers/DeleteParser.cpp
    src/sql/parsers/UpdateParser.cpp
    src/sql/parsers/UseParser.cpp
    src/net/Server.cpp
    src/db/StorageEngineIO.cpp
    src/sql/Executor.cpp)
