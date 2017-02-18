#include "ab.hpp"
#include <iostream>
#include "conjectures.hpp"

constexpr pos_t size = 7;

int main() {
    IterativeDeepening<size, AlphaBeta, NewickTree<size, Metrics<size, conjectures::All<size, PV<size>>>>> ab;
    State<size> root;
    for (;;) {
        int a;
        std::string b;
        std::cin >> a;
        if (a == 0)
            break;
        std::cin >> b;
        root.play(Move(b[0] == 'b' ? BLACK : WHITE, a - 1));
    }
    ab.callback = [&](auto val) {
        std::cout << "cutoff=" << ab.impl.impl.cutoff << "\tminimax=" << val.minimax
                  << "\tsearched=" << ab.impl.impl.num_nodes << "\n";
        PV<size>::print_path(val.get_path(), root);
        /*for (auto e : ab.impl.impl.bmtable) {
            Board<size> board = e.first;
            std::cout << board << ", ";
            for (int i = -(int)size; i <= (int)size; i++) {
                std::cout << e.second[i] << ", ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";*/
        ab.impl.impl.bmtable.clear();
    };

    auto node = ab.search(root);
    std::cout << "\nminimax " << node.minimax << "\n";
}
