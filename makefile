CONSTRAINTS=Function.h FunctionAC.h OneToOne.h Different.h Same.h BooleanOr.h \
    BooleanSum.h
FRAMEWORK=Problem.h $(CONSTRAINTS) Constraint.h Variable.h Domain.h Queue.h \
	  Option.h

OPTS=-Wall -O3 -std=c++0x

ifeq ($(wildcard Framework), )
    # build one puzzle in current directory
    PUZZLES=$(shell basename $(PWD))
    VPATH=../Framework
    INCS=-I../Framework
else
    # build all puzzles
    PUZZLES=Fiver/Fiver Sudoku/Sudoku SendMoreMoney/SendMoreMoney Zebra/Zebra \
	    Queens/Queens Strimko/Strimko Crossword/Crossword Nonogram/Nonogram \
	    MasterMind/MasterMind Kakuro/Kakuro Numberlink/Numberlink Sat/Sat
    VPATH=./Framework:./Nonogram
    INCS=-I./Framework
endif

all: $(PUZZLES)

$(filter Nonogram%,$(PUZZLES)): RunLength.h Automaton.h Set.h

$(PUZZLES): %: %.cpp $(FRAMEWORK)
	g++ $(OPTS) $(INCS) -o $@ $(filter %.cpp,$<)

.PHONY: clean test

clean:
	rm -f $(PUZZLES)

test: $(PUZZLES)
	./test.sh 2>&1 | tee test.out

