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
int alphabeta_black(State<size> &s, int alpha, int beta);
template<pos_t size>
int alphabeta_white(State<size> &s, int alpha, int beta);

template<pos_t size>
int alphabeta_white(State<size> &s, int alpha, int beta) {
    if (s.terminal()) {
        Score ss = s.board.score();
        return ss.black - ss.white;
    }
    std::vector<Move> moves;
    moves.reserve(size + 1);
    s.moves(WHITE, moves);
    int v = INT_MAX;
    for (Move m : moves) {
        s.play(m);
        v = std::min(v, alphabeta_black(s, alpha, beta));
        s.undo();
        beta = std::min(beta, v);
        if (beta <= alpha)
            break;
    }
    return v;
}
template<pos_t size>
int alphabeta_black(State<size> &s, int alpha, int beta) {
    if (s.terminal()) {
        Score ss = s.board.score();
        return ss.black - ss.white;
    }
    std::vector<Move> moves;
    moves.reserve(size + 1);
    s.moves(BLACK, moves);
    int v = INT_MIN;
    for (Move m : moves) {
        s.play(m);
        v = std::max(v, alphabeta_white(s, alpha, beta));
        s.undo();
        alpha = std::max(alpha, v);
        if (beta <= alpha)
            break;
    }
    return v;
}

int main() {
    State<6> s;
    std::cout << "minimax " << alphabeta_black(s, INT32_MIN, INT32_MAX) << std::endl;
}
