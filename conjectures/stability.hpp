#pragma once

#include "../ab.hpp"
#include <functional>
#include <unordered_set>

namespace conjectures {
template <pos_t size, typename Impl> struct Stability : Impl {
    static auto compute_stable_boards() {
        std::unordered_set<Board<size>, BoardHasher<size>> set;
        std::function<void(State<size> &, Cell, pos_t)> fill = [&](State<size> &cur, Cell color,
                                                                   pos_t pos) {
            if (pos >= size) {
                if (!cur.board.get(size - 2).is_empty() && cur.board.get(size - 1).is_empty())
                    set.insert(cur.board);
                return;
            }
            cur.play(Move(color, pos));
            fill(cur, color, pos + 1);
            fill(cur, color, pos + 2);
            fill(cur, color, pos + 3);
            fill(cur, color.flip(), pos + 2);
            cur.undo();
        };
        State<size> state;
        fill(state, BLACK, 1);
        fill(state, WHITE, 1);
        return set;
    }

    using minimax_t = typename Impl::minimax_t;
    using return_t = typename Impl::return_t;

    return_t make_node(Board<size> &board) {
        auto r = return_t(board.minimax());
        r.exact = true;
        r.type = NodeType::PV;
        return r;
    }

    return_t init_node(State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) {
        static auto stable_boards = compute_stable_boards();
        if (state.board.captured == 0) {
            auto it = stable_boards.find(state.board);
            if (it != stable_boards.end()) {
                terminal = true;
                return make_node(state.board);
            }
        }
        return Impl::init_node(state, alpha, beta, depth, terminal);
    }
};
};
