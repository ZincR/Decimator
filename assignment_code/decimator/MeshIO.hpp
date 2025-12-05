#ifndef MESH_IO_H_
#define MESH_IO_H_

#include <string>
#include <memory>
#include "simplification/SimplificationMesh.hpp"

namespace GLOO {

// Utility class for loading and saving mesh files
class MeshIO {
 public:
  // Load OBJ file
  static std::shared_ptr<SimplificationMesh> LoadOBJ(const std::string& filepath);
  
  // Save mesh to OBJ file
  static bool SaveOBJ(const std::string& filepath, const SimplificationMesh& mesh);
  
  // Load mesh using GLOO's MeshLoader (supports multiple formats)
  static std::shared_ptr<SimplificationMesh> LoadMesh(const std::string& filepath);

 private:
  // Helper for parsing OBJ lines
  static bool ParseVertex(const std::string& line, glm::vec3& vertex);
  static bool ParseNormal(const std::string& line, glm::vec3& normal);
  static bool ParseTexCoord(const std::string& line, glm::vec2& texcoord);
  static bool ParseFace(const std::string& line, std::vector<glm::uvec3>& indices);
};

}  // namespace GLOO

#endif

