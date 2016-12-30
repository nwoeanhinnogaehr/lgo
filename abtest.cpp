#include "catch.hpp"
#include "ab.hpp"

TEST_CASE("alpha beta size 1", "[alphabeta]") {
    AlphaBeta<1> ab;
    State<1> s;
    REQUIRE(ab.alphabeta(s, BLACK) == 0);
}
TEST_CASE("alpha beta size 2", "[alphabeta]") {
    AlphaBeta<2> ab;
    State<2> s;
    REQUIRE(ab.alphabeta(s, BLACK) == 0);
}
TEST_CASE("alpha beta size 3", "[alphabeta]") {
    AlphaBeta<3> ab;
    State<3> s;
    REQUIRE(ab.alphabeta(s, BLACK) == 3);
}
TEST_CASE("alpha beta size 4", "[alphabeta]") {
    AlphaBeta<4> ab;
    State<4> s;
    REQUIRE(ab.alphabeta(s, BLACK) == 4);
}
TEST_CASE("alpha beta size 5", "[alphabeta]") {
    AlphaBeta<5> ab;
    State<5> s;
    REQUIRE(ab.alphabeta(s, BLACK) == 0);
}
TEST_CASE("alpha beta size 6", "[alphabeta]") {
    AlphaBeta<6> ab;
    State<6> s;
    REQUIRE(ab.alphabeta(s, BLACK) == 1);
}
TEST_CASE("lgo_small pv 3:1", "[alphabeta]") {
    AlphaBeta<3> ab;
    State<3> s;
    s.play(Move(BLACK, 1));
    REQUIRE(ab.alphabeta(s, WHITE) == 3);
}
TEST_CASE("lgo_small pv 3:0,1", "[alphabeta]") {
    AlphaBeta<3> ab;
    State<3> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 1));
    REQUIRE(ab.alphabeta(s, BLACK) == -3);
}
TEST_CASE("lgo_small pv 4:1", "[alphabeta]") {
    AlphaBeta<4> ab;
    State<4> s;
    s.play(Move(BLACK, 1));
    REQUIRE(ab.alphabeta(s, WHITE) == 4);
}
TEST_CASE("lgo_small pv 4:0,2", "[alphabeta]") {
    AlphaBeta<4> ab;
    State<4> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 2));
    REQUIRE(ab.alphabeta(s, BLACK) == -4);
}
TEST_CASE("lgo_small pv 5:1,3", "[alphabeta]") {
    AlphaBeta<5> ab;
    State<5> s;
    s.play(Move(BLACK, 1));
    s.play(Move(WHITE, 3));
    REQUIRE(ab.alphabeta(s, BLACK) == 0);
}
TEST_CASE("lgo_small pv 5:2,1,3", "[alphabeta]") {
    AlphaBeta<5> ab;
    State<5> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    s.play(Move(BLACK, 3));
    REQUIRE(ab.alphabeta(s, WHITE) == 0);
}
TEST_CASE("lgo_small pv 5:0,3", "[alphabeta]") {
    AlphaBeta<5> ab;
    State<5> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 3));
    REQUIRE(ab.alphabeta(s, BLACK) == -5);
}
TEST_CASE("lgo_small pv 6:1,4,2", "[alphabeta]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 1));
    s.play(Move(WHITE, 4));
    s.play(Move(BLACK, 2));
    REQUIRE(ab.alphabeta(s, WHITE) == 1);
}
TEST_CASE("lgo_small pv 6:2,1", "[alphabeta]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    REQUIRE(ab.alphabeta(s, BLACK) == -6);
}
TEST_CASE("lgo_small pv 6:0,4", "[alphabeta]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 4));
    REQUIRE(ab.alphabeta(s, BLACK) == -6);
}
