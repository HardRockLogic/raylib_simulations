#include <cmath>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

#define LOG2D(v) std::cout << v.x << ", " << v.y << "\n"

int constexpr SIDE = 700;
Color const coral = {245, 105, 66, 255};
Color const not_tiffany_blue = {96, 179, 176, 255};
Vector2 const GRAV{0, 0.7};

float DegreeToRadians(float deg) { return (deg * (PI / 180)); }

float PositiveToNegative(float num) { return num >= 0 ? num * -1 : num; }

class Ball {
private:
  bool clockwise_ = false;
  float const thermal_loss = 0.93;

public:
  Vector2 pos{600, 45};
  Vector2 direction{0, 0};
  float radius = 20;

  void Draw() { DrawCircle(pos.x, pos.y, radius, coral); }

  void RotateDirection() {
    float deg;
    if (clockwise_) {
      deg = GetRandomValue(355, 356);
      clockwise_ = false;
    } else {
      deg = GetRandomValue(1, 2);
      clockwise_ = true;
    }
    float rad = DegreeToRadians(deg);
    // Multiplying direction vector and rotation matrix
    float x = std::cos(rad) * direction.x - std::sin(rad) * direction.y;
    float y = std::sin(rad) * direction.x + std::cos(rad) * direction.y;
    direction = {x, y};
    // LOG2D(direction);
  }

  void CheckWalls() {
    if (pos.x - radius <= 0) {
      direction.x = std::abs(direction.x);
      RotateDirection();
    }
    if (pos.x + radius >= SIDE) {
      direction.x = PositiveToNegative(direction.x);
      RotateDirection();
    }
    if (pos.y - radius <= 0) {
      direction.y = std::abs(direction.y);
      RotateDirection();
    }
    if (pos.y + radius >= SIDE) {
      // LOG2D(direction);

      if (pos.y + radius > SIDE) {
        // direction = {0, 0};
        pos = {pos.x, SIDE - radius};
      }

      direction.y = PositiveToNegative(direction.y);
      direction = Vector2Scale(direction, thermal_loss);
    }
  }

  void DragLeft() {
    Vector2 left{-0.25, 0};
    direction = Vector2Add(direction, left);
  }
  void DragRight() {
    Vector2 right{0.25, 0};
    direction = Vector2Add(direction, right);
  }
  void Jump() {
    Vector2 up{0, -20};
    direction = Vector2Add(direction, up);
  }

  void Update() {
    direction = Vector2Add(direction, GRAV);
    pos = Vector2Add(pos, direction);
    CheckWalls();
  }
};

int main() {
  InitWindow(SIDE, SIDE, "Ball");
  SetTargetFPS(60);
  Ball ball{};

  while (!WindowShouldClose()) {
    BeginDrawing();
    if (IsKeyDown(KEY_LEFT)) {
      ball.DragLeft();
    }
    if (IsKeyDown(KEY_RIGHT)) {
      ball.DragRight();
    }
    if (IsKeyPressed(KEY_SPACE)) {
      ball.Jump();
    }

    ball.Update();

    // Drawing
    ClearBackground(not_tiffany_blue);

    ball.Draw();

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
