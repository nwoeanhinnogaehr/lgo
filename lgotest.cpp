#include "catch.hpp"
#include "lgo.hpp"

TEST_CASE("Board get/set works properly", "[board_basic]") {
    for (pos_t i = 0; i < MAX_SIZE; i++) {
        Board s;
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
    Board s;
    for (pos_t i = 0; i < MAX_SIZE; i++) {
        for (pos_t c = 1; c < CELL_MAX; c++) {
            s.set(i, Cell(c));
            REQUIRE(!h.check(s));
            h.add(s);
            REQUIRE(h.check(s));
        }
    }
}
