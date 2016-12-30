#include "ab.hpp"
#include <iostream>

int main() {
    constexpr pos_t size = 6;
    AlphaBeta<size> ab;
    State<size> root;
    int minimax = ab.alphabeta(root, BLACK);
    for (auto it : ab.board_visits) {
        std::cout << it.first << " -> " << it.second << std::endl;
    }
    std::cout << "minimax " << minimax << std::endl;
    std::cout << "states " << ab.state_visits << std::endl;
}
