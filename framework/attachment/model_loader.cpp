#include "model_loader.hpp"

// use floats and med precision operations
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat2x2.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/geometric.hpp>

#include <iostream>

namespace model_loader {

std::vector<glm::fvec4> generate_tangents(tinyobj::mesh_t const& model);

model obj(std::string const& name, model::attrib_flag_t import_attribs){
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err = tinyobj::LoadObj(shapes, materials, name.c_str());

  if (!err.empty()) {
    if (err[0] == 'W' && err[1] == 'A' && err[2] == 'R') {
      std::cerr << "tinyobjloader: " << err << std::endl;    
    }
    else {
      throw std::logic_error("tinyobjloader: " + err);    
    }
  }

  model::attrib_flag_t attributes{model::POSITION | import_attribs};

  std::vector<float> vertex_data;
  std::vector<unsigned> triangles;

  GLuint vertex_offset = 0;

  for (auto& shape : shapes) {
    tinyobj::mesh_t& curr_mesh = shape.mesh;
    
    bool has_normals = import_attribs & model::NORMAL;
    if (has_normals) {
      // generate normals if necessary
      if (curr_mesh.normals.empty()) {
        generate_normals(curr_mesh);
      }
    }

    bool has_uvs = import_attribs & model::TEXCOORD;
    if (has_uvs) {
      if (curr_mesh.texcoords.empty()) {
        has_uvs = false;
        attributes ^= model::TEXCOORD;
        std::cerr << "Shape has no texcoords" << std::endl;
      }
    }

    bool has_tangents = import_attribs & model::TANGENT;
    std::vector<glm::fvec4> tangents;
    if (has_tangents) {
      if (!has_uvs) {
        has_tangents = false;
        attributes ^= model::TANGENT;
        std::cerr << "Shape has no texcoords" << std::endl;
      }
      else {
        tangents = generate_tangents(curr_mesh);
      }
    }

    // push back vertex attributes
    for (unsigned i = 0; i < curr_mesh.positions.size() / 3; ++i) {
      vertex_data.push_back(curr_mesh.positions[i * 3]);
      vertex_data.push_back(curr_mesh.positions[i * 3 + 1]);
      vertex_data.push_back(curr_mesh.positions[i * 3 + 2]);

      if (has_normals) {
        vertex_data.push_back(curr_mesh.normals[i * 3]);
        vertex_data.push_back(curr_mesh.normals[i * 3 + 1]);
        vertex_data.push_back(curr_mesh.normals[i * 3 + 2]);
      }

      if (has_uvs) {
        vertex_data.push_back(curr_mesh.texcoords[i * 2]);
        vertex_data.push_back(curr_mesh.texcoords[i * 2 + 1]);
      }

      if (has_tangents) {
        vertex_data.push_back(tangents[i].x);
        vertex_data.push_back(tangents[i].y);
        vertex_data.push_back(tangents[i].z);
        vertex_data.push_back(tangents[i].w);
      }
    }

    // add triangles
    for (unsigned i = 0; i < curr_mesh.indices.size(); ++i) {
      triangles.push_back(vertex_offset + curr_mesh.indices[i]);
    }

    vertex_offset += GLuint(curr_mesh.positions.size() / 3);
  }

  return model{vertex_data, attributes, triangles};
}

void generate_normals(tinyobj::mesh_t& model) {
  std::vector<glm::fvec3> positions(model.positions.size() / 3);

  for (unsigned i = 0; i < model.positions.size(); i+=3) {
    positions[i / 3] = glm::fvec3{model.positions[i], model.positions[i + 1], model.positions[i + 2]};
  }

  std::vector<glm::fvec3> normals(model.positions.size() / 3, glm::fvec3{0.0f});
  for (unsigned i = 0; i < model.indices.size(); i+=3) {
    glm::fvec3 normal = glm::cross(positions[model.indices[i+1]] - positions[model.indices[i]], positions[model.indices[i+2]] - positions[model.indices[i]]);

    normals[model.indices[i]] += normal;
    normals[model.indices[i+1]] += normal;
    normals[model.indices[i+2]] += normal;
  }

  model.normals.reserve(model.positions.size());
  for (unsigned i = 0; i < normals.size(); ++i) {
    glm::fvec3 normal = glm::normalize(normals[i]);
    model.normals[i * 3] = normal[0];
    model.normals[i * 3 + 1] = normal[1];
    model.normals[i * 3 + 2] = normal[2];
  }
}

std::vector<glm::fvec4> generate_tangents(tinyobj::mesh_t const& model) {
  // containers for vetex attributes
  std::vector<glm::fvec3> positions(model.positions.size() / 3);
  std::vector<glm::fvec3> normals(model.positions.size() / 3);
  std::vector<glm::fvec2> uvs(model.positions.size() / 3);
  std::vector<glm::fvec3> tangents(model.positions.size() / 3, glm::fvec3{0.0f});
  std::vector<glm::fvec3> bitangents(model.positions.size() / 3, glm::fvec3{0.0f});

  // get vertex positions and texture coordinates
  for (unsigned i = 0; i < model.positions.size(); i+=3) {
    positions[i / 3] = glm::fvec3{model.positions[i], model.positions[i + 1], model.positions[i + 2]};
    normals[i / 3] = glm::fvec3{model.normals[i], model.normals[i + 1], model.normals[i + 2]};
  }
  for (unsigned i = 0; i < model.texcoords.size(); i+=2) {
    uvs[i / 2] = glm::fvec2{model.texcoords[i], model.texcoords[i + 1]};
  }
  // calculate tangent for triangles
  for (unsigned i = 0; i < model.indices.size() / 3; i++) {
    // indices of vertices of triangle
    unsigned indices[3] = {model.indices[i * 3], model.indices[i * 3 + 1], model.indices[i * 3 + 2]};
    // triangle edge directions in model space
    glm::fvec3 s = positions[indices[1]] - positions[indices[0]];
    glm::fvec3 t = positions[indices[2]] - positions[indices[0]];

    // triangle edge directions in model space
    glm::fvec2 u = uvs[indices[1]] - uvs[indices[0]];
    glm::fvec2 v = uvs[indices[2]] - uvs[indices[0]];
    // calculate tangent and bitangent

    glm::mat3x2 tangent_bitangent = glm::mat2{v.y, -v.x, -u.y, u.x} * glm::mat3x2{s.x, s.y, s.z, t.x, t.y, t.z};
    // read vectors from matrix
    glm::fvec3 tangent = glm::normalize(glm::row(tangent_bitangent, 0));
    glm::fvec3 bitangent = glm::normalize(glm::row(tangent_bitangent, 1));

    // accumulate tangents for each vert of tri
    tangents[indices[0]] += tangent;
    tangents[indices[1]] += tangent;
    tangents[indices[2]] += tangent;

    bitangents[indices[0]] += bitangent;
    bitangents[indices[1]] += bitangent;
    bitangents[indices[2]] += bitangent;
  }

  std::vector<glm::fvec4> tangents_handed{model.positions.size() / 3, glm::fvec4{0.0f}};
  // orthogonalize and normalize all tangents
  for (unsigned i = 0; i < tangents.size(); ++i) {
    // orthogonalize tangent relative to normal
    tangents[i] = tangents[i] - glm::dot(tangents[i], normals[i]) * normals[i];
    tangents[i] = glm::normalize(tangents[i]);
    // correct handedness
    if (glm::dot(glm::cross(normals[i], tangents[i]), bitangents[i]) < 0.0f) {
      tangents_handed[i] = glm::fvec4{tangents[i], -1.0f};
    }
    else {
      tangents_handed[i] = glm::fvec4{tangents[i], 1.0f};
    }
  }

  return tangents_handed;
}

};