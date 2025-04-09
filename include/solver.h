#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "grid.h"
#include "util.h"

using std::cout;
using std::endl;
using std::reverse;
using std::vector;

class Solver {
public:
  const int dx[4] = {1, 0, -1, 0};
  const int dy[4] = {0, 1, 0, -1};

  int callstopath_;
  vector<pair<int, int>> solution_;
  std::map<pair<int, int>, pair<int, int>> vis_;

  pair<int, int> origin_;

  Grid grid_;

  Solver();

  Solver(Grid &g);

  void Set(Grid &g);

  void Path(pair<int, int> src, pair<int, int> prev);

  vector<pair<int, int>> Solve();

  string ToString();

  void Display();

  void Activate();

  void Deactivate();
};
