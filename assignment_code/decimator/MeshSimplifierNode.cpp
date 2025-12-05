#include "MeshSimplifierNode.hpp"
#include "MeshIO.hpp"

#include "gloo/MeshLoader.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"

// ImGui
#include "imgui.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <set>

namespace GLOO {
MeshSimplifierNode::MeshSimplifierNode() {
  // Constructor - initialize simplification algorithms
  edge_collapse_ = make_unique<EdgeCollapse>();
  vertex_decimation_ = make_unique<VertexDecimation>();
  vertex_clustering_ = make_unique<VertexClustering>();
  
  // Initialize selection system
  selection_ = make_unique<MeshSelection>();
  
  GetTransform().SetPosition(glm::vec3(0.f, 0.f, 0.f));
  GetTransform().SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);
  GetTransform().SetScale(glm::vec3(1.0f));
  
  // Load default mesh
  LoadMesh("decimator/sample.obj");
}

void MeshSimplifierNode::LoadMesh(const std::string& path) {
  // Load mesh using MeshIO
  original_mesh_ = MeshIO::LoadMesh(path);
  
  if (!original_mesh_ || original_mesh_->IsEmpty()) {
    std::cerr << "Failed to load mesh: " << path << std::endl;
    return;
  }
  
  // Set mesh for selection system
  selection_->SetMesh(original_mesh_);
  
  // Initialize simplified meshes to nullptr
  for (int i = 0; i < 3; i++) {
    simplified_meshes_[i] = nullptr;
  }
  
  UpdateMeshDisplay();
}

void MeshSimplifierNode::SimplifyWithCurrentMethod() {
  if (!original_mesh_) {
    return;
  }
  
  int method_idx = static_cast<int>(current_method_);
  
  // Apply the selected simplification method
  switch (current_method_) {
    case SimplificationMethod::EDGE_COLLAPSE:
      simplified_meshes_[method_idx] = 
          edge_collapse_->SimplifyByFactor(*original_mesh_, target_reduction_);
      break;
    case SimplificationMethod::VERTEX_DECIMATION:
      simplified_meshes_[method_idx] = 
          vertex_decimation_->SimplifyByFactor(*original_mesh_, target_reduction_);
      break;
    case SimplificationMethod::VERTEX_CLUSTERING:
      simplified_meshes_[method_idx] = 
          vertex_clustering_->SimplifyByFactor(*original_mesh_, target_reduction_);
      break;
  }
  
  UpdateMeshDisplay();
}

void MeshSimplifierNode::SimplifyAllMethods() {
  if (!original_mesh_) {
    return;
  }
  
  simplified_meshes_[0] = 
      edge_collapse_->SimplifyByFactor(*original_mesh_, target_reduction_);
  simplified_meshes_[1] = 
      vertex_decimation_->SimplifyByFactor(*original_mesh_, target_reduction_);
  simplified_meshes_[2] = 
      vertex_clustering_->SimplifyByFactor(*original_mesh_, target_reduction_);
  
  UpdateMeshDisplay();
}

void MeshSimplifierNode::UpdateMeshDisplay() {
  auto current_mesh = GetCurrentDisplayMesh();
  
  if (!current_mesh || current_mesh->IsEmpty()) {
    return;
  }
  
  // Create a VertexObject with lines for edges (wireframe)
  auto vertex_obj = std::make_shared<VertexObject>();
  
  // Extract all edges from faces and create line list
  auto positions = make_unique<PositionArray>();
  std::set<std::pair<int, int>> edges;
  
  // Build unique edge list
  for (const auto& face : current_mesh->faces) {
    auto add_edge = [&](int v1, int v2) {
      if (v1 > v2) std::swap(v1, v2);
      edges.insert({v1, v2});
    };
    add_edge(face.x, face.y);
    add_edge(face.y, face.z);
    add_edge(face.z, face.x);
  }
  
  // Create line list (2 vertices per edge)
  auto indices = make_unique<IndexArray>();
  for (const auto& edge : edges) {
    unsigned int idx = positions->size();
    positions->push_back(current_mesh->vertices[edge.first]);
    positions->push_back(current_mesh->vertices[edge.second]);
    indices->push_back(idx);
    indices->push_back(idx + 1);
  }
  
  if (positions->empty()) {
    return;
  }
  
  vertex_obj->UpdatePositions(std::move(positions));
  vertex_obj->UpdateIndices(std::move(indices));
  
  // Update rendering component with GL_LINES
  auto* render_comp = GetComponentPtr<RenderingComponent>();
  if (render_comp == nullptr) {
    CreateComponent<RenderingComponent>(vertex_obj);
    render_comp = GetComponentPtr<RenderingComponent>();
  } else {
    render_comp->SetVertexObject(vertex_obj);
  }
  
  if (render_comp) {
    render_comp->SetDrawMode(DrawMode::Lines);
  }
  
  // Use simple shader (no normals needed)
  auto* shading_comp = GetComponentPtr<ShadingComponent>();
  if (shading_comp == nullptr) {
    auto simple_shader = std::make_shared<SimpleShader>();
    CreateComponent<ShadingComponent>(simple_shader);
  }
  
  // Add green material for wireframe visibility
  auto* material_comp = GetComponentPtr<MaterialComponent>();
  if (material_comp == nullptr) {
    auto material = std::make_shared<Material>();
    material->SetDiffuseColor(glm::vec3(0.0f, 1.0f, 0.0f));  // Green wireframe
    CreateComponent<MaterialComponent>(material);
  }
}

void MeshSimplifierNode::SyncFromTransform() {
  if (synced_) return;
  const auto& T = GetTransform();
  ui_pos_ = T.GetPosition();
  ui_deg_ = glm::vec3(0.0f);
  ui_scale_ = T.GetScale();
  synced_ = true;
}

void MeshSimplifierNode::Update(double delta_time) {
  // Handle keyboard input for method toggles
  HandleKeyInput();
  
  // Handle mouse selection if enabled
  if (enable_selection_) {
    HandleMouseSelection();
  }
  
  // Render ImGui controls
  RenderUI();
}

void MeshSimplifierNode::HandleKeyInput() {
  // auto& input = InputManager::GetInstance();  // Not needed in current implementation
  
  // Method selection: 1-3
  if (IsKeyJustPressed('1')) {
    current_method_ = SimplificationMethod::EDGE_COLLAPSE;
    UpdateMeshDisplay();
  }
  if (IsKeyJustPressed('2')) {
    current_method_ = SimplificationMethod::VERTEX_DECIMATION;
    UpdateMeshDisplay();
  }
  if (IsKeyJustPressed('3')) {
    current_method_ = SimplificationMethod::VERTEX_CLUSTERING;
    UpdateMeshDisplay();
  }
  
  // Display toggles
  if (IsKeyJustPressed('O')) {
    show_original_ = !show_original_;
    UpdateMeshDisplay();
  }
  
  // Actions
  if (IsKeyJustPressed('S')) {
    SaveCurrentMesh();
  }
  if (IsKeyJustPressed('L')) {
    LoadMeshFromFile();
  }
  if (IsKeyJustPressed('R')) {
    SimplifyWithCurrentMethod();
  }
  if (IsKeyJustPressed('A')) {
    SimplifyAllMethods();
  }
}

bool MeshSimplifierNode::IsKeyJustPressed(char key) {
  auto& input = InputManager::GetInstance();
  bool is_pressed = input.IsKeyPressed(key);
  unsigned char key_index = static_cast<unsigned char>(key);
  bool was_pressed = key_pressed_[key_index];
  key_pressed_[key_index] = is_pressed;
  return is_pressed && !was_pressed;
}

void MeshSimplifierNode::RenderUI() {
  SyncFromTransform();

  if (ImGui::Begin("Mesh Decimator Controls")) {
    // Transform controls
    ImGui::TextUnformatted("Transform");
    if (ImGui::DragFloat3("Position", &ui_pos_.x, 0.1f, -100.f, 100.f)) {
      GetTransform().SetPosition(ui_pos_);
    }

    ImGui::DragFloat3("Rotation", &ui_deg_.x, 0.1f, -360.f, 360.f);
    {
      glm::vec3 r = glm::radians(ui_deg_);
      glm::mat4 R = glm::yawPitchRoll(r.y, r.x, r.z);
      glm::quat q = glm::quat_cast(R);
      GetTransform().SetRotation(q);
    }

    if (ImGui::DragFloat3("Scale", &ui_scale_.x, 0.1f, 0.001f, 100.f)) {
      GetTransform().SetScale(ui_scale_);
    }

    if (ImGui::Button("Reset Transform")) {
      ui_pos_ = glm::vec3(0.0f);
      ui_deg_ = glm::vec3(0.0f);
      ui_scale_ = glm::vec3(1.0f);
      GetTransform().SetPosition(ui_pos_);
      GetTransform().SetScale(ui_scale_);
      GetTransform().SetRotation(glm::quat(1, 0, 0, 0));
    }

    ImGui::Separator();
    RenderMethodControls();
    
    ImGui::Separator();
    RenderDisplayControls();
    
    ImGui::Separator();
    RenderSelectionControls();
    
    ImGui::Separator();
    RenderFileControls();
  }
  ImGui::End();
}

void MeshSimplifierNode::RenderMethodControls() {
  ImGui::TextUnformatted("Simplification Method");
  
  const char* method_names[] = {
    "Edge Collapse (Garland-Heckbert)",
    "Vertex Decimation (Schroeder-Zarge-Lorensen)",
    "Vertex Clustering (Rossignac-Borrel)"
  };
  
  int current = static_cast<int>(current_method_);
  if (ImGui::Combo("Method", &current, method_names, 3)) {
    current_method_ = static_cast<SimplificationMethod>(current);
    UpdateMeshDisplay();
  }
  
  ImGui::SliderFloat("Target Reduction", &target_reduction_, 0.01f, 0.99f);
  ImGui::Text("Keep %.1f%% of vertices", target_reduction_ * 100.0f);
  
  if (ImGui::Button("Simplify Current Method (R)")) {
    SimplifyWithCurrentMethod();
  }
  
  ImGui::SameLine();
  if (ImGui::Button("Simplify All Methods (A)")) {
    SimplifyAllMethods();
  }
  
  // Show mesh stats
  if (original_mesh_) {
    ImGui::Text("Original: %zu vertices, %zu faces",
                original_mesh_->GetVertexCount(),
                original_mesh_->GetFaceCount());
  }
  
  int method_idx = static_cast<int>(current_method_);
  if (simplified_meshes_[method_idx]) {
    ImGui::Text("Simplified: %zu vertices, %zu faces",
                simplified_meshes_[method_idx]->GetVertexCount(),
                simplified_meshes_[method_idx]->GetFaceCount());
  }
}

void MeshSimplifierNode::RenderDisplayControls() {
  ImGui::TextUnformatted("Display Options");
  
  if (ImGui::Checkbox("Show Original (O)", &show_original_)) {
    UpdateMeshDisplay();
  }
}

void MeshSimplifierNode::RenderSelectionControls() {
  ImGui::TextUnformatted("Selection");
  
  ImGui::Checkbox("Enable Selection", &enable_selection_);
  
  if (enable_selection_) {
    if (ImGui::Button("Delete Selected")) {
      DeleteSelectedElements();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Simplify Region")) {
      SimplifySelectedRegion();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear Selection")) {
      selection_->ClearSelection();
    }
  }
}

void MeshSimplifierNode::RenderFileControls() {
  ImGui::TextUnformatted("File Operations");
  
  if (ImGui::Button("Load Mesh (L)")) {
    LoadMeshFromFile();
  }
  
  ImGui::SameLine();
  if (ImGui::Button("Save Mesh (S)")) {
    SaveCurrentMesh();
  }
}

std::shared_ptr<SimplificationMesh> MeshSimplifierNode::GetCurrentDisplayMesh() const {
  if (show_original_ || !original_mesh_) {
    return original_mesh_;
  }
  
  int method_idx = static_cast<int>(current_method_);
  if (simplified_meshes_[method_idx]) {
    return simplified_meshes_[method_idx];
  }
  
  return original_mesh_;
}

const char* MeshSimplifierNode::GetMethodName(SimplificationMethod method) const {
  switch (method) {
    case SimplificationMethod::EDGE_COLLAPSE:
      return "Edge Collapse (Garland-Heckbert)";
    case SimplificationMethod::VERTEX_DECIMATION:
      return "Vertex Decimation (Schroeder-Zarge-Lorensen)";
    case SimplificationMethod::VERTEX_CLUSTERING:
      return "Vertex Clustering (Rossignac-Borrel)";
    default:
      return "Unknown";
  }
}

void MeshSimplifierNode::SaveCurrentMesh() {
  auto mesh = GetCurrentDisplayMesh();
  if (!mesh) {
    return;
  }
  
  std::string filename = "output_" + std::string(GetMethodName(current_method_)) + ".obj";
  std::replace(filename.begin(), filename.end(), ' ', '_');
  std::replace(filename.begin(), filename.end(), '(', '_');
  std::replace(filename.begin(), filename.end(), ')', '_');
  
  MeshIO::SaveOBJ(filename, *mesh);
}

void MeshSimplifierNode::LoadMeshFromFile() {
  // TODO: Add file dialog or prompt for filename
  LoadMesh("decimator/sample.obj");
}

void MeshSimplifierNode::HandleMouseSelection() {
  // TODO: Implement mouse-based selection using ray casting
}

void MeshSimplifierNode::DeleteSelectedElements() {
  // TODO: Delete selected vertices/edges
}

void MeshSimplifierNode::SimplifySelectedRegion() {
  // TODO: Apply simplification to selected region only
}

}  // namespace GLOO

