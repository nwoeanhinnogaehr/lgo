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

struct Score {
    Score() {}
    Score(const pos_t black, const pos_t white) : black(black), white(white) {}
    pos_t black = 0, white = 0;
    bool operator==(const Score o) const { return o.black == black && o.white == white; }
};

struct Board {
    Board(pos_t size) : size(size) {}
    Board(std::initializer_list<Cell> init) : size(init.size()) {
        for (auto it = init.begin(); it != init.end(); ++it)
            set(it - init.begin(), *it);
    }
    pos_t size;
    pos_t board = 0;
    inline Cell get(const pos_t pos) const { return Cell(board >> pos * CELL_WIDTH & CELL_MAX); }
    inline void set(const pos_t pos, const Cell color) {
        board ^= board & CELL_MAX << pos * CELL_WIDTH ^ color << pos * CELL_WIDTH;
    }
    Score score() const {
        Score sc;
        Board left_smear(size), right_smear(size);
        Cell last_left = EMPTY, last_right = EMPTY;
        for (pos_t i = 0; i < size; i++) {
            if (Cell next = get(i))
                last_right = next;
            right_smear.set(i, last_right);
            if (Cell next = get(size - i - 1))
                last_left = next;
            left_smear.set(size - i - 1, last_left);
        }
        for (pos_t i = 0; i < size; i++) {
            Cell left = left_smear.get(i), right = right_smear.get(i);
            if (left == BLACK && (left == right || right == EMPTY) ||
                right == BLACK && left == EMPTY)
                sc.black++;
            if (left == WHITE && (left == right || right == EMPTY) ||
                right == WHITE && left == EMPTY)
                sc.white++;
        }
        return sc;
    }
    bool operator==(const Board o) const { return o.board == board; }
    friend std::ostream& operator << ( std::ostream& os, const Board board ) {
        for (pos_t i = 0; i < board.size; i++)
            os << ".BW"[board.get(i)];
        return os;
    }
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
    State(pos_t size) : board(size) {}
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
