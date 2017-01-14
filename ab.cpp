#include "ab.hpp"
#include <iostream>

constexpr pos_t size = 6;

void print_pv_tree(ab::PV<size>::Node n, State<size> &s, int depth) {
    std::cout << n.minimax << "\t";
    Move m = n.move;
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
    for (auto c : n.children)
        print_pv_tree(c, s, depth + 1);
    if (m.color != EMPTY)
        s.undo();
}

State<size> print_path(std::vector<ab::PV<size>::Node> path, State<size> root) {
    std::reverse(path.begin(), path.end());
    for (auto n : path) {
        Move m = n.move;
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
    AlphaBeta<size, ab::PV<size>> ab;
    State<size> root;

    auto node = ab.alphabeta(root, BLACK);

    print_pv_tree(node, root, 0);

    std::cout << "\nlongest PV:\n";
    print_path(node.get_longest_path(), root);

    std::cout << "\nshortest PV:\n";
    print_path(node.get_shortest_path(), root);

    std::cout << "\nminimax " << node.minimax << "\n";
    std::cout << "states " << ab.impl.call_count << "\n";
}
