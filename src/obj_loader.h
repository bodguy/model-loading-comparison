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
#include <type_traits>
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
    face() :vertex_indices() {}
    std::vector<vertex_index> vertex_indices;
  };

  struct primitive_group {
    primitive_group() :face_group() { face_group.clear(); }
    bool is_empty() const { return face_group.empty(); }
    std::vector<face> face_group;
  };

  struct mesh {
    mesh() :indices(), num_face_vertices() { indices.clear(); num_face_vertices.clear(); }
    std::vector<vertex_index> indices;
    std::vector<unsigned char> num_face_vertices; // 3: tri, 4: quad etc... Up to 255 vertices per face
    std::vector<int> material_ids; // per face material IDs
  };

  struct shape {
    shape() :name(), mesh_group() {}
    std::string name;
    mesh mesh_group;
  };

  struct texture_option {
    texture_option() : clamp(true), blendu(true), blendv(true), bump_multiplier(1.f) {}
    bool clamp;
    bool blendu;
    bool blendv;
    float bump_multiplier;
  };

  struct texture {
    texture() : path(), option() {}
    std::string path;
    texture_option option;
  };

  struct enum_class_hash {
    template<typename T>
    std::size_t operator()(T t) const {
      return static_cast<std::size_t>(t);
    }
  };

  enum class tex_type {
    AMBIENT, // map_Ka
    DIFFUSE, // map_Kd
    SPECULAR, // map_Ks
    SPECULAR_HIGHLIGHT, // map_Ns
    BUMP, // map_bump, map_Bump, bump
    DISPLACEMENT, // disp
    ALPHA, // map_d
    REFLECTION, // refl
  };

  template <typename Key>
  using HashType = typename std::conditional<std::is_enum<Key>::value, enum_class_hash, std::hash<Key>>::type;

  template <typename Key, typename T>
  using my_unordered_map = std::unordered_map<Key, T, HashType<Key>>;

  struct material {
    material()
    :name(), ambient(), diffuse(), specular(), transmittance(), emission(),
      shininess(1.f), ior(1.f), dissolve(1.f), illum(0) {
      texture_map.clear();
    }

    std::string name;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float ior; // index of refraction
    float dissolve; // 1 == opaque; 0 == fully transparent
    int illum; // illumination model
    my_unordered_map<tex_type, texture> texture_map;
  };

  enum class parse_option {
    NONE = 0,
    TRIANGULATE = 1 << 0,
    FLIP_UV = 1 << 1,
    CALC_TANGENT = 1 << 2, // @TODO
    CALC_NORMAL = 1 << 3, // @TODO
    COMBINE_SUBMESHES = 1 << 4 // @TODO
  };

  bool operator&(const parse_option a, const parse_option b) {
    return static_cast<parse_option>(static_cast<int>(a) & static_cast<int>(b)) == b;
  }

  parse_option operator|(const parse_option a, const parse_option b) {
    return static_cast<parse_option>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline bool parsePrimitive(shape* shape_group, const primitive_group& prim_group, parse_option option, const int material_id, const std::string& name) {
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

        if (option & parse_option::TRIANGULATE) {
          // @TODO
        } else {
          for (size_t k = 0; k < npolys; k++) {
            shape_group->mesh_group.indices.emplace_back(face_group.vertex_indices[k]);
          }
          shape_group->mesh_group.num_face_vertices.emplace_back(static_cast<unsigned char>(npolys));
          shape_group->mesh_group.material_ids.emplace_back(material_id);
        }
      }
    }

    return true;
  }

  inline bool fixIndex(int idx, int n, int* ret) {
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

  inline bool parseIndices(const char** token, int vsize, int vnsize, int vtsize, vertex_index* ret) {
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

  inline std::string parseString(const char** token) {
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

  inline float parseReal(const char** token, float default_value) {
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

  inline void parseReal2(vec2& vt, const char** token) {
    vt.x = parseReal(token, 0.f);
    vt.y = parseReal(token, 0.f);
  }

  inline void parseReal3(vec3& vn, const char** token) {
    vn.x = parseReal(token, 0.f);
    vn.y = parseReal(token, 0.f);
    vn.z = parseReal(token, 0.f);
  }

  inline void parseReal4(vec4& v, const char** token) {
    v.x = parseReal(token, 0.f);
    v.y = parseReal(token, 0.f);
    v.z = parseReal(token, 0.f);
    v.w = parseReal(token, 1.f);
  }

  inline int parseInt(const char **token) {
    (*token) += strspn((*token), " \t");
    int i = atoi((*token));
    (*token) += strcspn((*token), " \t\r");
    return i;
  }

  inline std::istream& getline(std::istream& is, std::string& t) {
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

  inline bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
  }

  inline std::string substr_before_slash(const std::string& path) {
    const size_t last_slash_idx = path.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
      return path.substr(0, last_slash_idx + 1);
    }

    return std::string("");
  }

  inline std::string substr_after_slash(const std::string& path) {
    const size_t last_slash_idx = path.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
      return path.substr(last_slash_idx + 1);
    }

    return path;
  }

  inline void splitStr(std::vector<std::string>& elems, const char* delims, const char** token) {
    const char* end = (*token) + strcspn((*token), "\n\r");
    size_t offset = end - (*token);
    if (offset != 0) {
      char* dest = (char*)malloc(sizeof(char) * offset + 1);
      strncpy(dest, (*token), offset);
      *(dest + offset) = 0;
      const char* pch = strtok(dest, delims);
      while (pch != nullptr) {
        // trim relative path slash
        elems.emplace_back(substr_after_slash(pch));
        pch = strtok(nullptr, delims);
      }
      free(dest);
    }
  }

  inline bool parseOnOff(const char** token, bool default_value) {
    (*token) += strspn((*token), " \t");
    const char *end = (*token) + strcspn((*token), " \t\r");

    bool ret = default_value;
    if ((0 == strncmp((*token), "on", 2))) {
      ret = true;
    } else if ((0 == strncmp((*token), "off", 3))) {
      ret = false;
    }

    (*token) = end;
    return ret;
  }

  inline bool parseTexture(texture* tex, const char* token) {
    while (!IS_NEW_LINE((*token))) {
      token += strspn(token, " \t");  // skip space
      if ((0 == strncmp(token, "-clamp", 6)) && IS_SPACE((token[6]))) {
        token += 7;
        tex->option.clamp = parseOnOff(&token, /* default */ true);
      } else if ((0 == strncmp(token, "-blendu", 7)) && IS_SPACE((token[7]))) {
        token += 8;
        tex->option.blendu = parseOnOff(&token, /* default */ true);
      } else if ((0 == strncmp(token, "-blendv", 7)) && IS_SPACE((token[7]))) {
        token += 8;
        tex->option.blendv = parseOnOff(&token, /* default */ true);
      } else if ((0 == strncmp(token, "-bm", 3)) && IS_SPACE((token[3]))) {
        token += 4;
        tex->option.bump_multiplier = parseReal(&token, 1.f);
      } else {
        // only texture name without any options
        tex->path = parseString(&token);
      }
    }

    return true;
  }

  // NOTE: pbr material not support.
  inline bool load_mtl(std::vector<material>& materials, std::unordered_map<std::string, int>& material_map, std::istream &ifs) {
    material current_mat;
    bool has_d = false;
    std::string line_buf;

    while(!getline(ifs, line_buf).eof()) {

      // Trim trailing whitespace.
      if (!line_buf.empty()) {
        line_buf = line_buf.substr(0, line_buf.find_last_not_of(" \t") + 1);
      }

      // Trim newline '\r\n' or '\n'
      if (!line_buf.empty()) {
        if (line_buf[line_buf.size() - 1] == '\n')
          line_buf.erase(line_buf.size() - 1);
      }
      if (!line_buf.empty()) {
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

      if (token == nullptr) return false;
      if (token[0] == '\0') continue;  // empty line
      if (token[0] == '#') continue;  // comment line

      // new mtl
      if ((0 == strncmp(token, "newmtl", 6)) && IS_SPACE((token[6]))) {
        // save previous material
        if (!current_mat.name.empty()) {
          material_map.insert(std::make_pair(current_mat.name, static_cast<int>(materials.size())));
          materials.emplace_back(current_mat);
        }

        // reset material
        current_mat = material();
        has_d = false;

        // parse new mat name
        token += 7;
        current_mat.name = parseString(&token);
        continue;
      }

      // ambient
      if (token[0] == 'K' && token[1] == 'a' && IS_SPACE((token[2]))) {
        token += 2;
        vec3 ambient;
        parseReal3(ambient, &token);
        current_mat.ambient = ambient;
        continue;
      }

      // diffuse
      if (token[0] == 'K' && token[1] == 'd' && IS_SPACE((token[2]))) {
        token += 2;
        vec3 diffuse;
        parseReal3(diffuse, &token);
        current_mat.diffuse = diffuse;
        continue;
      }

      // specular
      if (token[0] == 'K' && token[1] == 's' && IS_SPACE((token[2]))) {
        token += 2;
        vec3 specular;
        parseReal3(specular, &token);
        current_mat.specular = specular;
        continue;
      }

      // transmittance
      if ((token[0] == 'K' && token[1] == 't' && IS_SPACE((token[2]))) ||
          (token[0] == 'T' && token[1] == 'f' && IS_SPACE((token[2])))) {
        token += 2;
        vec3 transmittance;
        parseReal3(transmittance, &token);
        current_mat.transmittance = transmittance;
        continue;
      }

      // ior(index of refraction)
      if (token[0] == 'N' && token[1] == 'i' && IS_SPACE((token[2]))) {
        token += 2;
        current_mat.ior = parseReal(&token, 0.f);
        continue;
      }

      // emission
      if (token[0] == 'K' && token[1] == 'e' && IS_SPACE(token[2])) {
        token += 2;
        vec3 emission;
        parseReal3(emission, &token);
        current_mat.emission = emission;
        continue;
      }

      // shininess
      if (token[0] == 'N' && token[1] == 's' && IS_SPACE(token[2])) {
        token += 2;
        current_mat.shininess = parseReal(&token, 0.f);
        continue;
      }

      // illum model
      if (0 == strncmp(token, "illum", 5) && IS_SPACE(token[5])) {
        token += 6;
        current_mat.illum = parseInt(&token);
        continue;
      }

      // dissolve (the non-transparency of the material), The default is 1.0 (not transparent at all)
      if ((token[0] == 'd' && IS_SPACE(token[1]))) {
        token += 1;
        current_mat.dissolve = parseReal(&token, 1.f);
        has_d = true;
        continue;
      }

      // dissolve (the transparency of the material): 1.0 - Tr, The default is 0.0 (not transparent at all)
      if (token[0] == 'T' && token[1] == 'r' && IS_SPACE(token[2])) {
        token += 2;
        if (!has_d) {
          current_mat.dissolve = 1.f - parseReal(&token, 0.f);
        }
        continue;
      }

      // ambient texture
      if ((0 == strncmp(token, "map_Ka", 6)) && IS_SPACE(token[6])) {
        token += 7;
        texture ambient;
        if (parseTexture(&ambient, token)) {
          current_mat.texture_map.insert(std::make_pair(tex_type::AMBIENT, ambient));
        }
        continue;
      }

      // diffuse texture
      if ((0 == strncmp(token, "map_Kd", 6)) && IS_SPACE(token[6])) {
        token += 7;
        texture diffuse;
        if (parseTexture(&diffuse, token)) {
          current_mat.texture_map.insert(std::make_pair(tex_type::DIFFUSE, diffuse));
        }
        continue;
      }

      // specular texture
      if ((0 == strncmp(token, "map_Ks", 6)) && IS_SPACE(token[6])) {
        token += 7;
        texture specular;
        if (parseTexture(&specular, token)) {
          current_mat.texture_map.insert(std::make_pair(tex_type::SPECULAR, specular));
        }
        continue;
      }

      // specular highlight texture
      if ((0 == strncmp(token, "map_Ns", 6)) && IS_SPACE(token[6])) {
        token += 7;
        texture specular_highlight;
        if (parseTexture(&specular_highlight, token)) {
          current_mat.texture_map.insert(
            std::make_pair(tex_type::SPECULAR_HIGHLIGHT, specular_highlight)
          );
        }
        continue;
      }

      // bump texture
      if (((0 == strncmp(token, "map_bump", 8)) && IS_SPACE(token[8])) ||
          ((0 == strncmp(token, "map_Bump", 8)) && IS_SPACE(token[8])) ||
          ((0 == strncmp(token, "bump", 4)) && IS_SPACE(token[4]))
      ) {
        continue;
      }

      // alpha texture
      if ((0 == strncmp(token, "map_d", 5)) && IS_SPACE(token[5])) {
        continue;
      }

      // displacement texture
      if ((0 == strncmp(token, "disp", 4)) && IS_SPACE(token[4])) {
        continue;
      }

      // reflection map
      if ((0 == strncmp(token, "refl", 4)) && IS_SPACE(token[4])) {
        continue;
      }
    }

    // flush last material

    return true;
  }

  inline bool parseMat(const std::string& mat_name, const std::string& base_mat_dir, std::vector<material>& materials, std::unordered_map<std::string, int>& material_map) {
    std::ifstream ifs(base_mat_dir + mat_name);
    if (!ifs) {
      return false;
    }
    load_mtl(materials, material_map, ifs);
    return true;
  }

  // NOTE: Geometry entities other than "facets" (including "points", "lines", "curves", etc.) are not supported.
  // smooth group is also not supported.
  bool load_obj(const std::string& path, std::vector<shape>& shapes, parse_option parseOption) {
    if (!endsWith(path, ".obj")) {
      return false;
    }

    std::ifstream ifs(path);
    if(!ifs) {
      return false;
    }

    std::vector<vec4> vertices;
    std::vector<vec2> texcoords;
    std::vector<vec3> normals;
    primitive_group prim_group;
    std::string current_object_name;
    int max_vindex = -1, max_vtindex = -1, max_vnindex = -1;
    shape shape_group;
    std::unordered_map<std::string, int> material_map;
    std::vector<material> materials;
    int current_material_id = -1;
    std::string mtl_base_dir = substr_before_slash(path);
    std::string line_buf;

    while(!getline(ifs, line_buf).eof()) {

      // Trim newline '\r\n' or '\n'
      if (!line_buf.empty()) {
        if (line_buf[line_buf.size() - 1] == '\n')
          line_buf.erase(line_buf.size() - 1);
      }
      if (!line_buf.empty()) {
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

      if (token == nullptr) return false;
      if (token[0] == '\0') continue;  // empty line
      if (token[0] == '#') continue;  // comment line

      // vertex
      if (token[0] == 'v' && IS_SPACE((token[1]))) {
        token += 2;
        vec4 v;
        parseReal4(v, &token);
        vertices.emplace_back(v);
        continue;
      }

      // normal
      if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
        token += 3;
        vec3 vn;
        parseReal3(vn, &token);
        normals.emplace_back(vn);
        continue;
      }

      // texcoord
      if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
        token += 3;
        vec2 vt;
        parseReal2(vt, &token);
        if (parseOption & parse_option::FLIP_UV) {
          vt.y = 1.f - vt.y;
        }
        texcoords.emplace_back(vt);
        continue;
      }

      // face
      if (token[0] == 'f' && IS_SPACE((token[1]))) {
        token += 2;
        token += strspn(token, " \t"); // Skip leading space.

        face f;
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
          parsePrimitive(&shape_group, prim_group, parseOption, current_material_id, current_object_name); // return value not used
          // clear current primitives shape groups
          prim_group.face_group.clear();
          // cache new material id
          current_material_id = new_material_id;
        }
        continue;
      }

      // load mtl
      if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
        token += 7;
        std::vector<std::string> filenames;
        // parse multiple mtl filenames split by whitespace
        splitStr(filenames, " ", &token);
        // load just one available mtl file in the list
        for (std::string& name : filenames) {
          if (parseMat(name, mtl_base_dir, materials, material_map)) {
            break;
          }
        }
        continue;
      }

      // group name
      if (token[0] == 'g' && IS_SPACE((token[1]))) {
        parsePrimitive(&shape_group, prim_group, parseOption, current_material_id, current_object_name); // return value not used
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
        parsePrimitive(&shape_group, prim_group, parseOption, current_material_id, current_object_name); // return value not used
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

    bool ret = parsePrimitive(&shape_group, prim_group, parseOption, current_material_id, current_object_name);
    if (ret || !shape_group.mesh_group.indices.empty()) {
      shapes.emplace_back(shape_group);
    }

    return true;
  }
}

#endif //MODEL_LOAD_OBJ_LOADER_H
