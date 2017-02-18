#pragma once

#include "../ab.hpp"

namespace conjectures {

// if current evaluation of the board is equal to +/- the board size and the opponent has no moves,
// then we cannot do any better, might as well stop now.
template <pos_t size, typename Impl> struct Full : Impl {
    using minimax_t = typename Impl::minimax_t;
    using return_t = typename Impl::return_t;
    return_t init_node(State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) {
        int minimax = state.board.minimax();
        if ((minimax == int(size) || minimax == -int(size)) &&
            state.legal_moves(state.to_play.flip()) == 0) {
            terminal = true;
            return return_t(minimax);
        }
        return Impl::init_node(state, alpha, beta, depth, terminal);
    }
};
};
