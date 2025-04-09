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
  int n_;
  int m_;
  vector<vector<std::shared_ptr<Entity>>> board_;
  set<pair<int, int>> starts_;
  set<pair<int, int>> ends_;
  set<pair<int, int>> dots_;
  set<pair<int, int>> triangles_;
  set<pair<int, int>> blobs_;
  set<pair<int, int>> stars_;
  set<pair<int, int>> blocks_;
  set<pair<int, int>> cancels_;
  set<pair<int, int>> ignored_; // ignored cancel operations

  pair<int, int> begin_;

  Grid();

  Grid(vector<vector<std::shared_ptr<Entity>>> &v);

  void defaultGrid();

  void DrawLine(pair<int, int> a, pair<int, int> b);

  void DrawPath(vector<pair<int, int>> v);

  virtual ~Grid();

  string ToString();

  void Display();

  // The verification algorithm

  bool Inside(pair<int, int> p);

  /* TODO: wtf??? <2025-04-09 22:09, @qiekn> */
  bool Ver(int sx, int sy);

  bool Check();

  bool ValidateRegion(int sx, int sy, vector<pair<int, int>> ban);
};
