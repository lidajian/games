#include <algorithm> // inverse
#include <chrono>    // milliseconds
#include <cstdlib>   // system
#include <cstring>   // memcpy
#include <iostream>  // cout
#include <memory>    // unique_ptr
#include <vector>    // vector
#include <thread>    // this_thread

#include "board.hpp"
#include "common.hpp"
#include "control_source.hpp"
#include "direction.hpp"

constexpr int M = 17;
constexpr int N = 25;
constexpr char CHAR_NONE = ' ';
constexpr char CHAR_EMPTY = 'O';
constexpr char CHAR_PLAYER[] = {'@', '*'};
constexpr char INIT_BOARD[M * N + 1] =
"            *            "
"           * *           "
"          * * *          "
"         * * * *         "
"O O O O O O O O O O O O O"
" O O O O O O O O O O O O "
"  O O O O O O O O O O O  "
"   O O O O O O O O O O   "
"    O O O O O O O O O    "
"   O O O O O O O O O O   "
"  O O O O O O O O O O O  "
" O O O O O O O O O O O O "
"O O O O O O O O O O O O O"
"         @ @ @ @         "
"          @ @ @          "
"           @ @           "
"            @            ";

enum status_type {
    PLAYER_WIN, PLAYER_CHANGE, CONTROL_CONT
};

class game_board: public board<char> {
    public:
        game_board(bool need_swap, int my_player = 0): board(::M, ::N), need_swap(need_swap), my_player(my_player) {}

        // next status of board
        status_type next(char c) {
            if (c == 'r') {
                if (selected) {
                    replay();
                }
            } else if (c == ' ') {
                if (selected) {
                    selected = false;
                    if (!trace.empty()) {
                        if (need_swap) {
                            if (check_win_from_top()) {
                                return PLAYER_WIN;
                            }
                        } else {
                            if (my_player == current_player) {
                                if (check_win_from_top()) {
                                    return PLAYER_WIN;
                                }
                            } else {
                                if (check_win_from_bottom()) {
                                    return PLAYER_WIN;
                                }
                            }
                        }
                        current_player = (current_player + 1) % NUM_PLAYER;
                        if (need_swap) {
                            std::reverse(brd, brd + MN);
                            find_from_bottom();
                        } else {
                            if (my_player == current_player) {
                                find_from_bottom();
                            } else {
                                find_from_top();
                            }
                        }
                        trace.clear();
                        return PLAYER_CHANGE;
                    }
                } else {
                    selected = true;
                }
            } else {
                direction_6::Enum direct;
                switch(c) {
                    case 'a':
                        [[fallthrough]]
                    case 'h':
                        direct = direction_6::LEFT;
                        break;
                    case 'w':
                        [[fallthrough]]
                    case 'u':
                        direct = direction_6::LEFT_UP;
                        break;
                    case 'e':
                        [[fallthrough]]
                    case 'i':
                        direct = direction_6::RIGHT_UP;
                        break;
                    case 'd':
                        [[fallthrough]]
                    case 'k':
                        direct = direction_6::RIGHT;
                        break;
                    case 'x':
                        [[fallthrough]]
                    case 'm':
                        direct = direction_6::RIGHT_DOWN;
                        break;
                    case 'z':
                        [[fallthrough]]
                    case 'n':
                        direct = direction_6::LEFT_DOWN;
                        break;
                    default:
                        return CONTROL_CONT;
                }
                if (selected) {
                    if (trace.size() == 1 &&
                            last_direction == direct &&
                            move_type == SINGLE_STEP &&
                            has_optional()) {
                        move_type = HOP;
                        move_to(optional_i, optional_j);
                        no_optional();
                    } else {
                        int stored_optional_i = optional_i;
                        int stored_optional_j = optional_j;
                        if (move_selected(direct)) {
                            last_direction = direct;
                        } else {
                            optional_i = stored_optional_i;
                            optional_j = stored_optional_j;
                        }
                    }
                } else {
                    move_unselected(direct);
                }
            }
            return CONTROL_CONT;
        }

        // reset board to initial status
        void init() {
            memcpy(brd, INIT_BOARD, M * N * sizeof(char));
            current_player = 0;
            if (my_player == 0) {
                find_from_bottom();
            } else {
                std::reverse(brd, brd + MN);
                find_from_top();
            }
            selected = false;
            trace.clear();
        }

        // true if trace is empty
        bool trace_empty() {
            return trace.empty();
        }

        // current player
        int get_current_player() {
            return current_player;
        }

        // print board
        void print() {
            system("clear");
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    char& c = at(i, j);
                    if (c == CHAR_NONE) {
                        std::cout << " ";
                    } else if (c == CHAR_EMPTY) {
                        std::cout << "\e[37m\u2b24\e[0m";
                    } else if (c == CHAR_PLAYER[0]) {
                        if (i == current_i && j == current_j) {
                            std::cout << "\e[91m\u2b24\e[0m";
                        } else {
                            std::cout << "\e[31m\u2b24\e[0m";
                        }
                    } else if (c == CHAR_PLAYER[1]) {
                        if (i == current_i && j == current_j) {
                            std::cout << "\e[92m\u2b24\e[0m";
                        } else {
                            std::cout << "\e[32m\u2b24\e[0m";
                        }
                    }
                }
                std::cout << std::endl;
            }
        }

    private:
        // set no optional location
        inline void no_optional() { optional_i = -1; }

        // return true if there is optional location
        inline bool has_optional() { return optional_i >= 0; }

        // replay moves
        void replay() {
            int stored_current_i = current_i;
            int stored_current_j = current_j;
            char char_player = CHAR_PLAYER[current_player];
            at(current_i, current_j) = CHAR_EMPTY;
            for (const std::pair<int, int>& p: trace) {
                current_i = p.first;
                current_j = p.second;
                at(current_i, current_j) = char_player;
                print();
                std::cout << "Replaying...\n";
                std::this_thread::sleep_for (std::chrono::milliseconds(500));
                at(current_i, current_j) = CHAR_EMPTY;
            }
            current_i = stored_current_i;
            current_j = stored_current_j;
            at(current_i, current_j) = char_player;
        }

        // move current location to (i, j)
        void move_to(int i, int j) {
            std::swap(at(current_i, current_j), at(i, j));
            current_i = i;
            current_j = j;
        }

        // base template class to provide movement and check for different direction
        template <direction_6::Enum Direction>
            class iterator {
                public:
                    void set(int v1_i, int v1_j, int v2_i, int v2_j) {
                        this->v1_i = v1_i;
                        this->v1_j = v1_j;
                        this->v2_i = v2_i;
                        this->v2_j = v2_j;
                    }

                    int get_i() { return v1_i; }

                    int get_j() { return v1_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT, int>::type = 0>
                        int get_init_end_i() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT, int>::type = 0>
                        int get_init_end_j() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT, int>::type = 0>
                        bool has_next() { return v1_j >= v2_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT, int>::type = 0>
                        void next() { v1_j -= 2; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT, int>::type = 0>
                        void get_target() { v1_j = v2_j - (v1_j - v2_j); }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT, int>::type = 0>
                        bool target_out_of_bound() { return v1_j < 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT, int>::type = 0>
                        int get_init_end_i() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT, int>::type = 0>
                        int get_init_end_j() { return ::N - 1; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT, int>::type = 0>
                        bool has_next() { return v1_j <= v2_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT, int>::type = 0>
                        void next() { v1_j += 2; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT, int>::type = 0>
                        void get_target() { v1_j = v2_j + (v2_j - v1_j); }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT, int>::type = 0>
                        bool target_out_of_bound() { return v1_j >= ::N; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_UP, int>::type = 0>
                        int get_init_end_i() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_UP, int>::type = 0>
                        int get_init_end_j() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_UP, int>::type = 0>
                        bool has_next() { return v1_i >= v2_i && v1_j >= v2_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_UP, int>::type = 0>
                        void next() { v1_i--; v1_j--; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_UP, int>::type = 0>
                        void get_target() { v1_i = v2_i - (v1_i - v2_i); v1_j = v2_j - (v1_j - v2_j); }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_UP, int>::type = 0>
                        bool target_out_of_bound() { return v1_i < 0 || v1_j < 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_UP, int>::type = 0>
                        int get_init_end_i() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_UP, int>::type = 0>
                        int get_init_end_j() { return ::N - 1; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_UP, int>::type = 0>
                        bool has_next() { return v1_i >= v2_i && v1_j <= v2_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_UP, int>::type = 0>
                        void next() { v1_i--; v1_j++; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_UP, int>::type = 0>
                        void get_target() { v1_i = v2_i - (v1_i - v2_i); v1_j = v2_j + (v2_j - v1_j); }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_UP, int>::type = 0>
                        bool target_out_of_bound() { return v1_i < 0 || v1_j >= ::N; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_DOWN, int>::type = 0>
                        int get_init_end_i() { return ::M - 1; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_DOWN, int>::type = 0>
                        int get_init_end_j() { return 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_DOWN, int>::type = 0>
                        bool has_next() { return v1_i <= v2_i && v1_j >= v2_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_DOWN, int>::type = 0>
                        void next() { v1_i++; v1_j--; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_DOWN, int>::type = 0>
                        void get_target() { v1_i = v2_i + (v2_i - v1_i); v1_j = v2_j - (v1_j - v2_j); }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::LEFT_DOWN, int>::type = 0>
                        bool target_out_of_bound() { return v1_i >= ::M || v1_j < 0; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_DOWN, int>::type = 0>
                        int get_init_end_i() { return ::M - 1; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_DOWN, int>::type = 0>
                        int get_init_end_j() { return ::N - 1; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_DOWN, int>::type = 0>
                        bool has_next() { return v1_i <= v2_i && v1_j <= v2_j; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_DOWN, int>::type = 0>
                        void next() { v1_i++; v1_j++; }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_DOWN, int>::type = 0>
                        void get_target() { v1_i = v2_i + (v2_i - v1_i); v1_j = v2_j + (v2_j - v1_j); }

                    template <direction_6::Enum _Direction = Direction, typename std::enable_if<_Direction == direction_6::RIGHT_DOWN, int>::type = 0>
                        bool target_out_of_bound() { return v1_i >= ::M || v1_j >= ::N; }
                private:
                    int v1_i;
                    int v1_j;
                    int v2_i;
                    int v2_j;
            };

        // move one step towards direction
        template <direction_6::Enum Direction>
            bool move_by_one(int& current_i, int& current_j, int& planned_i, int& planned_j, iterator<Direction>& it) {
                it.set(current_i, current_j, it.get_init_end_i(), it.get_init_end_j());
                it.next();
                if (it.has_next()) {
                    planned_i = it.get_i();
                    planned_j = it.get_j();
                    return true;
                }
                return false;
            }

        // try hop towards direction, return true if the plan is in board
        template <direction_6::Enum Direction>
            bool try_hop(int& planned_i, int& planned_j, bool& is_one_step) {
                char char_in_board;
                bool found = false;
                bool blocked = false;
                int target_i;
                int target_j;
                int bridge_i;
                int bridge_j;
                iterator<Direction> it;
                it.set(current_i, current_j, it.get_init_end_i(), it.get_init_end_j());
                while (it.next(), it.has_next()) {
                    bridge_i = it.get_i();
                    bridge_j = it.get_j();
                    char_in_board = at(bridge_i, bridge_j);
                    if (char_in_board == CHAR_NONE) {
                        break;
                    } else if (char_in_board != CHAR_EMPTY) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    no_optional();
                    is_one_step = true;
                    return move_by_one<Direction>(current_i, current_j, planned_i, planned_j, it);
                }

                // assertion:
                // (bridge_i, bridge_j) closer to edge of board than (current_i, current_j)
                // (bridge_i, bridge_i) not at edge of board -> move_by_one returns true
                it.set(current_i, current_j, bridge_i, bridge_j);
                it.get_target();
                target_i = it.get_i();
                target_j = it.get_j();
                if (it.target_out_of_bound() || at(target_i, target_j) == CHAR_NONE) {
                    no_optional();
                    is_one_step = true;
                    return move_by_one<Direction>(current_i, current_j, planned_i, planned_j, it);
                }

                // assertion:
                // (target_i, target_j) closer to edge of board than (bridge_i, bridge_j)
                // (target_i, target_j) not out of board
                it.set(bridge_i, bridge_j, target_i, target_j);
                while (it.next(), it.has_next()) {
                    planned_i = it.get_i();
                    planned_j = it.get_j();
                    if (at(planned_i, planned_j) != CHAR_EMPTY) {
                        blocked = true;
                        break;
                    }
                }
                if (blocked) {
                    no_optional();
                    is_one_step = true;
                    return move_by_one<Direction>(current_i, current_j, planned_i, planned_j, it);
                }

                // assertion:
                // (bridge_i, bridge_j) to (target_i, target_j) not blocked
                planned_i = target_i;
                planned_j = target_j;
                return move_by_one<Direction>(current_i, current_j, optional_i, optional_j, it);
            }

        // move when selected
        bool move_selected(direction_6::Enum direct) {
            int planned_i;
            int planned_j;
            bool is_one_step = false;
            bool in_bound;
            switch(direct) {
                case direction_6::LEFT:
                    in_bound = try_hop<direction_6::LEFT>(planned_i, planned_j, is_one_step);
                    break;
                case direction_6::RIGHT:
                    in_bound = try_hop<direction_6::RIGHT>(planned_i, planned_j, is_one_step);
                    break;
                case direction_6::LEFT_UP:
                    in_bound = try_hop<direction_6::LEFT_UP>(planned_i, planned_j, is_one_step);
                    break;
                case direction_6::RIGHT_UP:
                    in_bound = try_hop<direction_6::RIGHT_UP>(planned_i, planned_j, is_one_step);
                    break;
                case direction_6::LEFT_DOWN:
                    in_bound = try_hop<direction_6::LEFT_DOWN>(planned_i, planned_j, is_one_step);
                    break;
                case direction_6::RIGHT_DOWN:
                    in_bound = try_hop<direction_6::RIGHT_DOWN>(planned_i, planned_j, is_one_step);
                    break;
            }
            if (in_bound && (at(planned_i, planned_j) == CHAR_EMPTY)) {
                if (trace.empty()) { // No move yet
                    move_type = is_one_step ? SINGLE_STEP : HOP;
                    if (move_type == HOP) {
                        if (at(optional_i, optional_j) == CHAR_EMPTY) {
                            std::swap(planned_i, optional_i);
                            std::swap(planned_j, optional_j);
                            move_type = SINGLE_STEP;
                        } else {
                            no_optional();
                        }
                    }
                    trace.push_back(std::make_pair(current_i, current_j));
                    move_to(planned_i, planned_j);
                    return true;
                } else if (move_type == SINGLE_STEP) {
                    // If was single step, allow going back only
                    const std::pair<int, int>& last_pos = trace.back();
                    if (is_one_step) {
                        if (last_pos.first == planned_i && last_pos.second == planned_j) {
                            trace.pop_back();
                            move_to(planned_i, planned_j);
                            return true;
                        }
                    } else {
                        if (last_pos.first == optional_i && last_pos.second == optional_j) {
                            trace.pop_back();
                            move_to(optional_i, optional_j);
                            no_optional();
                            return true;
                        }
                    }
                } else if (!is_one_step) {
                    if (!trim_trace_if_exists(planned_i, planned_j)) {
                        trace.push_back(std::make_pair(current_i, current_j));
                    }
                    move_to(planned_i, planned_j);
                    no_optional();
                    return true;
                }
            }
            return false;
        }

        // move when not selected
        void move_unselected(direction_6::Enum direct) {
            int planned_i = current_i;
            int planned_j = current_j;
            char char_player = CHAR_PLAYER[current_player];
            char char_in_board = CHAR_NONE;
            bool found = false;
            switch(direct) {
                case direction_6::LEFT:
                    for (planned_j -= 2; planned_j >= 0; planned_j -= 2) {
                        char_in_board = at(planned_i, planned_j);
                        if (char_in_board == char_player) {
                            found = true;
                            break;
                        } else if (char_in_board == CHAR_NONE) {
                            break;
                        }
                    }
                    break;
                case direction_6::RIGHT:
                    for (planned_j += 2; planned_j < N; planned_j += 2) {
                        char_in_board = at(planned_i, planned_j);
                        if (char_in_board == char_player) {
                            found = true;
                            break;
                        } else if (char_in_board == CHAR_NONE) {
                            break;
                        }
                    }
                    break;
                case direction_6::LEFT_UP:
                    for (planned_i--; planned_i >= 0; planned_i--) {
                        for (planned_j -= ((current_i - planned_i) & 1); planned_j >= 0; planned_j -= 2) {
                            char_in_board = at(planned_i, planned_j);
                            if (char_in_board == char_player) {
                                found = true;
                                break;
                            }
                        }
                        if (found) {
                            break;
                        }
                        planned_j = current_j;
                    }
                    break;
                case direction_6::RIGHT_UP:
                    for (planned_i--; planned_i >= 0; planned_i--) {
                        for (planned_j += ((current_i - planned_i) & 1); planned_j < N; planned_j += 2) {
                            char_in_board = at(planned_i, planned_j);
                            if (char_in_board == char_player) {
                                found = true;
                                break;
                            }
                        }
                        if (found) {
                            break;
                        }
                        planned_j = current_j;
                    }
                    break;
                case direction_6::LEFT_DOWN:
                    for (planned_i++; planned_i < M; planned_i++) {
                        for (planned_j -= ((planned_i - current_i) & 1); planned_j >= 0; planned_j -= 2) {
                            char_in_board = at(planned_i, planned_j);
                            if (char_in_board == char_player) {
                                found = true;
                                break;
                            }
                        }
                        if (found) {
                            break;
                        }
                        planned_j = current_j;
                    }
                    break;
                case direction_6::RIGHT_DOWN:
                    for (planned_i++; planned_i < M; planned_i++) {
                        for (planned_j += ((planned_i - current_i) & 1); planned_j < N; planned_j += 2) {
                            char_in_board = at(planned_i, planned_j);
                            if (char_in_board == char_player) {
                                found = true;
                                break;
                            }
                        }
                        if (found) {
                            break;
                        }
                        planned_j = current_j;
                    }
                    break;
            }
            if (found) {
                current_i = planned_i;
                current_j = planned_j;
            }
        }

        // check if wins from top
        bool check_win_from_top() {
            char c = CHAR_PLAYER[current_player];
            for (int i = 0; i < 4; i++) {
                const int max_j = 12 + i;
                for (int j = 12 - i; j <= max_j; j += 2) {
                    if (at(i, j) != c) {
                        return false;
                    }
                }
            }
            return true;
        }

        // check if wins from bottom
        bool check_win_from_bottom() {
            char c = CHAR_PLAYER[current_player];
            for (int i = M - 4; i < M; i++) {
                const int max_j = 12 + (M - i - 1);
                for (int j = 12 - (M - i - 1); j <= max_j; j += 2) {
                    if (at(i, j) != c) {
                        return false;
                    }
                }
            }
            return true;
        }

        // trim trace if plan exists, return true if plan exists
        bool trim_trace_if_exists(int planned_i, int planned_j) {
            for (int i = 0; i < trace.size(); i++) {
                if (trace[i].first == planned_i && trace[i].second == planned_j) {
                    while (trace.back().first != planned_i && trace.back().second != planned_j) {
                        trace.pop_back();
                    }
                    trace.pop_back();
                    return true;
                }
            }
            return false;
        }

        // find from top
        void find_from_top() {
            char char_player = CHAR_PLAYER[current_player];
            for (int i = 0; i < MN; i++) {
                if (brd[i] == char_player) {
                    current_i = i / N;
                    current_j = i % N;
                    break;
                }
            }
        }

        // find from bottom
        void find_from_bottom() {
            char char_player = CHAR_PLAYER[current_player];
            for (int i = MN - 1; i > 0; i--) {
                if (brd[i] == char_player) {
                    current_i = i / N;
                    current_j = i % N;
                    break;
                }
            }
        }

        // Current move type
        enum {
            HOP, SINGLE_STEP
        } move_type;

        // current location is selected
        bool selected;

        // current player
        int current_player;

        // current location
        int current_i;
        int current_j;

        // optional location, exists in HOP move
        int optional_i;
        int optional_j;

        // last direction, used for HOP move
        direction_6::Enum last_direction;

        // previous location in order, used for replay
        std::vector<std::pair<int, int> > trace;

        // true if need swap
        const bool need_swap;

        // player of board
        const int my_player;
};

class cc_remote_control_source: public virtual remote_control_source<char> {
    public:
        cc_remote_control_source(const char* host, const char* port): remote_control_source<char>(host, port){}

        char get(board<char>* brd) {
            constexpr int LENGTH = 1;
            char write_buffer[] = {commands::GET};
            char read_buffer[LENGTH];
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            size_t write_length = boost::asio::write(s, boost::asio::buffer(write_buffer, LENGTH));
            if (write_length == 0) {
                return CHAR_EXIT;
            }
            size_t read_length = boost::asio::read(s, boost::asio::buffer(read_buffer, LENGTH));
            if (read_length == 0) {
                return CHAR_EXIT;
            }
            if (read_buffer[0] == ' ') {
                if (!dynamic_cast<game_board*>(brd)->trace_empty()) {
                    return CHAR_EXIT;
                }
            }
            return read_buffer[0];
        }

        bool send(char c) {
            constexpr int LENGTH = 2;
            char write_buffer[] = {commands::PUT, c};
            return boost::asio::write(s, boost::asio::buffer(write_buffer, LENGTH)) == LENGTH;
        }

        bool login(void* payload) {
            int player = *((int*)payload);
            constexpr int LENGTH = 2;
            char write_buffer[] = {commands::IN, (char)player};
            char read_buffer[1];
            size_t write_length = boost::asio::write(s, boost::asio::buffer(write_buffer, LENGTH));
            if (write_length == 0) {
                return false;
            }
            size_t read_length = boost::asio::read(s, boost::asio::buffer(read_buffer, 1));
            if (read_length == 0) {
                return false;
            }
            return read_buffer[0] == success_fail::SUCCESS;
        }

        void logout() {
            constexpr int LENGTH = 1;
            char write_buffer[] = {commands::OUT};
            boost::asio::write(s, boost::asio::buffer(write_buffer, LENGTH)) == LENGTH;
        }
};

int main(int argc, char** argv) {
    bool client_mode;
    if (argc == 1) {
        client_mode = false;
    } else if (argc == 4) {
        client_mode = true;
    } else {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <player>\n";
        return 1;
    }
    if (client_mode) {
        char c;
        int player = atoi(argv[3]);
        game_board brd(false, player);
        std::unique_ptr<control_source<char> > source1(new unix_keyboard_control_source<char>());
        std::unique_ptr<control_source<char> > source2(new cc_remote_control_source(argv[1], argv[2]));
        if (!dynamic_cast<remote_control_source<char>*>(source2.get())->login(&player)) {
            std::cerr << "Login failed! Room/player occupied.\n";
            return 1;
        }
        control_source_runner<char, false> runner1(source1.get(), &brd);
        control_source_runner<char, false> runner2(source2.get(), &brd);
        runner1.run();
        blocking_queue<false>& q1 = (player == 0 ? runner1 : runner2);
        blocking_queue<false>& q2 = (player == 0 ? runner2 : runner1);
        status_type status;
        do {
            brd.init();
            q1.clear();
            q2.clear();
            do {
                if (player == 1) {
                    runner2.run();
                }
                do {
                    brd.print();
                    c = q1.get();
                    if (player == 0) {
                        dynamic_cast<remote_control_source<char>*>(source2.get())->send(c);
                    }
                    status = brd.next(c);
                } while (status == CONTROL_CONT);
                if (status == PLAYER_CHANGE) {
                    if (player == 0) {
                        runner2.run();
                    }
                    do {
                        brd.print();
                        c = q2.get();
                        if (player == 1) {
                            dynamic_cast<remote_control_source<char>*>(source2.get())->send(c);
                        }
                        status = brd.next(c);
                    } while (status == CONTROL_CONT);
                }
            } while (status != PLAYER_WIN);
            if (player == brd.get_current_player()) {
                std::cout << "You win! Press any key for another game." << std::endl;
            } else {
                std::cout << "You lose! Press any key for another game." << std::endl;
            }
            runner1.get();
        } while(1);
    } else {
        game_board brd(true);
        std::unique_ptr<control_source<char> > source(new unix_keyboard_control_source<char>());
        control_source_runner<char, false> runner(source.get(), &brd);
        runner.run();
        blocking_queue<false>& q = runner;
        status_type status;
        do {
            brd.init();
            q.clear();
            do {
                brd.print();
            } while ((status = brd.next(q.get())) != PLAYER_WIN);
            std::cout << "You win! Press any key for another game." << std::endl;
            q.get();
        } while(1);
    }

    return 0;
}
