#include "ab.hpp"
#include "conjectures.hpp"
#include <algorithm>
#include <iostream>

constexpr pos_t size = 9;

int main() {
    auto stable_boards = conjectures::Stability<size, Minimax<size>>::compute_stable_boards();
    using Impl = NewickTree<size, Metrics<size, conjectures::All<size, PV<size>>>>;
    for (auto board : stable_boards) {
        std::vector<std::pair<pos_t, Move>> moves;
        for (pos_t i = 0; i < size; i++) {
            Cell c = board.get(i);
            if (c.is_stone())
                moves.emplace_back(i, Move(c, i));
        }
        auto cmp = [](auto a, auto b) { return a.first < b.first; };
        std::sort(moves.begin(), moves.end(), cmp);
        do {
            for (int i = 0; i < 2; i++) {
                IterativeDeepening<size, AlphaBeta, Impl> ab;
                State<size> state;
                for (auto p : moves) {
                    state.play(p.second);
                    std::cout << p.second.color << p.second.position + 1 << " ";
                }
                if (i == 1) {
                    state.to_play = state.to_play.flip();
                    std::cout << "F ";
                }
                auto val = ab.search(state, board.minimax() - 1, board.minimax() + 1);
                std::cout << val.minimax;
                std::cerr << " " << board.minimax();
                std::cout << std::endl;
            }
        } while (std::next_permutation(moves.begin(), moves.end(), cmp));
    }
}
