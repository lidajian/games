#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstring>  // memset, memcpy

#include "lockable.hpp"

template <typename T>
class board: public lockable {
    public:
        board(int M, int N): M(M), N(N), MN(M * N), brd(new T[MN]) {}

        bool copy_to(board<T>& _brd) {
            if (_brd.M != M || _brd.N != N) {
                return false;
            }
            ATOMIC_RUN(
                    memcpy(brd, _brd.brd, MN * sizeof(T));
                    )
            return true;
        }

        inline void clear() {
            memset(brd, 0, sizeof(T) * M * N);
        }

        inline T& at(int i, int j) {
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
        T* const brd;
};

#endif
