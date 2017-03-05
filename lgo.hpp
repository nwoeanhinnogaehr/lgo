#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <experimental/optional>
#include <iostream>
#include <random>
#include <stack>
#include <unordered_set>
#include <vector>

using std::experimental::optional;

// with these settings it works up to size 16
typedef uint32_t pos_t;         // increase the size of this for larger boards
constexpr pos_t CELL_WIDTH = 2; // number of bits per cell

constexpr pos_t CELL_MAX = (1 << CELL_WIDTH) - 1;          // max value of a cell
constexpr pos_t MAX_SIZE = sizeof(pos_t) * 8 / CELL_WIDTH; // max board size

// literal suffix for pos_t
constexpr pos_t operator"" _pos_t(unsigned long long v) { return v; }

struct Cell {
    pos_t value;
    Cell(pos_t value) : value(value) {}

    Cell flip() const {
        if (value == 1)
            return Cell(2);
        if (value == 2)
            return Cell(1);
        assert(false);
        return Cell(-1);
    }
    bool is_stone() const { return value == 1 || value == 2; }
    bool is_empty() const { return value == 0; }
    bool operator==(Cell o) const { return value == o.value; }
    bool operator!=(Cell o) const { return value != o.value; }
    operator bool() const { return is_stone(); }
    friend std::ostream &operator<<(std::ostream &os, const Cell cell) {
        os << ".BW*"[cell.value];
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
    Move(Cell color, pos_t position) : color(color), position(position), is_pass(false) {}
    Move(Cell color) : color(color), position(0), is_pass(true) {}
};

struct Score {
    pos_t black, white;
    Score() : black(0), white(0) {}
    Score(pos_t black, pos_t white) : black(black), white(white) {}

    pos_t for_player(Cell player) {
        if (player == WHITE)
            return white;
        if (player == BLACK)
            return black;
        assert(false);
        return -1;
    }
    bool operator==(Score o) const { return o.black == black && o.white == white; }
    bool operator!=(Score o) const { return o.black != black || o.white != white; }
};

template <pos_t size> struct Board {
    pos_t board, captured;
    Board() : board(0), captured(0) {}

    inline Cell get(pos_t pos) const {
        assert(pos < size);
        return Cell(board >> pos * CELL_WIDTH & CELL_MAX);
    }
    inline void set(pos_t pos, Cell cell) {
        assert(pos < size);
        assert(cell.value <= CELL_MAX);
        board ^= (board & CELL_MAX << pos * CELL_WIDTH) ^ cell.value << pos * CELL_WIDTH;
    }
    inline bool is_captured(pos_t pos) const {
        assert(pos < size);
        return captured >> pos & 1;
    }
    inline void set_captured(pos_t pos, bool value) {
        assert(pos < size);
        captured ^= (captured & 1_pos_t << pos) ^ value << pos;
    }
    Score score() const {
        Score sc;
        Board<size> left_smear, right_smear;
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
            if ((left == BLACK && (left == right || right.is_empty())) ||
                (right == BLACK && left.is_empty()))
                sc.black++;
            if ((left == WHITE && (left == right || right.is_empty())) ||
                (right == WHITE && left.is_empty()))
                sc.white++;
        }
        return sc;
    }
    int minimax() const {
        Score s = score();
        return int(s.black) - int(s.white);
    }
    // returns a bitset of empty positions on the board.
    pos_t empty_set() const {
        pos_t res = 0;
        for (pos_t i = size; i--;)
            res <<= 1, res |= get(i).is_empty();
        return res;
    }
    // remove all stones in range [start, end)
    void clear_chain(pos_t start, pos_t end) {
        assert(start < size);
        assert(end <= size);
        assert(start < end);

        // clear board positions
        pos_t mask = (end != MAX_SIZE) * ~((1_pos_t << CELL_WIDTH * end) - 1) |
                     ((1_pos_t << CELL_WIDTH * start) - 1);
        board &= mask;

        // set cleared bits to captured
        pos_t captured_mask = ((1_pos_t << end) - 1) ^ ((1_pos_t << start) - 1);
        captured |= captured_mask;
    }
    // clear any chains captured by a recent play at the given position
    // including suicide and return how many chains were cleared
    pos_t clear_captured(pos_t position) {
        Cell player = get(position);
        Cell opponent = player.flip();
        pos_t num_captured = 0;
        assert(player.is_stone());

        // find beginning of player chain
        pos_t i = position - 1;
        for (; i < size && get(i) == player; i--)
            ;
        // find end of player chain
        pos_t j = position + 1;
        for (; j < size && get(j) == player; j++)
            ;
        // find beginning of opponent chain
        pos_t s = i;
        for (; s < size && get(s) == opponent; s--)
            ;
        // find end of opponent chain
        pos_t t = j;
        for (; t < size && get(t) == opponent; t++)
            ;
        // capture left
        if ((s >= size || get(s) == player) && i != s) {
            clear_chain(s + 1, i + 1);
            num_captured++;
        }
        // capture right
        if ((t >= size || get(t) == player) && t != j) {
            clear_chain(j, t);
            num_captured++;
        }
        // suicide
        if ((i >= size || get(i) == opponent) && (j >= size || get(j) == opponent) && j != i) {
            clear_chain(i + 1, j);
            num_captured++;
        }
        return num_captured;
    }
    bool operator==(Board o) const { return o.board == board; }
    bool operator!=(Board o) const { return o.board != board; }
    friend std::ostream &operator<<(std::ostream &os, Board board) {
        for (pos_t i = 0; i < size; i++)
            os << board.get(i);
        return os;
    }
};

// a board is it's own hash code!
template <pos_t size> struct BoardHasher {
    size_t operator()(Board<size> b) const { return b.board; }
};

// specialize history to use a bitset (fast) if it will fit in memory,
// or a hashmap (slow) otherwise.
template <pos_t size, typename = void> struct History;
template <pos_t size> struct History<size, std::enable_if_t<(size >= 10)>> {
    std::unordered_set<Board<size>, BoardHasher<size>> states;
    void add(Board<size> s) { states.insert(s); }
    void remove(Board<size> s) { states.erase(s); }
    bool contains(Board<size> s) const { return states.find(s) != states.end(); }
    bool operator==(History h) const { return h.states == states; }
    bool operator!=(History h) const { return h.states != states; }
};
template <pos_t size> struct History<size, std::enable_if_t<(size < 10)>> {
    std::bitset<1ul << (size * 2)> states;
    void add(Board<size> s) { states[s.board] = true; }
    void remove(Board<size> s) { states[s.board] = false; }
    bool contains(Board<size> s) const { return states[s.board]; }
    bool operator==(History h) const { return h.states == states; }
    bool operator!=(History h) const { return h.states != states; }
};

// Zobrist hashing for states.
template <pos_t size, typename Hash = size_t> struct ZobristHasher {
    static constexpr size_t MAX_DEPTH = 100;
    Hash table[MAX_DEPTH][size + 1][CELL_MAX];
    ZobristHasher() {
        std::random_device rd;
        std::mt19937_64 e2(rd());
        std::uniform_int_distribution<Hash> dist;

        for (size_t d = 0; d < MAX_DEPTH; d++)
            for (size_t i = 0; i < size + 1; i++)
                for (size_t j = 0; j < CELL_MAX; j++)
                    table[d][i][j] = dist(e2);
    }
    Hash update(Hash hash, size_t depth, Move move) {
        assert(depth < MAX_DEPTH);
        return hash ^ table[depth][move.is_pass ? 0 : move.position + 1][move.color.value];
    }
};

template <pos_t size> struct State {
    enum GameState { NORMAL, PASS, GAME_OVER } game_state = NORMAL;
    Board<size> board;
    History<size> history;
    std::stack<std::tuple<GameState, Board<size>, Move, size_t>> past;
    Cell to_play = BLACK;
    size_t hash = 0;
    static ZobristHasher<size> hasher;
    struct Info {
        pos_t legal_moves;
        pos_t capturing_moves;
    };
    mutable optional<Info> info_cache[CELL_MAX];

    bool terminal() const { return game_state == GAME_OVER; }
    void play(Move move) {
        std::fill(info_cache, info_cache + CELL_MAX, optional<Info>{});
        past.emplace(game_state, board, move, hash);
        hash = hasher.update(hash, past.size(), move);
        assert(game_state != GAME_OVER);
        if (move.is_pass) {
            if (game_state == NORMAL)
                game_state = PASS;
            else if (game_state == PASS)
                game_state = GAME_OVER;
        } else {
            assert(legal_moves(move.color) & 1 << move.position);
            assert(board.get(move.position).is_empty());
            board.set(move.position, move.color);
            board.clear_captured(move.position);
            assert(!history.contains(board));
            history.add(board);
            game_state = NORMAL;
        }
        to_play = move.color.flip();
    }
    void undo() {
        std::fill(info_cache, info_cache + CELL_MAX, optional<Info>{});
        auto prev = past.top();
        past.pop();
        GameState gs = std::get<0>(prev);
        Board<size> b = std::get<1>(prev);
        Move m = std::get<2>(prev);
        hash = std::get<3>(prev);
        if (!(b == board))
            history.remove(board);
        game_state = gs;
        board = b;
        to_play = m.color;
    }

    pos_t capturing_moves(Cell color) const {
        if (!info_cache[color.value])
            compute_info(color);
        return info_cache[color.value]->capturing_moves;
    }

    // retuns a bitset of all legal moves for a given color
    pos_t legal_moves(Cell color) const {
        if (!info_cache[color.value])
            compute_info(color);
        return info_cache[color.value]->legal_moves;
    }

    void compute_info(Cell color) const {
        assert(color.is_stone());
        pos_t legal = board.empty_set();
        pos_t captured = 0;
        for (pos_t i = 0; i < size; i++) {
            if (legal & (1 << i)) {
                Board<size> b = board;
                b.set(i, color);
                captured |= (b.clear_captured(i) != 0) << i;
                // check for suicide
                if (b.get(i).is_empty())
                    legal &= ~(1 << i);
                // check history
                else if (history.contains(b))
                    legal &= ~(1 << i);
            }
        }
        info_cache[color.value] = Info{legal, captured};
    }

    bool operator==(State s) const {
        return s.hash == hash && s.board == board && s.game_state == game_state &&
               s.to_play == to_play && s.history == history;
    }
    bool operator!=(State s) const {
        return !(s == *this);
    }
};
template <pos_t size> ZobristHasher<size> State<size>::hasher;

template <pos_t size> struct StateHasher {
    size_t operator()(State<size> b) const { return b.hash; }
};
