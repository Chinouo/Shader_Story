#ifndef VK_MACROS_H
#define VK_MACROS_H
namespace ShaderStory {

// VK_SUCCESS IS 0.
#define VK_CHECK(RESULT, MESG)      \
  if (RESULT) {                     \
    throw std::runtime_error(MESG); \
  }

}  // namespace ShaderStory

#endif