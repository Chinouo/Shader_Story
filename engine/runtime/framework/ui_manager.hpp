#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP
#include "engine/component/base_component.hpp"

namespace ShaderStory {

class UIManager final {
 public:
  UIManager() = default;
  ~UIManager() = default;
  void Initialize();

  void AddUIComponent(ReflectUIComponent*);
  void RemoveComponent(ReflectUIComponent*);
  void RecordUIComponentDrawCommand();

 private:
  std::vector<ReflectUIComponent*> m_registered_components;
};

}  // namespace ShaderStory

#endif