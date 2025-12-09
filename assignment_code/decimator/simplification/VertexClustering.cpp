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
  // Rossignac-Borrel vertex clustering algorithm
  // Strategy: Divide space into uniform grid, merge vertices within same cell
  
  if (original_mesh.IsEmpty()) {
    return std::make_shared<SimplificationMesh>(original_mesh);
  }
  
  grid_resolution_ = grid_resolution;
  
  // 1. Compute bounding box
  glm::vec3 min_bounds, max_bounds;
  glm::vec3 grid_size = ComputeBoundingBox(original_mesh, min_bounds, max_bounds);
  
  if (grid_size.x <= 0.0f || grid_size.y <= 0.0f || grid_size.z <= 0.0f) {
    return std::make_shared<SimplificationMesh>(original_mesh);
  }
  
  // 2. Assign vertices to grid cells
  std::unordered_map<glm::ivec3, GridCell, GridCell::Hash> grid;
  AssignVerticesToCells(original_mesh, grid);
  
  // 3. Compute representative vertices for each cell
  for (auto& pair : grid) {
    GridCell& cell = pair.second;
    cell.representative_pos = ComputeRepresentative(cell, original_mesh);
  }
  
  // 4. Merge clusters and create new mesh
  auto result = std::make_shared<SimplificationMesh>();
  MergeClusters(original_mesh, grid, *result);
  
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
  // Compute axis-aligned bounding box of all vertices
  if (mesh.vertices.empty()) {
    min_bounds = max_bounds = glm::vec3(0.0f);
    return glm::vec3(0.0f);
  }
  
  min_bounds = max_bounds = mesh.vertices[0];
  
  // Find min/max for each axis
  for (const auto& vertex : mesh.vertices) {
    min_bounds.x = std::min(min_bounds.x, vertex.x);
    min_bounds.y = std::min(min_bounds.y, vertex.y);
    min_bounds.z = std::min(min_bounds.z, vertex.z);
    
    max_bounds.x = std::max(max_bounds.x, vertex.x);
    max_bounds.y = std::max(max_bounds.y, vertex.y);
    max_bounds.z = std::max(max_bounds.z, vertex.z);
  }
  
  // Add small epsilon to avoid division by zero
  glm::vec3 size = max_bounds - min_bounds;
  const float epsilon = 1e-6f;
  if (size.x < epsilon) size.x = epsilon;
  if (size.y < epsilon) size.y = epsilon;
  if (size.z < epsilon) size.z = epsilon;
  
  return size;
}

glm::ivec3 VertexClustering::GetGridCell(
    const glm::vec3& position, 
    const glm::vec3& min_bounds, 
    const glm::vec3& grid_size) const {
  // Compute which grid cell a vertex belongs to
  // Normalize position to [0, 1] range within bounding box
  glm::vec3 normalized = (position - min_bounds) / grid_size;
  
  // Clamp to [0, 1] to handle floating point errors
  normalized.x = std::max(0.0f, std::min(1.0f, normalized.x));
  normalized.y = std::max(0.0f, std::min(1.0f, normalized.y));
  normalized.z = std::max(0.0f, std::min(1.0f, normalized.z));
  
  // Convert to grid coordinates [0, grid_resolution_-1]
  int cell_x = static_cast<int>(normalized.x * grid_resolution_);
  int cell_y = static_cast<int>(normalized.y * grid_resolution_);
  int cell_z = static_cast<int>(normalized.z * grid_resolution_);
  
  // Clamp to valid grid range (handle edge case where normalized = 1.0)
  cell_x = std::min(cell_x, grid_resolution_ - 1);
  cell_y = std::min(cell_y, grid_resolution_ - 1);
  cell_z = std::min(cell_z, grid_resolution_ - 1);
  
  return glm::ivec3(cell_x, cell_y, cell_z);
}

void VertexClustering::AssignVerticesToCells(
    const SimplificationMesh& mesh, 
    std::unordered_map<glm::ivec3, GridCell, GridCell::Hash>& grid) {
  // Compute bounding box first
  glm::vec3 min_bounds, max_bounds;
  glm::vec3 grid_size = ComputeBoundingBox(mesh, min_bounds, max_bounds);
  
  // Assign each vertex to its corresponding grid cell
  for (size_t i = 0; i < mesh.vertices.size(); i++) {
    glm::ivec3 cell_coords = GetGridCell(mesh.vertices[i], min_bounds, grid_size);
    
    // Get or create the grid cell
    auto it = grid.find(cell_coords);
    if (it == grid.end()) {
      GridCell new_cell;
      new_cell.cell_coords = cell_coords;
      grid[cell_coords] = new_cell;
      it = grid.find(cell_coords);
    }
    
    // Add vertex index to this cell
    it->second.vertex_indices.push_back(static_cast<int>(i));
  }
}

glm::vec3 VertexClustering::ComputeRepresentative(
    const GridCell& cell, 
    const SimplificationMesh& mesh) const {
  // Compute representative vertex as the average (centroid) of all vertices in cell
  // This minimizes the average error for vertices in this cell
  
  if (cell.vertex_indices.empty()) {
    return glm::vec3(0.0f);
  }
  
  glm::vec3 sum(0.0f);
  for (int vertex_idx : cell.vertex_indices) {
    sum += mesh.vertices[vertex_idx];
  }
  
  return sum / static_cast<float>(cell.vertex_indices.size());
}

void VertexClustering::MergeClusters(
    const SimplificationMesh& original_mesh, 
    const std::unordered_map<glm::ivec3, GridCell, GridCell::Hash>& grid,
    SimplificationMesh& result) {
  
  result.Clear();
  
  // 1. Create mapping from old vertex indices to new representative indices
  std::vector<int> vertex_to_representative(original_mesh.vertices.size(), -1);
  std::unordered_map<glm::ivec3, int, GridCell::Hash> cell_to_new_index;
  
  int new_vertex_index = 0;
  
  // For each grid cell, create one new vertex (the representative)
  for (const auto& pair : grid) {
    const GridCell& cell = pair.second;
    if (cell.vertex_indices.empty()) continue;
    
    // Store representative vertex position
    result.vertices.push_back(cell.representative_pos);
    
    // Map all old vertices in this cell to the new representative index
    for (int old_idx : cell.vertex_indices) {
      vertex_to_representative[old_idx] = new_vertex_index;
    }
    
    // Store mapping for this cell
    cell_to_new_index[cell.cell_coords] = new_vertex_index;
    new_vertex_index++;
  }
  
  // 2. Remap face indices and remove degenerate faces
  for (const auto& face : original_mesh.faces) {
    // Get new indices for this face's vertices
    int new_idx0 = vertex_to_representative[face.x];
    int new_idx1 = vertex_to_representative[face.y];
    int new_idx2 = vertex_to_representative[face.z];
    
    // Skip if any vertex wasn't mapped (shouldn't happen, but safety check)
    if (new_idx0 < 0 || new_idx1 < 0 || new_idx2 < 0) {
      continue;
    }
    
    // Skip degenerate faces (where all three vertices map to same representative)
    if (new_idx0 == new_idx1 && new_idx1 == new_idx2) {
      continue;
    }
    
    // Add face with remapped indices
    result.faces.push_back(glm::uvec3(
        static_cast<unsigned int>(new_idx0),
        static_cast<unsigned int>(new_idx1),
        static_cast<unsigned int>(new_idx2)
    ));
  }
  
  // 3. Compute normals for the simplified mesh
  result.ComputeNormals();
}

}  // namespace GLOO

