include ../makefile

all: AutomatonTest

AutomatonTest: AutomatonTest.cpp Automaton.h
	g++ $(OPTS) $(INCS) -o $@ $(filter %.cpp,$<) 
