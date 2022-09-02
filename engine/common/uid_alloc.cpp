#include "engine/common/uid_alloc.hpp"
namespace ShaderStory {

template <class T>
UIDAllocator<T>::UIDAllocator() {}

template <class T>
UIDAllocator<T>::~UIDAllocator() {}

template <class T>
size_t UIDAllocator<T>::AllocUID(const T& ele) {
  const auto it = m_elem_id_map.find(ele);
  if (it != m_elem_id_map.cend()) {
    return it->second;
  } else {
    for (size_t i = 0; i < m_id_elem_map.size() + 1; i++) {
      size_t uid = i + 1;
      if (m_id_elem_map.find(uid) == m_id_elem_map.cend()) {
        m_elem_id_map.insert(std::make_pair(ele, uid));
        m_id_elem_map.insert(std::make_pair(uid, ele));
        return uid;
      }
    }
  }
  // when container is full.
  return invaildUID;
}

template <class T>
bool UIDAllocator<T>::HasElement(size_t uid) const {
  return m_id_elem_map.find(uid) == m_id_elem_map.cend();
}

template <class T>
bool UIDAllocator<T>::HasElement(const T& ele) const {
  return m_elem_id_map.find(ele) == m_elem_id_map.cend();
}

template <class T>
bool UIDAllocator<T>::RemoveElement(const T& ele) {
  auto it = m_elem_id_map.find(ele);
  if (it != m_elem_id_map.cend()) {
    auto uid = it->second;
    m_elem_id_map.erase(ele);
    m_id_elem_map.erase(uid);
    return true;
  }
  return false;
}

template <class T>
bool UIDAllocator<T>::RemoveElement(size_t uid) {
  auto it = m_id_elem_map.find(uid);
  if (it != m_id_elem_map.end()) {
    const auto& ele = it->second;
    m_elem_id_map.erase(ele);
    m_id_elem_map.erase(uid);
    return true;
  }
  return false;
}

template <class T>
void UIDAllocator<T>::Clear() {
  m_elem_id_map.clear();
  m_id_elem_map.clear();
}

}  // namespace ShaderStory