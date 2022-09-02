#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

#include "engine/common/macros.h"

namespace ShaderStory {

class InputSystem final {
 public:
  InputSystem() = default;
  ~InputSystem() = default;

 private:
  DISALLOW_COPY_ASSIGN_AND_MOVE(InputSystem);
};

}  // namespace ShaderStory

#endif