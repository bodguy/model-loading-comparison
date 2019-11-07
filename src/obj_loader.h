#ifndef MODEL_LOAD_OBJ_LOADER_H
#define MODEL_LOAD_OBJ_LOADER_H

#include <cstring>
#include "common.h"

bool load_obj(const std::string& filename, std::vector<mesh*>& out_mesh);
void process_vertex(FILE* file, const std::vector<vec2>& uvsIn, const std::vector<vec3>& normalsIn, std::vector<mesh*>& out_mesh);

bool load_obj(const std::string& filename, std::vector<mesh*>& out_mesh) {
  FILE* file = fopen(filename.c_str(), "r");
  if (file == NULL) return false;

  std::vector<vec3> vertices;
  std::vector<vec2> tex_coords;
  std::vector<vec3> normals;
  char word[256];

  while (1) {
    int res = fscanf(file, "%s", word);
    if (res == EOF) break;

    if (strcmp(word, "v") == 0) {
      vec3 v;
      fscanf(file, "%f %f %f\n", &v.x, &v.y, &v.z);
      vertices.push_back(v);
    } else if (strcmp(word, "vt") == 0) {
      vec2 uv;
      fscanf(file, "%f %f\n", &uv.x, &uv.y);
      tex_coords.push_back(uv);
    } else if (strcmp(word, "vn") == 0) {
      vec3 n;
      fscanf(file, "%f %f %f\n", &n.x, &n.y, &n.z);
      normals.push_back(n);
    } else if (strcmp(word, "f") == 0) {
      process_vertex(file, tex_coords, normals, out_mesh);
      break;
    }
  }

  while (1) {
    int res = fscanf(file, "%s", word);
    if (res == EOF) break;

    if (strcmp(word, "f") == 0) {
      process_vertex(file, tex_coords, normals, out_mesh);
    }
  }

  fclose(file);
  return true;
}

void process_vertex(FILE* file, const std::vector<vec2>& uvsIn, const std::vector<vec3>& normalsIn, std::vector<mesh*>& out_mesh) {
//  unsigned int vi[3];
//  unsigned int ti[3];
//  unsigned int ni[3];

//  if (states == States::NONE) {
//    fscanf(file, "%d %d %d\n", &vi[0], &vi[1], &vi[2]);
//  } else if (states == States::ONLY_A) {
//    fscanf(file, "%d/%d %d/%d %d/%d\n", &vi[0], &ti[0], &vi[1], &ti[1], &vi[2], &ti[2]);
//    // flip the uv coordinate
//    mUvs[vi[0] - 1] = vec2(uvsIn[ti[0] - 1].x, 1.f - uvsIn[ti[0] - 1].y);
//    mUvs[vi[1] - 1] = vec2(uvsIn[ti[1] - 1].x, 1.f - uvsIn[ti[1] - 1].y);
//    mUvs[vi[2] - 1] = vec2(uvsIn[ti[2] - 1].x, 1.f - uvsIn[ti[2] - 1].y);
//  } else if (states == States::ONLY_B) {
//    fscanf(file, "%d//%d %d//%d %d//%d\n", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2]);
//    mNormals[vi[0] - 1] = normalsIn[ni[0] - 1];
//    mNormals[vi[1] - 1] = normalsIn[ni[1] - 1];
//    mNormals[vi[2] - 1] = normalsIn[ni[2] - 1];
//  } else if (states == States::BOTH) {
//    fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vi[0], &ti[0], &ni[0], &vi[1], &ti[1], &ni[1], &vi[2], &ti[2], &ni[2]);
//    // flip the uv coordinate
//    mUvs[vi[0] - 1] = vec2(uvsIn[ti[0] - 1].x, 1.f - uvsIn[ti[0] - 1].y);
//    mUvs[vi[1] - 1] = vec2(uvsIn[ti[1] - 1].x, 1.f - uvsIn[ti[1] - 1].y);
//    mUvs[vi[2] - 1] = vec2(uvsIn[ti[2] - 1].x, 1.f - uvsIn[ti[2] - 1].y);
//
//    mNormals[vi[0] - 1] = normalsIn[ni[0] - 1];
//    mNormals[vi[1] - 1] = normalsIn[ni[1] - 1];
//    mNormals[vi[2] - 1] = normalsIn[ni[2] - 1];
//  }
//
//  mFaces.push_back(vi[0] - 1);
//  mFaces.push_back(vi[1] - 1);
//  mFaces.push_back(vi[2] - 1);
}

#endif //MODEL_LOAD_OBJ_LOADER_H
