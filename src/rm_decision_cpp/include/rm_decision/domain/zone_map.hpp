#pragma once

#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace rm_decision
{

struct Point2d_s
{
  double x{0.0};
  double y{0.0};
  double yaw{0.0};
};


struct RectZone_s
{
  std::string name;
  double min_x{0.0};
  double max_x{0.0};
  double min_y{0.0};
  double max_y{0.0};

  bool contains(double _x, double _y) const
  {
    return _x >= min_x && _x <= max_x && _y >= min_y && _y <= max_y;
  }
};


struct CircleZone_s
{
  std::string name;
  double cx{0.0};
  double cy{0.0};
  double radius{0.0};

  bool contains(double _x, double _y) const
  {
    const double dx = _x - cx;
    const double dy = _y - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
  }
};


class ZoneMap
{
public:
  void setNavPoints(std::vector<Point2d_s> _pts) { nav_points_ = std::move(_pts); }
  void addRect(RectZone_s _z) { rects_.push_back(std::move(_z)); }
  void addCircle(CircleZone_s _z) { circles_.push_back(std::move(_z)); }

  bool getNavPoint(int _idx, Point2d_s * _out) const
  {
    if (_out == nullptr || _idx < 0 || _idx >= static_cast<int>(nav_points_.size())) {
      return false;
    }
    *_out = nav_points_[static_cast<std::size_t>(_idx)];
    return true;
  }

  bool inZone(const std::string & _name, double _x, double _y) const
  {
    for (const auto & r : rects_) {
      if (r.name == _name && r.contains(_x, _y)) {
        return true;
      }
    }
    for (const auto & c : circles_) {
      if (c.name == _name && c.contains(_x, _y)) {
        return true;
      }
    }
    return false;
  }

  const std::vector<Point2d_s> & navPoints() const { return nav_points_; }

private:
  std::vector<Point2d_s> nav_points_;
  std::vector<RectZone_s> rects_;
  std::vector<CircleZone_s> circles_;
};

}  // namespace rm_decision
