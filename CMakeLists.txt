cmake_minimum_required(VERSION 3.15...4.0)
project(sql_db_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Используем FetchContent для зависимостей
include(FetchContent)

# nlohmann/json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.10.5
)

# Asio
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-28-0
)

FetchContent_MakeAvailable(nlohmann_json asio)

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
    src/db/Database.cpp
    src/db/Row.cpp
    src/db/StorageEngine.cpp
    src/db/StorageEngineIO.cpp
    src/db/Table.cpp
    src/db/ValueUtils.cpp
    src/sql/Executor.cpp
    src/sql/executors/CreateExecutor.cpp
    src/sql/executors/DropExecutor.cpp
    src/sql/executors/UseExecutor.cpp
    src/sql/executors/InsertExecutor.cpp
    src/sql/executors/SelectExecutor.cpp
    src/sql/executors/UpdateExecutor.cpp
    src/sql/executors/DeleteExecutor.cpp
)

target_include_directories(sql_db_engine PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${asio_SOURCE_DIR}/asio/include
)

target_link_libraries(sql_db_engine PRIVATE
    nlohmann_json::nlohmann_json
)