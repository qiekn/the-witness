#pragma once

#include "object.h"

class Endpoint : public Entity {
public:
  bool starting_;
  Endpoint(bool s) : Entity(), starting_(s) { is_path_ = true; }
};

/**
 * @class Dot
 * @brief Hexagon Dots
 *
 */
class Dot : public Entity {
public:
  Dot() : Entity() { is_path_ = true; }
};

/**
 * @class Blob
 * @brief Colored Squares
 *
 */
class Blob : public Entity {
public:
  Blob() : Entity(EntityColor::kBLACK) {}
  Blob(EntityColor c) : Entity(c) {}
};

class Star : public Entity {
public:
  Star() : Entity(EntityColor::kWHITE) {}
  Star(EntityColor c) : Entity(c) {}
};

class Triangle : public Entity {
public:
  int x_;
  Triangle(int n) : Entity(EntityColor::kORANGE), x_(n) {}
  Triangle(int n, EntityColor c) : Entity(c), x_(n) {}
};

/**
 * @class Cancel
 * @brief Zen symbol (Inverse "Y" symbol)
 *
 */
class Cancel : public Entity {
public:
  bool ignored_;
  Cancel() : Entity(), ignored_(false) {}
};
