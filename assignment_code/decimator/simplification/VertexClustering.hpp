#ifndef VERTEX_CLUSTERING_H_
#define VERTEX_CLUSTERING_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "SimplificationMesh.hpp"

namespace GLOO {

// Vertex Clustering using Rossignac-Borrel algorithm
// Reference: "Multi-resolution 3D approximations for rendering complex scenes"
class VertexClustering {
 public:
  VertexClustering();
  ~VertexClustering();

  // Simplify mesh with given grid resolution
  std::shared_ptr<SimplificationMesh> Simplify(const SimplificationMesh& original_mesh, 
                                      int grid_resolution);

  // Simplify mesh by reduction factor (0.0 to 1.0)
  std::shared_ptr<SimplificationMesh> SimplifyByFactor(const SimplificationMesh& original_mesh, 
                                               float reduction_factor);

  // Set grid resolution explicitly
  void SetGridResolution(int resolution) { grid_resolution_ = resolution; }

 private:
  int grid_resolution_ = 16;  // Default grid resolution

  struct GridCell {
    glm::ivec3 cell_coords;
    std::vector<int> vertex_indices;
    glm::vec3 representative_pos;
    int representative_index;
    
    // Hash function for use in unordered_map
    struct Hash {
      size_t operator()(const glm::ivec3& v) const {
        return std::hash<int>()(v.x) ^ 
               (std::hash<int>()(v.y) << 1) ^ 
               (std::hash<int>()(v.z) << 2);
      }
    };
  };

  glm::vec3 ComputeBoundingBox(const SimplificationMesh& mesh, 
                                glm::vec3& min_bounds, 
                                glm::vec3& max_bounds) const;
  glm::ivec3 GetGridCell(const glm::vec3& position, 
                          const glm::vec3& min_bounds, 
                          const glm::vec3& grid_size) const;
  void AssignVerticesToCells(const SimplificationMesh& mesh, 
                              std::unordered_map<glm::ivec3, GridCell, 
                                                 GridCell::Hash>& grid);
  glm::vec3 ComputeRepresentative(const GridCell& cell, 
                                   const SimplificationMesh& mesh) const;
  void MergeClusters(const SimplificationMesh& original_mesh, 
                     const std::unordered_map<glm::ivec3, GridCell, 
                                              GridCell::Hash>& grid,
                     SimplificationMesh& result);
};

}  // namespace GLOO

#endif

