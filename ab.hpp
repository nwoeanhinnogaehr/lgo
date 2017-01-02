#include "lgo.hpp"
#include <algorithm>
#include <climits>
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

struct PVNode {
    PVNode(int minimax) : move(EMPTY), minimax(minimax) {}
    Move move;
    int minimax;
    std::vector<PVNode> children;
};

int greater(int a, int b) { return a > b; }
int less(int a, int b) { return a < b; }

template <pos_t size> struct AlphaBeta {
    std::vector<std::vector<Move>> moves;
    size_t state_visits = 0;
    std::unordered_map<Board<size>, int, BoardHasher<size>> board_visits;
    bool construct_tree = false;

    PVNode alphabeta(State<size> &s, Cell color, int alpha = -size, int beta = size,
                     size_t depth = 0) {
        state_visits++;
        board_visits[s.board]++;
        if (s.terminal()) {
            Score ss = s.board.score();
            return PVNode(int(ss.black) - int(ss.white));
        }
        if (depth >= moves.size()) {
            moves.emplace_back();
            moves[depth].reserve(size + 1);
        } else
            moves[depth].clear();
        s.moves(color, moves[depth]);
        auto compare = color == BLACK ? greater : less;
        int &param = color == BLACK ? alpha : beta;
        PVNode pv(param);
        for (Move m : moves[depth]) {
            s.play(m);
            PVNode child = alphabeta(s, color.flip(), alpha, beta, depth + 1);
            s.undo();
            child.move = m;
            if (construct_tree) {
                if (compare(child.minimax, pv.minimax)) {
                    pv.minimax = child.minimax;
                    pv.children.clear();
                    pv.children.push_back(child);
                } else if (child.minimax == pv.minimax)
                    pv.children.push_back(child);
            } else if (compare(child.minimax, pv.minimax))
                pv.minimax = child.minimax;
            if (compare(pv.minimax, param))
                param = pv.minimax;
            if (beta <= alpha)
                break;
        }
        return pv;
    }
};
