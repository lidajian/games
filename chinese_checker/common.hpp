#ifndef COMMON_HPP
#define COMMON_HPP

namespace commands {
    enum {
        PUT = 'P',
        GET = 'G',
        IN  = 'I',
        OUT = 'O'
    };
};

namespace success_fail {
    enum {
        SUCCESS = 'S',
        FAIL    = 'F'
    };
};

constexpr char NUM_PLAYER = 2;

#endif
