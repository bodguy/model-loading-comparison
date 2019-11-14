#ifndef MODEL_LOAD_COMMON_H
#define MODEL_LOAD_COMMON_H

#include <string>
#include <vector>
#include <chrono>
#include <cmath>

struct vec4 {
  vec4() :x(0), y(0), z(0), w(0) {}
  vec4(float tx, float ty, float tz, float tw) { x = tx; y = ty; z = tz; w = tw; }
  float x, y, z, w;
};

float flt_epsilon = std::numeric_limits<float>::epsilon();
bool float_comapre(float a, float b) {
  return std::fabs(a - b) <= ( (std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * flt_epsilon);
}
struct vec3 {
  vec3() :x(0), y(0), z(0) {}
  vec3(float f) :x(f), y(f), z(f) {}
  vec3(float tx, float ty,  float tz) { x = tx; y = ty; z = tz; }
  float x, y, z;
  bool operator == (const vec3& rhs) {
    return (float_comapre(this->x, rhs.x) && float_comapre(this->y, rhs.y) && float_comapre(this->z, rhs.z));
  }
};

struct vec2 {
  vec2() :x(0), y(0) {}
  vec2(float tx, float ty) { x = tx; y = ty; }
  float x, y;
  bool operator == (const vec2& rhs) {
    return (float_comapre(this->x, rhs.x) && float_comapre(this->y, rhs.y));
  }
};

struct vertex {
  vec3 pos, normal;
  vec2 tex_coords;
};

struct texture {
    uint32_t id;
    std::string type;
    std::string path;
};

struct mesh {
    std::string name;
    std::vector<vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<texture> textures;
};

class StopWatch {
public:
    void start() {
      s = std::chrono::high_resolution_clock::now();
    }
    void stop() {
      e = std::chrono::high_resolution_clock::now();
    }

    float milli() {
      return std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(e - s).count();
    }

    float micro() {
      return std::chrono::duration_cast<std::chrono::duration<float, std::micro>>(e - s).count();
    }

    float nano() {
      return std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(e - s).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> s;
    std::chrono::time_point<std::chrono::high_resolution_clock> e;
};

#endif //MODEL_LOAD_COMMON_H
