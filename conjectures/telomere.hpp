#pragma once

#include "../ab.hpp"
#include <functional>
#include <unordered_set>

namespace conjectures {
template <pos_t size, typename Impl> struct Telomere : Impl {
    using minimax_t = typename Impl::minimax_t;
    using return_t = typename Impl::return_t;

    struct Instance {
        pos_t pattern;
        optional<int> upperbound;
        optional<int> minimax;
        pos_t legal;
    };

    const std::unordered_multimap<pos_t, Instance> instances{
        {2, {0b00, {}, size, 0b00}},         {2, {0b10, {}, size, 0b00}},

        {3, {0b000, {}, {}, 0b010}},         {3, {0b100, size, {}, 0b000}},
        {3, {0b010, size - 5, {}, 0b000}},   {3, {0b110, size, {}, 0b000}},

        {4, {0b0000, size - 7, {}, 0b0110}}, {4, {0b1000, size, {}, 0b0000}},
        {4, {0b0100, size - 7, {}, 0b0010}}, {4, {0b1100, size - 5, {}, 0b0010}},
        {4, {0b0010, size - 7, {}, 0b0100}}, {4, {0b1010, size - 5, {}, 0b0100}},
        {4, {0b0110, size - 7, {}, 0b0001}}, {4, {0b1110, size - 5, {}, 0b0000}},

        //{5, {0b01010, {}, {}, 0b00100}}
    };

    return_t init_node(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                       bool &terminal) {
        goto invalid;
        if (state.past.size() == 0)
            goto invalid;

        {
            Move last_move = std::get<2>(state.past.top());
            pos_t pos = last_move.position;
            Cell color = last_move.color;

            auto doit = [&](int dir, pos_t length) {
                auto range = instances.equal_range(length);
                for (auto it = range.first; it != range.second; it++) {
                    const Instance &inst = it->second;
                    pos_t legal = state.legal_moves(state.to_play);
                    bool has_liberty = false;

                    // check that pattern matches
                    for (pos_t i = 1; i <= length; i++) {
                        if ((inst.pattern & (1_pos_t << (i - 1)))) {
                            if (state.board.get(pos + dir * i) != color.flip())
                                goto next;
                        } else {
                            if (state.board.get(pos + dir * i).is_stone())
                                goto next;
                        }
                        if (state.board.is_captured(pos))
                            if (state.board.is_captured(pos + dir * i))
                                goto next;
                    }

                    // requirement - x has a liberty
                    // and opponent has no legal moves in telomere
                    for (pos_t i = pos; i < size; i -= dir) {
                        if (legal & (1 << i))
                            goto next;
                        Cell here = state.board.get(i);
                        if (here == color.flip())
                            goto next;
                        if (here == EMPTY && state.board.get(i + dir) != EMPTY)
                            has_liberty = true;
                    }
                    if (!has_liberty)
                        goto next;

                    // update legal moves
                    for (pos_t i = 0; i < length; i++) {
                        state.info_cache[state.to_play.value]->legal_moves ^=
                            (-((inst.legal >> i) & 1) ^
                             state.info_cache[state.to_play.value]->legal_moves) &
                            (1 << (pos + (i + 1) * dir));
                    }

                    if (inst.upperbound) {
                        if (state.to_play == BLACK) {
                            beta = std::min(beta, *inst.upperbound);
                        } else if (state.to_play == WHITE) {
                            alpha = std::max(alpha, -*inst.upperbound);
                        }
                    }
                    /*std::cout << "telomere hit " << state.board << "   "
                       << "pat=" << std::bitset<size>(inst.pattern)
                       << " length=" << length
                              << " legal=" << std::bitset<size>(state.info_cache[state.to_play.value]->legal_moves) <<
                       ", " << state.to_play <<
                       std::endl;*/
                    if (inst.minimax) {
                        terminal = true;
                        if (state.to_play == BLACK)
                            return return_t(-*inst.minimax);
                        if (state.to_play == WHITE)
                            return return_t(*inst.minimax);
                    }
                    break;
                next:;
                }
                return return_t(0);
            };
            if (pos <= 5) {
                int dir = -1;
                pos_t length = pos;
                auto mm = doit(dir, length);
                if (terminal)
                    return mm;
            }
            if (pos >= size - 6) {
                int dir = 1;
                pos_t length = size - pos - 1;
                auto mm = doit(dir, length);
                if (terminal)
                    return mm;
            }
        }
    invalid:
        return Impl::init_node(state, alpha, beta, depth, terminal);
    }
};
};
