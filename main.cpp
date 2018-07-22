#include <chrono>   // seconds
#include <cstdlib>  // atoi, system
#include <iostream> // cout
#include <memory>   // unique_ptr
#include <thread>   // this_thread

#include "control_source.hpp"
#include "game_board.hpp"

class countdown {
    public:
        countdown(int init_val, std::string prefix, std::string suffix): init_val(init_val), prefix(prefix), suffix(suffix) {}

        inline void reset() {
            current_val = init_val;
        }

        bool print() {
            if (current_val <= 0) {
                return false;
            }
            system("clear");
            std::cout << prefix << current_val << suffix;
            current_val--;
            return true;
        }
    private:
        int current_val;
        const int init_val;
        const std::string prefix;
        const std::string suffix;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <M> <N>\n";
        return 1;
    }
    int M = atoi(argv[1]), N = atoi(argv[2]);
    if (M <= 3 || N <= 3) {
        std::cout << "Invalid board\n";
        return 1;
    }
    char c;
    game_board brd(M, N);
    std::unique_ptr<control_source> source(new unix_keyboard_control_source());
    control_source_runner runner(source.get(), &brd);
    runner.run();
    blocking_queue& q = runner;
    countdown cdn(3, std::string('\n', M / 2) + std::string("Starting in "), std::string(" seconds\n"));
    cdn.reset();
    while(cdn.print()) {
        std::this_thread::sleep_for (std::chrono::seconds(1));
    }
    do {
        brd.clear();
        brd.init();
        do {
            brd.print();
            std::this_thread::sleep_for (std::chrono::milliseconds(brd.get_refresh_rate()));
            if (q.pop(c)) {
                switch(c) {
                    case 'w':
                        [[fallthrough]]
                    case 'i':
                        brd.set_direction(UP);
                        break;
                    case 's':
                        [[fallthrough]]
                    case 'k':
                        brd.set_direction(DOWN);
                        break;
                    case 'a':
                        [[fallthrough]]
                    case 'j':
                        brd.set_direction(LEFT);
                        break;
                    case 'd':
                        [[fallthrough]]
                    case 'l':
                        brd.set_direction(RIGHT);
                        break;
                    case ' ':
                        brd.reset_refresh_rate();
                        break;
                    default:
                        brd.slower();
                        break;
                }
            }
        } while (brd.next());
        cdn.reset();
        while(cdn.print()) {
            std::this_thread::sleep_for (std::chrono::seconds(1));
        }
    } while(1);

    return 0;
}
