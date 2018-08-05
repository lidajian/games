#ifndef DIRECTION_HPP
#define DIRECTION_HPP

namespace direction_4 {
    enum Enum {
        LEFT = 0, RIGHT = 3, UP = 1, DOWN = 2
    };

    bool is_opposite(Enum d1, Enum d2) {
        return d1 + d2 == 3;
    }
};

namespace direction_6 {
    enum Enum {
        LEFT = 0, RIGHT = 5, LEFT_UP = 1, RIGHT_DOWN = 4, RIGHT_UP = 2, LEFT_DOWN = 3
    };

    bool is_opposite(Enum d1, Enum d2) {
        return d1 + d2 == 5;
    }
};

#endif
