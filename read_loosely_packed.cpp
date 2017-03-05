#include "ab.hpp"
constexpr pos_t size = 8;
int main() {
    State<size> state;
    while (true) {
        std::string s;
        std::cin >> s;
        if (s == "")
            break;
        if (tolower(s[0]) == 'b')
            state.play(Move(BLACK, std::stoi(s.substr(1))-1));
        else if (tolower(s[0]) == 'w')
            state.play(Move(WHITE, std::stoi(s.substr(1))-1));
        else {
            std::cout << state.board << " eval " << state.board.minimax() << " got " << s << std::endl;
            if (state.board.minimax() != std::stoi(s))
                std::cout << "MONSTER\n";
            state = State<size>();
        }
    }

}
