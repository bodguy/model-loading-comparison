#ifndef MODEL_LOAD_ASSIMP_LOADER_H
#define MODEL_LOAD_ASSIMP_LOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "common.h"

bool load_model(const std::string& filename, std::vector<mesh*>& out_mesh);
void process_node(aiNode* node, const aiScene* scene, std::vector<mesh*>& meshes);
mesh* process_mesh(aiMesh* ai_mesh, const aiScene* scene);
std::vector<texture> load_texture(aiMaterial *mat, aiTextureType type, const std::string& typeName);

bool load_model(const std::string& filename, std::vector<mesh*>& out_mesh) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    return false;
  }
  process_node(scene->mRootNode, scene, out_mesh);
  return true;
}

void process_node(aiNode* node, const aiScene* scene, std::vector<mesh*>& meshes) {
  for (int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene));
  }

  for (int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene, meshes);
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

#endif //MODEL_LOAD_ASSIMP_LOADER_H
