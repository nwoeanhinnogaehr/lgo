#include "catch.hpp"
#include "lgo.hpp"

TEST_CASE("Board get/set works properly", "[board_basic]") {
    for (pos_t i = 0; i < MAX_SIZE; i++) {
        Board s(MAX_SIZE);
        for (pos_t c = 0; c < CELL_MAX; c++) {
            s.set(i, Cell(c));
            REQUIRE(s.get(i) == c);
            for (pos_t j = 0; j < MAX_SIZE; j++) {
                if (i != j)
                    REQUIRE(s.get(j) == EMPTY);
            }
        }
    }
}

TEST_CASE("History works", "[history]") {
    History h;
    Board s(MAX_SIZE);
    for (pos_t i = 0; i < MAX_SIZE; i++) {
        for (pos_t c = 1; c < CELL_MAX; c++) {
            s.set(i, Cell(c));
            REQUIRE(!h.check(s));
            h.add(s);
            REQUIRE(h.check(s));
        }
    }
}

TEST_CASE("Passing twice results in a terminal state", "[state_pass]") {
    State s(MAX_SIZE);
    REQUIRE(!s.terminal());
    s.play(Move(BLACK, 0, false));
    REQUIRE(!s.terminal());
    s.play(Move(WHITE, 0, true));
    REQUIRE(!s.terminal());
    s.play(Move(BLACK, 3, false));
    REQUIRE(!s.terminal());
    s.play(Move(WHITE, 0, true));
    REQUIRE(!s.terminal());
    s.play(Move(BLACK, 0, true));
    REQUIRE(s.terminal());
}

namespace Catch {
template <> struct StringMaker<Score> {
    static std::string convert(Score const &value) {
        return "{ B+" + std::to_string(value.black) + ", W+" + std::to_string(value.white) + " }";
    }
};
}

TEST_CASE("Board scoring size 1", "[score_board_1]") {
    Board b(1);
    REQUIRE(b.score() == Score(0, 0));
    b.set(0, BLACK);
    REQUIRE(b.score() == Score(1, 0));
    b.set(0, WHITE);
    REQUIRE(b.score() == Score(0, 1));
}

TEST_CASE("Board scoring size 2", "[score_board_2]") {
    Board b(2);
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

TEST_CASE("Board scoring size 3", "[score_board_3]") {
    Board b(3);
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

TEST_CASE("Board scoring size 4", "[score_board_4]") {
    Board b(4);
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
