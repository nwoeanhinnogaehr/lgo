#include "ab.hpp"
#include <iostream>

constexpr pos_t size = 6;

void print_pv_tree(PVNode n, State<size> &s, int depth) {
    Move m = n.get_move();
    if (m.color == EMPTY)
        std::cout << "init:\t";
    else if (m.is_pass)
        std::cout << m.color << " pass:\t";
    else
        std::cout << m.color << " " << m.position << ":\t";

    if (m.color != EMPTY)
        s.play(m);
    for (int i = 0; i < depth; i++)
        std::cout << "+";
    std::cout << s.board << std::endl;
    for (PVNode c : n.children)
        print_pv_tree(c, s, depth + 1);
    if (m.color != EMPTY)
        s.undo();
}

State<size> print_path(std::vector<PVNode> path, State<size> root) {
    std::reverse(path.begin(), path.end());
    for (PVNode n : path) {
        Move m = n.get_move();
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
    AlphaBeta<size, PVNode> ab;
    State<size> root;

    PVNode ret = ab.alphabeta(root, BLACK);

    std::cout << "\nlongest PV:\n";
    print_path(ret.get_longest_path(), root);

    std::cout << "\nshortest PV:\n";
    print_path(ret.get_shortest_path(), root);

    std::cout << "\nminimax " << ret.minimax << "\n";
    std::cout << "states " << ab.state_visits << "\n";
}
