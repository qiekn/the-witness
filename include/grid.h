#pragma once

#include "object.h"
#include "util.h"
#include <memory>
#include <set>
#include <string>
#include <vector>
using std::set;
using std::string;
using std::vector;

class Grid {
public:
  int m_; // lines
  int n_; // columns
  vector<vector<std::shared_ptr<Entity>>> board_;
  set<pair<int, int>> starts_;
  set<pair<int, int>> ends_;
  set<pair<int, int>> dots_;      // Hexagons
  set<pair<int, int>> triangles_; // Triangles
  set<pair<int, int>> blobs_;     // Color Squares (or black white squares)
  set<pair<int, int>> stars_;     // Stars
  set<pair<int, int>> blocks_;    // Tetris
  set<pair<int, int>> cancels_;   // Zen (Inverse Y)
  set<pair<int, int>> ignored_;   // Ignored cancel operations

  pair<int, int> begin_; // the begin point of the line you drawing

  Grid();

  Grid(vector<vector<std::shared_ptr<Entity>>> &v);

  void DefaultGrid();

  /** @brief Draw a straight line path from a to b */
  void DrawStraight(pair<int, int> a, pair<int, int> b);

  /**
   * @brief Draw a line (from start to end)
   *
   * @param v (a list of vertexes)
   */
  void DrawPath(vector<pair<int, int>> v);

  virtual ~Grid();

  string ToString();

  void Display();

  // The verification algorithm

  /** @brief check if the point p is inside the grid */
  bool Inside(pair<int, int> p);

  /**
   * @brief check if the point (sx, sy) is valid
   *
   * @param sx
   * @param sy
   * @return
   */
  bool IsValid(int sx, int sy);

  bool Check();

  bool ValidateRegion(int sx, int sy, vector<pair<int, int>> ban);
};
