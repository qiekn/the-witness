#pragma once

#include <climits>
#include <set>
#include <string>
#include <vector>

#include "object.h"

using std::pair;
using std::set;
using std::string;
using std::vector;

class BlockGroup : public Entity {
public:
  bool oriented;
  bool sub;
  set<pair<int, int>> pairs;
  int n;

  pair<int, int> bottomleft;
  pair<int, int> topright;
  pair<int, int> boundingbox;

  const pair<int, int> NIL = {INT_MIN, INT_MIN};

  BlockGroup(bool orientation, bool subtractive, vector<pair<int, int>> v,
             EntityColor c);

  BlockGroup(bool orientation, bool subtractive, vector<pair<int, int>> v);

  void updateBounds();

  // Utility Functions

  bool contains(pair<int, int> p);

  void add(pair<int, int> p);

  void remove(pair<int, int> p);

  void reset(vector<pair<int, int>> &p);

  BlockGroup clone();

  // General Functions

  void rotate(int x);

  void move(pair<int, int> p);

  void invmov(pair<int, int> p);

  void normalize();

  void removeRegion(BlockGroup x);

  void addRegion(BlockGroup x);

  string to_string();

  void disp();

  // Region testing

  bool containsbb(BlockGroup b);

  bool directoverlay(BlockGroup b);

  vector<pair<int, int>> fixedoverlay(BlockGroup b);

  vector<vector<pair<int, int>>> overlay(BlockGroup b);

  // Now for the real thing

  bool dfsUtil(BlockGroup region, vector<BlockGroup> &v, int index);

  bool solve(vector<BlockGroup> v);
};
