#include "ab.hpp"
#include <iostream>

constexpr pos_t size = 6;

void print_pv_tree(PVNode n, State<size>& s, int depth) {
    if (n.move.color == EMPTY)
        std::cout << "init:\t";
    else if (n.move.is_pass)
        std::cout << n.move.color << " pass:\t";
    else
        std::cout << n.move.color << " " << n.move.position << ":\t";

    if (n.move.color != EMPTY)
        s.play(n.move);
    for (int i = 0; i < depth; i++)
        std::cout << "+";
    std::cout << s.board << std::endl;
    for (PVNode c : n.children) {
        print_pv_tree(c, s, depth + 1);
    }
    if (n.move.color != EMPTY)
        s.undo();
}

int main() {
    AlphaBeta<size> ab;
    ab.construct_tree = true;
    State<size> root;
    PVNode ret = ab.alphabeta(root, BLACK);
    print_pv_tree(ret, root, 0);
    std::cout << "minimax " << ret.minimax << std::endl;
    std::cout << "states " << ab.state_visits << std::endl;
}
