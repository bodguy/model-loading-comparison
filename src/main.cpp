#include <iostream>
#include <chrono>
#include "assimp_loader.h"

int main() {
  std::vector<mesh*> mesh_assimp;
  std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
  load_model("../res/nanosuit.obj", mesh_assimp);
  std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
  float delta = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(end - start).count();
  std::cout << "elapsed time: " << delta << " milli sec" << std::endl;

  int i = 0;
  std::cout << "total mesh count: " << mesh_assimp.size() << std::endl;
  for (auto m : mesh_assimp) {
    std::cout << "mesh: " << i << ", vertex count: " << m->vertices.size() << std::endl;
    std::cout << "mesh: " << i << ", index count: " << m->indices.size() << std::endl;
    std::cout << "mesh: " << i << ", texture count: " << m->textures.size() << std::endl;
    std::cout << "===========================" << std::endl;
    i++;
  }

  // free memory
  for (auto m : mesh_assimp) {
    delete m;
  }

  return 0;
}

