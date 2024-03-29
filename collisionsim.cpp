#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <tuple>
#include <vector>

#define LOG2D(v) std::cout << v.x << ", " << v.y << "\n"

int constexpr SIDE = 850;
Color const coral = {245, 105, 66, 255};
Color const not_tiffany_blue = {96, 179, 176, 255};
// Vector2 const GRAV{0, 1.5};

double lastFPSUpdate = 0.0;

float DegreeToRadians(float deg) { return (deg * (PI / 180)); }

float PositiveToNegative(float num) { return num >= 0 ? num * -1 : num; }

bool ElapsedTime(double interval) {
  double currentTime = GetTime();
  if (currentTime - lastFPSUpdate >= interval) {
    lastFPSUpdate = currentTime;
    return true;
  }
  return false;
}

Color EnergyToColor(float energy) {
  energy = std::max(1000.0f, std::min(12000.0f, energy));

  float normalized = (energy - 1.0f) / 11999.0f;

  unsigned char blue = static_cast<unsigned char>((1.0f - normalized) * 255);
  unsigned char red = static_cast<unsigned char>(normalized * 255);
  unsigned char green = 0;

  return Color{red, green, blue, 255};
}

class Ball {
private:
  bool clockwise_ = false;
  bool calm_state = false;
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

  void Draw() const {
    // KE = 1/2(mv^2)
    float energy = (mass_ * Vector2LengthSqr(direction)) / 2;
    // std::cout << energy << '\n';
    DrawCircle(pos.x, pos.y, radius_, EnergyToColor(energy));
  }

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

  void ChangeDirection(Vector2 newDirection) { direction = newDirection; }

  void Update() {
    // if (Vector2LengthSqr(direction) > 0) {
    //   calm_state = false;
    // }
    // if (!calm_state) {
    //   direction = Vector2Add(direction, GRAV);
    // }
    pos = Vector2Add(pos, direction);
    CheckWalls();
  }
};

class CollisionControll {
private:
  unsigned min_radius_;
  unsigned max_radius_;

  std::tuple<float, float, float> RandomCircleData() {
    float randomRadius = GetRandomValue(min_radius_, max_radius_);
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
  void Create3Debug() {
    particles_.push_back(Ball(20, Vector2{40, 40}));
    particles_.push_back(Ball(30, Vector2{100, 80}));
    particles_.push_back(Ball(25, Vector2{250, 300}));
  }

  void CheckParticleCollisionGrid() {
    float cellSize = SIDE / 5.;
    for (float raw = 0; raw <= cellSize * 4 - 1; raw += cellSize - 1) {
      for (float coll = 0; coll <= cellSize * 4 - 1; coll += cellSize - 1) {
        std::vector<size_t> grid;
        Rectangle rec{coll, raw, cellSize, cellSize};
        for (size_t idx = 0; idx < particles_.size(); ++idx) {
          if (CheckCollisionCircleRec(particles_.at(idx).pos,
                                      particles_.at(idx).radius_, rec)) {
            grid.push_back(idx);
          }
        }
        for (size_t n = 0; n < grid.size(); ++n) {
          for (size_t m = n + 1; m < grid.size(); ++m) {
            size_t i = grid[n];
            size_t j = grid[m];
            if (CheckCollisionCircles(particles_[i].pos, particles_[i].radius_,
                                      particles_[j].pos,
                                      particles_[j].radius_)) {

              // Preventing particles overlap
              float sqrDist =
                  Vector2Distance(particles_[i].pos, particles_[j].pos);
              float deltaDist =
                  ((particles_[i].radius_ + particles_[j].radius_) - sqrDist) /
                  2;
              Vector2 normJI = Vector2Normalize(
                  Vector2Subtract(particles_[i].pos, particles_[j].pos));

              particles_[i].pos = Vector2Add(particles_[i].pos,
                                             Vector2Scale(normJI, deltaDist));
              particles_[j].pos =
                  Vector2Add(particles_[j].pos,
                             (Vector2Negate(Vector2Scale(normJI, deltaDist))));

              // v1 hat
              float iMass = 2 * particles_[j].mass_ /
                            (particles_[i].mass_ + particles_[j].mass_);
              float iDotProduct = Vector2DotProduct(
                  Vector2Subtract(particles_[i].direction,
                                  particles_[j].direction),
                  Vector2Subtract(particles_[i].pos, particles_[j].pos));
              float iSqrMagnitude = Vector2LengthSqr(
                  Vector2Subtract(particles_[i].pos, particles_[j].pos));
              Vector2 iRightPart = Vector2Scale(
                  Vector2Subtract(particles_[i].pos, particles_[j].pos),
                  iMass * (iDotProduct / iSqrMagnitude));
              Vector2 iNewDirection =
                  Vector2Subtract(particles_[i].direction, iRightPart);

              // v2 hat
              float jMass = 2 * particles_[i].mass_ /
                            (particles_[i].mass_ + particles_[j].mass_);
              float jDotProduct = Vector2DotProduct(
                  Vector2Subtract(particles_[j].direction,
                                  particles_[i].direction),
                  Vector2Subtract(particles_[j].pos, particles_[i].pos));
              float jSqrMagnitude = Vector2LengthSqr(
                  Vector2Subtract(particles_[j].pos, particles_[i].pos));
              Vector2 jRightPart = Vector2Scale(
                  Vector2Subtract(particles_[j].pos, particles_[i].pos),
                  jMass * (jDotProduct / jSqrMagnitude));
              Vector2 jNewDirection =
                  Vector2Subtract(particles_[j].direction, jRightPart);

              // LOG2D(iNewDirection);
              // LOG2D(jNewDirection);
              particles_[i].ChangeDirection(iNewDirection);
              particles_[j].ChangeDirection(jNewDirection);
            }
          }
        }
      }
    }
  }

  void CheckParticleCollision() {
    for (size_t i = 0; i < particles_.size(); ++i) {
      for (size_t j = i + 1; j < particles_.size(); ++j) {
        if (CheckCollisionCircles(particles_[i].pos, particles_[i].radius_,
                                  particles_[j].pos, particles_[j].radius_)) {
          // std::cout << "yeah, its me\n";
          // v1 hat
          float iMass = 2 * particles_[j].mass_ /
                        (particles_[i].mass_ + particles_[j].mass_);
          float iDotProduct = Vector2DotProduct(
              Vector2Subtract(particles_[i].direction, particles_[j].direction),
              Vector2Subtract(particles_[i].pos, particles_[j].pos));
          float iSqrMagnitude = Vector2LengthSqr(
              Vector2Subtract(particles_[i].pos, particles_[j].pos));
          Vector2 iRightPart = Vector2Scale(
              Vector2Subtract(particles_[i].pos, particles_[j].pos),
              iMass * (iDotProduct / iSqrMagnitude));
          Vector2 iNewDirection =
              Vector2Subtract(particles_[i].direction, iRightPart);

          // v2 hat
          float jMass = 2 * particles_[i].mass_ /
                        (particles_[i].mass_ + particles_[j].mass_);
          float jDotProduct = Vector2DotProduct(
              Vector2Subtract(particles_[j].direction, particles_[i].direction),
              Vector2Subtract(particles_[j].pos, particles_[i].pos));
          float jSqrMagnitude = Vector2LengthSqr(
              Vector2Subtract(particles_[j].pos, particles_[i].pos));
          Vector2 jRightPart = Vector2Scale(
              Vector2Subtract(particles_[j].pos, particles_[i].pos),
              jMass * (jDotProduct / jSqrMagnitude));
          Vector2 jNewDirection =
              Vector2Subtract(particles_[j].direction, jRightPart);

          // LOG2D(iNewDirection);
          // LOG2D(jNewDirection);
          particles_[i].ChangeDirection(iNewDirection);
          particles_[j].ChangeDirection(jNewDirection);
        }
      }
    }
  }

public:
  std::vector<Ball> particles_;

  CollisionControll(unsigned particlesNumber, unsigned minRadius,
                    unsigned maxRadius)
      : min_radius_(minRadius), max_radius_(maxRadius) {
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
    // CheckParticleCollision();
    CheckParticleCollisionGrid();
    for (Ball &particle : particles_) {
      particle.Update();
    }
  }
};

int main() {
  InitWindow(SIDE, SIDE, "Ball");
  SetTargetFPS(60);

  Shader blur = LoadShader(0, "glsl/blur.fs");

  // Number of particles, min && max radius
  // 6000 particles is starts droping fps to 50 if all of them colliding in one
  // small spot but then normalizes to 60
  CollisionControll space(60, 10, 13);

  double previousTime = GetTime();
  double currentTime = 0.0;
  double deltaTime = 0.0;

  while (!WindowShouldClose()) {
    BeginDrawing();

    space.Update();

    // Drawing
    ClearBackground(BLACK);

    // BeginShaderMode(blur);

    space.Draw();

    DrawText(TextFormat("FPS: %i", (int)(1.0 / deltaTime)), 5, 5, 40, GREEN);
    // DrawText(TextFormat("FPS: %i", GetFPS()), 5, 5, 40, GREEN);
    // DrawRectangle(0, 0, SIDE - 1, SIDE - 1, Color{111, 111, 111, 210});

    // EndShaderMode();
    EndDrawing();

    currentTime = GetTime();
    if (ElapsedTime(0.2)) {
      deltaTime = currentTime - previousTime;
    }
    previousTime = currentTime;
  }

  UnloadShader(blur);
  CloseWindow();
  return 0;
}
