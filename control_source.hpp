#ifndef CONTROL_SOURCE_HPP
#define CONTROL_SOURCE_HPP

#include <termios.h> // termios, tcsetattr, tcgetattr
#include <thread>    // thread

#include <boost/lockfree/queue.hpp> // queue

#include "game_board.hpp"

using blocking_queue = boost::lockfree::queue<char>;

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
        control_source_runner(control_source* source, board* brd): source(source), thrd(nullptr), brd(brd), blocking_queue(5) {}

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
                q->push(c);
            }
        }

        const control_source* source;
        const board* brd;
        std::thread* thrd;
};

# endif
