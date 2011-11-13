CONSTRAINTS=Function.h FunctionAC.h OneToOne.h Different.h Same.h
FRAMEWORK=Problem.h $(CONSTRAINTS) Constraint.h Variable.h Domain.h Queue.h \
	  Option.h

PUZZELS=Fiver Sudoku SendMoreMoney Zebra Queens Strimko Crossword Nonogram

all: $(PUZZELS)

clean:
	rm -f $(PUZZELS)

test:
	./test.sh 2>&1 | tee test.out

Fiver : Fiver.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^)

Sudoku : Sudoku.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^)

SendMoreMoney: SendMoreMoney.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Zebra: Zebra.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Queens: Queens.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Strimko: Strimko.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Crossword: Crossword.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Nonogram: Nonogram.cpp $(FRAMEWORK) RunLength.h Automaton.h
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

