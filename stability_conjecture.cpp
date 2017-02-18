#include "conjecture.hpp"
#include <unordered_set>

constexpr pos_t size = 7;

void test(State<size> state) {
    static IterativeDeepeningAlphaBeta<size, Minimax<size>> search;
    int expected = state.board.minimax();
    auto actual = search.search(state, expected - 1, expected + 1);
    if (expected != actual.minimax || actual.type != NodeType::PV) {
        std::cout << "UNSTABLE! OOPS: " << state.board << std::endl;
    } else {
        std::cout << "STABLE: " << state.board << std::endl;
    }
}

std::unordered_set<State<size>, StateHasher<size>> stable_states;
std::unordered_set<Board<size>, BoardHasher<size>> stable_boards;

// postcondition: cur is same as before. taken by reference to avoid copying.
void fill(State<size> &cur, Cell color, pos_t pos) {
    if (pos >= size) {
        if (!cur.board.get(size - 2).is_empty() && cur.board.get(size - 1).is_empty()) {
            stable_states.insert(cur);
            stable_boards.insert(cur.board);
        }
        return;
    }
    cur.play(Move(color, pos));
    fill(cur, color, pos + 1);
    fill(cur, color, pos + 2);
    fill(cur, color, pos + 3);
    fill(cur, color.flip(), pos + 2);
    cur.undo();
}

void generate_stable_states() {
    State<size> state;
    fill(state, BLACK, 1);
    fill(state, WHITE, 1);
}

template <pos_t size, typename Impl> struct StabilityConjecture : Impl {
    typedef typename Impl::minimax_t minimax_t;
    typedef typename Impl::return_t return_t;

    bool diverges(const State<size> &state) const {
        return stable_boards.find(state.board) != stable_boards.end() && state.board.captured == 0;
    }

    return_t init_node(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                      bool &terminal) {
        auto it = stable_boards.find(state.board);
        if (it != stable_boards.end() && state.board.captured == 0) {
            terminal = true;
            auto r = return_t(it->minimax());
            r.exact = true;
            r.type = NodeType::PV;
            return r;
        }
        return Impl::init_node(state, alpha, beta, depth, terminal);
    }
};

int main() {
    generate_stable_states();
    for (const State<size> &state : stable_states) {
        test(state);
    }
    std::cout << "total stable: " << stable_states.size() << std::endl;
    IterativeDeepeningAlphaBeta<size, Metrics<size, StabilityConjecture<size, PV<size>>>> search;
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
    search.callback = [&](auto val) {
        PV<size>::print_path(val.get_path(), root);
        std::cout << "cutoff=" << search.impl.impl.cutoff << "\tminimax=" << val.minimax
                  << "\tsearched=" << search.impl.impl.num_nodes << "\n";
    };
    auto node = search.search(root);
    std::cout << "minimax: " << node.minimax << std::endl;
}
