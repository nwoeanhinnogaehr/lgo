#include "lgo.hpp"
#include <algorithm>
#include <climits>
#include <functional>
#include <memory>
#include <unordered_map>

template <pos_t size> struct Minimax {
    struct Node {
        Node(int minimax) : minimax(minimax) {}
        int minimax;
        bool operator==(Node o) const { return o.minimax == minimax; }
    };
    typedef Node return_t;
    typedef int minimax_t;
    static constexpr minimax_t alpha_init() { return -size; }
    static constexpr minimax_t beta_init() { return size; }
    return_t on_enter(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) const {
        if ((terminal = state.terminal())) {
            Score ss = state.board.score();
            return return_t(minimax_t(ss.black) - minimax_t(ss.white));
        }
        return Node(state.to_play == BLACK ? -size : size);
    }
    void on_exit(const State<size> &s, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value) const {}
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                const return_t &child) const {
        if (move.color == BLACK) {
            parent.minimax = std::max(parent.minimax, child.minimax);
            alpha = std::max(alpha, parent.minimax);
        } else {
            parent.minimax = std::min(parent.minimax, child.minimax);
            beta = std::min(beta, parent.minimax);
        }
    }
    void gen_moves(const State<size> &state, std::vector<Move> &moves) const {
        state.moves(state.to_play, moves);
    }
};

template <pos_t size> struct PV : Minimax<size> {
    struct Node : Minimax<size>::Node {
        Node(typename Minimax<size>::return_t minimax)
            : Minimax<size>::return_t(minimax), move(EMPTY) {}
        enum Type { NIL, PV, MIN, MAX } type = NIL;
        bool exact = true;
        Move move;
        std::shared_ptr<Node> child;

        std::vector<Move> get_path() const {
            std::vector<Move> path{move};
            Node *next = child.get();
            while (next) {
                path.emplace_back(next->move);
                next = next->child.get();
            }
            return path;
        }
    };
    typedef Node return_t;
    typedef typename Minimax<size>::minimax_t minimax_t;
    return_t on_enter(const State<size> &s, minimax_t alpha, minimax_t beta, size_t depth,
                      bool &terminal) const {
        return Minimax<size>::on_enter(s, alpha, beta, depth, terminal);
    }
    void on_exit(const State<size> &s, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value) const {}
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                return_t &child) const {
        child.move = move;
        if (move.color == BLACK && child.minimax >= parent.minimax) {
            parent.exact &= child.exact;
            if (parent.type != Node::PV)
                parent.type = Node::MIN;
        }
        if (move.color == WHITE && child.minimax <= parent.minimax) {
            parent.exact &= child.exact;
            if (parent.type != Node::PV)
                parent.type = Node::MAX;
        }
        if (child.minimax > alpha && child.minimax < beta) {
            parent.exact &= child.exact;
            parent.child = std::make_shared<Node>(child);
            parent.type = Node::PV;
        }
        Minimax<size>::update(move, alpha, beta, parent, child);
    }
};

template <pos_t size, typename Impl = Minimax<size>> struct AlphaBeta {
    std::vector<std::vector<Move>> moves;
    Impl impl;

    typename Impl::return_t search(State<size> &state,
                                   typename Impl::minimax_t alpha = Impl::alpha_init(),
                                   typename Impl::minimax_t beta = Impl::beta_init(),
                                   size_t depth = 0) {
        bool terminal = false;
        auto parent = impl.on_enter(state, alpha, beta, depth, terminal);
        if (terminal)
            return parent;
        if (depth >= moves.size()) {
            moves.emplace_back();
            moves[depth].reserve(size + 1); // max # moves is board size + pass
        } else
            moves[depth].clear();
        impl.gen_moves(state, moves[depth]);
        if (beta > alpha)
            for (Move move : moves[depth]) {
                state.play(move);
                auto child = search(state, alpha, beta, depth + 1);
                state.undo();
                impl.update(move, alpha, beta, parent, child);
                if (beta <= alpha)
                    break;
            }
        impl.on_exit(state, alpha, beta, depth, parent);
        return parent;
    }
};

template <pos_t size, typename T> struct TranspositionTable {
    struct Entry {
        State<size> state;
        T val;
        bool valid = false;
    };
    static constexpr size_t SIZE = 1 << 20;
    StateHasher<size> hasher;
    Entry *table;

    TranspositionTable() { table = new Entry[SIZE]; }
    ~TranspositionTable() { delete[] table; }
    void insert(const State<size> &state, const T &val) {
        Entry &e = table[hasher(state) % SIZE];
        e.state = state;
        e.val = val;
        e.valid = true;
    }
    T *lookup(const State<size> &state) {
        Entry *e = &table[hasher(state) % SIZE];
        if (e->valid && e->state == state)
            return &e->val;
        return nullptr;
    }
};

template <pos_t size, template <pos_t, typename> typename ABImpl, typename Impl = PV<size>>
struct IterativeDeepening {
    struct Node : Impl::return_t {
        Node(typename Impl::minimax_t impl) : Impl::return_t(impl), best_move(EMPTY) {}
        Move best_move;
    };
    struct TTEntry {
        TTEntry() : node(0) {}
        TTEntry(Node node) : node(node) {}
        Node node;
    };

    static constexpr Node true_score(typename Impl::minimax_t value) {
        Node v(value);
        v.exact = true;
        return v;
    }
    static constexpr Node heuristic_score(typename Impl::minimax_t value) {
        Node v(value);
        v.exact = false;
        return v;
    }

    struct ImplWrapper : public Impl {
        typedef Node return_t;
        typedef typename Impl::minimax_t minimax_t;

        TranspositionTable<size, TTEntry> tt;
        TTEntry *entry = nullptr;
        size_t cutoff = 0;
        int searched = 0;

        static constexpr minimax_t alpha_init() { return -size; }
        static constexpr minimax_t beta_init() { return size; }

        return_t on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                          bool &terminal) {
            // NOTE: does not call Impl::on_enter
            searched++;
            if (depth >= cutoff) { // hit max depth, return heuristic score
                terminal = true;
                Score ss = state.board.score();
                return heuristic_score(minimax_t(ss.black) - minimax_t(ss.white));
            }
            if (state.terminal()) { // hit terminal state, return true score
                terminal = true;
                Score ss = state.board.score();
                return true_score(minimax_t(ss.black) - minimax_t(ss.white));
            }
            // hit true transposition table entry, return score
            entry = tt.lookup(state);
            if (entry && entry->node.exact) {
                if (entry->node.type == Node::PV) {
                    terminal = true;
                    return entry->node;
                } else if (entry->node.type == Node::MIN) {
                    alpha = std::max(alpha, entry->node.minimax);
                    return entry->node;
                } else if (entry->node.type == Node::MAX) {
                    beta = std::min(beta, entry->node.minimax);
                    return entry->node;
                }
            }
            return true_score(state.to_play == BLACK ? alpha_init() : beta_init());
        }
        void on_exit(const State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                     const return_t &value) {
            if (value.exact)
                tt.insert(state, TTEntry(value));
            Impl::on_exit(state, alpha, beta, depth, value);
        }
        void gen_moves(const State<size> &state, std::vector<Move> &moves) {
            // init with best move from TT entry
            // except make sure pass is always first
            moves.emplace_back(state.to_play);
            if (entry)
                moves.emplace_back(entry->node.best_move);

            entry = nullptr; // this pointer may be invalidated by insertion of child nodes, null it
                             // for safety
            Impl::gen_moves(state, moves);
        }
        void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                    return_t &child) const {
            parent.best_move = move;
            Impl::update(move, alpha, beta, parent, child);
        }
    };

    ABImpl<size, ImplWrapper> impl;
    typename ImplWrapper::return_t
    search(State<size> &state, typename ImplWrapper::minimax_t alpha = Impl::alpha_init(),
           typename ImplWrapper::minimax_t beta = Impl::beta_init(), size_t depth = 0) {
        while (true) {
            impl.impl.searched = 0;
            auto val = impl.search(state, alpha, beta, depth);
            std::cout << "cutoff=" << impl.impl.cutoff << "\tminimax=" << val.minimax
                      << "\tsearched=" << impl.impl.searched << std::endl;
            if (val.exact)
                return val;
            impl.impl.cutoff += 1;
        }
    }
};
template <pos_t size, typename Impl = PV<size>>
struct IterativeDeepeningAlphaBeta : IterativeDeepening<size, AlphaBeta, Impl> {};
