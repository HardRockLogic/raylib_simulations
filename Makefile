CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra # -DDEBUG
LDFLAGS = -lraylib

SRC = main.cpp
EXE = main

# SRC2 = main_2.cpp
# EXE2 = main2
.PHONY: collisions

all: $(EXE) $(EXE2)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

collisions: collisionsim.cpp
	$(CXX) -O3 $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# $(EXE2): $(SRC2)
# 	$(CXX) $(CXXFLAGS) -o $@ $^

clean: 
	@rm -f $(EXE)    
