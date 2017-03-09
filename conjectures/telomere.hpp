#pragma once

#include "../ab.hpp"
#include <functional>
#include <unordered_set>

namespace conjectures {
template <pos_t size, typename Impl> struct Telomere : Impl {
    using minimax_t = typename Impl::minimax_t;
    using return_t = typename Impl::return_t;

    void on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth) {
        if (state.past.size() == 0)
            goto invalid;

        {
            Move last_move = std::get<2>(state.past.top());
            int dir;
            int pos;
            Cell color = last_move.color;
            if (last_move.position == 3) {
                dir = -1;
                pos = 3;
            } else if (last_move.position == size - 4) {
                dir = 1;
                pos = size - 4;
            } else
                goto invalid;

            // requirement - telomere complement shape
            if (state.board.get(pos + dir) != EMPTY ||
                state.board.get(pos + dir * 2) != color.flip() ||
                state.board.get(pos + dir * 3) != EMPTY)
                goto invalid;

            // requirement - this is a new telomere
            if (state.board.is_captured(pos) || state.board.is_captured(pos + dir) ||
                state.board.is_captured(pos + dir * 2) || state.board.is_captured(pos + dir * 3)) {
                // if some have been captured, there are 3 states to check history for.
                const std::vector<std::vector<Cell>> prefixes{
                    {color, EMPTY, EMPTY}, {color, color, EMPTY}, {EMPTY, color, EMPTY}};
                for (auto prefix : prefixes) {
                    auto board = state.board;
                    for (size_t i = 0; i < prefix.size(); i++) {
                        board.set(pos + dir * (i + 1), prefix[prefix.size() - i - 1]);
                    }
                    if (state.history.contains(board))
                        goto invalid;
                }
            }

            // requirement - x has a liberty
            // opponent has no legal moves in telomere
            pos_t legal = state.legal_moves(state.to_play);
            bool has_liberty = false;
            for (pos_t i = pos; i < size && i >= 0; i -= dir) {
                if (legal & (1 << i))
                    goto invalid;
                Cell here = state.board.get(i);
                if (here == color.flip())
                    goto invalid;
                if (here == EMPTY && state.board.get(i + dir) != EMPTY) {
                    has_liberty = true;
                }
            }
            if (!has_liberty)
                goto invalid;

            state.info_cache[color.value]->legal_moves = 0;

            if (state.to_play == BLACK) {
                beta = std::min(beta, state.board.minimax());
            }
            else if (state.to_play == WHITE) {
                alpha = std::max(alpha, state.board.minimax());
            }
        }
invalid:
        Impl::on_enter(state, alpha, beta, depth);
    }
};
};
