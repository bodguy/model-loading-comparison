#include <iostream>
#include "assimp_loader.h"
#include "obj_loader.h"

void log_mesh_profile(const std::string& name, std::vector<obj_loader::shape>& sh);
void log_mesh_profile(const std::string& name, std::vector<mesh*>& mesh);

int main() {
  std::vector<std::string> file_list = { "nanosuit.obj", "sandal.obj", "teapot.obj", "cube.obj", "cow.obj", "p226.obj", "sponza.obj", "Five_Wheeler.obj", "Guss.obj", "Skull.obj" };
  std::vector<float> time_accumulate;
  StopWatch watch;
  std::vector<mesh*> mesh_assimp;

  for (auto& str : file_list) {
    watch.start();
    bool res = load_model("../res/" + str, mesh_assimp);
    watch.stop();
    time_accumulate.push_back(watch.milli());
    log_mesh_profile(str, mesh_assimp);
  }
  float average = 0.f;
  for (auto& t : time_accumulate) {
    average += t;
  }
  average /= time_accumulate.size();
  std::cout << "elapsed time (ASSIMP): " << average << " ms" << '\n';
  std::cout << "===========================================" << '\n';
  time_accumulate.clear();

  std::vector<obj_loader::shape> shape_out;
  for (auto& str : file_list) {
    watch.start();
    bool res = load_obj("../res/" + str, shape_out, obj_loader::ParseFlag::FLIP_UV);
    watch.stop();
    time_accumulate.push_back(watch.milli());
    log_mesh_profile(str, shape_out);
  }
  average = 0.f;
  for (auto& t : time_accumulate) {
    average += t;
  }
  average /= time_accumulate.size();
  std::cout << "elapsed time (OBJ): " << average << " ms" << '\n';
  std::cout << "===========================================" << '\n';
  time_accumulate.clear();

  return 0;
}

void log_mesh_profile(const std::string& name, std::vector<obj_loader::shape>& sh) {
  std::cout << "total mesh count (" << name << "): " << sh.size() << '\n';
  sh.clear();
}

void log_mesh_profile(const std::string& name, std::vector<mesh*>& mesh) {
  std::cout << "total mesh count (" << name << "): " << mesh.size() << '\n';
  for (auto m : mesh) {
    m = nullptr;
  }
  mesh.clear();
}