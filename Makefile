CXX = /usr/bin/g++
CXXFLAGS += -std=c++1z -g -O3 -Wall -Wextra -march=native -Wno-unused-parameter
CXXFLAGS += -DNDEBUG
OBJ_FILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
EXE := test ab conjecture_prover gen_loosely_packed

all : $(EXE)

DEPS := $(OBJ_FILES:.o=.d)
-include $(DEPS)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ -c $<

test : test.o lgotest.o abtest.o
	$(CXX) $(CXXFLAGS) $^ -o $@

ab : ab.o
	$(CXX) $(CXXFLAGS) $^ -o $@

conjecture_prover : conjecture_prover.o
	$(CXX) $(CXXFLAGS) $^ -o $@

gen_loosely_packed : gen_loosely_packed.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean :
	rm -rf *.o *.d $(EXE)
