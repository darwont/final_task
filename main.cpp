#include "json_loader.h"
#include "request_handler.h"
#include "http_server.h"
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>
#include <vector>

namespace net = boost::asio;

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: game_server <game-config-json>" << std::endl;
        return 1;
    }
    
    try {
        model::Game game = json_loader::LoadGame(argv[1]);

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec, int) {
            if (!ec) ioc.stop();
        });

        auto handler = std::make_shared<http_handler::RequestHandler>(game);
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        
        http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, auto&& send) {
            (*handler)(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        std::cout << "Server has started..." << std::endl;

        std::vector<std::thread> workers;
        for (unsigned i = 0; i < num_threads - 1; ++i) {
            workers.emplace_back([&ioc] { ioc.run(); });
        }
        ioc.run();
        for (auto& w : workers) { w.join(); }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
