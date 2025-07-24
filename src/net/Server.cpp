#include <asio.hpp>
#include <thread>
#include <iostream>
#include <atomic>
#include <string>
#include <functional>
#include "sql/Parser.hpp"
#include "db/StorageEngine.hpp"
#include "db/StorageEngineIO.hpp"
#include "sql/Executor.hpp"

using asio::ip::tcp;

namespace {

constexpr std::string_view dbfile = "dbdata.json";
std::atomic<bool> running{true};
db::StorageEngine engine;

void console_handler() {
    std::string input;
    while (running) {
        std::getline(std::cin, input);
        if (input == "stop" || input == "quit" || input == "exit") {
            std::cout << "Stopping server...\n";
            running = false;
            break;
        }
    }
}

void session(tcp::socket sock) {
    try {
        for (;;) {
            std::string data(1024, '\0');
            asio::error_code error;
            size_t length = sock.read_some(asio::buffer(data), error);
            if (error == asio::error::eof) break;
            if (error) throw asio::system_error(error);

            data.resize(length);
            auto res = sql::Parser::parse(data);
            std::string response;
            if (!res.valid) {
                response = "Parse error: " + res.error + "\n";
            } else {
                auto exec = sql::Executor::execute(res, engine);
                if (exec.ok) response = "OK\n";
                else response = "Error: " + exec.error + "\n";
            }
            asio::write(sock, asio::buffer(response), error);
            if (error) break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Session error: " << e.what() << std::endl;
    }
}

} // namespace

void run_server(short port) {
    // Загрузка БД
    if (!db::load_from_file(engine, dbfile)) {
        std::cout << "No DB file, starting fresh\n";
    } else {
        std::cout << "DB loaded\n";
    }

    asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Type 'stop' to shutdown server\n";

    std::thread console_thread(console_handler);

    // Асинхронный accept с лямбдой
    std::function<void()> do_accept;
    std::atomic<bool> accept_in_progress{false};

    do_accept = [&]() {
        if (!running) return;

        accept_in_progress = true;
        auto socket = std::make_shared<tcp::socket>(io_context);

        acceptor.async_accept(*socket, [&, socket](const asio::error_code& error) {
            accept_in_progress = false;

            if (!error) {
                std::thread(session, std::move(*socket)).detach();
                do_accept();
            } else if (error != asio::error::operation_aborted) {
                std::cerr << "Accept error: " << error.message() << std::endl;
            }
        });
    };

    do_accept();

    // Основной цикл обработки событий
    while (running) {
        io_context.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Завершение работы
    acceptor.cancel();
    acceptor.close();
    io_context.stop();
    io_context.poll();

    if (console_thread.joinable())
        console_thread.join();

    std::cout << "Saving database...\n";
    db::save_to_file(engine, dbfile);
    std::cout << "Server stopped\n";
}