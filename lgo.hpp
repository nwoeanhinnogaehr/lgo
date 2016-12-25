#include <cstdint>
#include <unordered_set>

// with these settings it works up to size 16
typedef uint32_t pos_t;         // increase the size of this for larger boards
constexpr pos_t CELL_WIDTH = 2; // number of bits per cell

constexpr pos_t CELL_MAX = (1 << CELL_WIDTH) - 1;          // max value of a cell
constexpr pos_t MAX_SIZE = sizeof(pos_t) * 8 / CELL_WIDTH; // max board size

enum Cell : pos_t { EMPTY = 0, BLACK = 1, WHITE = 2 };

struct Move {
    Move(Cell color, pos_t position, bool pass) : color(color), position(position), pass(pass) {}
    Cell color;
    pos_t position;
    bool pass;
};

struct Board {
    pos_t board = 0;
    inline Cell get(const pos_t pos) const { return Cell(board >> pos * CELL_WIDTH & CELL_MAX); }
    inline void set(const pos_t pos, const Cell color) {
        board ^= board & CELL_MAX << pos * CELL_WIDTH ^ color << pos * CELL_WIDTH;
    }
    bool operator==(const Board o) const { return o.board == board; }
};

struct BoardHasher {
    size_t operator()(const Board b) const { return b.board; }
};

struct History {
    std::unordered_set<Board, BoardHasher> states;
    void add(const Board s) { states.insert(s); }
    bool check(const Board s) const { return states.find(s) != states.end(); }
};

struct State {
    Board board;
    History history;
    enum GameState { NORMAL, PASS, GAME_OVER } game_state = NORMAL;
    void play(const Move move) {
        if (move.pass) {
            if (game_state == NORMAL)
                game_state = PASS;
            else if (game_state == PASS)
                game_state = GAME_OVER;
        } else {
            board.set(move.position, move.color);
            history.add(board);
            game_state = NORMAL;
        }
    }
    bool terminal() const { return game_state == GAME_OVER; }
};
