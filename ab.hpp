#include "lgo.hpp"
#include <algorithm>
#include <climits>
#include <functional>
#include <unordered_map>

/*
01 function alphabeta(node, depth, α, β, maximizingPlayer)
02      if depth = 0 or node is a terminal node
03          return the heuristic value of node
04      if maximizingPlayer
05          v := -∞
06          for each child of node
07              v := max(v, alphabeta(child, depth – 1, α, β, FALSE))
08              α := max(α, v)
09              if β ≤ α
10                  break (* β cut-off *)
11          return v
12      else
13          v := ∞
14          for each child of node
15              v := min(v, alphabeta(child, depth – 1, α, β, TRUE))
16              β := min(β, v)
17              if β ≤ α
18                  break (* α cut-off *)
19          return v;
*/

namespace ab {
template <pos_t size> struct Minimax {
    typedef int return_t;
    typedef int minimax_t;
    static constexpr minimax_t alpha_init() { return -size; }
    static constexpr minimax_t beta_init() { return size; }
    return_t on_enter(State<size> &s, Cell color, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) {
        if ((terminal = s.terminal())) {
            Score ss = s.board.score();
            return return_t(minimax_t(ss.black) - minimax_t(ss.white));
        }
        return color == BLACK ? -size : size;
    }
    void on_exit(State<size> &s, Cell color, minimax_t alpha, minimax_t beta, size_t depth,
                 return_t &value) {}
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                const return_t &child) const {
        if (move.color == BLACK) {
            parent = std::max(parent, child);
            alpha = std::max(alpha, parent);
        } else {
            parent = std::min(parent, child);
            beta = std::min(beta, parent);
        }
    }
};

template <pos_t size> struct PV : Minimax<size> {
    struct Node {
        Node(int minimax) : move(EMPTY), minimax(minimax) {}
        Move move;
        int minimax;
        std::vector<Node> children;

        std::vector<Node> get_shortest_path() const {
            return get_path(INT_MAX, [](int a, int b) { return a < b; });
        }
        std::vector<Node> get_longest_path() const {
            return get_path(INT_MIN, [](int a, int b) { return a > b; });
        }
        std::vector<Node> get_path(int init, std::function<bool(int, int)> compare) const {
            if (children.size() == 0)
                return std::vector<Node>{*this};

            int best_size = init;
            std::vector<Node> best_child;
            for (size_t i = 0; i < children.size(); i++) {
                auto child = children[i].get_path(init, compare);
                int child_size = child.size() + 1;
                if (compare(child_size, best_size)) {
                    best_size = child_size;
                    best_child = child;
                }
            }
            best_child.push_back(*this);
            return best_child;
        }
    };
    int call_count = 0;
    typedef Node return_t;
    typedef typename Minimax<size>::return_t minimax_t;
    return_t on_enter(State<size> &s, Cell color, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) {
        call_count++;
        return Minimax<size>::on_enter(s, color, alpha, beta, depth, terminal);
    }
    void on_exit(State<size> &s, Cell color, minimax_t alpha, minimax_t beta, size_t depth,
                 return_t &value) {}
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                return_t &child) const {
        child.move = move;
        if (move.color == BLACK) {
            if (child.minimax > parent.minimax) {
                parent.minimax = child.minimax;
                parent.children.clear();
            }
            if (child.minimax == parent.minimax && child.minimax > alpha && child.minimax < beta)
                parent.children.push_back(child);
            alpha = std::max(alpha, parent.minimax);
        } else {
            if (child.minimax < parent.minimax) {
                parent.minimax = child.minimax;
                parent.children.clear();
            }
            if (child.minimax == parent.minimax && child.minimax > alpha && child.minimax < beta)
                parent.children.push_back(child);
            beta = std::min(beta, parent.minimax);
        }
    }
};

template <pos_t size, typename Impl> struct TranspositionTable : public Impl {
    typedef typename Impl::return_t return_t;
    typedef typename Impl::minimax_t minimax_t;
    std::unordered_map<State<size>, return_t, StateHasher<size>> table;

    return_t on_enter(State<size> &state, Cell color, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) {
        auto ret = table.find(state);
        if (ret != table.end()) {
            terminal = true;
            return ret->second;
        }
        return Impl::on_enter(state, color, alpha, beta, depth, terminal);
    }
    void on_exit(State<size> &state, Cell color, minimax_t alpha, minimax_t beta, size_t depth,
                 return_t &value) {
        Impl::on_exit(state, color, alpha, beta, depth, value);
        table.insert({state, value});
    }
};
}

template <pos_t size, typename Impl = ab::Minimax<size>> struct AlphaBeta {
    std::vector<std::vector<Move>> moves;
    Impl impl;

    typename Impl::return_t alphabeta(State<size> &state, Cell color,
                                      typename Impl::minimax_t alpha = Impl::alpha_init(),
                                      typename Impl::minimax_t beta = Impl::beta_init(),
                                      size_t depth = 0) {
        bool terminal = false;
        auto parent = impl.on_enter(state, color, alpha, beta, depth, terminal);
        if (terminal)
            return parent;
        if (depth >= moves.size()) {
            moves.emplace_back();
            moves[depth].reserve(size + 1); // max # moves is board size + pass
        } else
            moves[depth].clear();
        state.moves(color, moves[depth]);
        for (Move move : moves[depth]) {
            state.play(move);
            auto child = alphabeta(state, color.flip(), alpha, beta, depth + 1);
            state.undo();
            impl.update(move, alpha, beta, parent, child);
            if (beta <= alpha)
                break;
        }
        impl.on_exit(state, color, alpha, beta, depth, parent);
        return parent;
    }
};
