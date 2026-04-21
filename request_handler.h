#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>
#include <string>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

struct Endpoints {
    static constexpr std::string_view MAPS = "/api/v1/maps";
    static constexpr std::string_view MAPS_PREFIX = "/api/v1/maps/";
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game);

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        send(HandleRequest(req.method(), req.target(), req.version(), req.keep_alive()));
    }

private:
    model::Game& game_;

    http::response<http::string_body> HandleRequest(
        http::verb method, std::string_view target, unsigned version, bool keep_alive) const;

    http::response<http::string_body> MakeResponse(
        http::status status, unsigned version, bool keep_alive,
        const std::string& body,
        const std::string& ext_header_name = "",
        const std::string& ext_header_val = "") const;

    json::array SerializeMaps() const;
    json::object SerializeMap(const model::Map& map) const;
};

} // namespace http_handler
