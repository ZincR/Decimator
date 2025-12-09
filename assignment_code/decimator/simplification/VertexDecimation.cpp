#include "VertexDecimation.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <limits>

namespace GLOO {

VertexDecimation::VertexDecimation() {
  // Constructor
}

VertexDecimation::~VertexDecimation() {
  // Destructor
}

std::shared_ptr<SimplificationMesh> VertexDecimation::Simplify(
    const SimplificationMesh& original_mesh, 
    int target_vertex_count) {
  // Implement Schroeder-Zarge-Lorensen vertex decimation
  // 1. Classify vertices (feature, boundary, interior)
  // 2. Compute distance error for each vertex
  // 3. Sort vertices by suitability for removal
  // 4. Iteratively remove vertices and retriangulate
  // 5. Continue until target vertex count is reached

  // Work on a copy since we'll be modifying the mesh
  auto result = std::make_shared<SimplificationMesh>(original_mesh);
  
  // Continue until we reach target vertex count
  while (static_cast<int>(result->vertices.size()) > target_vertex_count) {
    // Classify all vertices
    std::vector<VertexInfo> vertex_info;
    ClassifyVertices(*result, vertex_info);
    
    // Sort by distance error (lowest error = easiest to remove)
    std::sort(vertex_info.begin(), vertex_info.end(), 
              [](const VertexInfo& a, const VertexInfo& b) {
                return a.distance_error < b.distance_error;
              });
    
    // Try to remove vertices starting with lowest error
    bool removed_any = false;
    for (const auto& info : vertex_info) {
      if (static_cast<int>(result->vertices.size()) <= target_vertex_count) {
        break;
      }
      
      if (CanRemoveVertex(*result, info)) {
        std::vector<int> boundary_vertices = CollectBoundaryVertices(*result, info.index);
        
        RemoveVertex(*result, info.index);
        // Update boundary vertex indices after removal
        for (int& v : boundary_vertices) {
          if (v > info.index) {
            v--;
          }
        }
        
        if (boundary_vertices.size() >= 3) {
          RetriangulateHole(*result, boundary_vertices);
        }
        
        removed_any = true;
        break;
      }
    }
    
    if (!removed_any) {
      break;
    }
  }
  
  return result;
}

std::shared_ptr<SimplificationMesh> VertexDecimation::SimplifyByFactor(
    const SimplificationMesh& original_mesh, 
    float reduction_factor) {
  int target_count = static_cast<int>(
      original_mesh.vertices.size() * reduction_factor);
  return Simplify(original_mesh, target_count);
}

bool VertexDecimation::IsFeatureVertex(const SimplificationMesh& mesh, 
                                        int vertex_index) const {
  // For this vertex, check if any pair of adjacent faces have a dihedral angle > feature_angle_
  // 1. For each face adjacent to vertex_index, get its face normal
  // 2. For each pair of adjacent faces that share an edge at the vertex,
  //    compute the dihedral angle between their normals.
  // 3. If the angle > feature_angle_, classify as a feature vertex.

  // Find all faces adjacent to this vertex
  std::vector<int> adjacent_faces;
  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& face = mesh.faces[i];
    if (face.x == vertex_index || face.y == vertex_index || face.z == vertex_index) {
      adjacent_faces.push_back(static_cast<int>(i));
    }
  }

  if (adjacent_faces.size() < 2) {
    return false;
  }

  // Compute normals for all adjacent faces
  std::vector<glm::vec3> normals(adjacent_faces.size());
  for (size_t i = 0; i < adjacent_faces.size(); ++i) {
    const auto& face = mesh.faces[adjacent_faces[i]];
    const glm::vec3& v0 = mesh.vertices[face.x];
    const glm::vec3& v1 = mesh.vertices[face.y];
    const glm::vec3& v2 = mesh.vertices[face.z];
    normals[i] = glm::normalize(glm::cross(v1 - v0, v2 - v0));
  }

  // Check dihedral angle between each pair of adjacent faces
  float cos_thresh = std::cos(glm::radians(feature_angle_));
  for (size_t i = 0; i < normals.size(); ++i) {
    for (size_t j = i + 1; j < normals.size(); ++j) {
      float cosine = glm::dot(normals[i], normals[j]);
      // Clamp for numerical stability
      cosine = std::max(-1.0f, std::min(1.0f, cosine));
      if (cosine < cos_thresh) {
        return true;
      }
    }
  }
  return false;
}

bool VertexDecimation::IsBoundaryVertex(const SimplificationMesh& mesh, 
                                         int vertex_index) const {
  
  std::map<std::pair<int, int>, int> edge_face_count;
  
  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& face = mesh.faces[i];
    
    // Check each edge of the face
    auto check_edge = [&](int v1, int v2) {
      if (v1 == vertex_index || v2 == vertex_index) {
        // Ensure consistent ordering (smaller index first)
        int e1 = std::min(v1, v2);
        int e2 = std::max(v1, v2);
        edge_face_count[{e1, e2}]++;
      }
    };
    
    check_edge(face.x, face.y);
    check_edge(face.y, face.z);
    check_edge(face.z, face.x);
  }
  
  for (const auto& pair : edge_face_count) {
    if (pair.second == 1) {
      return true;
    }
  }
  
  return false;
}

float VertexDecimation::ComputeDistanceError(const SimplificationMesh& mesh, 
                                              int vertex_index) const {
  // 1. Find all adjacent faces
  // 2. Compute average plane
  // 3. Compute distance from vertex to plane
  std::vector<int> adjacent_faces;
  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& face = mesh.faces[i];
    if (face.x == vertex_index || face.y == vertex_index || face.z == vertex_index) {
      adjacent_faces.push_back(static_cast<int>(i));
    }
  }

  if (adjacent_faces.empty()) {
    return 0.0f;
  }

  // Compute average normal of adjacent faces
  glm::vec3 average_normal = glm::vec3(0.0f);
  for (size_t i = 0; i < adjacent_faces.size(); ++i) {
    const auto& face = mesh.faces[adjacent_faces[i]];
    const glm::vec3& v0 = mesh.vertices[face.x];
    const glm::vec3& v1 = mesh.vertices[face.y];
    const glm::vec3& v2 = mesh.vertices[face.z];
    average_normal += glm::normalize(glm::cross(v1 - v0, v2 - v0));
  }
  average_normal = glm::normalize(average_normal);

  // Find a point on the plane (use a neighbor vertex from the first adjacent face)
  // Get a vertex from the first face that's not the current vertex
  const auto& first_face = mesh.faces[adjacent_faces[0]];
  int point_on_plane_idx = first_face.x;
  if (point_on_plane_idx == vertex_index) {
    point_on_plane_idx = first_face.y;
    if (point_on_plane_idx == vertex_index) {
      point_on_plane_idx = first_face.z;
    }
  }
  const glm::vec3& point_on_plane = mesh.vertices[point_on_plane_idx];

  // Compute distance from vertex to plane
  // |dot(normal, (vertex - point_on_plane))|
  const glm::vec3& v = mesh.vertices[vertex_index];
  return std::abs(glm::dot(average_normal, v - point_on_plane));
}

float VertexDecimation::ComputeTriangleAspectRatio(
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) const {
  // Compute edge lengths
  float e0 = glm::length(v1 - v0);
  float e1 = glm::length(v2 - v1);
  float e2 = glm::length(v0 - v2);
  
  // Find longest and shortest edges
  float longest = std::max({e0, e1, e2});
  float shortest = std::min({e0, e1, e2});
  // Aspect ratio = longest / shortest
  if (shortest < 1e-6f) {
    return std::numeric_limits<float>::max();  // Degenerate triangle
  }
  return longest / shortest;
}

bool VertexDecimation::CheckResultingTrianglesAspectRatio(
    const SimplificationMesh& mesh, int vertex_index) const {
  // Find all neighbor vertices
  std::set<int> neighbor_vertices;
  
  for (const auto& face : mesh.faces) {
    if (face.x == vertex_index) {
      neighbor_vertices.insert(face.y);
      neighbor_vertices.insert(face.z);
    } else if (face.y == vertex_index) {
      neighbor_vertices.insert(face.x);
      neighbor_vertices.insert(face.z);
    } else if (face.z == vertex_index) {
      neighbor_vertices.insert(face.x);
      neighbor_vertices.insert(face.y);
    }
  }
  
  if (neighbor_vertices.size() < 3) {
    return false;
  }
  
  // Convert set to vector for easier iteration
  std::vector<int> neighbors(neighbor_vertices.begin(), neighbor_vertices.end());
  for (size_t i = 1; i + 1 < neighbors.size(); ++i) {
    const glm::vec3& v0 = mesh.vertices[neighbors[0]];
    const glm::vec3& v1 = mesh.vertices[neighbors[i]];
    const glm::vec3& v2 = mesh.vertices[neighbors[i + 1]];
    
    float aspect_ratio = ComputeTriangleAspectRatio(v0, v1, v2);
    if (aspect_ratio > aspect_ratio_) {
      return false;
    }
  }
  
  return true;
}

void VertexDecimation::ClassifyVertices(
    const SimplificationMesh& mesh, 
    std::vector<VertexInfo>& vertex_info) {
  vertex_info.resize(mesh.vertices.size());
  for (size_t i = 0; i < mesh.vertices.size(); i++) {
    vertex_info[i].index = i;
    vertex_info[i].is_feature_vertex = IsFeatureVertex(mesh, i);
    vertex_info[i].is_boundary_vertex = IsBoundaryVertex(mesh, i);
    vertex_info[i].distance_error = ComputeDistanceError(mesh, i);
  }
}

bool VertexDecimation::CanRemoveVertex(const SimplificationMesh& mesh, const VertexInfo& info) const {
  // Allow boundary vertices if distance error is very small
  if (info.is_boundary_vertex && info.distance_error > max_distance_ * 0.5f) {
    return false;
  }
  if (info.is_feature_vertex) {
    return false;
  }
  if (info.distance_error > max_distance_) {
    return false;
  }
  if (!CheckResultingTrianglesAspectRatio(mesh, info.index)) {
    return false;
  }
  
  return true;
}

std::vector<int> VertexDecimation::CollectBoundaryVertices(
    const SimplificationMesh& mesh, int vertex_index) const {
  std::set<int> neighbors;
  
  for (const auto& face : mesh.faces) {
    if (face.x == vertex_index || face.y == vertex_index || face.z == vertex_index) {
      if (face.x != vertex_index) neighbors.insert(face.x);
      if (face.y != vertex_index) neighbors.insert(face.y);
      if (face.z != vertex_index) neighbors.insert(face.z);
    }
  }
  
  return std::vector<int>(neighbors.begin(), neighbors.end());
}

void VertexDecimation::RemoveVertex(SimplificationMesh& mesh, int vertex_index) {
  // Remove all faces that contain this vertex (creates a hole)
  for (size_t i = 0; i < mesh.faces.size(); ) {
    const auto& face = mesh.faces[i];
    if (face.x == vertex_index || face.y == vertex_index || face.z == vertex_index) {
      mesh.faces.erase(mesh.faces.begin() + i);
    } else {
      i++;
    }
  }
  
  // Update all face indices that reference vertices after the removed one
  for (auto& face : mesh.faces) {
    if (face.x > vertex_index) face.x--;
    if (face.y > vertex_index) face.y--;
    if (face.z > vertex_index) face.z--;
  }
  
  // Remove the vertex itself
  mesh.vertices.erase(mesh.vertices.begin() + vertex_index);
  
  // Update normals if they exist
  if (!mesh.normals.empty() && mesh.normals.size() == mesh.vertices.size() + 1) {
    mesh.normals.erase(mesh.normals.begin() + vertex_index);
  }
}

void VertexDecimation::RetriangulateHole(
    SimplificationMesh& mesh, 
    const std::vector<int>& boundary_vertices) {
  // Simple fan triangulation: connect all boundary vertices to the first one
  if (boundary_vertices.size() < 3) {
    return;
  }
  
  // Remove duplicates
  std::vector<int> unique_vertices;
  for (int v : boundary_vertices) {
    if (unique_vertices.empty() || unique_vertices.back() != v) {
      unique_vertices.push_back(v);
    }
  }
  
  unsigned int anchor = static_cast<unsigned int>(unique_vertices[0]);
  for (size_t i = 1; i + 1 < unique_vertices.size(); ++i) {
    glm::uvec3 new_face(
        anchor,
        static_cast<unsigned int>(unique_vertices[i]),
        static_cast<unsigned int>(unique_vertices[i + 1])
    );
    mesh.faces.push_back(new_face);
  }
}

}  // namespace GLOO

