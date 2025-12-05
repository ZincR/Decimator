#include "WireframeRenderer.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/gl_wrapper/BindGuard.hpp"
#include <unordered_set>

namespace GLOO {

WireframeRenderer::WireframeRenderer() {
  // Constructor - set up rendering components
}

WireframeRenderer::~WireframeRenderer() {
  // Destructor
}

void WireframeRenderer::SetMesh(std::shared_ptr<SimplificationMesh> mesh) {
  mesh_ = mesh;
  
  // Update rendering based on current mode
  UpdateVertexPoints();
  UpdateEdgeLines();
  UpdateSolidMesh();
}

void WireframeRenderer::UpdateVertexPoints() {
  // TODO: Create vertex object for rendering vertices as points
  if (!mesh_ || mesh_->vertices.empty()) {
    return;
  }
  
  vertex_points_ = std::make_shared<VertexObject>();
  
  // Create position buffer for vertices
  auto positions = make_unique<PositionArray>();
  for (const auto& v : mesh_->vertices) {
    positions->push_back(v);
  }
  vertex_points_->UpdatePositions(std::move(positions));
  
  // TODO: Add color buffer for vertex highlighting
}

void WireframeRenderer::UpdateEdgeLines() {
  // TODO: Create vertex object for rendering edges as lines
  if (!mesh_ || mesh_->faces.empty()) {
    return;
  }
  
  edge_lines_ = std::make_shared<VertexObject>();
  
  // Extract unique edges from faces
  std::vector<glm::vec3> edge_vertices;
  CreateEdgeList(*mesh_, edge_vertices);
  
  // Create position buffer
  auto positions = make_unique<PositionArray>();
  for (const auto& v : edge_vertices) {
    positions->push_back(v);
  }
  edge_lines_->UpdatePositions(std::move(positions));
}

void WireframeRenderer::UpdateSolidMesh() {
  // TODO: Create vertex object for solid mesh rendering
  if (!mesh_) {
    return;
  }
  
  solid_mesh_ = mesh_->ToVertexObject();
}

void WireframeRenderer::CreateEdgeList(const SimplificationMesh& mesh,
                                        std::vector<glm::vec3>& edge_vertices) {
  // TODO: Extract edges from faces
  // Use a set to avoid duplicates
  struct EdgeHash {
    size_t operator()(const std::pair<int, int>& e) const {
      return std::hash<int>()(e.first) ^ (std::hash<int>()(e.second) << 1);
    }
  };
  
  std::unordered_set<std::pair<int, int>, EdgeHash> edges;
  
  for (const auto& face : mesh.faces) {
    // Add three edges of the triangle
    auto add_edge = [&](int v1, int v2) {
      if (v1 > v2) std::swap(v1, v2);
      edges.insert({v1, v2});
    };
    
    add_edge(face.x, face.y);
    add_edge(face.y, face.z);
    add_edge(face.z, face.x);
  }
  
  // Convert edges to line segments (each edge = 2 vertices)
  edge_vertices.reserve(edges.size() * 2);
  for (const auto& edge : edges) {
    edge_vertices.push_back(mesh.vertices[edge.first]);
    edge_vertices.push_back(mesh.vertices[edge.second]);
  }
}

void WireframeRenderer::SetSelectedVertices(const std::vector<int>& indices) {
  selected_vertices_ = indices;
  // TODO: Update vertex colors to highlight selection
}

void WireframeRenderer::SetSelectedEdges(
    const std::vector<std::pair<int, int>>& edges) {
  selected_edges_ = edges;
  // TODO: Update edge colors to highlight selection
}

}  // namespace GLOO

