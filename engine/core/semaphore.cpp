#include "engine/core/semaphore.hpp"

namespace ShaderStory {

Semaphore::Semaphore(int count) : count_(count){};

Semaphore::~Semaphore(){};

void Semaphore::Wait() {
  std::unique_lock<std::mutex> lock(mtx_);
  --count_;
  cv_.wait(lock, [this]() { return count_ >= 0; });
};

void Semaphore::Signal() {
  std::unique_lock<std::mutex> lock(mtx_);
  ++count_;
  if (count_ >= 0) {
    cv_.notify_all();
  }
};

// reture true is resources available.
bool Semaphore::TryWait() {
  if (count_ < 0) return false;
  Signal();
  return true;
};

}  // namespace ShaderStory