#include "MeshData.hpp"
#include "gloo/gl_wrapper/BindGuard.hpp"
#include <iostream>

namespace GLOO {

MeshData MeshData::FromVertexObject(const VertexObject& vertex_obj) {
  // TODO: Convert VertexObject to MeshData
  // Extract positions, indices, normals, etc.
  MeshData data;
  
  // This will need to access the VertexObject's internal buffers
  // May need to extend VertexObject API or use MeshLoader directly
  
  return data;
}

std::shared_ptr<VertexObject> MeshData::ToVertexObject() const {
  // TODO: Convert MeshData to VertexObject for rendering
  auto vertex_obj = std::make_shared<VertexObject>();
  
  // Create position buffer
  auto positions = make_unique<PositionArray>();
  for (const auto& v : vertices) {
    positions->push_back(v);
  }
  vertex_obj->UpdatePositions(std::move(positions));
  
  // Create index buffer
  auto indices = make_unique<IndexArray>();
  for (const auto& f : faces) {
    indices->push_back(f.x);
    indices->push_back(f.y);
    indices->push_back(f.z);
  }
  vertex_obj->UpdateIndices(std::move(indices));
  
  // Create normal buffer if available
  if (!normals.empty()) {
    auto normal_array = make_unique<NormalArray>();
    for (const auto& n : normals) {
      normal_array->push_back(n);
    }
    vertex_obj->UpdateNormals(std::move(normal_array));
  }
  
  return vertex_obj;
}

void MeshData::ComputeNormals() {
  // TODO: Compute per-vertex normals from face data
  normals.clear();
  normals.resize(vertices.size(), glm::vec3(0.0f));
  
  // For each face, compute face normal and add to vertex normals
  for (const auto& face : faces) {
    const glm::vec3& v0 = vertices[face.x];
    const glm::vec3& v1 = vertices[face.y];
    const glm::vec3& v2 = vertices[face.z];
    
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 face_normal = glm::normalize(glm::cross(edge1, edge2));
    
    normals[face.x] += face_normal;
    normals[face.y] += face_normal;
    normals[face.z] += face_normal;
  }
  
  // Normalize vertex normals
  for (auto& normal : normals) {
    if (glm::length(normal) > 0.0f) {
      normal = glm::normalize(normal);
    }
  }
}

bool MeshData::Validate() const {
  // TODO: Check for degenerate faces, invalid indices, etc.
  
  // Check if any face has duplicate vertices
  for (const auto& face : faces) {
    if (face.x == face.y || face.y == face.z || face.z == face.x) {
      return false;  // Degenerate face
    }
    
    // Check if indices are in bounds
    if (face.x >= vertices.size() || 
        face.y >= vertices.size() || 
        face.z >= vertices.size()) {
      return false;  // Invalid index
    }
  }
  
  return true;
}

}  // namespace GLOO

