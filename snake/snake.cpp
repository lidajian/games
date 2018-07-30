#include <chrono>   // seconds
#include <cstdlib>  // atoi, system, srand, rand
#include <ctime>    // time
#include <iostream> // cout
#include <memory>   // unique_ptr
#include <thread>   // this_thread

#include "board.hpp"
#include "countdown.hpp"
#include "control_source.hpp"

class game_board: public board {
    public:
        game_board(int M, int N): board(M, N), refresh_rate(DEFAULT_REFRESH_RATE) {}

        bool next() {
            switch(direct) {
                case UP:
                    head_i--;
                    break;
                case DOWN:
                    head_i++;
                    break;
                case LEFT:
                    head_j--;
                    break;
                case RIGHT:
                    head_j++;
                    break;
            }
            if (head_i < 0 || head_i == M || head_j < 0 || head_j == N) {
                return false;
            }
            if (at(head_i, head_j) > 1) {
                return false;
            }
            int& n = at(head_i, head_j);
            if (n < 0) {
                length++;
                next_food_run = rand() % MAX_ADD_FOOD_INTERVAL + 1;
                has_food = false;
            } else {
                ATOMIC_RUN(
                        for (int i = 0; i < MN; i++) {
                            if (brd[i] > 0) {
                                brd[i]--;
                            }
                        }
                        )
            }
            n = length;
            if (!has_food) {
                next_food_run--;
                if (next_food_run < 0) {
                    int food_i = rand() % M;
                    int food_j = rand() % N;
                    ATOMIC_RUN(
                            if (at(food_i, food_j) == 0) {
                                at(food_i, food_j) = -1;
                                has_food = true;
                            } else {
                                next_food_run = 0;
                            }
                            )
                }
            }
            return true;
        }

        void init() {
            int i = M / 2;
            int j = N / 2;
            head_i = i;
            head_j = j;
            direct = UP;
            length = INIT_SNAKE_LENGTH;
            srand(time(NULL));
            has_food = false;
            next_food_run = rand() % MAX_ADD_FOOD_INTERVAL + 1;
            for (int n = length; n > 0; n--) {
                at(i, j) = n;
                n--;
                i++;
            }
        }

        void print() {
            system("clear");
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    int& n = at(i, j);
                    if (n > 0) {
                        std::cout << "\u2B1B";
                    } else if (n == 0) {
                        std::cout << "\u2B1C";
                    } else {
                        std::cout << "\u2B1B";
                    }
                }
                std::cout << '\n';
            }
            std::cout << "Length: " << length << std::endl;
        }

        inline void slower() {
            refresh_rate += int((MAX_REFRESH_RATE - refresh_rate) * get_gradient() * 3);
        }

        inline void faster() {
            refresh_rate -= int((refresh_rate - MIN_REFRESH_RATE) * get_gradient());
        }

        inline void reset_refresh_rate() {
            refresh_rate = DEFAULT_REFRESH_RATE;
        }

        void set_direction(direction direct) {
            if (this->direct == direct) {
                faster();
            } else if (this->direct + direct != 3) {
                this->direct = direct;
            } else {
                slower();
            }
        }

        inline int get_refresh_rate() {
            return refresh_rate;
        }
    private:
        constexpr float get_gradient() {
            return (float)MAX_DELTA_REFRESH_RATE / (MAX_REFRESH_RATE - MIN_REFRESH_RATE);
        }

        constexpr static int INIT_SNAKE_LENGTH = 2;
        constexpr static int MAX_ADD_FOOD_INTERVAL = 10;
        constexpr static int DEFAULT_REFRESH_RATE = 100;
        constexpr static int MIN_REFRESH_RATE = 30;
        constexpr static int MAX_REFRESH_RATE = 250;
        constexpr static int MAX_DELTA_REFRESH_RATE = 10;
        int refresh_rate;
        int head_i;
        int head_j;
        int length;
        direction direct;
        int next_food_run;
        bool has_food;
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
        q.clear();
        do {
            brd.print();
            std::this_thread::sleep_for (std::chrono::milliseconds(brd.get_refresh_rate()));
            if (q.has_next()) {
                switch(q.get()) {
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
