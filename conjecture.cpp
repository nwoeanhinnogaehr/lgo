#include "conjecture.hpp"

int main() {
    constexpr pos_t size = 6;
    ConjectureProver<size, Minimax<size>, AlphaBeta, NullPruningConjecture> solver;
    State<size> root;
    solver.search(root);
}
