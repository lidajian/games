#ifndef CONTROL_SOURCE_HPP
#define CONTROL_SOURCE_HPP

#include <queue>     // queue
#include <termios.h> // termios, tcsetattr, tcgetattr
#include <thread>    // thread

#include "board.hpp"
#include "lockable.hpp"

class blocking_queue: public lockable {
    public:
        blocking_queue() {}

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
        std::queue<char> q;
};

class control_source {
    public:
        virtual char get(board* brd) const = 0;
        virtual ~control_source() {};
};

class unix_keyboard_control_source: public virtual control_source {
    public:
        unix_keyboard_control_source() {
            tcgetattr(0, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON);
        }

        char get(board* brd) const {
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
        static void listen(blocking_queue* q, const control_source* source, board* brd) {
            char c;
            while (c = source->get(brd)) {
                q->put(c);
            }
        }

        const control_source* source;
        board* brd;
        std::thread* thrd;
};

# endif
