#include "catch.hpp"
#include "ab.hpp"
#include "conjectures.hpp"

TEST_CASE("alpha beta size 1", "[search]") {
    AlphaBeta<1> ab;
    State<1> s;
    REQUIRE(ab.search(s) == 0);
}
TEST_CASE("alpha beta size 2", "[search]") {
    AlphaBeta<2> ab;
    State<2> s;
    REQUIRE(ab.search(s) == 0);
}
TEST_CASE("alpha beta size 3", "[search]") {
    AlphaBeta<3> ab;
    State<3> s;
    REQUIRE(ab.search(s) == 3);
}
TEST_CASE("alpha beta size 4", "[search]") {
    AlphaBeta<4> ab;
    State<4> s;
    REQUIRE(ab.search(s) == 4);
}
TEST_CASE("alpha beta size 5", "[search]") {
    AlphaBeta<5> ab;
    State<5> s;
    REQUIRE(ab.search(s) == 0);
}
TEST_CASE("alpha beta size 6", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    REQUIRE(ab.search(s) == 1);
}
TEST_CASE("lgo_small pv 3:1", "[search]") {
    AlphaBeta<3> ab;
    State<3> s;
    s.play(Move(BLACK, 1));
    REQUIRE(ab.search(s) == 3);
}
TEST_CASE("lgo_small pv 3:0,1", "[search]") {
    AlphaBeta<3> ab;
    State<3> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 1));
    REQUIRE(ab.search(s) == -3);
}
TEST_CASE("lgo_small pv 4:1", "[search]") {
    AlphaBeta<4> ab;
    State<4> s;
    s.play(Move(BLACK, 1));
    REQUIRE(ab.search(s) == 4);
}
TEST_CASE("lgo_small pv 4:0,2", "[search]") {
    AlphaBeta<4> ab;
    State<4> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 2));
    REQUIRE(ab.search(s) == -4);
}
TEST_CASE("lgo_small pv 5:1,3", "[search]") {
    AlphaBeta<5> ab;
    State<5> s;
    s.play(Move(BLACK, 1));
    s.play(Move(WHITE, 3));
    REQUIRE(ab.search(s) == 0);
}
TEST_CASE("lgo_small pv 5:2,1,3", "[search]") {
    AlphaBeta<5> ab;
    State<5> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    s.play(Move(BLACK, 3));
    REQUIRE(ab.search(s) == 0);
}
TEST_CASE("lgo_small pv 5:0,3", "[search]") {
    AlphaBeta<5> ab;
    State<5> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 3));
    REQUIRE(ab.search(s) == -5);
}
TEST_CASE("lgo_small pv 6:1,4,2", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 1));
    s.play(Move(WHITE, 4));
    s.play(Move(BLACK, 2));
    REQUIRE(ab.search(s) == 1);
}
TEST_CASE("lgo_small pv 6:2,1", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    REQUIRE(ab.search(s) == -1);
}
TEST_CASE("lgo_small pv 6:0,4", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 0));
    s.play(Move(WHITE, 4));
    REQUIRE(ab.search(s) == -6);
}
TEST_CASE("lgo_small pv 6:1,2", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 1));
    s.play(Move(WHITE, 2));
    REQUIRE(ab.search(s) == 6);
}
TEST_CASE("lgo_small pv 6:1,3", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 1));
    s.play(Move(WHITE, 3));
    REQUIRE(ab.search(s) == 6);
}
TEST_CASE("lgo_small pv 6:0", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 0));
    REQUIRE(ab.search(s) == -6);
}
TEST_CASE("lgo_small pv 6:2", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 2));
    REQUIRE(ab.search(s) == -1);
}
TEST_CASE("lgo_small pv 6:2,1,3", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    s.play(Move(BLACK, 3));
    REQUIRE(ab.search(s) == -6);
}
TEST_CASE("lgo_small pv 6:2,1,4", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    s.play(Move(BLACK, 4));
    REQUIRE(ab.search(s) == -6);
}
TEST_CASE("lgo_small pv 6:2,1,5", "[search]") {
    AlphaBeta<6> ab;
    State<6> s;
    s.play(Move(BLACK, 2));
    s.play(Move(WHITE, 1));
    s.play(Move(BLACK, 5));
    REQUIRE(ab.search(s) == -6);
}

TEST_CASE("size 5", "[search]") {
    constexpr int size = 5;
    using Impl = conjectures::All<size, PV<size>>;
    IterativeDeepening<size, AlphaBeta, Impl> ab;
    {
        State<size> root;
        root.play(Move(BLACK, 0));
        REQUIRE(ab.search(root).minimax == -5);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 1));
        REQUIRE(ab.search(root).minimax == 0);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 2));
        REQUIRE(ab.search(root).minimax == 0);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 3));
        REQUIRE(ab.search(root).minimax == 0);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 4));
        REQUIRE(ab.search(root).minimax == -5);
    }
}

TEST_CASE("size 6", "[search]") {
    constexpr int size = 6;
    using Impl = conjectures::All<size, PV<size>>;
    IterativeDeepening<size, AlphaBeta, Impl> ab;
    {
        State<size> root;
        root.play(Move(BLACK, 0));
        REQUIRE(ab.search(root).minimax == -6);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 1));
        REQUIRE(ab.search(root).minimax == 1);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 2));
        REQUIRE(ab.search(root).minimax == -1);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 3));
        REQUIRE(ab.search(root).minimax == -1);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 4));
        REQUIRE(ab.search(root).minimax == 1);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 5));
        REQUIRE(ab.search(root).minimax == -6);
    }
}

TEST_CASE("size 7", "[search]") {
    constexpr int size = 7;
    using Impl = conjectures::All<size, PV<size>>;
    IterativeDeepening<size, AlphaBeta, Impl> ab;
    {
        State<size> root;
        root.play(Move(BLACK, 0));
        REQUIRE(ab.search(root).minimax == -7);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 1));
        REQUIRE(ab.search(root).minimax == 2);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 2));
        REQUIRE(ab.search(root).minimax == -2);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 3));
        REQUIRE(ab.search(root).minimax == 2);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 4));
        REQUIRE(ab.search(root).minimax == -2);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 5));
        REQUIRE(ab.search(root).minimax == 2);
    }
    {
        State<size> root;
        root.play(Move(BLACK, 6));
        REQUIRE(ab.search(root).minimax == -7);
    }
}
