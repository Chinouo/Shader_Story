#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <cstdlib>
namespace ShaderStory {

typedef struct {
} EngineConfig;

// current only support mouse pointer and move event.
typedef struct {
  // microseconds
  size_t timestamp;
  double x;
  double y;
} EnginePointerEvent;

}  // namespace ShaderStory

#endif