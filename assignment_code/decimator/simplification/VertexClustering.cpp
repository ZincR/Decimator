#include "VertexClustering.hpp"
#include <algorithm>
#include <cmath>

namespace GLOO {

VertexClustering::VertexClustering() {
  // Constructor
}

VertexClustering::~VertexClustering() {
  // Destructor
}

std::shared_ptr<SimplificationMesh> VertexClustering::Simplify(
    const SimplificationMesh& original_mesh, 
    int grid_resolution) {
  // TODO: Implement Rossignac-Borrel vertex clustering
  // 1. Compute bounding box
  // 2. Create uniform grid
  // 3. Assign each vertex to a grid cell
  // 4. For each cell, compute representative vertex
  // 5. Merge all vertices in same cell to representative
  // 6. Remove degenerate faces
  
  grid_resolution_ = grid_resolution;
  auto result = std::make_shared<SimplificationMesh>(original_mesh);
  return result;
}

std::shared_ptr<SimplificationMesh> VertexClustering::SimplifyByFactor(
    const SimplificationMesh& original_mesh, 
    float reduction_factor) {
  // Estimate grid resolution based on reduction factor
  // Higher resolution = less reduction
  int estimated_resolution = static_cast<int>(
      std::cbrt(original_mesh.vertices.size() * reduction_factor));
  estimated_resolution = std::max(2, estimated_resolution);
  
  return Simplify(original_mesh, estimated_resolution);
}

glm::vec3 VertexClustering::ComputeBoundingBox(
    const SimplificationMesh& mesh, 
    glm::vec3& min_bounds, 
    glm::vec3& max_bounds) const {
  // TODO: Compute mesh bounding box
  if (mesh.vertices.empty()) {
    min_bounds = max_bounds = glm::vec3(0.0f);
    return glm::vec3(0.0f);
  }
  
  min_bounds = max_bounds = mesh.vertices[0];
  // Find min/max for each axis
  
  return max_bounds - min_bounds;
}

glm::ivec3 VertexClustering::GetGridCell(
    const glm::vec3& position, 
    const glm::vec3& min_bounds, 
    const glm::vec3& grid_size) const {
  // TODO: Compute grid cell coordinates for a position
  glm::vec3 normalized = (position - min_bounds) / grid_size;
  return glm::ivec3(
      static_cast<int>(normalized.x * grid_resolution_),
      static_cast<int>(normalized.y * grid_resolution_),
      static_cast<int>(normalized.z * grid_resolution_)
  );
}

void VertexClustering::AssignVerticesToCells(
    const SimplificationMesh& mesh, 
    std::unordered_map<glm::ivec3, GridCell, GridCell::Hash>& grid) {
  // TODO: Assign each vertex to its grid cell
}

glm::vec3 VertexClustering::ComputeRepresentative(
    const GridCell& cell, 
    const SimplificationMesh& mesh) const {
  // TODO: Compute representative position for cell
  // Options:
  // - Average of all vertices in cell
  // - Median position
  // - Vertex closest to center
  return glm::vec3(0.0f);
}

void VertexClustering::MergeClusters(
    const SimplificationMesh& original_mesh, 
    const std::unordered_map<glm::ivec3, GridCell, GridCell::Hash>& grid,
    SimplificationMesh& result) {
  // TODO: Create new mesh with merged vertices
  // 1. Create mapping from old vertex indices to new ones
  // 2. Update all face indices
  // 3. Remove degenerate faces (where all vertices map to same point)
}

}  // namespace GLOO

