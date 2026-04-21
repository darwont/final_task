#pragma once
#include <string>
#include <vector>
#include <memory>

namespace model {
struct Point { int x, y; };
struct Size { int width, height; };
struct Rectangle { Point position; Size size; };
struct Offset { int dx, dy; };

class Road {
public:
    enum Direction { HORIZONTAL, VERTICAL };
    Road(Direction dir, Point start, int end) : dir_(dir), start_(start), end_{dir == HORIZONTAL ? end : start.x, dir == VERTICAL ? end : start.y} {}
    bool IsHorizontal() const { return dir_ == HORIZONTAL; }
    Point GetStart() const { return start_; }
    Point GetEnd() const { return end_; }
private:
    Direction dir_;
    Point start_, end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) : bounds_(bounds) {}
    const Rectangle& GetBounds() const { return bounds_; }
private:
    Rectangle bounds_;
};

class Office {
public:
    Office(std::string id, Point position, Offset offset) : id_(std::move(id)), position_(position), offset_(offset) {}
    const std::string& GetId() const { return id_; }
    Point GetPosition() const { return position_; }
    Offset GetOffset() const { return offset_; }
private:
    std::string id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    Map(std::string id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}
    const std::string& GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    void AddRoad(const Road& road) { roads_.push_back(road); }
    void AddBuilding(const Building& b) { buildings_.push_back(b); }
    void AddOffice(const Office& o) { offices_.push_back(o); }
    const std::vector<Road>& GetRoads() const { return roads_; }
    const std::vector<Building>& GetBuildings() const { return buildings_; }
    const std::vector<Office>& GetOffices() const { return offices_; }
private:
    std::string id_, name_;
    std::vector<Road> roads_;
    std::vector<Building> buildings_;
    std::vector<Office> offices_;
};

class Game {
public:
    void AddMap(Map map) { maps_.emplace_back(std::make_shared<Map>(std::move(map))); }
    const std::vector<std::shared_ptr<Map>>& GetMaps() const { return maps_; }
    const Map* FindMap(const std::string& id) const {
        for (const auto& map : maps_) {
            if (map->GetId() == id) return map.get();
        }
        return nullptr;
    }
private:
    std::vector<std::shared_ptr<Map>> maps_;
};
}
