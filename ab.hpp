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

bool greater(int a, int b) { return a > b; }
bool less(int a, int b) { return a < b; }

// Since constructing the whole principal variation tree is so much slower than just finding the
// minimax value, two seperate node implementations are here so you can avoid redundant computation.
// It's not really ideal, but it works for now. to be improved later.
struct PVNode {
    PVNode(int minimax) : move(EMPTY), minimax(minimax) {}
    Move move;
    int minimax;
    std::vector<PVNode> children;

    void set_move(Move m) { move = m; }
    Move get_move() const { return move; }
    void set_minimax(int val) { minimax = val; }
    int get_minimax() const { return minimax; }
    void add_child(PVNode &child) { children.emplace_back(child); }
    void clear_children() { children.clear(); }

    std::vector<PVNode> get_shortest_path() { return get_path(INT_MAX, less); }
    std::vector<PVNode> get_longest_path() { return get_path(INT_MIN, greater); }
    std::vector<PVNode> get_path(int init, std::function<bool(int, int)> compare) {
        if (children.size() == 0)
            return std::vector<PVNode>{*this};

        int best_size = init;
        std::vector<PVNode> best_child;
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
// SimpleNode only stores the minimax value, discarding everything else
struct SimpleNode {
    SimpleNode(int minimax) : minimax(minimax) {}
    int minimax;

    void set_move(Move m) {}
    Move get_move() const { return Move(EMPTY); }
    void set_minimax(int val) { minimax = val; }
    int get_minimax() const { return minimax; }
    void add_child(SimpleNode &child) {}
    void clear_children() {}
};

template <pos_t size, typename NodeT = SimpleNode> struct AlphaBeta {
    std::vector<std::vector<Move>> moves;
    size_t state_visits = 0;
    std::unordered_map<Board<size>, int, BoardHasher<size>> board_visits;

    NodeT alphabeta(State<size> &s, Cell color, int alpha = -size - 1, int beta = size + 1,
                    size_t depth = 0) {
        state_visits++;
        board_visits[s.board]++;
        if (s.terminal()) {
            Score ss = s.board.score();
            return NodeT(int(ss.black) - int(ss.white));
        }
        if (depth >= moves.size()) {
            moves.emplace_back();
            moves[depth].reserve(size + 1);
        } else
            moves[depth].clear();
        s.moves(color, moves[depth]);
        auto compare = color == BLACK ? greater : less;
        int &param = color == BLACK ? alpha : beta;
        NodeT pv(color == BLACK ? -size : size);
        for (Move m : moves[depth]) {
            s.play(m);
            NodeT child = alphabeta(s, color.flip(), alpha, beta, depth + 1);
            s.undo();
            child.set_move(m);
            if (compare(child.get_minimax(), pv.get_minimax())) {
                pv.set_minimax(child.get_minimax());
                pv.clear_children();
            }
            if (child.get_minimax() == pv.get_minimax() && child.get_minimax() > alpha &&
                child.get_minimax() < beta)
                pv.add_child(child);
            if (compare(pv.get_minimax(), param))
                param = pv.get_minimax();
            if (beta <= alpha)
                break;
        }
        return pv;
    }
};
