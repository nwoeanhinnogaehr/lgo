#include "catch.hpp"
#include "lgo.hpp"

TEST_CASE("Board get/set works properly", "[board]") {
    for (pos_t i = 0; i < MAX_SIZE; i++) {
        Board<MAX_SIZE> s;
        for (pos_t c = 0; c < CELL_MAX; c++) {
            s.set(i, Cell(c));
            REQUIRE(s.get(i) == Cell(c));
            for (pos_t j = 0; j < MAX_SIZE; j++) {
                if (i != j)
                    REQUIRE(s.get(j) == EMPTY);
            }
        }
    }
}

TEST_CASE("History works", "[history]") {
    History<MAX_SIZE> h;
    Board<MAX_SIZE> s;
    for (pos_t i = 0; i < MAX_SIZE; i++) {
        for (pos_t c = 1; c < CELL_MAX; c++) {
            s.set(i, Cell(c));
            REQUIRE(!h.contains(s));
            h.add(s);
            REQUIRE(h.contains(s));
        }
    }
}

TEST_CASE("History works (size 13)", "[history]") {
    History<13> h;
    Board<13> s;
    for (pos_t i = 0; i < 13; i++) {
        for (pos_t c = 1; c < CELL_MAX; c++) {
            s.set(i, Cell(c));
            REQUIRE(!h.contains(s));
            h.add(s);
            REQUIRE(h.contains(s));
        }
    }
}

TEST_CASE("Passing twice results in a terminal state", "[state]") {
    State<MAX_SIZE> s;
    REQUIRE(!s.terminal());
    s.play(Move(BLACK, 0));
    REQUIRE(!s.terminal());
    s.play(Move(WHITE));
    REQUIRE(!s.terminal());
    s.play(Move(BLACK, 3));
    REQUIRE(!s.terminal());
    s.play(Move(WHITE));
    REQUIRE(!s.terminal());
    s.play(Move(BLACK));
    REQUIRE(s.terminal());
}

namespace Catch {
template <> struct StringMaker<Score> {
    static std::string convert(Score const &value) {
        return "{ B+" + std::to_string(value.black) + ", W+" + std::to_string(value.white) + " }";
    }
};
}

TEST_CASE("Board scoring size 1", "[board]") {
    Board<1> b;
    REQUIRE(b.score() == Score(0, 0));
    b.set(0, BLACK);
    REQUIRE(b.score() == Score(1, 0));
    b.set(0, WHITE);
    REQUIRE(b.score() == Score(0, 1));
}

TEST_CASE("Board scoring size 2", "[board]") {
    Board<2> b;
    REQUIRE(b.score() == Score(0, 0));
    b.set(0, BLACK);
    REQUIRE(b.score() == Score(2, 0));
    b.set(1, WHITE);
    REQUIRE(b.score() == Score(1, 1));
    b.set(0, WHITE);
    REQUIRE(b.score() == Score(0, 2));
    b.set(1, BLACK);
    REQUIRE(b.score() == Score(1, 1));
    b.set(1, EMPTY);
    REQUIRE(b.score() == Score(0, 2));
}

TEST_CASE("Board scoring size 3", "[board]") {
    Board<3> b;
    REQUIRE(b.score() == Score(0, 0));
    b.set(1, BLACK);
    REQUIRE(b.score() == Score(3, 0));
    b.set(0, BLACK);
    REQUIRE(b.score() == Score(3, 0));
    b.set(1, WHITE);
    REQUIRE(b.score() == Score(1, 2));
    b.set(0, WHITE);
    REQUIRE(b.score() == Score(0, 3));
    b.set(2, BLACK);
    REQUIRE(b.score() == Score(1, 2));
    b.set(1, EMPTY);
    REQUIRE(b.score() == Score(1, 1));
}

TEST_CASE("Board scoring size 4", "[board]") {
    Board<4> b;
    REQUIRE(b.score() == Score(0, 0));
    b.set(1, BLACK);
    REQUIRE(b.score() == Score(4, 0));
    b.set(2, WHITE);
    REQUIRE(b.score() == Score(2, 2));
    b.set(1, WHITE);
    REQUIRE(b.score() == Score(0, 4));
    b.set(0, BLACK);
    REQUIRE(b.score() == Score(1, 3));
    b.set(3, BLACK);
    REQUIRE(b.score() == Score(2, 2));
    b.set(1, EMPTY);
    REQUIRE(b.score() == Score(2, 1));
}

TEST_CASE("Legal moves size 1", "[state]") {
    State<1> s;
    // no one can play because the stone would have no liberties
    REQUIRE(s.legal_moves(BLACK) == 0b0);
    REQUIRE(s.legal_moves(WHITE) == 0b0);
}

TEST_CASE("Legal moves size 2", "[state]") {
    State<2> s;
    REQUIRE(s.legal_moves(BLACK) == 0b11);
    REQUIRE(s.legal_moves(WHITE) == 0b11);

    s.play(Move(BLACK, 1));
    REQUIRE(s.legal_moves(BLACK) == 0b00);
    REQUIRE(s.legal_moves(WHITE) == 0b01);

    s.play(Move(WHITE, 0)); // capture black stone
    REQUIRE(s.legal_moves(BLACK) == 0b00); // would recreate previous state
    REQUIRE(s.legal_moves(WHITE) == 0b00);

}

TEST_CASE("Legal moves size 3", "[state]") {
    State<3> s;
    REQUIRE(s.legal_moves(BLACK) == 0b111);
    REQUIRE(s.legal_moves(WHITE) == 0b111);

    s.play(Move(WHITE, 0));
    REQUIRE(s.legal_moves(BLACK) == 0b110);
    REQUIRE(s.legal_moves(WHITE) == 0b110);

    s.play(Move(WHITE, 2));
    REQUIRE(s.legal_moves(BLACK) == 0b010);
    REQUIRE(s.legal_moves(WHITE) == 0b000);
}

TEST_CASE("Legal moves size 5", "[state]") {
    State<5> s;
    // All moves possible at start of game
    REQUIRE(s.legal_moves(BLACK) == 0b11111);
    REQUIRE(s.legal_moves(WHITE) == 0b11111);

    s.play(Move(BLACK, 1));
    REQUIRE(s.legal_moves(BLACK) == 0b11101);
    REQUIRE(s.legal_moves(WHITE) == 0b11100);

    s.play(Move(WHITE, 3));
    REQUIRE(s.legal_moves(BLACK) == 0b00101);
    REQUIRE(s.legal_moves(WHITE) == 0b10100);

    s.play(Move(BLACK, 0));
    REQUIRE(s.legal_moves(BLACK) == 0b00000);
    REQUIRE(s.legal_moves(WHITE) == 0b10100);

    s.play(Move(WHITE, 2));
    REQUIRE(s.legal_moves(BLACK) == 0b00011);
    REQUIRE(s.legal_moves(WHITE) == 0b10011);
}

TEST_CASE("Capture", "[state]") {
    State<16> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 1));
    REQUIRE(s.board.get(0) == EMPTY);
    REQUIRE(s.board.is_captured(0));
    REQUIRE(s.board.get(1) == WHITE);
    REQUIRE(!s.board.is_captured(1));

    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 3));
    REQUIRE(s.board.get(1) == WHITE);
    REQUIRE(!s.board.is_captured(1));
    REQUIRE(s.board.get(2) == EMPTY);
    REQUIRE(s.board.is_captured(2));
    REQUIRE(s.board.get(3) == WHITE);
    REQUIRE(!s.board.is_captured(3));

    s.play(Move(BLACK, 4));
    s.play(Move(WHITE, 7));
    s.play(Move(BLACK, 6));
    s.play(Move(WHITE, 5));
    REQUIRE(s.board.get(3) == WHITE);
    REQUIRE(!s.board.is_captured(3));
    REQUIRE(s.board.get(4) == EMPTY);
    REQUIRE(s.board.is_captured(4));
    REQUIRE(s.board.get(5) == WHITE);
    REQUIRE(!s.board.is_captured(5));
    REQUIRE(s.board.get(6) == EMPTY);
    REQUIRE(s.board.is_captured(6));
    REQUIRE(s.board.get(7) == WHITE);
    REQUIRE(!s.board.is_captured(7));

    s.play(Move(BLACK, 9));
    s.play(Move(WHITE, 8));
    s.play(Move(BLACK, 10));
    s.play(Move(WHITE, 11));
    REQUIRE(s.board.get(8) == WHITE);
    REQUIRE(!s.board.is_captured(8));
    REQUIRE(s.board.get(9) == EMPTY);
    REQUIRE(s.board.is_captured(9));
    REQUIRE(s.board.get(10) == EMPTY);
    REQUIRE(s.board.is_captured(10));
    REQUIRE(s.board.get(11) == WHITE);
    REQUIRE(!s.board.is_captured(11));

    s.play(Move(WHITE, 15));
    s.play(Move(WHITE, 14));
    s.play(Move(BLACK, 13));
    REQUIRE(s.board.get(13) == BLACK);
    REQUIRE(!s.board.is_captured(13));
    REQUIRE(s.board.get(14) == EMPTY);
    REQUIRE(s.board.is_captured(14));
    REQUIRE(s.board.get(15) == EMPTY);
    REQUIRE(s.board.is_captured(15));
}

TEST_CASE("Indirect capture", "[state]") {
    State<5> s;
    REQUIRE(s.legal_moves(BLACK) == 0b11111);
    REQUIRE(s.legal_moves(WHITE) == 0b11111);
    s.play(Move(BLACK, 0));
    REQUIRE(s.legal_moves(BLACK) == 0b11110);
    REQUIRE(s.legal_moves(WHITE) == 0b11110);
    s.play(Move(BLACK, 2));
    REQUIRE(s.legal_moves(BLACK) == 0b11010);
    REQUIRE(s.legal_moves(WHITE) == 0b11010);
    s.play(Move(WHITE, 3));
    REQUIRE(s.legal_moves(BLACK) == 0b10000);
    REQUIRE(s.legal_moves(WHITE) == 0b00010);
    s.play(Move(BLACK, 4));
    REQUIRE(s.legal_moves(BLACK) == 0b01010);
    REQUIRE(s.legal_moves(WHITE) == 0b00010);
    s.play(Move(WHITE, 1));
    REQUIRE(s.legal_moves(BLACK) == 0b00000);
    REQUIRE(s.legal_moves(WHITE) == 0b01000);
}
