#ifndef MESH_SIMPLIFIER_NODE_H_
#define MESH_SIMPLIFIER_NODE_H_

#include "gloo/SceneNode.hpp"
#include "simplification/SimplificationMesh.hpp"
#include "simplification/EdgeCollapse.hpp"
#include "simplification/VertexDecimation.hpp"
#include "simplification/VertexClustering.hpp"
#include "MeshSelection.hpp"
#include "WireframeRenderer.hpp"
#include <memory>

namespace GLOO {
class MeshSimplifierNode : public SceneNode {
 public:
  MeshSimplifierNode();
  void Update(double delta_time) override;

  // Simplification methods
  enum class SimplificationMethod {
    EDGE_COLLAPSE,        // Garland-Heckbert
    VERTEX_DECIMATION,    // Schroeder-Zarge-Lorensen
    VERTEX_CLUSTERING     // Rossignac-Borrel
  };

 private:
  // Mesh data
  std::shared_ptr<SimplificationMesh> original_mesh_;
  std::shared_ptr<SimplificationMesh> simplified_meshes_[3];  // One for each method
  SimplificationMethod current_method_ = SimplificationMethod::EDGE_COLLAPSE;
  
  // Simplification algorithms
  std::unique_ptr<EdgeCollapse> edge_collapse_;
  std::unique_ptr<VertexDecimation> vertex_decimation_;
  std::unique_ptr<VertexClustering> vertex_clustering_;
  
  // Selection and rendering
  std::unique_ptr<MeshSelection> selection_;
  std::shared_ptr<WireframeRenderer> renderer_;
  
  // Display settings
  bool show_original_ = true;
  bool enable_selection_ = false;
  float target_reduction_ = 0.5f;
  int grid_resolution_ = 10;  // For Vertex Clustering method
  
  // UI state
  glm::vec3 ui_pos_ = glm::vec3(0.0f);
  glm::vec3 ui_deg_ = glm::vec3(0.0f);
  glm::vec3 ui_scale_ = glm::vec3(1.0f);
  bool synced_ = false;
  
  // Keyboard state tracking
  bool key_pressed_[256] = {false};
  
  // Methods
  void LoadMesh(const std::string& path);
  void SimplifyWithCurrentMethod();
  void SimplifyAllMethods();
  void UpdateMeshDisplay();
  void SyncFromTransform();
  
  // Key handling for toggles
  void HandleKeyInput();
  bool IsKeyJustPressed(char key);
  
  // UI methods
  void RenderUI();
  void RenderMethodControls();
  void RenderDisplayControls();
  void RenderSelectionControls();
  void RenderFileControls();
  
  // File operations
  void SaveCurrentMesh();
  void LoadMeshFromFile();
  
  // Selection operations
  void HandleMouseSelection();
  void DeleteSelectedElements();
  void SimplifySelectedRegion();
  
  // Get current mesh based on display mode
  std::shared_ptr<SimplificationMesh> GetCurrentDisplayMesh() const;
  
  // Method names for UI
  const char* GetMethodName(SimplificationMethod method) const;
};
}  // namespace GLOO

#endif

