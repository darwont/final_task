#include "json_loader.h"
#include <fstream>
#include <sstream>
#include <boost/json.hpp>
#include <boost/json/src.hpp>

namespace json = boost::json;

namespace json_loader {
model::Game LoadGame(const std::filesystem::path& json_path) {
    model::Game game;
    std::ifstream file(json_path);
    if (!file) throw std::runtime_error("Cannot open config file");

    std::stringstream buffer;
    buffer << file.rdbuf();
    auto value = json::parse(buffer.str());
    
    for (const auto& map_val : value.as_object().at("maps").as_array()) {
        const auto& map_obj = map_val.as_object();
        model::Map map(map_obj.at("id").as_string().c_str(), map_obj.at("name").as_string().c_str());
        
        for (const auto& r_val : map_obj.at("roads").as_array()) {
            const auto& r = r_val.as_object();
            int x0 = r.at("x0").as_int64(), y0 = r.at("y0").as_int64();
            if (r.contains("x1")) map.AddRoad(model::Road(model::Road::HORIZONTAL, {x0, y0}, r.at("x1").as_int64()));
            else map.AddRoad(model::Road(model::Road::VERTICAL, {x0, y0}, r.at("y1").as_int64()));
        }
        
        if (map_obj.contains("buildings")) {
            for (const auto& b_val : map_obj.at("buildings").as_array()) {
                const auto& b = b_val.as_object();
                map.AddBuilding(model::Building{model::Rectangle{
                    {static_cast<int>(b.at("x").as_int64()), static_cast<int>(b.at("y").as_int64())},
                    {static_cast<int>(b.at("w").as_int64()), static_cast<int>(b.at("h").as_int64())}
                }});
            }
        }
        
        if (map_obj.contains("offices")) {
            for (const auto& o_val : map_obj.at("offices").as_array()) {
                const auto& o = o_val.as_object();
                map.AddOffice(model::Office{
                    o.at("id").as_string().c_str(),
                    {static_cast<int>(o.at("x").as_int64()), static_cast<int>(o.at("y").as_int64())},
                    {static_cast<int>(o.at("offsetX").as_int64()), static_cast<int>(o.at("offsetY").as_int64())}
                });
            }
        }
        game.AddMap(std::move(map));
    }
    return game;
}
}
