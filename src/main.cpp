#include <iostream>
#include <chrono>
#include "assimp_loader.h"
#include "obj_loader.h"

void log_mesh_profile(const std::vector<obj_loader::shape>& sh);
void log_mesh_profile(const std::vector<mesh*>& mesh);

int main() {
  StopWatch watch;
  std::vector<mesh*> mesh_assimp;
  watch.start();
  load_model("../res/nanosuit.obj", mesh_assimp);
  watch.stop();
  std::cout << "elapsed time (ASSIMP): " << watch.milli() << " ms" << std::endl;
  log_mesh_profile(mesh_assimp);

  std::vector<obj_loader::shape> shape_out;
  watch.start();
  load_obj("../res/nanosuit.obj", shape_out, obj_loader::ParseFlag::FLIP_UV);
  watch.stop();
  std::cout << "elapsed time (OBJ): " << watch.milli() << " ms" << std::endl;
  log_mesh_profile(shape_out);

  return 0;
}

void log_mesh_profile(const std::vector<obj_loader::shape>& sh) {
  for (auto& s : sh) {
    std::cout << "===========================" << std::endl;
    std::cout << "name: " << s.name << std::endl;
    std::cout << "indices: " << s.mesh_group.indices.size() << std::endl;
  }
}

void log_mesh_profile(const std::vector<mesh*>& mesh) {
  int i = 0;
  std::cout << "total mesh count: " << mesh.size() << std::endl;
  for (auto m : mesh) {
    std::cout << "mesh: " << i << ", vertex count: " << m->vertices.size() << std::endl;
    std::cout << "mesh: " << i << ", index count: " << m->indices.size() << std::endl;
    std::cout << "mesh: " << i << ", texture count: " << m->textures.size() << std::endl;
    std::cout << "===========================" << std::endl;
    i++;
    delete m;
  }
}