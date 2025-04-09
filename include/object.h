#pragma once

#include <climits>
#include <memory>

enum class EntityColor {
  NIL = INT_MAX, // Empty color

  kRED = 0xFF0000,
  kGREEN = 0x00FF00,
  kBLUE = 0x0000FF,
  kYELLOW = 0xFFFF00,
  kCYAN = 0x00FFFF,
  kMAGENTA = 0xFF00FF,
  kBLACK = 0,
  kWHITE = 0xFFFFFF,

  kORANGE = 0xFF8000,
  kGREY = 0x808080,
  kDARK = 0x404040,
  kLIGHT = 0x0C0C0C,

  // Furries!
  // 2025-04-09 21:52 ? What-the-fuck?
  kSP2 = 0xCEB7FF,
  kSP1 = 0x8AD8FF
};

class Entity {
public:
  EntityColor color_;
  bool is_path_;          // Is Pathable (false if cut or is symbol cell)
  bool is_path_occupied_; // Is there a path here

  Entity()
      : is_path_(false), is_path_occupied_(false), color_(EntityColor::NIL) {}
  Entity(EntityColor c)
      : is_path_(false), is_path_occupied_(false), color_(c) {}

  std::shared_ptr<Entity> Clear() {
    auto res = std::make_shared<Entity>();
    res->is_path_ = is_path_;
    res->is_path_occupied_ = is_path_occupied_;
    res->color_ = EntityColor::NIL;
    return res;
  }

  virtual ~Entity() {}
};
