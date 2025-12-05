#ifndef WIREFRAME_RENDERER_H_
#define WIREFRAME_RENDERER_H_

#include <memory>
#include <glm/glm.hpp>
#include "gloo/VertexObject.hpp"
#include "gloo/SceneNode.hpp"
#include "simplification/SimplificationMesh.hpp"

namespace GLOO {

// Renderer for displaying meshes as wireframes with visible vertices
class WireframeRenderer : public SceneNode {
 public:
  WireframeRenderer();
  ~WireframeRenderer();
  
  // Set the mesh to render
  void SetMesh(std::shared_ptr<SimplificationMesh> mesh);
  
  // Rendering modes
  enum class RenderMode {
    WIREFRAME,          // Only edges
    VERTICES,           // Only vertices as dots
    WIREFRAME_VERTICES, // Both edges and vertices
    SOLID,              // Standard solid mesh
    SOLID_WIREFRAME     // Solid with wireframe overlay
  };
  
  void SetRenderMode(RenderMode mode) { render_mode_ = mode; }
  RenderMode GetRenderMode() const { return render_mode_; }
  
  // Vertex/edge display settings
  void SetVertexSize(float size) { vertex_size_ = size; }
  void SetEdgeWidth(float width) { edge_width_ = width; }
  void SetVertexColor(const glm::vec3& color) { vertex_color_ = color; }
  void SetEdgeColor(const glm::vec3& color) { edge_color_ = color; }
  
  // Highlight selected elements
  void SetSelectedVertices(const std::vector<int>& indices);
  void SetSelectedEdges(const std::vector<std::pair<int, int>>& edges);
  void SetSelectionColor(const glm::vec3& color) { selection_color_ = color; }

 private:
  std::shared_ptr<SimplificationMesh> mesh_;
  RenderMode render_mode_ = RenderMode::WIREFRAME_VERTICES;
  
  // Display settings
  float vertex_size_ = 5.0f;
  float edge_width_ = 1.0f;
  glm::vec3 vertex_color_ = glm::vec3(1.0f, 0.0f, 0.0f);  // Red dots
  glm::vec3 edge_color_ = glm::vec3(0.0f, 0.0f, 0.0f);    // Black lines
  glm::vec3 selection_color_ = glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow
  
  // Selection highlighting
  std::vector<int> selected_vertices_;
  std::vector<std::pair<int, int>> selected_edges_;
  
  // Rendering components
  std::shared_ptr<VertexObject> vertex_points_;
  std::shared_ptr<VertexObject> edge_lines_;
  std::shared_ptr<VertexObject> solid_mesh_;
  
  // Helper methods
  void UpdateVertexPoints();
  void UpdateEdgeLines();
  void UpdateSolidMesh();
  void CreateEdgeList(const SimplificationMesh& mesh, 
                      std::vector<glm::vec3>& edge_vertices);
};

}  // namespace GLOO

#endif

