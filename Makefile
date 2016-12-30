CXX ?= g++
CXXFLAGS += -std=c++11 -g -O3 -Wall -Wextra -march=native -Wno-unused-parameter
#CXXFLAGS += -DNDEBUG
OBJ_FILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all : test ab

test : test.o lgotest.o abtest.o lgo.o
	$(CXX) $(CXXFLAGS) $^ -o $@

ab : ab.o lgo.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test.o : test.cpp catch.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o : %.cpp *.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -rf *.o test ab
