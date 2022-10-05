#ifndef BASE_COMPONENT_HPP
#define BASE_COMPONENT_HPP

#include "engine/runtime/global/global.hpp"
#include "engine/runtime/io/input_system.hpp"
#include "engine/runtime/window/window_system.hpp"
#include "third_party/imgui/imgui.h"

namespace ShaderStory {

class ReflectUIComponent {
 public:
  virtual void OnDrawUI() = 0;
  virtual ~ReflectUIComponent() = default;
};

class GameCommandReceiver {
 public:
  GameCommandReceiver();

  virtual void onReceiveGameCommand(u_int32_t command);

  virtual ~GameCommandReceiver();

 private:
};
}  // namespace ShaderStory

#endif