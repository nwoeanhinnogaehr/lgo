CXX = /usr/bin/g++
CXXFLAGS += -std=c++1z -g -O3 -Wall -Wextra -march=native -Wno-unused-parameter
CXXFLAGS += -DNDEBUG
OBJ_FILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all : test ab conjecture

test : test.o lgotest.o abtest.o
	$(CXX) $(CXXFLAGS) $^ -o $@

ab : ab.o
	$(CXX) $(CXXFLAGS) $^ -o $@

conjecture : conjecture.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test.o : test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

DEPS := $(OBJ_FILES:.o=.d)
-include $(DEPS)
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ -c $<

clean :
	rm -rf *.o *.d test ab conjecture
