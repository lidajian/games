#ifndef LOCKABLE_HPP
#define LOCKABLE_HPP

#define ATOMIC_RUN(op) lock(); op unlock();

#include <atomic>    // atomic_flag

class lockable {
    public:
        lockable(): _lock(ATOMIC_FLAG_INIT) {}

        void lock() {
            while (_lock.test_and_set(std::memory_order_acquire));
        }

        void unlock() {
            _lock.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag _lock;
};

#endif
