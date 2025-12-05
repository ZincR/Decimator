#ifndef EDGE_COLLAPSE_H_
#define EDGE_COLLAPSE_H_

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "SimplificationMesh.hpp"

namespace GLOO {

// Edge Collapse using Garland-Heckbert algorithm
// Reference: "Surface Simplification Using Quadric Error Metrics" [GH97]
class EdgeCollapse {
 public:
  EdgeCollapse();
  ~EdgeCollapse();

  // Simplify mesh to target vertex count
  std::shared_ptr<SimplificationMesh> Simplify(const SimplificationMesh& original_mesh, 
                                      int target_vertex_count);

  // Simplify mesh by reduction factor (0.0 to 1.0)
  std::shared_ptr<SimplificationMesh> SimplifyByFactor(const SimplificationMesh& original_mesh, 
                                               float reduction_factor);

 private:
  // TODO: Implement quadric error metric computation
  // TODO: Implement edge collapse operations
  // TODO: Implement priority queue for edge selection
  
  struct QuadricMatrix {
    // 4x4 symmetric matrix for quadric error metric
    float data[10];  // Only store 10 values due to symmetry
    
    QuadricMatrix();
    void AddPlane(const glm::vec3& normal, float d);
    float ComputeError(const glm::vec3& v) const;
    QuadricMatrix operator+(const QuadricMatrix& other) const;
  };
  
  struct Edge {
    int v1, v2;  // Vertex indices
    float error;  // Collapse error
    glm::vec3 optimal_pos;  // Optimal position after collapse
    
    bool operator<(const Edge& other) const {
      return error > other.error;  // Min heap
    }
  };

  // Helper methods (to be implemented)
  void ComputeQuadrics(const SimplificationMesh& mesh, 
                       std::vector<QuadricMatrix>& quadrics);
  void BuildEdgeList(const SimplificationMesh& mesh, std::vector<Edge>& edges);
  float ComputeEdgeCollapseError(const Edge& edge, 
                                  const std::vector<QuadricMatrix>& quadrics);
  glm::vec3 ComputeOptimalPosition(const Edge& edge, 
                                     const std::vector<QuadricMatrix>& quadrics);
  void CollapseEdge(SimplificationMesh& mesh, const Edge& edge);
};

}  // namespace GLOO

#endif

