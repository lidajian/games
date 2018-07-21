#ifndef CONTROL_SOURCE_HPP
#define CONTROL_SOURCE_HPP

#define ATOMIC_RUN(op) lock(); op unlock();

#include <atomic>    // atomic_bool
#include <queue>     // queue
#include <termios.h> // termios, tcsetattr, tcgetattr
#include <thread>    // thread

#include "game_board.hpp"

class blocking_queue {
    public:
        blocking_queue() {
            std::atomic_init(&_mutex, false);
        }

        void put(char c) {
            ATOMIC_RUN(
                    q.push(c);
                    )
        }

        char get() {
            ATOMIC_RUN(
                    char c = q.front();
                    q.pop();
                    )
            return c;
        }

        bool has_next() {
            return !q.empty();
        }

        void clear() {
            ATOMIC_RUN(
                    std::queue<char> empty;
                    empty.swap(q);
                    )
        }
    private:
        void lock() {
            static bool _locked = false;
            while (!_mutex.compare_exchange_weak(_locked, true, std::memory_order_relaxed, std::memory_order_relaxed));
        }

        void unlock() {
            static bool _unlocked = true;
            while (!_mutex.compare_exchange_weak(_unlocked, false, std::memory_order_relaxed, std::memory_order_relaxed));
        }

        std::atomic_bool _mutex;
        std::queue<char> q;
};

class control_source {
    public:
        virtual char get() const = 0;
        virtual ~control_source() {};
};

class unix_keyboard_control_source: public virtual control_source {
    public:
        unix_keyboard_control_source() {
            tcgetattr(0, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON);
        }

        char get() const {
            tcsetattr(0, TCSANOW, &newt);
            return getchar();
        }

        ~unix_keyboard_control_source() {
            tcsetattr(0, TCSANOW, &oldt);
        }
    private:
        struct termios oldt, newt;
};

class control_source_runner: public blocking_queue {
    public:
        control_source_runner(control_source* source, board* brd): source(source), thrd(nullptr), brd(brd) {}

        void run() {
            thrd = new std::thread(listen, this, source, brd);
        }

        ~control_source_runner() {
            if (thrd != nullptr) {
                delete thrd;
            }
        }
    private:
        static void listen(blocking_queue* q, const control_source* source, const board* brd) {
            char c;
            while (c = source->get()) {
                q->put(c);
            }
        }

        const control_source* source;
        const board* brd;
        std::thread* thrd;
};

# endif
