#ifndef DIRECTIONAL_LIGHT_H_
#define DIRECTIONAL_LIGHT_H_

#include "gloo/lights/LightBase.hpp"
#include <glm/vec3.hpp>

namespace GLOO {
class DirectionalLight : public LightBase {
 public:
  explicit DirectionalLight(const glm::vec3& dir = glm::vec3(0.0f, -1.0f, 0.0f))
      : dir_(dir) {}

  LightType GetType() const override { return LightType::Directional; }

  const glm::vec3& GetDirection() const { return dir_; }
  void SetDirection(const glm::vec3& dir) { dir_ = dir; }

 private:
  glm::vec3 dir_;
};
}  // namespace GLOO

#endif



