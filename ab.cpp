#include "ab.hpp"
#include <iostream>

constexpr pos_t size = 6;

State<size> print_path(std::vector<Move> path, State<size> root) {
    for (Move &m : path) {
        if (m.color == EMPTY)
            std::cout << "init:\t";
        else if (m.is_pass)
            std::cout << m.color << " pass:\t";
        else
            std::cout << m.color << " " << m.position << ":\t";

        if (m.color != EMPTY && !m.is_pass) {
            root.play(m);
        }
        std::cout << root.board << std::endl;
    }
    return root;
}

int main() {
    IterativeDeepening<size, AlphaBeta> ab;
    //AlphaBeta<size, ab::PV<size>> ab;
    State<size> root;

    auto node = ab.search(root);

    std::cout << "\nprincipal variation:\n";
    print_path(node.get_path(), root);

    std::cout << "\nminimax " << node.minimax << "\n";
}
