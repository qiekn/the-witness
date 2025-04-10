#include <algorithm>
#include <cfloat>
#include <iostream>
#include <memory>
#include <raylib.h>

#include "raylibutils.h"
#include "witnessclone.h"

// Global variables
const int kScreenWidth = 800;
const int kScreenHeight = 600;

// Half-thickness of the lines as a proportion of the
// grid minimum dimension (minimum of width and height)
double THICKNESS = 0.02;

// Spacing of the grid lines as a proportion of the grid minimum dimension
double SPACING;

// Size of the gaps as a proportion of the line spacing. MUST BE AT MOST 0.5
double GAPPROP = 1.0 / 3.0;

// Radius of the starting points as a proportion of the thickness
double START_RAD = 2.5;

// Protrusion of the endpoints as a proportion of the line spacing
double PROTRUSION = 0.25;

// The actual protrusion distance = PROTRUSION * SPACING
double PRODIST = 0;

std::pair<double, double> CE;
std::pair<double, double> TL;
std::pair<double, double> BR;
std::pair<double, double> GRIDTL;
std::pair<double, double> GRIDBR;
int RANKS = 4;
int FILES = 4;

Color BG = BLACK;
Color PATH = GRAY;
Color LINE = LIGHTGRAY;
Color LITLINE = WHITE;

Solver sx;
RandGrid randomgrid;

Grid thegrid;

// Game state

bool STARTED = false;
bool LOCKEDIN = false;
bool SOLVED = false;
bool RESET = true;
Vector2 cursorpos;
std::vector<Vector2> pathpos;
std::set<std::pair<int, int>> pathset;
int LATENCY = 0;
std::pair<int, int> startpos;

double INTERACTION_EPSILON = 1;

int SCORE = 0;

// UI

bool EnableSkip = true;
bool SkipShowsSolution = false;
bool EnableShowSolution = true;
bool IN_SETTINGS = false;
int P_LATENCY = 0;

// WHAT TO DO?

bool P_TRIX = true;    // Challenge triangles
bool P_BLOB3 = true;   // Challenge blobs (9-3-2)
bool P_BLOB2 = true;   // Challenge blobs (8-2-4)
bool P_BLOCK = true;   // Challenge blocks
bool P_STARDOT = true; // Challenge stars and dots
bool P_DOT = true;     // 4 dots 2 cuts
bool P_MAZE = true;    // Random maze
bool P_STAR = true;    // Random 4 stars

// RANK = Y | FILE = X

inline int rankfromindex(int p) { return GRIDTL.second + 0.5 * SPACING * p; }

inline int filefromindex(int p) { return GRIDTL.first + 0.5 * SPACING * p; }

Vector2 closestrankfile(Vector2 p) {
  for (int i = 0; i < RANKS; i++) {
    if (std::abs(p.y - rankfromindex(i)) <= INTERACTION_EPSILON)
      p.y = rankfromindex(i);
  }
  for (int i = 0; i < FILES; i++) {
    if (std::abs(p.x - filefromindex(i)) <= INTERACTION_EPSILON)
      p.x = filefromindex(i);
  }
  return p;
}

inline std::pair<double, double> posfromindex(std::pair<int, int> p) {
  return std::pair<double, double>{GRIDTL.first + 0.5 * SPACING * p.second,
                                   GRIDTL.second + 0.5 * SPACING * p.first};
}

inline Vector2 vec2fromindex(std::pair<int, int> p) {
  return Vector2{float(GRIDTL.first + 0.5 * SPACING * p.second),
                 float(GRIDTL.second + 0.5 * SPACING * p.first)};
}

inline Vector2 endpointdisplacement(Grid g, int rank, int file,
                                    double prodist = PRODIST) {
  if (rank == 0)
    return {0, static_cast<float>(-prodist)};
  else if (rank == g.n_ - 1)
    return {0, static_cast<float>(prodist)};
  else if (file == 0)
    return {static_cast<float>(-prodist), 0};
  else if (file == g.m_ - 1)
    return {static_cast<float>(prodist), 0};
  else
    return {0, 0};
  // else return {prodist, prodist};
}

inline void drawEndPoint(int x, int y, Grid g, std::pair<double, double> POS,
                         double prodist = PRODIST, Color path = PATH) {
  Vector2 disp = endpointdisplacement(g, y, x, prodist);
  if (disp.x == 0 && disp.y == 0)
    DrawCircleLines(POS.first, POS.second, THICKNESS * 2, path);
  DrawSlot(POS.first, POS.second, POS.first + disp.x, POS.second + disp.y,
           THICKNESS, path);
}

inline void pickgrid() {
  thegrid = randomgrid.randMaze();
  const int NUM_PUZ = 8;

  std::vector<int> shuffle;
  for (int i = 0; i < NUM_PUZ; i++) {
    shuffle.push_back(i);
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(shuffle.begin(), shuffle.end(), g);

  int index = 0;
  while (index < shuffle.size()) {
    int decisionmaking = shuffle[index];
    if (decisionmaking == 0 && P_BLOCK)
      thegrid = randomgrid.randChallengeBlocks(2);
    else if (decisionmaking == 1 && P_BLOB3)
      thegrid = randomgrid.randBlobs(9, 3, 2);
    else if (decisionmaking == 2 && P_STARDOT)
      thegrid = randomgrid.randChallengeStars(2);
    else if (decisionmaking == 3 && P_TRIX)
      thegrid = randomgrid.randTriangles(10, 2);
    else if (decisionmaking == 4 && P_BLOB2)
      thegrid = randomgrid.randBlobs(8, 2, 4);
    else if (decisionmaking == 5 && P_DOT)
      thegrid = randomgrid.randDots(4, 2);
    else if (decisionmaking == 6 && P_STAR)
      thegrid = randomgrid.randStars();
    else if (decisionmaking == 7 && P_MAZE)
      thegrid = randomgrid.randMaze();
    else {
      index++;
      continue;
    }

    break;
  }
}

void Render(Grid &g, const int width, const int height, double marginprop = 0.1,
            bool buffer = true, bool clear = true) {
  if (buffer)
    BeginDrawing();
  if (clear)
    ClearBackground(BG);
  int n = std::min(width, height);

  // First coordinate (Y coordinate or RANK coordinate) is first index
  // Second coordinate (X coordinate or FILE coordinate) is second index

  THICKNESS = n * 0.02;
  FILES = g.m_ / 2;
  RANKS = g.n_ / 2;
  int nrows = std::max(RANKS, FILES);

  Color DOT = DARKGRAY;

  CE = {width * 0.5, height * 0.5};
  TL = {CE.first - 0.5 * n + n * marginprop,
        CE.second - 0.5 * n + n * marginprop};
  BR = {CE.first + 0.5 * n - n * marginprop,
        CE.second + 0.5 * n - n * marginprop};

  SPACING = (BR.first - TL.first) / nrows;
  double halfspacing = SPACING * 0.5;

  GRIDTL = {CE.first - 0.5 * SPACING * FILES,
            CE.second - 0.5 * SPACING * RANKS};
  GRIDBR = {CE.first + 0.5 * SPACING * FILES,
            CE.second + 0.5 * SPACING * RANKS};
  /*
    disp(CE);
    disp(TL);
    disp(BR);
    disp(GRIDTL);
    disp(GRIDBR);
    disp(std::make_pair(RANKS, FILES));
    */

  // cout << "\n" << THICKNESS << " " << SPACING << "\n";

  // Draw the grid and the endpoints

  // Vertical lines
  for (int i = 0; i <= FILES; i++) {
    double offset = SPACING * i;
    Rect2Points(GRIDTL.first - THICKNESS + offset, GRIDTL.second,
                GRIDTL.first + THICKNESS + offset, GRIDBR.second, PATH);
  }

  // Horizontal lines
  for (int i = 0; i <= RANKS; i++) {
    double offset = SPACING * i;
    Rect2Points(GRIDTL.first, GRIDTL.second - THICKNESS + offset, GRIDBR.first,
                GRIDTL.second + THICKNESS + offset, PATH);
  }

  for (int i = 0; i <= RANKS; i++) {
    for (int j = 0; j <= FILES; j++) {
      DrawCircle(GRIDTL.first + SPACING * j, GRIDTL.second + SPACING * i,
                 THICKNESS, PATH);
    }
  }

  for (int y = 0; y < g.n_; y++) {
    for (int x = 0; x < g.m_; x++) {
      pair<double, double> POS = {GRIDTL.first + halfspacing * x,
                                  GRIDTL.second + halfspacing * y};
      if (x % 2 != 0 && y % 2 != 0)
        continue;
      std::shared_ptr<Entity> entity = (g.board_)[y][x];
      if (!(entity->is_path_))
        DrawRectangle(POS.first - GAPPROP * halfspacing,
                      POS.second - GAPPROP * halfspacing, GAPPROP * SPACING,
                      GAPPROP * SPACING, BG);
      if (isStartingPoint(entity))
        DrawCircle(POS.first, POS.second, THICKNESS * START_RAD, PATH);
      else if (isEndingPoint(entity)) {
        // cout << "ENDPOINT FOUND" << x << " " << y << "\n";
        PRODIST = PROTRUSION * SPACING;
        drawEndPoint(x, y, g, POS, PRODIST, PATH);
      }
    }
  }

  // Draw Symbols

  for (int y = 0; y < g.n_; y++) {
    for (int x = 0; x < g.m_; x++) {
      pair<double, double> POS = {GRIDTL.first + halfspacing * x,
                                  GRIDTL.second + halfspacing * y};
      std::shared_ptr<Entity> e = g.board_[y][x];
      if (instanceof<Dot>(e))
        DrawPoly(vec2pd(POS), 6, THICKNESS * 0.8, 0, DOT);
      if (instanceof<Blob>(e)) {
        const double OBJ_SIZE = 0.8;
        double obj_width = OBJ_SIZE * halfspacing * GAPPROP;
        Rectangle rect = {static_cast<float>(POS.first - obj_width),
                          static_cast<float>(POS.second - obj_width),
                          static_cast<float>(2 * obj_width),
                          static_cast<float>(2 * obj_width)};
        DrawRectangleRounded(rect, 0.5, 8, getColor(e->color_));
      }
      if (instanceof<Star>(e)) {
        const double OBJ_SIZE = 00.8;
        double obj_width = OBJ_SIZE * halfspacing * GAPPROP;
        DrawPoly(vec2pd(POS), 4, obj_width, 0, getColor(e->color_));
        DrawPoly(vec2pd(POS), 4, obj_width, 45, getColor(e->color_));
      }
      if (instanceof<Triangle>(e)) {
        int count = (std::dynamic_pointer_cast<Triangle>(e))->x_;
        const double OBJ_SIZE = 0.2;
        const double OBJ_SPAC = 0.2;

        double leftmost = POS.first - SPACING * OBJ_SPAC * 0.5 * (count - 1);

        for (int i = 0; i < count; i++)
          DrawPoly(vec2pd({leftmost + SPACING * OBJ_SPAC * i, POS.second}), 3,
                   OBJ_SIZE * SPACING * 0.5, -90, getColor(e->color_));
      }
      if (instanceof<Cancel>(e)) {
        const double OBJ_SIZE = 0.2;
        const double OBJ_THICK = 0.05;

        DrawRotatedRect(POS.first, POS.second, POS.first,
                        POS.second - OBJ_SIZE * SPACING, OBJ_THICK * SPACING,
                        WHITE);
        DrawRotatedRect(
            POS.first, POS.second, POS.first - sqrt(0.75) * OBJ_SIZE * SPACING,
            POS.second + 0.5 * OBJ_SIZE * SPACING, OBJ_THICK * SPACING, WHITE);
        DrawRotatedRect(
            POS.first, POS.second, POS.first + sqrt(0.75) * OBJ_SIZE * SPACING,
            POS.second + 0.5 * OBJ_SIZE * SPACING, OBJ_THICK * SPACING, WHITE);
      }
      if (instanceof<BlockGroup>(e)) {
        // std::cout << "BLOCK GROUP LOCATED AT POSITION " << POS.first << " "
        // << POS.second << std::endl;
        std::shared_ptr<BlockGroup> bg =
            std::dynamic_pointer_cast<BlockGroup>(e);
        // std::cout << "BLOCK GROUP AT INDEX " << x << " " << y << " ORIENTED?
        // " << bg->oriented << "\n";
        bg->normalize();
        bg->updateBounds();
        std::pair<int, int> bb = bg->boundingbox;
        const double OBJ_SIZE = std::min(0.2, 0.9 / std::max(FILES, RANKS));
        const double OBJ_SPAC = OBJ_SIZE * 1.1;
        const double ROTATION = -1 * acos(-1) / 8;

        double leftmost = POS.first - SPACING * OBJ_SPAC * 0.5 * (bb.first - 1);
        double bottommost =
            POS.second + SPACING * OBJ_SPAC * 0.5 * (bb.second - 1);
        for (auto i : bg->pairs) {
          double xp = leftmost + SPACING * OBJ_SPAC * i.first;
          double yp = bottommost - SPACING * OBJ_SPAC * i.second;
          std::pair<double, double> thepos = {xp, yp};
          if (bg->oriented)
            DrawPoly(vec2pd(thepos), 4, OBJ_SIZE * SPACING * 0.5, 45,
                     getColor(e->color_));
          else {
            thepos = rotpd(thepos, POS, ROTATION);
            DrawPoly(vec2pd(thepos), 4, OBJ_SIZE * SPACING * 0.5,
                     45 + ROTATION * RAD2DEG, getColor(e->color_));
          }
        }
      }
    }
  }

  // Draw the solution path is there is one.

  for (int y = 0; y < g.n_; y++) {
    for (int x = 0; x < g.m_; x++) {
      pair<double, double> POS = {GRIDTL.first + halfspacing * x,
                                  GRIDTL.second + halfspacing * y};
      std::shared_ptr<Entity> e = g.board_[y][x];

      if (e->is_path_occupied_) {
        if (x % 2 == 0 && y % 2 == 0)
          DrawCircle(POS.first, POS.second, THICKNESS, LINE);
        else if (x % 2 == 0)
          DrawRectangle(POS.first - THICKNESS, POS.second - halfspacing,
                        THICKNESS * 2, SPACING, LINE);
        else if (y % 2 == 0)
          DrawRectangle(POS.first - halfspacing, POS.second - THICKNESS,
                        SPACING, THICKNESS * 2, LINE);

        if (isStartingPoint(e))
          DrawCircle(POS.first, POS.second, THICKNESS * START_RAD, LINE);
        else if (isEndingPoint(e)) {

          PRODIST = PROTRUSION * SPACING;
          drawEndPoint(x, y, g, POS, PRODIST, LINE);
        }
      }
    }
  }

  if (buffer)
    EndDrawing();
}

inline void DrawTitleScreen() {
  ClearBackground(RAYWHITE);
  double cx = kScreenWidth / 2.0;  // center x
  double cy = kScreenHeight / 2.0; // center y

  const int FS = 20; // font size
  const double SP = cy * 0.5;
  auto color = LIGHTGRAY;

  DrawCenteredText("Weclone to Witness(clone)", cx, SP, FS, color);
  DrawCenteredText("Press rmb to begin", cx, SP + FS, FS, color);

  DrawCenteredText("LMB to draw lines/submit", cx, SP + 3 * FS, FS, color);
  DrawCenteredText("RMB to cancel/change puzzle", cx, SP + 4 * FS, FS, color);
  if (EnableSkip) {
    if (SkipShowsSolution)
      DrawCenteredText("Skip to solve for you", cx, SP + 5 * FS, FS, color);
    else
      DrawCenteredText("Skip to move to next puz", cx, SP + 5 * FS, FS, color);
  }
  if (EnableShowSolution)
    DrawCenteredText("? To show solution", cx, SP + 6 * FS, FS, color);

  DrawCenteredText("P For settings", cx, SP + 7 * FS, FS, color);
}

// Game state functions

void resetProgress() {
  std::cout << "PROGRESS RESET\n";
  SCORE = 0;
}

inline void toggleSettings(int x) {
  if (x == 0)
    P_TRIX = !P_TRIX; // Challenge triangles
  if (x == 1)
    P_BLOB3 = !P_BLOB3; // Challenge blobs (9-3-2)
  if (x == 2)
    P_BLOB2 = !P_BLOB2; // Challenge blobs (8-2-4)
  if (x == 3)
    P_BLOCK = !P_BLOCK; // Challenge blocks
  if (x == 4)
    P_STARDOT = !P_STARDOT; // Challenge stars and dots
  if (x == 5)
    P_DOT = !P_DOT; // 4 dots 2 cuts
  if (x == 6)
    P_MAZE = !P_MAZE; // Random maze
  if (x == 7)
    P_STAR = !P_STAR; // Random 4 stars
}

inline const char *onoff(bool b) { return (b ? "ON" : "OFF"); }

inline Color onoffcol(bool b) { return (b ? GREEN : RED); }
inline Color onoffdim(bool b) { return (b ? DARKGREEN : MAROON); }

/** @brief Draw pause menu */
inline void DoSettings() {
  ClearBackground(BLACK);
  double cx = kScreenWidth / 2.0;  // center pos_x
  double cy = kScreenHeight / 2.0; // center pox_y

  const int FS = 40; // fontsize
  const double SP = std::max(60.0, cy * 0.2);
  const double BS = 60;
  const double BL = 600; // button length
  const double BH = 50;  // button height
  const double BD = 4;   // button border

  DrawCenteredText("GAME SETTINGS", cx, 0, FS, WHITE);
  DrawCenteredButton(
      "Triangle", cx, SP, BL, BH, [] { toggleSettings(0); }, FS, WHITE,
      onoffcol(P_TRIX), onoffdim(P_TRIX), BD);
  DrawCenteredButton(
      "Ternary Squares", cx, SP + BS, BL, BH, [] { toggleSettings(1); }, FS,
      WHITE, onoffcol(P_BLOB3), onoffdim(P_BLOB3), BD);
  DrawCenteredButton(
      "Binary Squares", cx, SP + 2 * BS, BL, BH, [] { toggleSettings(2); }, FS,
      WHITE, onoffcol(P_BLOB2), onoffdim(P_BLOB2), BD);
  DrawCenteredButton(
      "Tetris", cx, SP + 3 * BS, BL, BH, [] { toggleSettings(3); }, FS, WHITE,
      onoffcol(P_BLOCK), onoffdim(P_BLOCK), BD);
  DrawCenteredButton(
      "Stars + Hexagons", cx, SP + 4 * BS, BL, BH, [] { toggleSettings(4); },
      FS, WHITE, onoffcol(P_STARDOT), onoffdim(P_STARDOT), BD);
  DrawCenteredButton(
      "Hexagons", cx, SP + 5 * BS, BL, BH, [] { toggleSettings(5); }, FS, WHITE,
      onoffcol(P_DOT), onoffdim(P_DOT), BD);
  DrawCenteredButton(
      "Maze", cx, SP + 6 * BS, BL, BH, [] { toggleSettings(6); }, FS, WHITE,
      onoffcol(P_MAZE), onoffdim(P_MAZE), BD);
  DrawCenteredButton(
      "Stars", cx, SP + 7 * BS, BL, BH, [] { toggleSettings(7); }, FS, WHITE,
      onoffcol(P_STAR), onoffdim(P_STAR), BD);
  DrawCenteredButton(
      "RESET PROGRESS", cx, kScreenHeight - 60, BL, BH, [] { resetProgress(); },
      FS, WHITE, BLUE, DARKBLUE, BD);
}

// Main method

int main() {
  srand(time(0));

  randomgrid.pathfind();

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(kScreenWidth, kScreenHeight, "game");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    if (!STARTED)
      DrawTitleScreen();

    // get mouse input
    std::pair<double, double> mp = pdvec2(GetMousePosition());
    Vector2 md = GetMouseDelta();

    // used for checking buttom press
    if (LATENCY > 0)
      LATENCY--;
    if (P_LATENCY > 0)
      P_LATENCY--;

    // open game settings (or pause menu)
    if (STARTED && GetKeyPressed() == KEY_P && P_LATENCY == 0) {
      std::cout << "P PRESSED " << IN_SETTINGS << std::endl;
      IN_SETTINGS = !IN_SETTINGS;
      P_LATENCY = 8;
    }

    if (STARTED && IN_SETTINGS) {
      LOCKEDIN = false;
      SOLVED = false;
      RESET = false;

      DoSettings();
      EndDrawing();
      continue;
    }

    // render grid
    Render(thegrid, kScreenWidth, kScreenHeight, 0.1, false, true);

    bool dreamsreallycometrue = false;

    // game start
    if (LOCKEDIN && (LATENCY == 0)) {
      HideCursor();
      DisableCursor();

      double MAX_SPEED = 64;

      md.x = Clamp(md.x, -MAX_SPEED, MAX_SPEED);
      md.y = Clamp(md.y, -MAX_SPEED, MAX_SPEED);

      int blah = std::max(1.0f, std::max(std::abs(md.x), std::abs(md.y)));
      md.x /= blah;
      md.y /= blah;

      // Mouse movement

      Vector2 originalcurspos = cursorpos;

      std::vector<Vector2> things;

      for (int r = 0; r < thegrid.m_; r++) {
        for (int c = 0; c < thegrid.n_; c++) {
          if (!isEndingPoint(thegrid.board_[r][c]))
            continue;
          Vector2 v = endpointdisplacement(thegrid, r, c);
          Vector2 endpointpos = vec2fromindex({r, c});
          Vector2 endpointend = {endpointpos.x + v.x, endpointpos.y + v.y};
          if (rsqvec2(cursorpos, endpointend) <=
              INTERACTION_EPSILON * INTERACTION_EPSILON)
            dreamsreallycometrue = true;
        }
      }

      for (int block = 0; block < blah; block++) {
        Vector2 newcurspos = cursorpos;

        // Rank and file of if you are on a rank or file corresponding to a grid
        // line/intersection
        int rankno = -1;
        int fileno = -1;

        // Closest rank and file
        int closestrank = -1;
        int closestfile = -1;

        // Rank and file of if you are on an edge, corresponding to the
        // rank/file between the two nodes you are between.
        int edgerank = -1;
        int edgefile = -1;

        double mindi = DBL_MAX;
        for (int i = 0; i < thegrid.n_; i++) {
          double dist = std::abs(cursorpos.y - rankfromindex(i));
          if (dist < mindi) {
            mindi = dist;
            closestrank = i;
          }
          if (dist <= 0.5 * SPACING - THICKNESS && (i & 1))
            edgerank = i;
          if (dist <= INTERACTION_EPSILON)
            rankno = i;
        }
        mindi = DBL_MAX;
        for (int i = 0; i < thegrid.m_; i++) {
          double dist = std::abs(cursorpos.x - filefromindex(i));
          if (dist < mindi) {
            mindi = dist;
            closestfile = i;
          }
          if (dist <= 0.5 * SPACING - THICKNESS && (i & 1))
            edgefile = i;
          if (dist <= INTERACTION_EPSILON)
            fileno = i;
        }

        if (block < 0) {
          std::cout << "R/F";
          disp({RANKS, FILES});
          disp({rankno, fileno});
          disp({closestrank, closestfile});
          disp({edgerank, edgefile});
          std::cout << "\n";
        }

        // If at an endpoint and your dreams come true

        if (newcurspos.x < GRIDTL.first - PRODIST && md.x < 0)
          continue;
        if (newcurspos.x > GRIDBR.first + PRODIST && md.x > 0)
          continue;
        if (newcurspos.y < GRIDTL.second - PRODIST && md.y < 0)
          continue;
        if (newcurspos.y > GRIDBR.second + PRODIST && md.y > 0)
          continue;

        bool dreamscometrue = false;

        if (closestrank >= 0 && closestfile >= 0 &&
            isEndingPoint(thegrid.board_[closestrank][closestfile])) {
          Vector2 v = endpointdisplacement(thegrid, closestrank, closestfile);
          Vector2 endpointpos = vec2fromindex({closestrank, closestfile});
          Vector2 endpointend = {endpointpos.x + v.x, endpointpos.y + v.y};
          // std::cout << "ENDPOINT FOUND NEAR YOU " << v.x << " " << v.y <<
          // "\n";

          if (inRange(newcurspos, endpointpos, endpointpos, INTERACTION_EPSILON,
                      INTERACTION_EPSILON)) {
            // std::cout << "AT INTERSECTION";
            if (v.x != 0 && v.x * md.x > 0)
              newcurspos.x += md.x;
            if (v.y != 0 && v.y * md.y > 0)
              newcurspos.y += md.y;
          } else if (inRange(newcurspos, endpointpos, endpointend,
                             INTERACTION_EPSILON, INTERACTION_EPSILON)) {
            // std::cout << "IN RANGE";
            if (v.x != 0) {
              newcurspos.y = endpointpos.y;
              newcurspos.x += md.x;
            }
            if (v.y != 0) {
              newcurspos.x = endpointpos.x;
              newcurspos.y += md.y;
            }
            dreamscometrue = true;
          }
        }

        if (dreamscometrue)
          ;
        // In a row but not an intersection
        else if ((fileno < 0 || fileno & 1) && rankno >= 0) {
          bool badtrace = false;
          // Path test
          Vector2 thevec = {
              static_cast<float>(newcurspos.x + 2 * sgn(md.x) * THICKNESS),
              newcurspos.y};
          for (int i = 0; i < pathpos.size() - 0.5 * SPACING; i++) {
            if (rsqvec2(thevec, pathpos[i]) <=
                4 * INTERACTION_EPSILON * INTERACTION_EPSILON)
              badtrace = true;
          }
          // Cut test
          for (int r = 0; r < thegrid.n_; r++) {
            for (int f = 0; f < thegrid.m_; f++) {
              if (thegrid.board_[r][f]->is_path_)
                continue;
              thevec = vec2fromindex({r, f});

              bool dirreq = (md.x > 0 && thevec.x > newcurspos.x) ||
                            (md.x < 0 && thevec.x < newcurspos.x);
              bool distreq = rsqvec2(thevec, newcurspos) <=
                             (0.5 * GAPPROP * SPACING + THICKNESS) *
                                 (0.5 * GAPPROP * SPACING + THICKNESS);
              if (distreq && dirreq)
                badtrace = true;
            }
          }
          if (!badtrace && md.x != 0)
            newcurspos.x += md.x;
        }
        // In a column but not an intersection
        else if ((rankno < 0 || rankno & 1) && fileno >= 0) {
          bool badtrace = false;
          // Path test
          Vector2 thevec = {
              newcurspos.x,
              static_cast<float>(newcurspos.y + 2 * sgn(md.y) * THICKNESS)};
          for (int i = 0; i < pathpos.size() - 0.5 * SPACING; i++) {
            if (rsqvec2(thevec, pathpos[i]) <=
                4 * INTERACTION_EPSILON * INTERACTION_EPSILON)
              badtrace = true;
          }
          // Cut test
          for (int r = 0; r < thegrid.n_; r++) {
            for (int f = 0; f < thegrid.m_; f++) {
              if (thegrid.board_[r][f]->is_path_)
                continue;
              thevec = vec2fromindex({r, f});
              bool dirreq = (md.y > 0 && thevec.y > newcurspos.y) ||
                            (md.y < 0 && thevec.y < newcurspos.y);
              bool distreq = rsqvec2(thevec, newcurspos) <=
                             (0.5 * GAPPROP * SPACING + THICKNESS) *
                                 (0.5 * GAPPROP * SPACING + THICKNESS);
              if (distreq && dirreq)
                badtrace = true;
            }
          }
          if (!badtrace && md.y != 0)
            newcurspos.y += md.y;
        }

        // Four corners
        else if (rankno == 0 && fileno == 0) {
          if (std::abs(md.x) > std::abs(md.y) && md.x > 0)
            newcurspos.x += md.x;
          else if (md.y > 0)
            newcurspos.y += md.y;
        } else if (rankno == thegrid.n_ - 1 && fileno == 0) {
          if (std::abs(md.x) > std::abs(md.y) && md.x > 0)
            newcurspos.x += md.x;
          else if (md.y < 0)
            newcurspos.y += md.y;
        } else if (rankno == 0 && fileno == thegrid.m_ - 1) {
          if (std::abs(md.x) > std::abs(md.y) && md.x < 0)
            newcurspos.x += md.x;
          else if (md.y > 0)
            newcurspos.y += md.y;
        } else if (rankno == thegrid.n_ - 1 && fileno == thegrid.m_ - 1) {
          if (std::abs(md.x) > std::abs(md.y) && md.x < 0)
            newcurspos.x += md.x;
          else if (md.y < 0)
            newcurspos.y += md.y;
        }
        // Top side
        else if (rankno == 0 && fileno >= 0 && fileno % 2 == 0) {
          if (std::abs(md.x) > std::abs(md.y))
            newcurspos.x += md.x;
          else if (md.y > 0)
            newcurspos.y += md.y;
        }
        // Bottom side
        else if (rankno == thegrid.n_ - 1 && fileno >= 0 && fileno % 2 == 0) {
          if (std::abs(md.x) > std::abs(md.y))
            newcurspos.x += md.x;
          else if (md.y < 0)
            newcurspos.y += md.y;
        }
        // Left side
        else if (fileno == 0 && rankno >= 0 && rankno % 2 == 0) {
          if (std::abs(md.x) > std::abs(md.y) && md.x > 0)
            newcurspos.x += md.x;
          else
            newcurspos.y += md.y;
        }
        // Right side
        else if (fileno == thegrid.m_ - 1 && rankno >= 0 && rankno % 2 == 0) {
          if (std::abs(md.x) > std::abs(md.y) && md.x < 0)
            newcurspos.x += md.x;
          else
            newcurspos.y += md.y;
        }
        // Internal intersections
        else if (fileno > 0 && rankno > 0 && fileno % 2 == 0 &&
                 rankno % 2 == 0) {
          if (std::abs(md.x) > std::abs(md.y))
            newcurspos.x += md.x;
          else
            newcurspos.y += md.y;
        }
        // Bad position
        else {
          std::cout << "BAD POSITION!!" << newcurspos.x << " " << newcurspos.y
                    << "\n";
          newcurspos.x = filefromindex(closestfile);
          newcurspos.y = rankfromindex(closestrank);
          cursorpos = newcurspos;
          break;
        }
        things.push_back(closestrankfile(cursorpos));
        cursorpos = newcurspos;
      }

      // What happened?
      Vector2 cursposmod = cursorpos;

      if (things.size() > 0) {
        // std::cout << "MOVEMENT DETECTED " << pathpos.size() << "\n";
        // std::cout << originalcurspos.x << " " << originalcurspos.y << " = "
        // << cursposmod.x << " " << cursposmod.y << "\n"; for (auto i : things)
        // std::cout << i.x << " " << i.y << "\n";
        std::vector<Vector2> mvt;
        things.push_back(closestrankfile(cursorpos));
        for (int i = 0; i < things.size() - 1; i++) {
          for (auto j : precvec2(things[i], things[i + 1]))
            mvt.push_back(j);
        }
        int MARGIN = 2;
        int MARGIN2 = 1;
        for (auto i : mvt) {
          bool ohno = false;
          for (auto iter = pathpos.end() - MARGIN;
               iter != pathpos.end() - MARGIN2; iter++) {
            if (rsqvec2(i, *iter) <= INTERACTION_EPSILON + 1)
              ohno = true;
          }
          if (ohno) {
            Vector2 thing = *(pathpos.rbegin());
            std::pair<int, int> vv = {int(thing.x), int(thing.y)};
            if (pathset.find(vv) != pathset.end())
              pathset.erase(pathset.find(vv));
            pathpos.pop_back();
          } else {
            pathpos.push_back(i);
            pathset.insert({int(i.x), int(i.y)});
          }
        }
      }

      // Draw the path and the cursor

      for (auto i : pathpos)
        DrawCircleV(i, THICKNESS, (dreamsreallycometrue ? LITLINE : LINE));
      // for (auto i : pathset) DrawCircle(i.first, i.second, THICKNESS / 2,
      // BLUE);
      DrawCircleV(cursorpos, THICKNESS, WHITE);
    } else {
      ShowCursor();
    }

    // std::cout << "MOUSE POS" << mp.first << " " << mp.second << "\n";

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !LOCKEDIN) {
      disp(mp);
      disp(pdvec2(md));

      for (int i = 0; i < thegrid.n_; i++) {
        for (int j = 0; j < thegrid.m_; j++)
          thegrid.board_[i][j]->is_path_occupied_ = false;
      }

      for (auto i : thegrid.starts_) {
        std::cout << "START! ";
        disp(posfromindex(i));
        std::cout << "\n";
        if (rsqpd(mp, posfromindex(i)) <=
            THICKNESS * START_RAD * THICKNESS * START_RAD) {
          startpos = {i.first, i.second};
          thegrid.board_[i.first][i.second]->is_path_occupied_ = true;
          cursorpos = vec2fromindex(i);
          LOCKEDIN = true;
          while (pathpos.size() > 0)
            pathpos.pop_back();
          pathset.clear();
          pathpos.push_back(cursorpos);
          DisableCursor();
          LATENCY = 4;
          LINE = LIGHTGRAY;
          LITLINE = WHITE;
          break;
        }
      }
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && LOCKEDIN &&
               dreamsreallycometrue) {
      std::cout << "MOUSE BUTTON PRESSED";
      LOCKEDIN = false;
      EnableCursor();

      const double CHECK_THRESHOLD = std::min(THICKNESS, 0.1 * SPACING);

      for (auto i : pathpos) {
        for (int r = 0; r < thegrid.n_; r++) {
          for (int f = 0; f < thegrid.m_; f++) {
            if (!thegrid.board_[r][f]->is_path_)
              continue;
            Vector2 v = vec2fromindex({r, f});
            if (rsqvec2(i, v) <= CHECK_THRESHOLD * CHECK_THRESHOLD)
              thegrid.board_[r][f]->is_path_occupied_ = true;
          }
        }
      }

      if (thegrid.IsValid(startpos.first, startpos.second)) {
        SCORE++;
        LINE = GREEN;
        SOLVED = true;
      } else
        LINE = RED;
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      if (!STARTED)
        STARTED = true;
      LOCKEDIN = false;
      EnableCursor();

      for (int i = 0; i < thegrid.n_; i++) {
        for (int j = 0; j < thegrid.m_; j++)
          thegrid.board_[i][j]->is_path_occupied_ = false;
      }

      if (SOLVED || RESET) {
        std::cout << "SOLVED PUZZLE" << std::endl;

        /*

            std::vector<std::vector<std::shared_ptr<Entity>>> v (7,
           std::vector<std::shared_ptr<Entity>>(5)); for (int i = 0; i < 7; i++)
           { for (int j = 0; j < 5; j++) v[i][j] = std::shared_ptr<Entity>(new
           Entity());
            }
            v[1][3] = std::shared_ptr<Triangle>(new Triangle(1));
            v[6][0] = std::shared_ptr<Endpoint>(new Endpoint(false));
            v[6][3] = std::shared_ptr<Endpoint>(new Endpoint(false));
            v[4][4] = std::shared_ptr<Endpoint>(new Endpoint(false));
            v[0][4] = std::shared_ptr<Endpoint>(new Endpoint(true));
            v[0][0] = std::shared_ptr<Endpoint>(new Endpoint(true));
            thegrid = Grid(v);

            thegrid.defaultGrid();
            thegrid.board[1][0]->isPath = false;
            thegrid.board[4][3]->isPath = false;


            */
        sx.Deactivate();
        pickgrid();
        SOLVED = false;
        RESET = false;
        LOCKEDIN = false;
      }
    }

    // UI
    if (STARTED) {
      // DrawFPS(10, 10);
      std::string scoredata = std::to_string(SCORE) + " SOLVED";
      DrawText(scoredata.c_str(), 10, 10, 20, WHITE);

      if (EnableSkip) {

        DrawRectangle(kScreenWidth - 96, kScreenHeight - 48, 128, 48, BLUE);
        DrawText("SKIP", kScreenWidth - 88, kScreenHeight - 40, 32, WHITE);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          auto mpmp = GetMousePosition();
          if (mpmp.x >= kScreenWidth - 96 && mpmp.y >= kScreenHeight - 48) {
            if (SkipShowsSolution) {
              LINE = WHITE;
              sx.Set(thegrid);
              sx.Solve();
              sx.Activate();
              RESET = true;
            } else {
              sx.Deactivate();
              pickgrid();
              SOLVED = false;
              RESET = false;
              LOCKEDIN = false;
            }
          }
        }
      }

      if (EnableShowSolution) {
        DrawRectangle(kScreenWidth - 80, kScreenHeight - 96, 128, 48, GRAY);
        DrawText("Hint", kScreenWidth - 64, kScreenHeight - 88, 32, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          auto mpmp = GetMousePosition();
          if (mpmp.x >= kScreenWidth - 80 && mpmp.y >= kScreenHeight - 96 &&
              mpmp.y < kScreenHeight - 48) {
            LINE = BLUE;
            LOCKEDIN = false;
            RESET = false;
            sx.Set(thegrid);
            sx.Solve();
            sx.Activate();
          }
        }
      }
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
