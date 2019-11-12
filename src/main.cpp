#include <iostream>
#include "conditional.h"
#ifdef ASSIMP_PROFILE
#include "assimp_loader.h"
#endif
#ifdef TBJ_PROFILE
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"
#endif
#ifdef OBJL_PROFILE
#include "OBJ_Loader_third.h"
#endif
#ifdef MY_PROFILE
#include "obj_loader.h"
#endif

namespace std {
  template <typename _CharT, typename _Traits>
  inline basic_ostream<_CharT, _Traits> &
  tab(basic_ostream<_CharT, _Traits> &__os) {
    return __os.put(__os.widen('\t'));
  }
}

#ifdef ASSIMP_PROFILE
void log_mesh_profile(const std::string& name, std::vector<mesh*>& mesh, bool res, float elapsed) {
  std::cout << "total mesh count (" << name << "): " << mesh.size() << " (" << std::boolalpha << res << "), " << elapsed << "ms" << '\n';
  for (auto m : mesh) {
    delete m;
    m = nullptr;
  }
  mesh.clear();
}
#endif

#ifdef TBJ_PROFILE
void log_mesh_profile(const std::string& name, std::vector<tinyobj::shape_t>& sh, bool res, float elapsed) {
  std::cout << "total mesh count (" << name << "): " << sh.size() << " (" << std::boolalpha << res << "), " << elapsed << "ms" << '\n';
  sh.clear();
}
#endif

#ifdef OBJL_PROFILE
void log_mesh_profile(const std::string& name, objl::Loader& loader, bool res, float elapsed) {
  std::cout << "total mesh count (" << name << "): " << loader.LoadedMeshes.size() << " (" << std::boolalpha << res << "), " << elapsed << "ms" << '\n';
}
#endif

#ifdef MY_PROFILE
void log_mesh_profile(const std::string& name, const obj_loader::Scene& scene, bool res, float elapsed) {
  std::cout << "mesh count (" << name << "): " << scene.meshes.size() << '\n';
  std::cout << std::tab << "result: " << std::boolalpha << res << '\n';
  std::cout << std::tab << "time: " << elapsed << "ms" << '\n';
  std::cout << std::tab << "vertex count: " << scene.vertices.size() << '\n';
  std::cout << std::tab << "uv count: " << scene.texcoords.size() << '\n';
  std::cout << std::tab << "normal count: " << scene.normals.size() << '\n';
  std::cout << std::tab << "material count: " << scene.materials.size() << '\n';
  std::cout << "==================================================" << '\n';
}
#endif

int main() {
  std::vector<std::string> file_list = {
    "nanosuit/nanosuit.obj", "sandal.obj", "teapot.obj", "cube.obj", "cow.obj", "sponza.obj", "Five_Wheeler.obj", "Skull.obj", "sphere.obj", "dragon.obj", "monkey.obj",
    "budda/budda.obj", "Merged_Extract8.obj", "officebot/officebot.obj", "revolver/Steampunk_Revolver1.obj"
  };
  std::vector<float> time_accumulate;
  StopWatch watch;
  float average = 0.f;

#ifdef ASSIMP_PROFILE
  // assimp
  std::vector<mesh*> mesh_assimp;
  for (auto& str : file_list) {
    watch.start();
    bool res = load_model("../res/" + str, mesh_assimp);
    watch.stop();
    float elapsed = watch.milli();
    time_accumulate.push_back(elapsed);
    log_mesh_profile(str, mesh_assimp, res, elapsed);
  }
  for (auto& t : time_accumulate) {
    average += t;
  }
  average /= time_accumulate.size();
  std::cout << "average elapsed time (ASSIMP): " << average << " ms" << '\n';
  std::cout << "===========================================" << '\n';
  time_accumulate.clear();
#endif

#ifdef TBJ_PROFILE
  // Tiny obj loader
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> meshes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  for (auto& str : file_list) {
    watch.start();
    bool res = tinyobj::LoadObj(&attrib, &meshes, &materials, &warn, &err, ("../res/" + str).c_str());
    watch.stop();
    float elapsed = watch.milli();
    time_accumulate.push_back(elapsed);
    log_mesh_profile(str, meshes, res, elapsed);
  }
  average = 0.f;
  for (auto& t : time_accumulate) {
    average += t;
  }
  average /= time_accumulate.size();
  std::cout << "average elapsed time (TBL): " << average << " ms" << '\n';
  std::cout << "===========================================" << '\n';
  time_accumulate.clear();
#endif

#ifdef MY_PROFILE
  // my loader
  for (auto& str : file_list) {
    obj_loader::Scene scene;
    watch.start();
    bool res = loadObj("../res/" + str, scene, obj_loader::ParseOption::FLIP_UV);
    watch.stop();
    float elapsed = watch.milli();
    time_accumulate.push_back(elapsed);
    log_mesh_profile(str, scene, res, elapsed);
  }
  average = 0.f;
  for (auto& t : time_accumulate) {
    average += t;
  }
  average /= time_accumulate.size();
  std::cout << "average elapsed time (OBJ): " << average << " ms" << '\n';
  time_accumulate.clear();
#endif

#ifdef OBJL_PROFILE
  // OBJ Loader
  for (auto& str : file_list) {
    watch.start();
    objl::Loader Loader;
    bool res = Loader.LoadFile("../res/" + str);
    watch.stop();
    float elapsed = watch.milli();
    time_accumulate.push_back(elapsed);
    log_mesh_profile(str, Loader, res, elapsed);
  }
  average = 0.f;
  for (auto& t : time_accumulate) {
    average += t;
  }
  average /= time_accumulate.size();
  std::cout << "average elapsed time (OBJL): " << average << " ms" << '\n';
  std::cout << "===========================================" << '\n';
  time_accumulate.clear();
#endif

  return 0;
}
