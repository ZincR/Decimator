#ifndef VERTEX_DECIMATION_H_
#define VERTEX_DECIMATION_H_

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "SimplificationMesh.hpp"

namespace GLOO {

// Vertex Decimation using Schroeder-Zarge-Lorensen algorithm
// Reference: "Decimation of Triangle Meshes" [SZL92]
class VertexDecimation {
 public:
  VertexDecimation();
  ~VertexDecimation();

  // Simplify mesh to target vertex count
  std::shared_ptr<SimplificationMesh> Simplify(const SimplificationMesh& original_mesh, 
                                      int target_vertex_count);

  // Simplify mesh by reduction factor (0.0 to 1.0)
  std::shared_ptr<SimplificationMesh> SimplifyByFactor(const SimplificationMesh& original_mesh, 
                                               float reduction_factor);

  // Set decimation criteria parameters
  void SetFeatureAngle(float angle) { feature_angle_ = angle; }
  void SetAspectRatio(float ratio) { aspect_ratio_ = ratio; }
  void SetMaxDistance(float dist) { max_distance_ = dist; }

 private:
  // TODO: Implement vertex classification
  // TODO: Implement vertex removal and retriangulation
  
  float feature_angle_ = 60.0f;   // Feature angle threshold (degrees)
  float aspect_ratio_ = 10.0f;    // Maximum aspect ratio for triangles
  float max_distance_ = 0.1f;     // Maximum distance from vertex to average plane

  struct VertexInfo {
    int index;
    bool is_feature_vertex;
    bool is_boundary_vertex;
    float distance_error;  // Distance to average plane of neighbors
    std::vector<int> neighbor_vertices;
    std::vector<int> adjacent_faces;
  };

  // Helper methods (to be implemented)
  bool IsFeatureVertex(const SimplificationMesh& mesh, int vertex_index) const;
  bool IsBoundaryVertex(const SimplificationMesh& mesh, int vertex_index) const;
  float ComputeDistanceError(const SimplificationMesh& mesh, int vertex_index) const;
  void ClassifyVertices(const SimplificationMesh& mesh, 
                        std::vector<VertexInfo>& vertex_info);
  bool CanRemoveVertex(const VertexInfo& info) const;
  void RemoveVertex(SimplificationMesh& mesh, int vertex_index);
  void RetriangulateHole(SimplificationMesh& mesh, 
                         const std::vector<int>& boundary_vertices);
};

}  // namespace GLOO

#endif

