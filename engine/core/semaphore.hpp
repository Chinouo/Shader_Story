#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <mutex>

#include "engine/common/macros.h"

namespace ShaderStory {

class Semaphore {
 public:
  Semaphore(int count);
  ~Semaphore();

  void Wait();
  void Signal();

  // reture true is resources available.
  bool TryWait();

 private:
  int count_;
  std::mutex mtx_;
  std::condition_variable cv_;

  DISALLOW_COPY_ASSIGN_AND_MOVE(Semaphore);
};

};  // namespace ShaderStory

#endif