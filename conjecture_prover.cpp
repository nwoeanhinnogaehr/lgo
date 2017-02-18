#include "conjecture_prover.hpp"

int main() {
    constexpr pos_t size = 6;
    ConjectureProver<size, Minimax<size>, AlphaBeta, Cell2Conjecture> solver;
    State<size> root;
    solver.search(root);
}
