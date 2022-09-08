#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

#include <algorithm>
#include <vector>

#include "engine/common/macros.h"

namespace ShaderStory {

constexpr u_int32_t k_complement_control_command = 0xFFFFFFFF;

enum class GameCommand : u_int32_t {
  forward = 1 << 0,   // W
  backward = 1 << 1,  // S
  left = 1 << 2,      // A
  right = 1 << 3,     // D
};

typedef struct {
  double x;
  double y;
} CursorPosState;

class GameCommandReceiver;

/// @brief  Actually, i thought named it GameCommand Genratetor would be better.
class InputSystem final {
 public:
  InputSystem() = default;
  ~InputSystem() = default;

  void Initialize();
  void Dispose();

  // current not implement.
  void AddGameCommandReceiver(GameCommandReceiver*);
  void RemoverGameCommandReceiver(GameCommandReceiver*);

  void onKeyEvent(int, int, int, int);
  void onCursorPos(double, double);

  u_int32_t GetCurrentGamenCommandState() const { return m_command_state; };
  CursorPosState GetCursorPosState() const { return m_cursor_position_state; };
  // void Tick();

 private:
  DISALLOW_COPY_ASSIGN_AND_MOVE(InputSystem);

  // TODO: use hash and vector can O(1) remove and add.
  std::vector<GameCommandReceiver*> m_receivers;

  u_int32_t m_command_state;

  CursorPosState m_cursor_position_state{0.0, 0.0};
};

}  // namespace ShaderStory

#endif