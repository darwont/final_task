#include "request_handler.h"

namespace http_handler {

RequestHandler::RequestHandler(model::Game& game)
    : game_{game} {
}

http::response<http::string_body> RequestHandler::MakeResponse(
    http::status status, unsigned version, bool keep_alive,
    const std::string& body,
    const std::string& ext_header_name,
    const std::string& ext_header_val) const {
    
    http::response<http::string_body> res(status, version);
    res.set(http::field::content_type, "application/json");
    
    if (!ext_header_name.empty()) {
        res.set(ext_header_name, ext_header_val);
    }
    
    res.body() = body;
    res.content_length(body.size());
    res.keep_alive(keep_alive);
    return res;
}

json::array RequestHandler::SerializeMaps() const {
    json::array maps_arr;
    for (const auto& map : game_.GetMaps()) {
        maps_arr.push_back({
            {"id", map->GetId()},
            {"name", map->GetName()}
        });
    }
    return maps_arr;
}

json::object RequestHandler::SerializeMap(const model::Map& map) const {
    json::array roads_arr;
    for (const auto& r : map.GetRoads()) {
        if (r.IsHorizontal()) {
            roads_arr.push_back({
                {"x0", r.GetStart().x},
                {"y0", r.GetStart().y},
                {"x1", r.GetEnd().x}
            });
        } else {
            roads_arr.push_back({
                {"x0", r.GetStart().x},
                {"y0", r.GetStart().y},
                {"y1", r.GetEnd().y}
            });
        }
    }
    
    json::array bldgs_arr;
    for (const auto& b : map.GetBuildings()) {
        bldgs_arr.push_back({
            {"x", b.GetBounds().position.x},
            {"y", b.GetBounds().position.y},
            {"w", b.GetBounds().size.width},
            {"h", b.GetBounds().size.height}
        });
    }
    
    json::array off_arr;
    for (const auto& o : map.GetOffices()) {
        off_arr.push_back({
            {"id", o.GetId()},
            {"x", o.GetPosition().x},
            {"y", o.GetPosition().y},
            {"offsetX", o.GetOffset().dx},
            {"offsetY", o.GetOffset().dy}
        });
    }

    return {
        {"id", map.GetId()},
        {"name", map.GetName()},
        {"roads", roads_arr},
        {"buildings", bldgs_arr},
        {"offices", off_arr}
    };
}

http::response<http::string_body> RequestHandler::HandleRequest(
    http::verb method, std::string_view target, unsigned version, bool keep_alive) const {
    
    if (method != http::verb::get && method != http::verb::head) {
        return MakeResponse(http::status::method_not_allowed, version, keep_alive,
                            "{\"code\": \"invalidMethod\", \"message\": \"Invalid method\"}",
                            "Allow", "GET, HEAD");
    }

    if (target == Endpoints::MAPS || target == std::string(Endpoints::MAPS) + "/") {
        return MakeResponse(http::status::ok, version, keep_alive,
                            json::serialize(SerializeMaps()));
    }

    if (target.starts_with(Endpoints::MAPS_PREFIX)) {
        std::string map_id(target.substr(Endpoints::MAPS_PREFIX.size()));
        const auto* map = game_.FindMap(map_id);
        
        if (!map) {
            return MakeResponse(http::status::not_found, version, keep_alive,
                                "{\"code\": \"mapNotFound\", \"message\": \"Map not found\"}");
        }
        
        return MakeResponse(http::status::ok, version, keep_alive,
                            json::serialize(SerializeMap(*map)));
    }

    return MakeResponse(http::status::bad_request, version, keep_alive,
                        "{\"code\": \"badRequest\", \"message\": \"Bad request\"}");
}

} // namespace http_handler
