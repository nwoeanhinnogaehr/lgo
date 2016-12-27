CXX ?= g++
CXXFLAGS += -std=c++11 -g -O3 -Wall -Wextra
OBJ_FILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all : test

test : test.o lgotest.o
	$(CXX) $^ -o $@ $(CXXFLAGS)

test.o : test.cpp catch.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o : %.cpp *.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -rf *.o test
