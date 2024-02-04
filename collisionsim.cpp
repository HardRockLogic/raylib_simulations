#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <tuple>
#include <vector>

#define LOG2D(v) std::cout << v.x << ", " << v.y << "\n"

int constexpr SIDE = 700;
Color const coral = {245, 105, 66, 255};
Color const not_tiffany_blue = {96, 179, 176, 255};
// Vector2 const GRAV{0, 1.5};

float DegreeToRadians(float deg) { return (deg * (PI / 180)); }

float PositiveToNegative(float num) { return num >= 0 ? num * -1 : num; }

class Ball {
private:
  bool clockwise_ = false;
  // bool calm_state = false;
  // float const thermal_loss = 0.93;

public:
  float radius_ = 20;
  Vector2 pos{600, 45};
  Vector2 direction{3, 5};
  float mass_ = 1;

  Ball(float radius) : radius_(radius) {
    float const size_mass_ratio = 1.f / 20.f;
    mass_ = radius / size_mass_ratio;
  }
  Ball(float radius, Vector2 position) : radius_(radius), pos(position) {
    float const size_mass_ratio = 1.f / 20.f;
    mass_ = radius / size_mass_ratio;
  }

  void Draw() const { DrawCircle(pos.x, pos.y, radius_, coral); }

  void RotateDirection() {
    float deg;
    if (clockwise_) {
      deg = GetRandomValue(355, 357);
      clockwise_ = false;
    } else {
      deg = GetRandomValue(1, 3);
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
    if (pos.x - radius_ <= 0) {
      direction.x = std::abs(direction.x);
      RotateDirection();
    }
    if (pos.x + radius_ >= SIDE) {
      direction.x = PositiveToNegative(direction.x);
      RotateDirection();
    }
    if (pos.y - radius_ <= 0) {
      direction.y = std::abs(direction.y);
      RotateDirection();
    }
    if (pos.y + radius_ >= SIDE) {
      // std::cout << direction.y << '\n';
      // if (std::abs(direction.y) <= 1) {
      //   calm_state = true;
      //   direction = {0, 0};
      // }
      direction.y = PositiveToNegative(direction.y);
      RotateDirection();
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
    // calm_state = false;
    Vector2 up{0, -20};
    direction = Vector2Add(direction, up);
  }
  void ChangeDirection(Vector2 newDirection) { direction = newDirection; }

  void Update() {
    // if (!calm_state) {
    //   direction = Vector2Add(direction, GRAV);
    // }
    pos = Vector2Add(pos, direction);
    CheckWalls();
  }
};

class CollisionControll {
private:
  std::tuple<float, float, float> RandomCircleData() {
    float randomRadius = GetRandomValue(20, 40);
    float x = GetRandomValue(0 + randomRadius, SIDE - randomRadius - 1);
    float y = GetRandomValue(0 + randomRadius, SIDE - randomRadius - 1);
    return {randomRadius, x, y};
  }

  void Populate() {
    auto [randomRadius, x, y] = RandomCircleData();

    if (particles_.empty()) {
      particles_.push_back(Ball(randomRadius, Vector2{x, y}));
    } else {
      bool notValid = true;
      float randomRadius, x, y;
      for (size_t i = 0; i < particles_.size(); ++i) {
        notValid =
            CheckCollisionCircles(particles_[i].pos, particles_[i].radius_,
                                  Vector2{x, y}, randomRadius);
        if (notValid) {
          std::tie(randomRadius, x, y) = RandomCircleData();
          i = 0;
        }
      }
      particles_.push_back(Ball(randomRadius, Vector2{x, y}));
    }
  }

public:
  std::vector<Ball> particles_;

  CollisionControll(unsigned particlesNumber) {
    for (unsigned n = 0; n <= particlesNumber; ++n) {
      Populate();
    }
  }

  void Draw() {
    for (Ball const &particle : particles_) {
      particle.Draw();
    }
  }
  void Update() {
    for (Ball &particle : particles_) {
      particle.Update();
    }
  }
};

int main() {
  InitWindow(SIDE, SIDE, "Ball");
  SetTargetFPS(60);
  // Ball ball(40);
  CollisionControll space(4);

  while (!WindowShouldClose()) {
    BeginDrawing();
    // if (IsKeyDown(KEY_LEFT)) {
    //   ball.DragLeft();
    // }
    // if (IsKeyDown(KEY_RIGHT)) {
    //   ball.DragRight();
    // }
    // if (IsKeyPressed(KEY_SPACE)) {
    //   ball.Jump();
    // }

    space.Update();

    // Drawing
    ClearBackground(not_tiffany_blue);

    space.Draw();

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
