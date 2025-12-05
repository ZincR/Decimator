#ifndef DECIMATOR_APP_H_
#define DECIMATOR_APP_H_

#include "gloo/Application.hpp"

namespace GLOO {
class DecimatorApp : public Application {
 public:
  DecimatorApp(const std::string& app_name, glm::ivec2 window_size);
  void SetupScene() override;
};
}  // namespace GLOO

#endif

