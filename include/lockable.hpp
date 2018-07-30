#ifndef LOCKABLE_HPP
#define LOCKABLE_HPP

#define ATOMIC_RUN(op) lock(); op unlock();

#include <atomic>    // atomic_bool

class lockable {
    public:
        lockable() {
            std::atomic_init(&_mutex, false);
        }

        void lock() {
            static bool _locked = false;
            while (!_mutex.compare_exchange_weak(_locked, true, std::memory_order_relaxed, std::memory_order_relaxed));
        }

        void unlock() {
            static bool _unlocked = true;
            while (!_mutex.compare_exchange_weak(_unlocked, false, std::memory_order_relaxed, std::memory_order_relaxed));
        }
    private:
        std::atomic_bool _mutex;
};

#endif
