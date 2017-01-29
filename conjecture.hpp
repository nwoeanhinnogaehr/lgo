#include "ab.hpp"
#include <experimental/optional>

using std::experimental::optional;

// search for nodes which conflict with conjecture. example usage:
// ConjectureProver<size, Minimax<size>, AlphaBeta, NullPruningConjecture> prover;
// State<size> state;
// prover.search(state);
template <pos_t size, typename Impl, template <pos_t, typename, typename> typename Conjecture>
struct ConjectureProverImplWrapper : Impl {
    typedef typename Impl::minimax_t minimax_t;
    typedef typename Impl::return_t return_t;
    Conjecture<size, minimax_t, return_t> conj;
    void on_exit(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value) {
        Impl::on_exit(state, alpha, beta, depth, value);
        if (optional<std::string> msg = conj.expect(state, alpha, beta, value)) {
            std::cout << "At board " << state.board << " with player " << state.to_play
                      << " at depth " << depth << ": " << *msg << std::endl;
        }
    }
};
template <pos_t size, typename Impl, template <pos_t, typename> typename Search,
          template <pos_t, typename, typename> typename Conjecture>
struct ConjectureProver : Search<size, ConjectureProverImplWrapper<size, Impl, Conjecture>> {};

// do search using pruning as specified by conjecture(s). example usage:
// PrunedSearch<size, Minimax<size>, AlphaBeta, PruningConjecture1, PruningConjecture2, ...> solver;
// State<size> state;
// solver.search(state);
template <pos_t size, typename Impl, template <pos_t, typename, typename> typename First>
struct PrunedSearchImplWrapperImpl : Impl {
    typedef typename Impl::minimax_t minimax_t;
    typedef typename Impl::return_t return_t;
    First<size, minimax_t, return_t> conj;
    return_t on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                      bool &terminal) {
        if (optional<return_t> conjected = conj.apply(state, alpha, beta)) {
            terminal = true;
            return *conjected;
        }
        return Impl::on_enter(state, alpha, beta, depth, terminal);
    }
};
template <pos_t size, typename Impl, template <pos_t, typename, typename> typename First,
          template <pos_t, typename, typename> typename... Rest>
struct PrunedSearchImplWrapper
    : PrunedSearchImplWrapperImpl<size, PrunedSearchImplWrapper<size, Impl, Rest...>, First> {};
template <pos_t size, typename Impl, template <pos_t, typename, typename> typename First>
struct PrunedSearchImplWrapper<size, Impl, First> : PrunedSearchImplWrapperImpl<size, Impl, First> {
};
template <pos_t size, typename Impl, template <pos_t, typename> typename Search,
          template <pos_t, typename, typename> typename... Prunes>
struct PrunedSearch {
    using ImplWrapper = PrunedSearchImplWrapper<size, Impl, Prunes...>;
    Search<size, ImplWrapper> impl;
    typename ImplWrapper::return_t search(State<size> &state) { return impl.search(state); }
};

// conjectures below
template <pos_t size, typename minimax_t, typename return_t> struct NullPruningConjecture {
    optional<return_t> apply(const State<size> &state, minimax_t &alpha, minimax_t &beta) {
        return {};
    }
    optional<std::string> expect(const State<size> &state, const minimax_t &alpha,
                                 const minimax_t &beta, const return_t &value) {
        return {};
    }
};
