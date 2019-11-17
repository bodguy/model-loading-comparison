#include <iostream>
#include "common.h"
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

/*
obj, fbx, blend, gltf, ply, stl, dae, 3ds
 */

#ifdef ASSIMP_PROFILE
void log_mesh_profile(const std::string& name, std::vector<mesh*>& mesh, bool res, float elapsed, bool verbos) {
  std::cout << "## mesh (" << name << "): " << mesh.size() << std::boolalpha << " (" << res << ")" << '\n';
//  std::cout << std::tab << "time: " << elapsed << "ms" << '\n';
  if (verbos) {
    for (auto m : mesh) {
      printf("mesh name: %s\n", m->name.c_str());
      printf("verts size: %ld\n", m->vertices.size());
      printf("indices size: %ld\n", m->indices.size());
    }
  }
}
#endif

#ifdef TBJ_PROFILE
void log_mesh_profile(const std::string& name, std::vector<tinyobj::shape_t>& sh, bool res, float elapsed, bool verbos) {
  std::cout << "## mesh (" << name << "): " << sh.size() << std::boolalpha << " (" << res << ")" << '\n';
  std::cout << std::tab << "time: " << elapsed << "ms" << '\n';
  if (verbos) {

  }
}
#endif

#ifdef OBJL_PROFILE
void log_mesh_profile(const std::string& name, objl::Loader& loader, bool res, float elapsed, bool verbos) {
  std::cout << "## mesh (" << name << "): " << loader.LoadedMeshes.size() << std::boolalpha << " (" << res << ")" << '\n';
  std::cout << std::tab << "time: " << elapsed << "ms" << '\n';
  if (verbos) {
    for (auto m : loader.LoadedMeshes) {
      printf("mesh name: %s\n", m.MeshName.c_str());
      printf("verts size: %ld\n", m.Vertices.size());
      printf("indices size: %ld\n", m.Indices.size());
    }
  }
}
#endif

#ifdef MY_PROFILE
void log_mesh_profile(const std::string& name, const obj_loader::Scene& scene, bool res, float elapsed, bool verbos) {
  std::cout << "## mesh (" << name << "): " << scene.meshes.size() << std::boolalpha << " (" << res << ")" << '\n';
//  std::cout << std::tab << "time: " << elapsed << "ms" << '\n';
  if (verbos) {
    for (const auto& m : scene.meshes) {
      printf("mesh name: %s\n", m.name.c_str());
      printf("verts size: %ld\n", m.vertices.size());
      printf("indices size: %ld\n", m.indices.size());
    }
  }
}
#endif

int main() {
  std::vector<std::string> file_list = {
    "nanosuit/nanosuit.obj"
//    , "sandal.obj", "teapot.obj", "cube.obj", "cow.obj", "sponza.obj", "Five_Wheeler.obj", "Skull.obj", "sphere.obj", "dragon.obj", "monkey.obj",
//    "budda/budda.obj", "Merged_Extract8.obj", "officebot/officebot.obj", "revolver/Steampunk_Revolver1.obj", "panda/PandaMale.obj"
  };
  Profiler profiler;
  bool verbos = false;

#ifdef ASSIMP_PROFILE
  // assimp
  for (auto& str : file_list) {
    std::vector<mesh*> mesh_assimp;
    profiler.Start();
    bool res = load_model("../res/" + str, mesh_assimp);
    log_mesh_profile(str, mesh_assimp, res, profiler.Stop(), verbos);
  }
  std::cout << "average elapsed time (ASSIMP): " << profiler.Average() << " ms" << '\n';
  std::cout << "===========================================================" << '\n';
#endif

#ifdef TBJ_PROFILE
  // Tiny obj loader
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> meshes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  for (auto& str : file_list) {
    profiler.Start();
    bool res = tinyobj::LoadObj(&attrib, &meshes, &materials, &warn, &err, ("../res/" + str).c_str());
    log_mesh_profile(str, meshes, res, profiler.Stop(), verbos);
  }
  std::cout << "average elapsed time (TBL): " << profiler.Average() << " ms" << '\n';
#endif

#ifdef MY_PROFILE
  // my loader
  for (auto& str : file_list) {
    obj_loader::Scene scene;
    profiler.Start();
    bool res = obj_loader::loadObj("../res/" + str, scene, obj_loader::ParseOption::FLIP_UV);
    log_mesh_profile(str, scene, res, profiler.Stop(), verbos);
  }
  std::cout << "average elapsed time (OBJ): " << profiler.Average() << " ms" << '\n';
  std::cout << "===========================================================" << '\n';
#endif

#ifdef OBJL_PROFILE
  // OBJ Loader
  for (auto& str : file_list) {
    objl::Loader Loader;
    profiler.Start();
    bool res = Loader.LoadFile("../res/" + str);
    log_mesh_profile(str, Loader, res, profiler.Stop(), verbos);
  }
  std::cout << "average elapsed time (OBJL): " << profiler.Average() << " ms" << '\n';
#endif

  return 0;
}
