#ifndef MODEL_LOAD_OBJ_LOADER_H
#define MODEL_LOAD_OBJ_LOADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cassert>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include "common.h"

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

namespace obj_loader {
  struct vertex_index {
    int v_idx, vt_idx, vn_idx;
    vertex_index() :v_idx(-1), vt_idx(-1), vn_idx(-1) {}
    explicit vertex_index(int idx) :v_idx(idx), vt_idx(idx), vn_idx(idx) {}
    vertex_index(int vidx, int vtidx, int vnidx) :v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
  };

  struct face {
    unsigned int smoothing_group_id;
    std::vector<vertex_index> vertex_indices;
    face() :smoothing_group_id(0) {}
  };

  struct primitive_group {
    primitive_group() :face_group() { face_group.clear(); }
    std::vector<face> face_group;
    bool is_empty() const { return face_group.empty(); }
  };

  struct mesh {
    mesh() :indices(), num_face_vertices(), smoothing_group_ids() {
      indices.clear(); num_face_vertices.clear(); smoothing_group_ids.clear();
    }
    std::vector<vertex_index> indices;
    std::vector<unsigned char> num_face_vertices; // 3: tri, 4: quad etc... Up to 255 vertices per face
    std::vector<unsigned int> smoothing_group_ids; // per face smoothing group IDs (0 = off. positive value = group id)
    std::vector<int> material_ids; // per face material IDs
  };

  struct shape {
    shape() :name(), mesh_group() {}
    std::string name;
    mesh mesh_group;
  };

  enum class ParseFlag {
    NONE = 0,
    TRIANGULATE = 1 << 0,
    FLIP_UV = 1 << 1,
    CALC_TANGENT = 1 << 2, // @TODO
    CALC_NORMAL = 1 << 3 // @TODO
  };

  bool operator&(const ParseFlag a, const ParseFlag b) {
    return static_cast<ParseFlag>(static_cast<int>(a) & static_cast<int>(b)) == b;
  }

  ParseFlag operator|(const ParseFlag a, const ParseFlag b) {
    return static_cast<ParseFlag>(static_cast<int>(a) | static_cast<int>(b));
  }

  bool parse_prim_group(shape* shape_group, const primitive_group& prim_group, ParseFlag flag, const int material_id, const std::string& name) {
    if (prim_group.is_empty()) {
      return false;
    }
    shape_group->name = name;

    // make polygon
    if (!prim_group.face_group.empty()) {
      for (size_t i = 0; i < prim_group.face_group.size(); i++) {
        const face& face_group = prim_group.face_group[i];
        size_t npolys = face_group.vertex_indices.size();

        if (npolys < 3) {
          // face must have at least 3+ vertices.
          continue;
        }

        if (flag & ParseFlag::TRIANGULATE) {
          // @TODO
        } else {
          for (size_t k = 0; k < npolys; k++) {
            shape_group->mesh_group.indices.emplace_back(face_group.vertex_indices[k]);
          }
          shape_group->mesh_group.num_face_vertices.emplace_back(static_cast<unsigned char>(npolys));
          shape_group->mesh_group.smoothing_group_ids.emplace_back(face_group.smoothing_group_id);
          shape_group->mesh_group.material_ids.emplace_back(material_id);
        }
      }
    }

    return true;
  }

  bool fixIndex(int idx, int n, int* ret) {
    if (!ret || idx == 0) {
      return false;
    }

    if (idx > 0) {
      (*ret) = idx - 1;
    }

    if (idx < 0) {
      (*ret) = n + idx;
    }

    return true;
  }

  bool parseIndices(const char** token, int vsize, int vnsize, int vtsize, vertex_index* ret) {
    if (!ret) {
      return false;
    }

    vertex_index vi(-1);
    // i
    if (!fixIndex(atoi((*token)), vsize, &(vi.v_idx))) {
      return false;
    }
    (*token) += strcspn((*token), "/ \t\r"); // go to next slash
    // check if only have vertex
    if ((*token)[0] != '/') {
      (*ret) = vi;
      return true;
    }
    (*token)++;

    // i//k
    //   +--- here
    if ((*token)[0] == '/') {
      (*token)++; // now then, token is pointing at 'k'
      if (!fixIndex(atoi((*token)), vnsize, &(vi.vn_idx))) {
        return false;
      }
      (*token) += strcspn((*token), "/ \t\r"); // go to next slash (although, it's not exist)
      (*ret) = vi;
      return true;
    }

    // i/j/k or i/j
    //   +--- here
    if (!fixIndex(atoi((*token)), vtsize, &(vi.vt_idx))) {
      return false;
    }
    (*token) += strcspn((*token), "/ \t\r"); // go to next slash
    if ((*token)[0] != '/') {
      // it's i/j case
      (*ret) = vi;
      return true;
    }

    // process last case
    // i/j/k
    (*token)++; // now then, token is pointing at 'k'
    if (!fixIndex(atoi((*token)), vnsize, &(vi.vn_idx))) {
      return false;
    }
    (*token) += strcspn((*token), "/ \t\r"); // go to next slash (although, it's not exist)
    (*ret) = vi;

    return true;
  }

  std::string parseString(const char** token) {
    (*token) += strspn((*token), " \t");
    const char* end = (*token) + strcspn((*token), " \t\r");
    size_t offset = end - (*token);
    std::string str;
    if (offset != 0) {
      char* dest = (char*)malloc(sizeof(char) * offset + 1);
      strncpy(dest, (*token), offset);
      *(dest + offset) = 0;
      str.assign(dest);
      free(dest);
    }

    (*token) = end;
    return str;
  }

  float parseReal(const char** token, float default_value) {
    (*token) += strspn((*token), " \t");
    const char* end = (*token) + strcspn((*token), " \t\r");
    size_t offset = end - (*token);
    float f = default_value;
    if (offset != 0) {
      char* dest = (char*)malloc(sizeof(char) * offset + 1);
      strncpy(dest, (*token), offset);
      *(dest + offset) = 0;
      f = (float)atof(dest);
      free(dest);
    }

    (*token) = end;
    return f;
  }

  void parseVT(float* x, float* y, const char** token, ParseFlag flag,
               float default_x = 0.f, float default_y = 0.f) {
    (*x) = parseReal(token, default_x);
    if (flag & ParseFlag::FLIP_UV) {
      (*y) = 1.f - parseReal(token, default_y);
    } else {
      (*y) = parseReal(token, default_y);
    }
  }

  void parseVN(float* x, float* y, float* z, const char** token,
               float default_x = 0.f, float default_y = 0.f, float default_z = 0.f) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);
  }

  void parseV(float* x, float* y, float* z, float* w, const char** token,
              float default_x = 0.f, float default_y = 0.f, float default_z = 0.f, float default_w = 1.f) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);
    (*w) = parseReal(token, default_w);
  }

  void parseSm(const char** token, unsigned int* out) {
    if ((*token)[0] == '\0') {
      return;
    }

    if ((*token)[0] == '\r' || (*token)[1] == '\n') {
      return;
    }

    if (strlen((*token)) >= 3 && (0 == strncmp((*token), "off", 3))) {
      (*out) = 0;
    } else {
      int tmp_id = atoi((*token));
      (*out) = tmp_id < 0 ? 0 : static_cast<unsigned int>(tmp_id);
    }
    (*token) += strcspn((*token), " \t\r");
  }

  std::istream& safeGetline(std::istream& is, std::string& t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
      int c = sb->sbumpc();
      switch (c) {
        case '\n':
          return is;
        case '\r':
          if(sb->sgetc() == '\n')
            sb->sbumpc();
          return is;
        case std::streambuf::traits_type::eof():
          // Also handle the case when the last line has no line ending
          if(t.empty())
            is.setstate(std::ios::eofbit);
          return is;
        default:
          t += static_cast<char>(c);
      }
    }
  }

  bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
  }

// NOTE: not support color(w), point, line shape
  bool load_obj(const std::string& path, std::vector<shape>& shapes, ParseFlag flag) {
    if (!endsWith(path, ".obj")) {
      return false;
    }

    std::ifstream ifs(path.c_str());
    if(!ifs) {
      return false;
    }

    std::vector<vec4> vertices;
    std::vector<vec2> texcoords;
    std::vector<vec3> normals;
    primitive_group prim_group;
    std::string current_object_name;
    unsigned int current_smoothing_id = 0;
    int max_vindex = -1, max_vtindex = -1, max_vnindex = -1;
    shape shape_group;
    std::unordered_map<std::string, int> material_map;
    int current_material_id = -1;

    int line_no = 0;
    std::string line_buf;
    while(ifs.peek() != -1) {
      safeGetline(ifs, line_buf); // read line by line
      ++line_no;

      // Trim newline '\r\n' or '\n'
      if (line_buf.size() > 0) {
        if (line_buf[line_buf.size() - 1] == '\n')
          line_buf.erase(line_buf.size() - 1);
      }
      if (line_buf.size() > 0) {
        if (line_buf[line_buf.size() - 1] == '\r')
          line_buf.erase(line_buf.size() - 1);
      }

      // Skip if empty line.
      if (line_buf.empty()) {
        continue;
      }

      // Skip leading space.
      const char *token = line_buf.c_str(); // read only token
      token += strspn(token, " \t");

      assert(token);
      if (token[0] == '\0') continue;  // empty line
      if (token[0] == '#') continue;  // comment line

      // vertex
      if (token[0] == 'v' && IS_SPACE((token[1]))) {
        token += 2;
        float x,y,z,w;
        parseV(&x, &y, &z, &w, &token);
        vertices.emplace_back(x,y,z,w);
        continue;
      }

      // normal
      if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
        token += 3;
        float x,y,z;
        parseVN(&x, &y, &z, &token);
        normals.emplace_back(x,y,z);
        continue;
      }

      // texcoord
      if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
        token += 3;
        float x,y;
        parseVT(&x, &y, &token, flag);
        texcoords.emplace_back(x,y);
        continue;
      }

      // face
      if (token[0] == 'f' && IS_SPACE((token[1]))) {
        token += 2;
        token += strspn(token, " \t"); // Skip leading space.

        face f;
        f.smoothing_group_id = current_smoothing_id;
        f.vertex_indices.reserve(3);

        while (!IS_NEW_LINE(token[0])) {
          vertex_index vi;
          if (!parseIndices(&token, vertices.size(), normals.size(), texcoords.size(), &vi)) {
            return false;
          }
          // profile max v,vt,vn index
          max_vindex = std::max<int>(max_vindex, vi.v_idx);
          max_vtindex = std::max<int>(max_vtindex, vi.vt_idx);
          max_vnindex = std::max<int>(max_vnindex, vi.vn_idx);

          // finish parse indices
          f.vertex_indices.emplace_back(vi);
          token += strspn(token, " \t\r"); // skip space
        }

        prim_group.face_group.emplace_back(f);
        continue;
      }

      // use mtl
      if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
        token += 7;
        std::string new_material_name = parseString(&token);
        int new_material_id = -1;
        // found
        if (material_map.find(new_material_name) != material_map.end()) {
          new_material_id = material_map[new_material_name];
        }
        // check current material and previous
        if (new_material_id != current_material_id) {
          // just make group and don't push it to shapes
          parse_prim_group(&shape_group, prim_group, flag, current_material_id, current_object_name); // return value not used
          // clear current primitives shape groups
          prim_group.face_group.clear();
          // cache new material id
          current_material_id = new_material_id;
        }
        continue;
      }

      // @TODO
      // load mtl
      if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
        
        continue;
      }

      // group name
      if (token[0] == 'g' && IS_SPACE((token[1]))) {
        parse_prim_group(&shape_group, prim_group, flag, current_material_id, current_object_name); // return value not used
        if (!shape_group.mesh_group.indices.empty()) {
          shapes.emplace_back(shape_group);
        }

        // reset
        prim_group = primitive_group();
        shape_group = shape();

        token += 2;

        // multi group not support
        std::vector<std::string> names;
        while (!IS_NEW_LINE(token[0])) {
          names.emplace_back(parseString(&token));
          token += strspn(token, " \t\r"); // skip space
        }

        if (!names.empty()) {
          std::stringstream ss;
          std::vector<std::string>::const_iterator it = names.begin();
          ss << *it++;
          for (; it != names.end(); it++) {
            ss << " " << *it;
          }
          current_object_name = ss.str();
        }

        continue;
      }

      // object name
      if (token[0] == 'o' && IS_SPACE((token[1]))) {
        parse_prim_group(&shape_group, prim_group, flag, current_material_id, current_object_name); // return value not used
        if (!shape_group.mesh_group.indices.empty()) {
          shapes.emplace_back(shape_group);
        }

        // reset
        prim_group = primitive_group();
        shape_group = shape();

        token += 2;
        current_object_name = parseString(&token);
        continue;
      }

      // smoothing group
      // s 1
      // s 2 ...
      // s off
      if (token[0] == 's' && IS_SPACE(token[1])) {
        token += 2;
        token += strspn(token, " \t");  // Skip leading space.
        parseSm(&token, &current_smoothing_id);
        continue;
      }
    }

    // check max v,vt,vn is lesser than vertices, texcoords, normals size
    if (max_vindex >= static_cast<int>(vertices.size())) {
      return false;
    }
    if (max_vtindex >= static_cast<int>(texcoords.size())) {
      return false;
    }
    if (max_vnindex >= static_cast<int>(normals.size())) {
      return false;
    }

    bool ret = parse_prim_group(&shape_group, prim_group, flag, current_material_id, current_object_name);
    if (ret || !shape_group.mesh_group.indices.empty()) {
      shapes.emplace_back(shape_group);
    }

//    std::cout << "The file contains " << line_no << " lines." << std::endl;
//    std::cout << "vertices " << vertices.size() << " lines." << std::endl;
//    std::cout << "normals " << normals.size() << " lines." << std::endl;
//    std::cout << "texcoords " << texcoords.size() << " lines." << std::endl;
//    std::cout << "max v " << max_vindex << std::endl;
//    std::cout << "max vt " << max_vtindex << std::endl;
//    std::cout << "max vn " << max_vnindex << std::endl;

    return true;
  }
}

#endif //MODEL_LOAD_OBJ_LOADER_H
