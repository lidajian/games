#ifndef COUNTDOWN_HPP
#define COUNTDOWN_HPP

#include <string>
#include <iostream>

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

#endif
