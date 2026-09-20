#include "rm_decision/domain/zone_loader.hpp"

#include <yaml-cpp/yaml.h>

#include <stdexcept>
#include <utility>
#include <vector>

namespace rm_decision
{

void loadDefaultRmucZones(ZoneMap * _out)
{
  if (_out == nullptr) {
    return;
  }
  ZoneMap map;
  map.setNavPoints({
    {3.0, 2.6, 0.0},    // HOME
    {12.8, 5.5, 0.0},   // BONUS
    {15.2, 11.2, 0.0},  // ENEMY_OUTPOST
    {7.2, 7.5, 0.0},    // OWN_FORT
    {22.0, 7.5, 0.0},   // ENEMY_FORT
    {12.1, 3.9, 0.0},   // OWN_OUTPOST
    {8.8, 13.6, 0.0},   // HERO_GUARD
  });
  // own_supply: bit Area_Square top_left(3.8,4.4) bottom_right(1.5,0.0)
  map.addRect({"own_supply", 1.5, 3.8, 0.0, 4.4});
  // own_outpost approx AABB of bit polygon
  map.addRect({"own_outpost", 11.1, 12.4, 2.5, 4.5});
  map.addCircle({"enemy_fort", 22.0, 7.5, 1.0});
  *_out = std::move(map);
}


bool loadZoneMapFromYaml(const std::string & _path, ZoneMap * _out, std::string * _err)
{
  if (_out == nullptr) {
    if (_err) {
      *_err = "null ZoneMap";
    }
    return false;
  }
  try {
    const YAML::Node root = YAML::LoadFile(_path);
    ZoneMap map;
    std::vector<Point2d_s> pts;
    if (root["nav_points"]) {
      for (const auto & n : root["nav_points"]) {
        Point2d_s p;
        p.x = n["x"].as<double>();
        p.y = n["y"].as<double>();
        p.yaw = n["yaw"] ? n["yaw"].as<double>() : 0.0;
        pts.push_back(p);
      }
    }
    if (pts.empty()) {
      throw std::runtime_error("nav_points empty");
    }
    map.setNavPoints(std::move(pts));

    if (root["rects"]) {
      for (const auto & n : root["rects"]) {
        RectZone_s z;
        z.name = n["name"].as<std::string>();
        z.min_x = n["min_x"].as<double>();
        z.max_x = n["max_x"].as<double>();
        z.min_y = n["min_y"].as<double>();
        z.max_y = n["max_y"].as<double>();
        map.addRect(std::move(z));
      }
    }
    if (root["circles"]) {
      for (const auto & n : root["circles"]) {
        CircleZone_s z;
        z.name = n["name"].as<std::string>();
        z.cx = n["cx"].as<double>();
        z.cy = n["cy"].as<double>();
        z.radius = n["radius"].as<double>();
        map.addCircle(std::move(z));
      }
    }
    *_out = std::move(map);
    return true;
  } catch (const std::exception & ex) {
    if (_err) {
      *_err = ex.what();
    }
    loadDefaultRmucZones(_out);
    return false;
  }
}

}  // namespace rm_decision
