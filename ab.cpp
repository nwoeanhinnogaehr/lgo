#include "lgo.hpp"
#include <algorithm>
#include <iostream>
#include <climits>

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

template<pos_t size>
int alphabeta_black(State<size> &s, int alpha, int beta, int depth);
template<pos_t size>
int alphabeta_white(State<size> &s, int alpha, int beta, int depth);

int term;
std::vector<std::vector<Move>> moves;
template<pos_t size>
int alphabeta_white(State<size> &s, int alpha, int beta, int depth) {
    if (s.terminal()) {
        term++;
        Score ss = s.board.score();
        return ss.black - ss.white;
    }
    if (depth >= moves.size()) {
        moves.emplace_back();
        moves[depth].reserve(size + 1);
    } else {
        moves[depth].clear();
    }
    s.moves(WHITE, moves[depth]);
    int v = INT_MAX;
    for (Move m : moves[depth]) {
        s.play(m);
        v = std::min(v, alphabeta_black(s, alpha, beta, depth + 1));
        s.undo();
        beta = std::min(beta, v);
        if (beta <= alpha)
            break;
    }
    return v;
}
template<pos_t size>
int alphabeta_black(State<size> &s, int alpha, int beta, int depth) {
    if (s.terminal()) {
        term++;
        Score ss = s.board.score();
        return ss.black - ss.white;
    }
    if (depth >= moves.size()) {
        moves.emplace_back();
        moves[depth].reserve(size + 1);
    } else {
        moves[depth].clear();
    }
    s.moves(BLACK, moves[depth]);
    int v = INT_MIN;
    for (Move m : moves[depth]) {
        s.play(m);
        v = std::max(v, alphabeta_white(s, alpha, beta, depth + 1));
        s.undo();
        alpha = std::max(alpha, v);
        if (beta <= alpha)
            break;
    }
    return v;
}

int main() {
    State<6> s;
    std::cout << "minimax " << alphabeta_black(s, INT32_MIN, INT32_MAX, 0) << std::endl;
    std::cout << term << " terminal states" << std::endl;
}
