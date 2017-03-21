#pragma once

#include "lgo.hpp"
#include <algorithm>

template <pos_t size> struct GoodPlayer {
    const State<size> &state;

    GoodPlayer(const State<size> &state) : state(state) {}

    void atari_moves(Cell color, pos_t &legal, std::vector<Move> &moves) const {
        pos_t capturing = state.capturing_moves(color);
        for (pos_t i = 1; i < size-1; i++) {
            if ((legal & (1 << i)) && (capturing & (1 << i))) {
                moves.emplace_back(color, i);
                legal &= ~(1 << i);
            }
        }
        if ((legal & 1) && (capturing & 1)) {
            moves.emplace_back(color, 0);
            legal &= ~1;
        }
        if ((legal & (1 << (size - 1))) && (capturing & (1 << (size - 1)))) {
            moves.emplace_back(color, size - 1);
            legal &= ~(1 << (size - 1));
        }
    }
    void cell_2_conjecture_simple(Cell color, pos_t &legal, std::vector<Move> &moves) const {
        // it's safe to ignore the warnings. waiting for constexpr if...
        if (size < 4)
            return;
/*
        if ((legal & 3) == 3 && !state.board.is_captured(1) && !state.board.is_captured(0)) {
            moves.emplace_back(color, 1);
            legal &= ~3;
        }
        if ((legal & (3 << (size - 2))) == (3 << (size - 2)) &&
            !state.board.is_captured(size - 2) && !state.board.is_captured(size - 1)) {
            moves.emplace_back(color, size - 2);
            legal &= ~(3 << (size - 2));
        }*/
        if ((legal & 3) == 3) {
            moves.emplace_back(color, 1);
            legal &= ~2;
        }
        if ((legal & (3 << (size - 2))) == (3 << (size - 2))) {
            moves.emplace_back(color, size - 2);
            legal &= ~(1 << (size - 2));
        }
    }
    void cell_2_conjecture_full(Cell color, pos_t &legal, std::vector<Move> &moves) const {
        if (size < 4)
            return;
        for (pos_t i = 0; i < size - 2; i += 2) {
            if ((legal & (3 << i)) == 3_pos_t << i) {
                moves.emplace_back(color, i + 1);
                legal &= ~(2 << i);
            }
            if ((legal & (3 << (size - i - 2))) == (3_pos_t << (size - i - 2))) {
                moves.emplace_back(color, size - i - 2);
                legal &= ~(1 << (size - i - 2));
            }
        }
    }
    void safe_moves(Cell color, pos_t &legal, std::vector<Move> &moves) const {
        for (pos_t i = 1; i < size - 1; i++) {
            if (legal & (1 << i)) {
                if ((state.board.get(i-1) != color.flip() && state.board.get(i+1).is_stone()) ||
                        (state.board.get(i+1) != color.flip() && state.board.get(i-1).is_stone())) {
                    moves.emplace_back(color, i);
                    legal &= ~(1 << i);
                }
            }
        }
    }
    void other_moves(Cell color, pos_t &legal, std::vector<Move> &moves) const {
        // add all other legal moves
        for (pos_t i = 1; i < size - 1; i++)
            if (legal & (1 << i))
                moves.emplace_back(color, i);
        if (legal & 1)
            moves.emplace_back(color, 0);
        if (legal & (1 << (size - 1)))
            moves.emplace_back(color, size - 1);
    }
    void moves(Cell color, std::vector<Move> &moves) const {
        pos_t legal = state.legal_moves(color);

        // symmetry at root
        if (state.past.size() == 0) {
            legal &= ((1 << ((size - 1) / 2 + 1)) - 1); // mirror moves
            legal &= ~1;                                // and first cell
        }
        // symmetry when only stone is in the center on odd board sizes
        if (size % 2 == 1 && state.past.size() == 1 && state.board.get(size/2).is_stone()) {
            legal &= ((1 << ((size - 1) / 2 + 1)) - 1); // mirror moves
        }

        // if moves is already initialized, prune any illegal moves and update legal
        bool has_pass = false;
        for (size_t i = 0; i < moves.size();) {
            Move m = moves[i];
            if (m.color != color) {
                moves.erase(moves.begin() + i);
            } else {
                if (m.is_pass) {
                    if (has_pass)
                        moves.erase(moves.begin() + i);
                    else
                        i++;
                    has_pass = true;
                } else {
                    if ((legal & (1 << m.position)) == 0)
                        moves.erase(moves.begin() + i);
                    else {
                        legal &= ~(1 << m.position);
                        i++;
                    }
                }
            }
        }

        if (!has_pass)
            moves.emplace_back(color);

        cell_2_conjecture_simple(color, legal, moves);
        cell_2_conjecture_full(color, legal, moves);
        atari_moves(color, legal, moves);
        //safe_moves(color, legal, moves);
        other_moves(color, legal, moves);
        /*
        int scbefore = state.board.minimax();
        std::vector<std::pair<int, Move>> ms;
        const std::vector<int> weights{-1,2,1,1,1,1,2,-1};
        for (size_t i = 0; i < moves.size(); i++) {
            Move &m = moves[i];
            Board<size> b = state.board;
            if (!m.is_pass) {
                b.set(m.position, m.color);
                b.clear_captured(m.position);
            }
            int score = b.minimax();
            score = score - scbefore;
            if (!m.is_pass)
                score += weights[m.position];
            score += moves.size() - i;
            ms.emplace_back(score, m);
        }
        std::sort(ms.begin(), ms.end(), [](const std::pair<int, Move> &a, const std::pair<int, Move> &b) { return a.first > b.first; });
        for (size_t i = 0; i < moves.size(); i++) {
            moves[i] = ms[i].second;
        }*/
    }
};
