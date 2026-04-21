#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>
#include <string>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game) : game_{game} {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        send(HandleRequest(std::move(req)));
    }

private:
    model::Game& game_;

    template <typename Body, typename Allocator>
    http::response<http::string_body> HandleRequest(http::request<Body, http::basic_fields<Allocator>>&& req) {
        std::string target(req.target());

        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            return MakeResponse(http::status::method_not_allowed, req.version(), req.keep_alive(),
                                "{\"code\": \"invalidMethod\", \"message\": \"Invalid method\"}", "Allow", "GET, HEAD");
        }

        if (target == "/api/v1/maps" || target == "/api/v1/maps/") {
            json::array maps_arr;
            for (const auto& map : game_.GetMaps()) {
                maps_arr.push_back({{"id", map->GetId()}, {"name", map->GetName()}});
            }
            return MakeResponse(http::status::ok, req.version(), req.keep_alive(), json::serialize(maps_arr));
        }

        if (target.starts_with("/api/v1/maps/")) {
            std::string map_id = target.substr(13);
            const auto* map = game_.FindMap(map_id);
            if (!map) {
                return MakeResponse(http::status::not_found, req.version(), req.keep_alive(),
                                    "{\"code\": \"mapNotFound\", \"message\": \"Map not found\"}");
            }
            
            json::array roads_arr;
            for (const auto& r : map->GetRoads()) {
                if (r.IsHorizontal()) roads_arr.push_back({{"x0", r.GetStart().x}, {"y0", r.GetStart().y}, {"x1", r.GetEnd().x}});
                else roads_arr.push_back({{"x0", r.GetStart().x}, {"y0", r.GetStart().y}, {"y1", r.GetEnd().y}});
            }
            
            json::array bldgs_arr;
            for (const auto& b : map->GetBuildings()) {
                bldgs_arr.push_back({{"x", b.GetBounds().position.x}, {"y", b.GetBounds().position.y},
                                     {"w", b.GetBounds().size.width}, {"h", b.GetBounds().size.height}});
            }
            
            json::array off_arr;
            for (const auto& o : map->GetOffices()) {
                off_arr.push_back({{"id", o.GetId()}, {"x", o.GetPosition().x}, {"y", o.GetPosition().y},
                                   {"offsetX", o.GetOffset().dx}, {"offsetY", o.GetOffset().dy}});
            }

            json::object map_obj = {{"id", map->GetId()}, {"name", map->GetName()}, 
                                    {"roads", roads_arr}, {"buildings", bldgs_arr}, {"offices", off_arr}};
            return MakeResponse(http::status::ok, req.version(), req.keep_alive(), json::serialize(map_obj));
        }

        return MakeResponse(http::status::bad_request, req.version(), req.keep_alive(),
                            "{\"code\": \"badRequest\", \"message\": \"Bad request\"}");
    }

    http::response<http::string_body> MakeResponse(http::status status, unsigned version, bool keep_alive,
                                                   const std::string& body,
                                                   const std::string& ext_header_name = "",
                                                   const std::string& ext_header_val = "") {
        http::response<http::string_body> res(status, version);
        res.set(http::field::content_type, "application/json");
        if (!ext_header_name.empty()) res.set(ext_header_name, ext_header_val);
        res.body() = body;
        res.content_length(body.size());
        res.keep_alive(keep_alive);
        return res;
    }
};
}
