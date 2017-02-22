#pragma once

#include "lgo.hpp"
#include "player.hpp"
#include <algorithm>
#include <array>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>

enum class NodeType { NIL, PV, MIN, MAX };
std::ostream &operator<<(std::ostream &os, const NodeType type) {
    const char *names[] = {"NIL", "PV", "Min", "Max"};
    os << names[size_t(type)];
    return os;
}

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

    return_t init_node(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                       bool &terminal) const {
        if ((terminal = state.terminal())) {
            return return_t(state.board.minimax());
        }
        return Node(state.to_play == BLACK ? alpha : beta);
    }
    void on_enter(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth) const {}
    void on_exit(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                 const return_t &value, bool terminal) const {}
    void pre_update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent, size_t depth,
                    size_t index) const {}
    void update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent,
                const return_t &child) const {
        if (move.color == BLACK) {
            if (child.minimax == alpha || parent.minimax == child.minimax)
                parent.exact |= child.exact;
            if (std::max(child.minimax, parent.minimax) > alpha || child.minimax > parent.minimax)
                parent.exact = child.exact;
            parent.minimax = std::max(parent.minimax, child.minimax);
            alpha = std::max(alpha, parent.minimax);
        }
        if (move.color == WHITE) {
            if (child.minimax == beta || parent.minimax == child.minimax)
                parent.exact |= child.exact;
            if (std::min(child.minimax, parent.minimax) < beta || child.minimax < parent.minimax)
                parent.exact = child.exact;
            parent.minimax = std::min(parent.minimax, child.minimax);
            beta = std::min(beta, parent.minimax);
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
        Impl::update(move, alpha, beta, parent, child);
        if (child.type == NodeType::PV)
            parent.child = std::make_shared<Node>(child);
    }
    void on_exit(const State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                 return_t &value, bool terminal) {
        Impl::on_exit(state, alpha, beta, depth, value, terminal);
    }
};

size_t murmur(size_t key) {
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;
    return key;
}

template <pos_t size, typename Impl = Minimax<size>> struct AlphaBeta {
    std::vector<std::vector<Move>> moves;
    Impl impl;
    static constexpr size_t ORDER_TABLE_SIZE = 1 << 22;
    struct compare {
        bool operator()(std::pair<size_t, Move> a, std::pair<size_t, Move> b) {
            return a.first < b.first;
        }
    };
    std::vector<std::set<std::pair<size_t, Move>, compare>> order_table =
        std::vector<std::set<std::pair<size_t, Move>, compare>>(ORDER_TABLE_SIZE);
    size_t node_count = 0;

    typename Impl::return_t search(State<size> &state,
                                   typename Impl::minimax_t alpha = Impl::alpha_init(),
                                   typename Impl::minimax_t beta = Impl::beta_init(),
                                   size_t depth = 0) {
        node_count++;
        typename Impl::minimax_t ab = alpha, bb = beta;
        bool terminal = false;
        auto parent = impl.init_node(state, alpha, beta, depth, terminal);
        if (terminal) {
            if (parent.minimax > ab && parent.minimax < bb)
                parent.type = NodeType::PV;
            else if (parent.minimax >= bb)
                parent.type = NodeType::MAX;
            else if (parent.minimax <= ab)
                parent.type = NodeType::MIN;
            impl.on_exit(state, alpha, beta, depth, parent, true);
            return parent;
        }
        impl.on_enter(state, alpha, beta, depth);
        if (depth >= moves.size()) {
            while (depth >= moves.size()) {
                moves.emplace_back();
                moves[depth].reserve(size + 1); // max # moves is board size + pass
            }
        } else
            moves[depth].clear();

        auto &order = order_table[(murmur(state.board.board) ^ murmur(depth)) % ORDER_TABLE_SIZE];
        for (auto &p : order) {
            moves[depth].emplace_back(p.second);
        }
        order.clear();
        impl.gen_moves(state, moves[depth]);
        bool all_exact = true;
        if (beta > alpha) {
            size_t index = 0;
            for (Move move : moves[depth]) {
                impl.pre_update(move, alpha, beta, parent, depth, index);
                state.play(move);
                size_t size_before = node_count;
                auto child = search(state, alpha, beta, depth + 1);
                size_t subtree_size = node_count - size_before;
                all_exact &= child.exact;
                state.undo();
                if (child.exact) {
                    impl.update(move, alpha, beta, parent, child);
                    auto &order =
                        order_table[(murmur(state.board.board) ^ murmur(depth)) % ORDER_TABLE_SIZE];
                    order.emplace(subtree_size, move);
                }
                if (beta <= alpha && child.exact) {
                    all_exact = parent.exact;
                    break;
                }
                index++;
            }
        }
        if (parent.minimax > ab && parent.minimax < bb)
            parent.type = NodeType::PV;
        else if (parent.minimax >= bb)
            parent.type = NodeType::MAX;
        else if (parent.minimax <= ab)
            parent.type = NodeType::MIN;
        parent.exact = all_exact;
        impl.on_exit(state, alpha, beta, depth, parent, false);
        if (depth == 0)
            node_count = 0;
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
        return;
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
        return nullptr;
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
        static constexpr minimax_t alpha_init() { return Impl::alpha_init(); }
        static constexpr minimax_t beta_init() { return Impl::beta_init(); }

        return_t init_node(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                           bool &terminal) {
            return_t implv = return_t(Impl::init_node(state, alpha, beta, depth, terminal));
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
                } else if (entry->node.type == NodeType::MAX) {
                    alpha = std::max(alpha, entry->node.minimax);
                    return entry->node;
                } else if (entry->node.type == NodeType::MIN) {
                    beta = std::min(beta, entry->node.minimax);
                    return entry->node;
                }
            }
            return true_score(state.to_play == BLACK ? alpha_init() : beta_init());
        }
        void on_exit(const State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                     return_t &value, bool terminal) {
            if (value.exact)
                tt.insert(state, TTEntry(value));
            Impl::on_exit(state, alpha, beta, depth, value, terminal);
        }
        void gen_moves(const State<size> &state, std::vector<Move> &moves) {
            // init with best move from TT entry
            // except make sure pass is always first
            // moves.emplace_back(state.to_play);
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
        impl.impl.cutoff = 1;
        typename ImplWrapper::minimax_t last_minimax = 0;
        last_minimax = std::max(alpha + 1, std::min(beta - 1, last_minimax));
        std::srand(unsigned(std::time(0)));
        while (true) {
            std::cout << "Doing aspiration search around " << last_minimax << std::endl;
            auto aspiration = impl.search(state, last_minimax - 1, last_minimax + 1, depth);
            std::cout << "Result type: " << (aspiration.exact ? "exact " : "inexact ")
                      << aspiration.type << std::endl;
            if (callback)
                callback(aspiration);
            if (aspiration.exact && aspiration.type == NodeType::PV)
                return aspiration;
            if (aspiration.exact && aspiration.type == NodeType::MIN)
                beta = std::min(beta, aspiration.minimax);
            if (aspiration.exact && aspiration.type == NodeType::MAX)
                alpha = std::max(alpha, aspiration.minimax);
            last_minimax = aspiration.minimax;
            last_minimax = std::max(alpha, std::min(beta, last_minimax));
            impl.impl.cutoff += 1;
            if (aspiration.type == NodeType::PV)
                continue;
            if (alpha < beta - 2) {
                std::cout << std::endl;
                std::cout << "Doing full search in [" << alpha << ", " << beta << "]" << std::endl;
                auto val = impl.search(state, alpha, beta, depth);
                std::cout << "Result type: " << (val.exact ? "exact " : "inexact ") << val.type
                          << std::endl;
                if (callback)
                    callback(val);
                if (val.exact && val.type == NodeType::PV)
                    return val;
                if (val.exact && val.type == NodeType::MIN)
                    beta = std::min(beta, val.minimax);
                if (val.exact && val.type == NodeType::MAX)
                    alpha = std::max(alpha, val.minimax);
                last_minimax = val.minimax;
                last_minimax = std::max(alpha, std::min(beta, last_minimax));
                impl.impl.cutoff += 1;
            }
            std::cout << std::endl;
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

    return_t init_node(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                       bool &terminal) {
        return Impl::init_node(state, alpha, beta, depth, terminal);
    }
    void on_exit(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                 return_t &value, bool terminal) {
        num_nodes++;
        if (value.type == NodeType::PV && value.exact)
            bmtable[state.board][value.minimax]++;
        Impl::on_exit(state, alpha, beta, depth, value, terminal);
    }
};

template <pos_t size, typename Impl> struct NewickTree : Impl {
    typedef typename Impl::return_t return_t;
    typedef typename Impl::minimax_t minimax_t;

    size_t tree_depth_cutoff = 4;
    std::string output_filename = "searchtree.nhx";

    std::stringstream output;
    std::stack<bool> need_close;
    size_t node_count = 0;
    std::stack<size_t> size_before;
    std::stack<size_t> max_depth;
    std::stack<minimax_t> alpha_before, beta_before;

    return_t init_node(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth,
                       bool &terminal) {
        if (depth == 0)
            max_depth.push(0);
        max_depth.push(depth);
        alpha_before.push(alpha);
        beta_before.push(beta);
        size_before.push(node_count);
        node_count++;
        return Impl::init_node(state, alpha, beta, depth, terminal);
    }
    void on_enter(State<size> &state, minimax_t &alpha, minimax_t &beta, size_t depth) {
        if (depth < tree_depth_cutoff && beta > alpha) {
            output << "(";
            need_close.push(true);
        } else
            need_close.push(false);
        Impl::on_enter(state, alpha, beta, depth);
    }
    void pre_update(Move move, minimax_t &alpha, minimax_t &beta, return_t &parent, size_t depth,
                    size_t index) {
        if (depth < tree_depth_cutoff && index != 0)
            output << ",";
    }
    void on_exit(const State<size> &state, minimax_t alpha, minimax_t beta, size_t depth,
                 return_t &value, bool terminal) {
        size_t last_depth = max_depth.top();
        size_t subtree_size = node_count - size_before.top();
        size_before.pop();
        minimax_t prev_alpha = alpha_before.top();
        alpha_before.pop();
        minimax_t prev_beta = beta_before.top();
        beta_before.pop();
        if (!terminal) {
            if (need_close.top())
                output << ")";
            need_close.pop();
        }
        if (depth <= tree_depth_cutoff) {
            output << state.board;
            output << "[&&NHX";
            output << ":minimax=" << value.minimax;
            output << ":type=" << value.type;
            output << ":exact=" << value.exact;
            output << ":alpha=" << prev_alpha << "->" << alpha;
            output << ":beta=" << prev_beta << "->" << beta;
            output << ":subtree_size=" << subtree_size;
            output << ":max_depth=" << last_depth;
            output << ":to_play=" << state.to_play;
            output << "]";
        }
        max_depth.pop();
        size_t &parent_depth = max_depth.top();
        parent_depth = std::max(parent_depth, last_depth);

        if (depth == 0) {
            output << ";" << std::endl;
            std::ofstream outfile;
            outfile.open(output_filename, std::ios::out | std::ios::trunc);
            outfile << output.str();
            outfile.close();
            output.str(std::string());
            output.clear();
            node_count = 0;
        }
        Impl::on_exit(state, alpha, beta, depth, value, terminal);
    }
};
