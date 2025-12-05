#include "MeshSelection.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace GLOO {

MeshSelection::MeshSelection() {
  // Constructor
}

MeshSelection::~MeshSelection() {
  // Destructor
}

void MeshSelection::SetMesh(std::shared_ptr<SimplificationMesh> mesh) {
  mesh_ = mesh;
  ClearSelection();
}

IntersectionResult MeshSelection::Intersect(const Ray& ray, float max_distance) {
  // TODO: Implement ray-mesh intersection
  IntersectionResult result;
  
  if (!mesh_ || mesh_->faces.empty()) {
    return result;
  }
  
  float closest_distance = max_distance;
  
  // Test ray against all triangles
  for (size_t i = 0; i < mesh_->faces.size(); i++) {
    const auto& face = mesh_->faces[i];
    const glm::vec3& v0 = mesh_->vertices[face.x];
    const glm::vec3& v1 = mesh_->vertices[face.y];
    const glm::vec3& v2 = mesh_->vertices[face.z];
    
    float t;
    glm::vec3 barycentric;
    
    if (RayTriangleIntersect(ray, v0, v1, v2, t, barycentric)) {
      if (t < closest_distance) {
        closest_distance = t;
        result.hit = true;
        result.distance = t;
        result.face_index = i;
        result.hit_point = ray.PointAt(t);
        result.barycentric = barycentric;
        
        // Find closest vertex
        float d0 = glm::length(result.hit_point - v0);
        float d1 = glm::length(result.hit_point - v1);
        float d2 = glm::length(result.hit_point - v2);
        
        if (d0 <= d1 && d0 <= d2) {
          result.vertex_index = face.x;
        } else if (d1 <= d2) {
          result.vertex_index = face.y;
        } else {
          result.vertex_index = face.z;
        }
      }
    }
  }
  
  return result;
}

bool MeshSelection::RayTriangleIntersect(const Ray& ray,
                                          const glm::vec3& v0,
                                          const glm::vec3& v1,
                                          const glm::vec3& v2,
                                          float& t,
                                          glm::vec3& barycentric) {
  // TODO: Implement Möller-Trumbore ray-triangle intersection
  const float EPSILON = 0.0000001f;
  
  glm::vec3 edge1 = v1 - v0;
  glm::vec3 edge2 = v2 - v0;
  glm::vec3 h = glm::cross(ray.direction, edge2);
  float a = glm::dot(edge1, h);
  
  if (a > -EPSILON && a < EPSILON) {
    return false;  // Ray is parallel to triangle
  }
  
  float f = 1.0f / a;
  glm::vec3 s = ray.origin - v0;
  float u = f * glm::dot(s, h);
  
  if (u < 0.0f || u > 1.0f) {
    return false;
  }
  
  glm::vec3 q = glm::cross(s, edge1);
  float v = f * glm::dot(ray.direction, q);
  
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }
  
  t = f * glm::dot(edge2, q);
  
  if (t > EPSILON) {
    barycentric = glm::vec3(1.0f - u - v, u, v);
    return true;
  }
  
  return false;
}

void MeshSelection::SelectVertex(const Ray& ray, float threshold) {
  auto result = Intersect(ray);
  
  if (result.hit) {
    if (!additive_selection_) {
      selected_vertices_.clear();
    }
    selected_vertices_.insert(result.vertex_index);
  }
}

void MeshSelection::SelectEdge(const Ray& ray, float threshold) {
  auto result = Intersect(ray);
  
  if (result.hit && result.face_index >= 0) {
    const auto& face = mesh_->faces[result.face_index];
    
    // Find closest edge of the intersected face
    MeshEdge edges[3] = {
      MeshEdge(face.x, face.y),
      MeshEdge(face.y, face.z),
      MeshEdge(face.z, face.x)
    };
    
    float min_dist = std::numeric_limits<float>::max();
    int closest_edge = -1;
    
    for (int i = 0; i < 3; i++) {
      const glm::vec3& v1 = mesh_->vertices[edges[i].v1];
      const glm::vec3& v2 = mesh_->vertices[edges[i].v2];
      float dist = PointToLineSegmentDistance(result.hit_point, v1, v2);
      
      if (dist < min_dist) {
        min_dist = dist;
        closest_edge = i;
      }
    }
    
    if (closest_edge >= 0 && min_dist < threshold) {
      if (!additive_selection_) {
        selected_edges_.clear();
      }
      selected_edges_.insert(edges[closest_edge]);
    }
  }
}

void MeshSelection::SelectRegion(const Ray& ray, float radius) {
  auto result = Intersect(ray);
  
  if (result.hit) {
    if (!additive_selection_) {
      selected_vertices_.clear();
    }
    
    // Select all vertices within radius of hit point
    for (size_t i = 0; i < mesh_->vertices.size(); i++) {
      float dist = glm::length(mesh_->vertices[i] - result.hit_point);
      if (dist <= radius) {
        selected_vertices_.insert(i);
      }
    }
  }
}

void MeshSelection::ClearSelection() {
  selected_vertices_.clear();
  selected_edges_.clear();
}

int MeshSelection::FindClosestVertex(const glm::vec3& point, float max_distance) {
  // TODO: Find closest vertex to point
  int closest = -1;
  float min_dist = max_distance;
  
  for (size_t i = 0; i < mesh_->vertices.size(); i++) {
    float dist = glm::length(mesh_->vertices[i] - point);
    if (dist < min_dist) {
      min_dist = dist;
      closest = i;
    }
  }
  
  return closest;
}

float MeshSelection::PointToLineSegmentDistance(const glm::vec3& point,
                                                  const glm::vec3& line_start,
                                                  const glm::vec3& line_end) {
  // TODO: Compute distance from point to line segment
  glm::vec3 line_vec = line_end - line_start;
  float line_length_sq = glm::dot(line_vec, line_vec);
  
  if (line_length_sq < 1e-6f) {
    return glm::length(point - line_start);
  }
  
  float t = glm::clamp(glm::dot(point - line_start, line_vec) / line_length_sq, 
                       0.0f, 1.0f);
  glm::vec3 projection = line_start + t * line_vec;
  
  return glm::length(point - projection);
}

void MeshSelection::DeleteSelectedVertices() {
  // TODO: Remove selected vertices and adjacent faces
  // This is a destructive operation on the mesh
}

void MeshSelection::DeleteSelectedEdges() {
  // TODO: Collapse selected edges
}

}  // namespace GLOO

