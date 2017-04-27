#include "ab.hpp"
#include "cmdparser.hpp"
#include "conjectures.hpp"
#include <iostream>
#include <thread>

constexpr pos_t size = 8;

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

    /*ab.callback = [&](auto val) {
        std::cout << "cutoff=" << ab.impl.impl.cutoff << "\tminimax=" << val.minimax
                  << "\tsearched=" << ab.impl.impl.num_nodes << "\n";
        PV<size>::print_path(val.get_path(), root);
    };*/

    int alpha = parser.get<int>("a"), beta = parser.get<int>("b");
    if (alpha > beta) {
        std::cout << "invalid bounds!" << std::endl;
        return 0;
    }
    bool searching = false;
    while (true) {
        try {
            std::cout << "> ";
            std::string line;
            if (!std::getline(std::cin, line))
                break;
            if (line[0] == 'r') {
                if (searching) {
                    std::cout << "You must halt the current search first." << std::endl;
                    continue;
                }
                std::thread([=, &ab, &searching]() {
                    try {
                        State<size> root;
                        std::istringstream iss(line.substr(1));
                        std::string move;
                        while (iss >> move) {
                            char player = tolower(move[0]);
                            int pos = std::stoi(move.substr(1));
                            root.play(Move(player == 'b' ? BLACK : WHITE, pos - 1));
                        }
                        std::cout << "Searching " << root.board << std::endl;
                        ab.impl.node_count = 0;
                        auto node = ab.search(root, alpha, beta);
                        if (!ab.impl.quit)
                            std::cout << "\n"
                                      << root.board << " minimax " << node.minimax << "\n> "
                                      << std::flush;
                    } catch (...) {
                    }
                    ab.impl.quit = false;
                    searching = false;
                }).detach();
                searching = true;
            } else if (line[0] == 'h') {
                if (!searching) {
                    std::cout << "No search to halt." << std::endl;
                } else {
                    std::cout << "Halting..." << std::endl;
                    ab.impl.quit = true;
                }
            } else if (line[0] == 'i') {
                std::cout << "nodes searched=" << ab.impl.impl.num_nodes << std::endl;
                std::cout << "best guess=" << ab.last_result.minimax << std::endl;
            } else {
                std::cout << "commands:\n"
                             "\t?\t\t\t :: show this help\n"
                             "\tr move1 move2 ...\t :: run search\n"
                             "\th\t\t\t :: halt current search\n"
                             "\ti\t\t\t :: print info about current search\n";
            }
        } catch (...) {
        }
    }
}
