#ifndef MODEL_LOAD_COMMON_H
#define MODEL_LOAD_COMMON_H

#include <string>
#include <vector>
#include <chrono>

struct vec4 {
  vec4() :x(0), y(0), z(0), w(0) {}
  vec4(float tx, float ty, float tz, float tw) { x = tx; y = ty; z = tz; w = tw; }
  float x, y, z, w;
};

struct vec3 {
  vec3() :x(0), y(0), z(0) {}
  vec3(float f) :x(f), y(f), z(f) {}
  vec3(float tx, float ty,  float tz) { x = tx; y = ty; z = tz; }
  float x, y, z;
};

struct vec2 {
  vec2() :x(0), y(0) {}
  vec2(float tx, float ty) { x = tx; y = ty; }
  float x, y;
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
