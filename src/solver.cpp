#include "solver.h"

#include <ctime>
#include <iostream>
using std::pair;

Solver::Solver() { solution_ = vector<pair<int, int>>(); }

Solver::Solver(Grid &g) {
  grid_ = g;
  solution_ = vector<pair<int, int>>();
}

void Solver::Set(Grid &g) {
  grid_ = g;
  solution_.clear();
}

void Solver::Path(pair<int, int> src, pair<int, int> prev) {
  callstopath_++;
  // cout << "[" << src.first << " " << src.second << "]\n";
  if (solution_.size() > 0)
    return;
  if ((grid_.ends_).find(src) != (grid_.ends_).end()) {
    grid_.board_[src.first][src.second]->is_path_occupied_ = true;
    // cout << "ENDPOINT " << src.first << " " << src.second << endl;
    // grid.disp();

    bool check = grid_.IsValid(origin_.first, origin_.second);
    // cout << (check ? "PASSED\n" : "FAILED\n");
    if (check) {
      // cout << "SOLUTION FOUND" << endl;
      vis_.insert({src, prev});
      // for (auto i : vis) cout << "[" << i.first.first << " " <<
      // i.first.second << "] [" << i.second.first << " " << i.second.second <<
      // "]\n";
      while (vis_.find(src) != vis_.end() && vis_.at(src) != src) {
        // cout << src.first << " " << src.second << endl;
        solution_.push_back(src);
        src = vis_.at(src);
      }
      solution_.push_back(src);
      reverse(solution_.begin(), solution_.end());
    }

    grid_.board_[src.first][src.second]->is_path_occupied_ = false;
    return;
  }

  // Basic pruning action
  // This can be toggled by changing the loop constraints.

  for (int ii = 0; ii < 4; ii++) {
    pair<int, int> x0 = {src.first + dx[(ii + 0) % 4],
                         src.second + dy[(ii + 0) % 4]};
    pair<int, int> x1 = {src.first + dx[(ii + 1) % 4],
                         src.second + dy[(ii + 1) % 4]};
    // pair<int, int> x2 = {src.first + dx[(ii + 2) % 4], src.second + dy[(ii +
    // 2) % 4]};
    pair<int, int> x3 = {src.first + dx[(ii + 3) % 4],
                         src.second + dy[(ii + 3) % 4]};
    bool blocked0 = !grid_.Inside(x0);
    bool blocked1 = !grid_.Inside(x1) ||
                    grid_.board_[x1.first][x1.second]->is_path_occupied_;
    // bool blocked2 = !grid.inside(x2);
    bool blocked3 = !grid_.Inside(x3) ||
                    grid_.board_[x3.first][x3.second]->is_path_occupied_;

    vector<pair<int, int>> banned({src, x0});

    if (blocked0 && !blocked1 && !blocked3) {
      // cout << "BLOCKED!!!  " << src.first << " " << src.second << endl;
      // grid.disp();
      bool r1 = grid_.ValidateRegion(x1.first, x1.second, banned);
      bool r3 = grid_.ValidateRegion(x3.first, x3.second, banned);

      if (!r1 && !r3) {
        // cout << "INVALID" << endl;
        return;
      }
      break;
    }
  }

  vis_.insert({src, prev});
  grid_.board_[src.first][src.second]->is_path_occupied_ = true;

  srand(time(0));
  int offset = rand() % 4;

  for (int ii = 0; ii < 4; ii++) {
    int i = (ii + offset) % 4;
    pair<int, int> next = {src.first + dx[i], src.second + dy[i]};
    if (!grid_.Inside(next))
      continue;
    if (!grid_.board_[next.first][next.second]->is_path_)
      continue;
    if (vis_.find(next) != vis_.end())
      continue;
    Path(next, src);
  }
  vis_.erase(vis_.find(src));
  grid_.board_[src.first][src.second]->is_path_occupied_ = false;
}

vector<pair<int, int>> Solver::Solve() {
  // cout << "SOLVING" << endl;
  callstopath_ = 0;
  solution_.clear();
  for (auto i : grid_.starts_) {
    origin_ = i;
    // cout << i.first << " " << i.second << endl;
    vis_.clear();
    vis_.insert({i, i});
    Path(i, i);
    if (solution_.size() > 0)
      break;
  }
  return solution_;
}

string Solver::ToString() {
  for (auto i : grid_.board_) {
    for (auto j : i)
      j->is_path_occupied_ = false;
  }
  for (auto i : solution_)
    grid_.board_[i.first][i.second]->is_path_occupied_ = true;
  string res = grid_.ToString();
  for (auto i : solution_)
    grid_.board_[i.first][i.second]->is_path_occupied_ = false;
  return res;
}

void Solver::Display() {
  cout << ToString() << endl;
  cout << callstopath_ << " CALLS TO PATH\n\n";
}

void Solver::Activate() {
  for (auto i : grid_.board_) {
    for (auto j : i)
      j->is_path_occupied_ = false;
  }
  for (auto i : solution_)
    grid_.board_[i.first][i.second]->is_path_occupied_ = true;
}

void Solver::Deactivate() {
  for (auto i : grid_.board_) {
    for (auto j : i)
      j->is_path_occupied_ = false;
  }
}
