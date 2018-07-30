#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstring>  // memset, memcpy

#include "lockable.hpp"

enum direction {
    LEFT = 0, RIGHT = 3, UP = 1, DOWN = 2
};

class board: public lockable {
    public:
        board(int M, int N): M(M), N(N), MN(M * N), brd(new int[MN]) {}

        bool copy_to(board& _brd) {
            if (_brd.M != M || _brd.N != N) {
                return false;
            }
            ATOMIC_RUN(
                    memcpy(brd, _brd.brd, MN * sizeof(int));
                    )
            return true;
        }

        inline void clear() {
            memset(brd, 0, sizeof(int) * M * N);
        }

        inline int& at(int i, int j) {
            return brd[i * N + j];
        }

        int getM() const {
            return M;
        }

        int getN() const {
            return N;
        }

        ~board() {
            delete[] brd;
        }
    protected:
        const int M;
        const int N;
        const int MN;
        int* const brd;
};

#endif
