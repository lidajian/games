#ifndef CONTROL_SOURCE_HPP
#define CONTROL_SOURCE_HPP

#include <condition_variable> // condition_variable
#include <mutex>     // mutex, unique_lock
#include <queue>     // queue
#include <termios.h> // termios, tcsetattr, tcgetattr
#include <thread>    // thread, this_thread

#include <boost/asio.hpp>

#include "board.hpp"
#include "lockable.hpp"

using boost::asio::ip::tcp;

char CHAR_CONT = '\x16';
char CHAR_EXIT = '\0';

template<bool Is_Lock_Free = true>
class blocking_queue: public lockable {
    public:
        blocking_queue() {}

        template<bool enabled = Is_Lock_Free>
        void put(typename std::enable_if<enabled, char>::type c) {
            ATOMIC_RUN(
                    q.push(c);
                    )
        }

        template<bool enabled = Is_Lock_Free>
        void put(typename std::enable_if<!enabled, char>::type c) {
            std::unique_lock<std::mutex> lck(mtx);
            while (!q.empty()) {
                lck.unlock();
                std::this_thread::yield();
                lck.lock();
            }
            q.push(c);
            cv.notify_one();
        }

        template<bool enabled = Is_Lock_Free>
        typename std::enable_if<enabled, char>::type get() {
            ATOMIC_RUN(
                    char c = q.front();
                    q.pop();
                    )
            return c;
        }

        template<bool enabled = Is_Lock_Free>
        typename std::enable_if<!enabled, char>::type get() {
            std::unique_lock<std::mutex> lck(mtx);
            if (q.empty()) {
                cv.wait(lck);
            }
            char c = q.front();
            q.pop();
            return c;
        }

        template<bool enabled = Is_Lock_Free>
        typename std::enable_if<enabled, bool>::type has_next() {
            return !q.empty();
        }

        template<bool enabled = Is_Lock_Free>
        typename std::enable_if<enabled>::type clear() {
            ATOMIC_RUN(
                    std::queue<char> empty;
                    empty.swap(q);
                    )
        }

        template<bool enabled = Is_Lock_Free>
        typename std::enable_if<!enabled>::type clear() {
            std::queue<char> empty;
            std::unique_lock<std::mutex> lck(mtx);
            empty.swap(q);
        }
    private:
        std::queue<char> q;
        std::condition_variable cv;
        std::mutex mtx;
};

template <typename Board_T>
class control_source {
    public:
        virtual char get(board<Board_T>* brd) = 0;
        virtual ~control_source() {};
};

template <typename Board_T>
class unix_keyboard_control_source: public virtual control_source<Board_T> {
    public:
        unix_keyboard_control_source() {
            tcgetattr(0, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON);
        }

        char get(board<Board_T>* brd) {
            tcsetattr(0, TCSANOW, &newt);
            return getchar();
        }

        ~unix_keyboard_control_source() {
            tcsetattr(0, TCSANOW, &oldt);
        }
    private:
        struct termios oldt, newt;
};

template <typename Board_T>
class remote_control_source: public virtual control_source<Board_T> {
    public:
        remote_control_source(const char* host, const char* port): io_context(), s(io_context) {
            tcp::resolver resolver(io_context);
            boost::asio::connect(s, resolver.resolve(host, port));
        }

        ~remote_control_source() {
            s.close();
        }

        virtual bool send(char c) = 0;
    protected:
        boost::asio::io_context io_context;
        tcp::socket s;
};

template <typename Board_T, bool Is_Lock_Free>
class control_source_runner: public blocking_queue<Is_Lock_Free> {
    public:
        control_source_runner(control_source<Board_T>* source, board<Board_T>* brd): source(source), thrd(nullptr), brd(brd) {}

        void run() {
            thrd = new std::thread(listen, this, source, brd);
        }

        ~control_source_runner() {
            if (thrd != nullptr) {
                delete thrd;
            }
        }
    private:
        static void listen(blocking_queue<Is_Lock_Free>* q, control_source<Board_T>* source, board<Board_T>* brd) {
            char c;
            try {
                while (c = source->get(brd)) {
                    if (c == CHAR_EXIT) {
                        break;
                    } else if (c == CHAR_CONT) {
                        continue;
                    }
                    q->put(c);
                }
            } catch (std::exception& e) {
                return;
            }
            q->put(' ');
        }

        control_source<Board_T>* source;
        board<Board_T>* brd;
        std::thread* thrd;
};

# endif
