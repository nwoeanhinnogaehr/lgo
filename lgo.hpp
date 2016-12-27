#include <cassert>
#include <cstdint>
#include <unordered_set>

// with these settings it works up to size 16
typedef uint32_t pos_t;         // increase the size of this for larger boards
constexpr pos_t CELL_WIDTH = 2; // number of bits per cell

constexpr pos_t CELL_MAX = (1 << CELL_WIDTH) - 1;          // max value of a cell
constexpr pos_t MAX_SIZE = sizeof(pos_t) * 8 / CELL_WIDTH; // max board size

struct Cell {
    pos_t value;
    Cell(pos_t value) : value(value) {}

    Cell flip() const {
        if (value == 1)
            return Cell(2);
        if (value == 2)
            return Cell(1);
        assert(false);
    }
    bool is_stone() const { return value == 1 || value == 2; }
    bool operator==(Cell o) const { return value == o.value; }
    operator bool() const { return value; }
    friend std::ostream &operator<<(std::ostream &os, const Cell cell) {
        os << ".BW"[cell.value];
        return os;
    }
};
const Cell EMPTY = Cell(0);
const Cell BLACK = Cell(1);
const Cell WHITE = Cell(2);

struct Move {
    Cell color;
    pos_t position; // meaningless if is_pass is true
    bool is_pass;
    Move(Cell color, pos_t position, bool is_pass = false)
        : color(color), position(position), is_pass(is_pass) {}
    static Move pass(Cell color) { return Move(color, 0, true); }
};

struct Score {
    pos_t black, white;
    Score() : black(0), white(0) {}
    Score(pos_t black, pos_t white) : black(black), white(white) {}

    bool operator==(Score o) const { return o.black == black && o.white == white; }
};

// a chain represents a sequence of stones with the same cell value
struct Chain {
    Cell cell;
    pos_t position, size;
    Chain(Cell cell, pos_t position, pos_t size) : cell(cell), position(position), size(size) {}
};

struct Board {
    const pos_t size;
    pos_t board;
    Board(pos_t size) : size(size), board(0) {}

    inline Cell get(pos_t pos) const {
        assert(pos < size);
        return Cell(board >> pos * CELL_WIDTH & CELL_MAX);
    }
    inline void set(pos_t pos, Cell cell) {
        assert(pos < size);
        assert(cell.value <= CELL_MAX);
        board ^= (board & CELL_MAX << pos * CELL_WIDTH) ^ cell.value << pos * CELL_WIDTH;
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
            if ((left == BLACK && (left == right || right == EMPTY)) ||
                (right == BLACK && left == EMPTY))
                sc.black++;
            if ((left == WHITE && (left == right || right == EMPTY)) ||
                (right == WHITE && left == EMPTY))
                sc.white++;
        }
        return sc;
    }
    // returns a bitset of empty positions on the board.
    pos_t empty_set() const {
        pos_t res = 0;
        for (pos_t i = size; i--;)
            res <<= 1, res |= get(i) == EMPTY;
        return res;
    }
    // retuns a vector of all chains on the board, ordered.
    std::vector<Chain> chains() const {
        std::vector<Chain> chains{Chain(get(0), 0, 1)};
        for (pos_t i = 1; i < size; i++) {
            if (get(i) == chains.back().cell)
                chains.back().size++;
            else
                chains.push_back(Chain(get(i), i, 1));
        }
        return chains;
    }
    // remove all stones in a chain
    void clear_chain(Chain chain) {
        pos_t mask = (chain.position + chain.size != MAX_SIZE) * // handle overflow edge case
                         ~((1 << CELL_WIDTH * (chain.position + chain.size)) - 1) |
                     ((1 << CELL_WIDTH * chain.position) - 1);
        board &= mask;
    }
    // clear any chains captured by a recent play at the given position
    bool clear_captured(pos_t position) {
        return iter_captured(position, [this](Chain c) { clear_chain(c); });
    }
    bool iter_captured(pos_t position, std::function<void(Chain)> fn) {
        // this can probably be optimized
        assert(get(position).is_stone());
        auto ch = chains();
        bool any = false;
        for (size_t i = 0; i < ch.size(); i++) {
            if (ch[i].cell.is_stone() &&
                (position == ch[i].position - 1 || position == ch[i].position + ch[i].size)) {
                // left boundary
                if (i == 0 && i + 1 < ch.size() && ch[i + 1].cell.is_stone())
                    fn(ch[i]), any |= true;
                // right boundary
                else if (i > 0 && i + 1 == ch.size() && ch[i - 1].cell.is_stone())
                    fn(ch[i]), any |= true;
                // middle
                else if (i > 0 && i + 1 < ch.size() && ch[i - 1].cell.is_stone() &&
                         ch[i + 1].cell.is_stone() && ch[i - 1].cell == ch[i + 1].cell)
                    fn(ch[i]), any |= true;
            }
        }
        return any;
    }
    bool operator==(Board o) const { return o.board == board; }
    friend std::ostream &operator<<(std::ostream &os, Board board) {
        for (pos_t i = 0; i < board.size; i++)
            os << board.get(i);
        return os;
    }
};

struct BoardHasher {
    size_t operator()(Board b) const { return b.board; }
};

struct History {
    std::unordered_set<Board, BoardHasher> states;

    void add(Board s) { states.insert(s); }
    // returns true if the given board has previously been added
    bool check(Board s) const { return states.find(s) != states.end(); }
};

struct State {
    Board board;
    History history;
    enum GameState { NORMAL, PASS, GAME_OVER } game_state = NORMAL;
    State(pos_t size) : board(size) {}

    bool terminal() const { return game_state == GAME_OVER; }
    void play(Move move) {
        assert(game_state != GAME_OVER);
        if (move.is_pass) {
            if (game_state == NORMAL)
                game_state = PASS;
            else if (game_state == PASS)
                game_state = GAME_OVER;
        } else {
            assert(legal_moves(move.color) & 1 << move.position);
            assert(board.get(move.position) == EMPTY);
            board.set(move.position, move.color);
            board.clear_captured(move.position);
            assert(!history.check(board));
            history.add(board);
            game_state = NORMAL;
        }
    }
    // retuns a bitset of all legal moves for a given color
    pos_t legal_moves(Cell color) const {
        assert(color.is_stone());
        pos_t legal = board.empty_set();
        auto illegal = [&](pos_t i) { legal &= ~(1 << i); };

        // check history
        for (pos_t i = 0; i < board.size; i++) {
            Board b = board;
            b.set(i, color);
            b.clear_captured(i);
            if (history.check(b))
                illegal(i);
        }

        // check liberties
        auto ch = board.chains();
        for (size_t i = 0; i < ch.size(); i++) {
            if (ch[i].cell == EMPTY && ch[i].size == 1) {
                Board b = board;
                b.set(ch[i].position, color);
                bool captured = b.iter_captured(ch[i].position, [](Chain) {});

                // suicide is possible only if we didn't capture any stones
                if (!captured) {
                    // left suicide
                    if (i == 0 && i < ch.size() - 2 && ch[i + 1].cell == color.flip())
                        illegal(ch[i].position);
                    // right suicide
                    else if (i == ch.size() - 1 && i > 1 && ch[i - 1].cell == color.flip())
                        illegal(ch[i].position);
                    // middle suicide
                    else if (i > 0 && i < ch.size() - 1 && ch[i - 1].cell == color.flip() &&
                             ch[i + 1].cell == color.flip())
                        illegal(ch[i].position);
                }
            }
        }
        return legal;
    }

    // append legal moves of a specific type and color to a vector.
    // TODO
    void atari_moves(Cell color, std::vector<Move> &moves) const {}
    void cell_2_conjecture(Cell color, std::vector<Move> &moves) const {}
    void safe_moves(Cell color, std::vector<Move> &moves) const {}
    void other_moves(Cell color, std::vector<Move> &moves) const {}
    void moves(Cell color, std::vector<Move> &moves) const {
        moves.push_back(Move::pass(color));
        atari_moves(color, moves);
        cell_2_conjecture(color, moves);
        safe_moves(color, moves);
        other_moves(color, moves);
    }
};
