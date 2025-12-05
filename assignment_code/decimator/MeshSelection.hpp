#ifndef MESH_SELECTION_H_
#define MESH_SELECTION_H_

#include <vector>
#include <set>
#include <unordered_set>
#include <glm/glm.hpp>
#include "simplification/SimplificationMesh.hpp"

namespace GLOO {

// Ray structure for picking
struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;
  
  Ray(const glm::vec3& o, const glm::vec3& d) 
      : origin(o), direction(glm::normalize(d)) {}
  
  glm::vec3 PointAt(float t) const { return origin + t * direction; }
};

// Intersection result
struct IntersectionResult {
  bool hit;
  float distance;
  int face_index;
  int vertex_index;  // Closest vertex to hit point
  glm::vec3 hit_point;
  glm::vec3 barycentric;  // Barycentric coordinates
  
  IntersectionResult() : hit(false), distance(FLT_MAX), 
                          face_index(-1), vertex_index(-1) {}
};

// Selection manager for mesh vertices and edges
class MeshSelection {
 public:
  MeshSelection();
  ~MeshSelection();
  
  // Set the mesh to operate on
  void SetMesh(std::shared_ptr<SimplificationMesh> mesh);
  
  // Ray-mesh intersection for picking
  IntersectionResult Intersect(const Ray& ray, float max_distance = FLT_MAX);
  
  // Select vertex closest to ray intersection
  void SelectVertex(const Ray& ray, float threshold = 0.1f);
  
  // Select edge closest to ray intersection
  void SelectEdge(const Ray& ray, float threshold = 0.1f);
  
  // Select region (all vertices within radius of picked point)
  void SelectRegion(const Ray& ray, float radius);
  
  // Clear selection
  void ClearSelection();
  
  // Toggle selection mode
  void SetSelectionMode(bool additive) { additive_selection_ = additive; }
  
  // Get selected vertices
  const std::set<int>& GetSelectedVertices() const { 
    return selected_vertices_; 
  }
  
  // Get selected edges
  const std::unordered_set<MeshEdge, MeshEdge::Hash>& GetSelectedEdges() const { 
    return selected_edges_; 
  }
  
  // Delete selected vertices
  void DeleteSelectedVertices();
  
  // Delete selected edges (collapse them)
  void DeleteSelectedEdges();

 private:
  std::shared_ptr<SimplificationMesh> mesh_;
  std::set<int> selected_vertices_;
  std::unordered_set<MeshEdge, MeshEdge::Hash> selected_edges_;
  bool additive_selection_ = false;
  
  // Ray-triangle intersection (Möller-Trumbore algorithm)
  bool RayTriangleIntersect(const Ray& ray, 
                             const glm::vec3& v0, 
                             const glm::vec3& v1, 
                             const glm::vec3& v2,
                             float& t, 
                             glm::vec3& barycentric);
  
  // Find closest vertex to a point
  int FindClosestVertex(const glm::vec3& point, float max_distance);
  
  // Find closest edge to a point
  MeshEdge FindClosestEdge(const glm::vec3& point, float max_distance);
  
  // Point-line segment distance
  float PointToLineSegmentDistance(const glm::vec3& point,
                                    const glm::vec3& line_start,
                                    const glm::vec3& line_end);
};

}  // namespace GLOO

#endif

