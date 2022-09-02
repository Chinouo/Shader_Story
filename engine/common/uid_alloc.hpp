#ifndef UID_ALLOC_HPP
#define UID_ALLOC_HPP
#include <unordered_map>

#include "engine/common/macros.h"
namespace ShaderStory {

/// Allocate a id for resources.
/// Notice: map id to element may perform copy, the behavior is handled by user.
template <class T>
class UIDAllocator {
 public:
  static constexpr size_t invaildUID = 0;

  UIDAllocator();

  ~UIDAllocator();

  size_t AllocUID(const T&);

  bool HasElement(size_t uid) const;
  bool HasElement(const T&) const;

  bool RemoveElement(const T&);
  bool RemoveElement(size_t uid);

  void Clear();

 private:
  // map data to id.
  std::unordered_map<T, size_t> m_elem_id_map;
  // map id to data.
  std::unordered_map<size_t, T> m_id_elem_map;

  DISALLOW_COPY_ASSIGN_AND_MOVE(UIDAllocator);
};

}  // namespace ShaderStory

#endif