#ifndef MESH_DATA_H_
#define MESH_DATA_H_

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "gloo/VertexObject.hpp"

namespace GLOO {

// Mesh data structure for simplification algorithms
struct MeshData {
  // Vertex positions
  std::vector<glm::vec3> vertices;
  
  // Face indices (triangles)
  std::vector<glm::uvec3> faces;
  
  // Optional: vertex normals
  std::vector<glm::vec3> normals;
  
  // Optional: vertex colors
  std::vector<glm::vec3> colors;
  
  // Optional: texture coordinates
  std::vector<glm::vec2> texcoords;

  MeshData() = default;
  MeshData(const MeshData& other) = default;
  MeshData& operator=(const MeshData& other) = default;

  // Convert from VertexObject
  static MeshData FromVertexObject(const VertexObject& vertex_obj);
  
  // Convert to VertexObject for rendering
  std::shared_ptr<VertexObject> ToVertexObject() const;
  
  // Utility methods
  size_t GetVertexCount() const { return vertices.size(); }
  size_t GetFaceCount() const { return faces.size(); }
  size_t GetTriangleCount() const { return faces.size(); }
  
  void Clear() {
    vertices.clear();
    faces.clear();
    normals.clear();
    colors.clear();
    texcoords.clear();
  }
  
  bool IsEmpty() const { return vertices.empty() || faces.empty(); }
  
  // Compute normals from geometry
  void ComputeNormals();
  
  // Validate mesh (check for degenerate faces, etc.)
  bool Validate() const;
};

// Edge structure for mesh operations
struct MeshEdge {
  int v1, v2;  // Vertex indices (v1 < v2)
  
  MeshEdge(int a, int b) : v1(std::min(a, b)), v2(std::max(a, b)) {}
  
  bool operator==(const MeshEdge& other) const {
    return v1 == other.v1 && v2 == other.v2;
  }
  
  struct Hash {
    size_t operator()(const MeshEdge& e) const {
      return std::hash<int>()(e.v1) ^ (std::hash<int>()(e.v2) << 1);
    }
  };
};

}  // namespace GLOO

#endif

