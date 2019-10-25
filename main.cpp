#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y;
} vec2;

typedef struct {
    vec3 pos, normal;
    vec2 tex_coords;
} vertex;

typedef struct {
    uint32_t id;
    std::string type;
    std::string path;
} texture;

typedef struct {
    std::vector<vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<texture> textures;
} mesh;

std::vector<mesh*> meshes;

void load_model(const std::string& filename);
void process_node(aiNode* node, const aiScene* scene);
mesh* process_mesh(aiMesh* ai_mesh, const aiScene* scene);
std::vector<texture> load_texture(aiMaterial *mat, aiTextureType type, const std::string& typeName);

int main() {
  std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
  load_model("../res/nanosuit.obj");
  std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
  float delta = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(end - start).count();
  std::cout << "elapsed time: " << delta << " milli sec" << std::endl;

  int i = 0;
  std::cout << "total mesh count: " << meshes.size() << std::endl;
  for (auto m : meshes) {
    std::cout << "mesh: " << i << ", vertex count: " << m->vertices.size() << std::endl;
    std::cout << "mesh: " << i << ", index count: " << m->indices.size() << std::endl;
    std::cout << "mesh: " << i << ", texture count: " << m->textures.size() << std::endl;
    std::cout << "===========================" << std::endl;
    i++;
  }

  // free memory
  for (auto m : meshes) {
    delete m;
  }

  return 0;
}

void load_model(const std::string& filename) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cout << "ASSIMP::ERROR:" << importer.GetErrorString() << std::endl;
    return;
  }
  process_node(scene->mRootNode, scene);
}

void process_node(aiNode* node, const aiScene* scene) {
  for (int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene));
  }

  for (int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}

mesh* process_mesh(aiMesh* ai_mesh, const aiScene* scene) {
  mesh* new_mesh = new mesh();

  // process vertex
  new_mesh->vertices.reserve(ai_mesh->mNumVertices);
  for (int i = 0; i < ai_mesh->mNumVertices; i++) {
    vertex vertex;
    vec3 v;
    v.x = ai_mesh->mVertices[i].x;
    v.y = ai_mesh->mVertices[i].y;
    v.z = ai_mesh->mVertices[i].z;
    vertex.pos = v;
    v.x = ai_mesh->mNormals[i].x;
    v.y = ai_mesh->mNormals[i].y;
    v.z = ai_mesh->mNormals[i].z;
    vertex.normal = v;
    if (ai_mesh->mTextureCoords[0]) {
      vec2 vec;
      vec.x = ai_mesh->mTextureCoords[0][i].x;
      vec.y = ai_mesh->mTextureCoords[0][i].y;
      vertex.tex_coords = vec;
    } else {
      vec2 vec;
      vec.x = 0.f; vec.y = 0.f;
      vertex.tex_coords = vec;
    }
    new_mesh->vertices.push_back(vertex);
  }

  // process indices
  new_mesh->indices.reserve(ai_mesh->mNumFaces);
  for (int i = 0; i < ai_mesh->mNumFaces; i++) {
    aiFace face = ai_mesh->mFaces[i];
    for (int j = 0; j < face.mNumIndices; j++) {
      new_mesh->indices.push_back((uint32_t)face.mIndices[j]);
    }
  }

  // process material
  if (ai_mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
    std::vector<texture> diffuseMaps = load_texture(material, aiTextureType_DIFFUSE, "texture_diffuse");
    new_mesh->textures.insert(new_mesh->textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<texture> specularMaps = load_texture(material, aiTextureType_SPECULAR, "texture_specular");
    new_mesh->textures.insert(new_mesh->textures.end(), specularMaps.begin(), specularMaps.end());
  }

  return new_mesh;
}

std::vector<texture> load_texture(aiMaterial *mat, aiTextureType type, const std::string& typeName) {
  std::vector<texture> textures;
  for (int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    texture t;
    t.id = -5; // no need to load texture here
    t.type = typeName;
    t.path = str.C_Str();
    textures.push_back(t);
  }

  return textures;
}