#include "DecimatorApp.hpp"
#include "MeshSimplifierNode.hpp"

#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/lights/AmbientLight.hpp"

namespace GLOO {
DecimatorApp::DecimatorApp(const std::string& app_name,
                             glm::ivec2 window_size)
    : Application(app_name, window_size) {
}

void DecimatorApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  // Setup camera
  auto camera_node = make_unique<ArcBallCameraNode>();
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  // Add ambient light (Renderer requires at least one light)
  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.3f));
  root.CreateComponent<LightComponent>(ambient_light);

  // Add mesh simplifier node
  root.AddChild(make_unique<MeshSimplifierNode>());
}
}  // namespace GLOO

