#include "lgo.hpp"
#include "player.hpp"
#include <algorithm>
#include <climits>
#include <functional>
#include <memory>
#include <unordered_map>

enum class NodeType { NIL, PV, MIN, MAX };

template <pos_t size> struct Minimax {
    struct Node {
        Node(int minimax) : minimax(minimax) {}
        NodeType type = NodeType::NIL;
        bool exact = true;
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
            return return_t(state.board.minimax());
        }
        return Node(state.to_play == BLACK ? -size : size);
    }
    void on_exit(const State<size> &s, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value) const {}
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                const return_t &child) const {
        if (child.minimax > alpha && child.minimax < beta) {
            parent.exact &= child.exact;
            parent.type = NodeType::PV;
        }
        if (move.color == BLACK && child.minimax >= parent.minimax) {
            parent.minimax = std::max(parent.minimax, child.minimax);
            alpha = std::max(alpha, parent.minimax);
            parent.exact &= child.exact;
            if (parent.type != NodeType::PV)
                parent.type = NodeType::MIN;
        }
        if (move.color == WHITE && child.minimax <= parent.minimax) {
            parent.minimax = std::min(parent.minimax, child.minimax);
            beta = std::min(beta, parent.minimax);
            parent.exact &= child.exact;
            if (parent.type != NodeType::PV)
                parent.type = NodeType::MAX;
        }
    }
    void gen_moves(const State<size> &state, std::vector<Move> &moves) const {
        GoodPlayer<size> player(state);
        player.moves(state.to_play, moves);
    }
};

template <pos_t size, typename Impl = Minimax<size>> struct PV : Impl {

    static State<size> print_path(std::vector<Move> path, State<size> root) {
        for (Move &m : path) {
            if (m.color == EMPTY)
                std::cout << "init:\t";
            else if (m.is_pass)
                std::cout << m.color << " pass:\t";
            else
                std::cout << m.color << " " << m.position << ":\t";

            if (m.color != EMPTY && !m.is_pass) {
                root.play(m);
            }
            std::cout << root.board << std::endl;
        }
        return root;
    }

    struct Node : Impl::Node {
        Node(typename Impl::return_t minimax) : Impl::return_t(minimax), move(EMPTY) {}
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
    typedef typename Impl::minimax_t minimax_t;
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                return_t &child) const {
        child.move = move;
        if (child.minimax > alpha && child.minimax < beta) {
            parent.child = std::make_shared<Node>(child);
        }
        Impl::update(move, alpha, beta, parent, child);
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
            while (depth >= moves.size()) {
                moves.emplace_back();
                moves[depth].reserve(size + 1); // max # moves is board size + pass
            }
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
        Board<size> board;
        History<size> history;
        typename State<size>::GameState game_state;
        size_t hash;
        T val;
        bool valid = false;
    };
    static constexpr size_t SIZE = 1 << 16;
    StateHasher<size> hasher;
    Entry *table;

    TranspositionTable() { table = new Entry[SIZE]; }
    ~TranspositionTable() { delete[] table; }
    void insert(const State<size> &state, const T &val) {
        size_t hash = hasher(state);
        Entry &e = table[hash % SIZE];
        e.hash = hash;
        e.board = state.board;
        e.history = state.history;
        e.game_state = state.game_state;
        e.val = val;
        e.valid = true;
    }
    T *lookup(const State<size> &state) {
        size_t hash = hasher(state);
        Entry *e = &table[hash % SIZE];
        if (e->valid && e->hash == hash && e->board == state.board &&
            e->game_state == state.game_state && e->history == state.history)
            return &e->val;
        return nullptr;
    }
};

template <pos_t size, template <pos_t, typename> typename ABImpl, typename Impl = PV<size>>
struct IterativeDeepening {
    struct Node : Impl::return_t {
        Node(typename Impl::return_t impl) : Impl::return_t(impl), best_move(EMPTY) {}
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

        static constexpr minimax_t alpha_init() { return -size; }
        static constexpr minimax_t beta_init() { return size; }

        return_t on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                          bool &terminal) {
            return_t implv = return_t(Impl::on_enter(state, alpha, beta, depth, terminal));
            if (terminal)
                return implv;

            if (depth >= cutoff) { // hit max depth, return heuristic score
                terminal = true;
                return heuristic_score(state.board.minimax());
            }
            if (state.terminal()) { // hit terminal state, return true score
                terminal = true;
                return true_score(state.board.minimax());
            }
            // hit true transposition table entry, return score
            entry = tt.lookup(state);
            if (entry && entry->node.exact) {
                if (entry->node.type == NodeType::PV) {
                    terminal = true;
                    return entry->node;
                } else if (entry->node.type == NodeType::MIN) {
                    alpha = std::max(alpha, entry->node.minimax);
                    return entry->node;
                } else if (entry->node.type == NodeType::MAX) {
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
    std::function<void(typename ImplWrapper::return_t)> callback;
    typename ImplWrapper::return_t
    search(State<size> &state, typename ImplWrapper::minimax_t alpha = Impl::alpha_init(),
           typename ImplWrapper::minimax_t beta = Impl::beta_init(), size_t depth = 0) {
        impl.impl.cutoff = 0;
        while (true) {
            auto val = impl.search(state, alpha, beta, depth);
            if (callback)
                callback(val);
            if (val.exact)
                return val;
            impl.impl.cutoff += 1;
        }
    }
};
template <pos_t size, typename Impl = PV<size>>
using IterativeDeepeningAlphaBeta = IterativeDeepening<size, AlphaBeta, Impl>;

// throw random info you need in here
template <pos_t size, typename Impl> struct Metrics : Impl {
    std::unordered_map<Board<size>, std::unordered_map<int, int>, BoardHasher<size>> bmtable;
    size_t num_nodes = 0;

    typedef typename Impl::return_t return_t;
    typedef typename Impl::minimax_t minimax_t;

    void on_exit(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value) {
        num_nodes++;
        if (value.type == NodeType::PV && value.exact)
            bmtable[state.board][value.minimax]++;
        Impl::on_exit(state, alpha, beta, depth, value);
    }
};
