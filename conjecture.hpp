#include "ab.hpp"
#include <experimental/optional>

using std::experimental::optional;

// helper to build a class hierarchy from a variadic list of types
template <template <pos_t, typename, template <pos_t, typename> typename>
          typename Interior,
          pos_t size, typename Impl, template <pos_t, typename> typename First,
          template <pos_t, typename> typename... Rest>
struct ManyImplWrapper : Interior<size, ManyImplWrapper<Interior, size, Impl, Rest...>, First> {};
template <template <pos_t, typename, template <pos_t, typename> typename>
          typename Interior,
          pos_t size, typename Impl, template <pos_t, typename> typename First>
struct ManyImplWrapper<Interior, size, Impl, First> : Interior<size, Impl, First> {};

// search for nodes which conflict with conjecture. example usage:
// ConjectureProver<size, Minimax<size>, AlphaBeta, PruningConjecture1, PruningConjecture2> prover;
// State<size> state;
// prover.search(state);
template <pos_t size, typename Impl, template <pos_t, typename> typename Conjecture>
struct ConjectureProverImplWrapper : Impl {
    typedef typename Impl::minimax_t minimax_t;
    typedef typename Impl::return_t return_t;
    Conjecture<size, Impl> conj;
    AlphaBeta<size, Conjecture<size, Impl>> conj_search;
    std::stack<optional<return_t>> conjected;
    return_t on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                      bool &terminal) {
        if (conj.diverges(state))
            conjected.emplace(conj_search.search(state, alpha, beta, depth));
        else
            conjected.emplace();
        return Impl::on_enter(state, alpha, beta, depth, terminal);
    }
    void on_exit(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value) {
        optional<return_t> expected = conjected.top();
        conjected.pop();
        if (expected && !(*expected == value)) {
            std::cout << "At board " << state.board << " with player " << state.to_play
                      << " at depth " << depth << std::endl;
        }
        Impl::on_exit(state, alpha, beta, depth, value);
    }
};
template <pos_t size, typename Impl, template <pos_t, typename> typename Search,
          template <pos_t, typename> typename... Conjectures>
using ConjectureProver =
    Search<size, ManyImplWrapper<ConjectureProverImplWrapper, size, Impl, Conjectures...>>;

// do search using pruning as specified by conjecture(s). example usage:
// PrunedSearch<size, Minimax<size>, AlphaBeta, PruningConjecture1, PruningConjecture2, ...> solver;
// State<size> state;
// solver.search(state);
template <pos_t size, typename Impl, template <pos_t, typename> typename Conjecture>
struct PrunedSearchImplWrapper : Impl {
    typedef typename Impl::minimax_t minimax_t;
    typedef typename Impl::return_t return_t;
    Conjecture<size, Impl> conj;
    return_t on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                      bool &terminal) {
        if (optional<return_t> conjected = conj.apply(state, alpha, beta)) {
            terminal = true;
            return *conjected;
        }
        return Impl::on_enter(state, alpha, beta, depth, terminal);
    }
};
template <pos_t size, typename Impl, template <pos_t, typename> typename Search,
          template <pos_t, typename> typename... Conjectures>
using PrunedSearch =
    Search<size, ManyImplWrapper<PrunedSearchImplWrapper, size, Impl, Conjectures...>>;

// conjectures below
template <pos_t size, typename Impl> struct NullConjecture : Impl {
    bool diverges(const State<size> &state) const {
        return false;
    }
};

template <pos_t size, typename Impl> struct Cell2Conjecture : Impl {
    bool diverges(const State<size> &state) const {
        return state.board.get(0) == EMPTY && state.board.get(1) == EMPTY &&
               !state.board.is_captured(0) && !state.board.is_captured(1);
    }
    void gen_moves(const State<size> &state, std::vector<Move> &moves) const {
        Impl::gen_moves(state, moves);
        if (diverges(state)) {
            for (size_t i = 0; i < moves.size(); i++) {
                if (moves[i].position == 0 && !moves[i].is_pass) {
                    moves.erase(moves.begin() + i);
                    break;
                }
            }
        }
    }
};
