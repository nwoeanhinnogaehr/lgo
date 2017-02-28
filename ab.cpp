#include "ab.hpp"
#include "cmdparser.hpp"
#include "conjectures.hpp"
#include <iostream>

constexpr pos_t size = 7;

void configure_parser(cli::Parser &parser) {
    parser.set_optional<int>("a", "alpha", -size, "Initial alpha value");
    parser.set_optional<int>("b", "beta", size, "Initial beta value");
    parser.set_optional<int>("g", "guess", -100000,
                             "Minimax guess. Overrides alpha and beta options.");
    parser.set_optional<std::vector<std::string>>(
        "", "state", std::vector<std::string>(),
        "Moves in the form {color}{position}, where color is b or w");
}

int main(int argc, char **argv) {
    cli::Parser parser(argc, argv);
    configure_parser(parser);
    parser.run_and_exit_if_error();

    using Impl = NewickTree<size, Metrics<size, conjectures::All<size, PV<size>>>>;
    IterativeDeepening<size, AlphaBeta, Impl> ab;
    State<size> root;

    std::vector<std::string> state = parser.get<std::vector<std::string>>("");
    for (auto move : state) {
        char player = tolower(move[0]);
        int pos = std::stoi(move.substr(1));
        root.play(Move(player == 'b' ? BLACK : WHITE, pos - 1));
    }
    ab.callback = [&](auto val) {
        std::cout << "cutoff=" << ab.impl.impl.cutoff << "\tminimax=" << val.minimax
                  << "\tsearched=" << ab.impl.impl.num_nodes << "\n";
        PV<size>::print_path(val.get_path(), root);
        /*for (auto e : ab.impl.impl.bmtable) {
            Board<size> board = e.first;
            std::cout << board << ", ";
            for (int i = -(int)size; i <= (int)size; i++) {
                std::cout << e.second[i] << ", ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";*/
        ab.impl.impl.bmtable.clear();
    };

    int alpha = parser.get<int>("a"), beta = parser.get<int>("b");
    if (alpha > beta) {
        std::cout << "invalid bounds!" << std::endl;
        return 0;
    }
    int guess = parser.get<int>("g");
    if (guess != -100000) {
        alpha = guess - 1;
        beta = guess + 1;
    }
    auto node = ab.search(root, alpha, beta);
    std::cout << "\nminimax " << node.minimax << "\n";
}
