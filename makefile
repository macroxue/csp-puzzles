CONSTRAINTS=Function.h FunctionAC.h OneToOne.h Different.h Same.h
FRAMEWORK=Problem.h $(CONSTRAINTS) Constraint.h Variable.h Domain.h Queue.h \
	  Option.h

PUZZLES=Fiver Sudoku SendMoreMoney Zebra Queens Strimko Crossword Nonogram \
        MasterMind Kakuro

OPTS=-Wall -O3

all: $(PUZZLES)

clean:
	rm -f $(PUZZLES)

test:
	./test.sh 2>&1 | tee test.out

Fiver : Fiver.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^)

Sudoku : Sudoku.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^)

SendMoreMoney: SendMoreMoney.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

Zebra: Zebra.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

Queens: Queens.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

Strimko: Strimko.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

Crossword: Crossword.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

Nonogram: Nonogram.cpp $(FRAMEWORK) RunLength.h Automaton.h
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

MasterMind: MasterMind.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

Kakuro: Kakuro.cpp $(FRAMEWORK)
	g++ $(OPTS) -o $@ $(filter %.cpp,$^) 

