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
Fiver.g : Fiver.cpp $(FRAMEWORK)
	g++ -Wall -g -o $@ $(filter %.cpp,$^) -DVERBOSE

Sudoku : Sudoku.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^)
Sudoku.g : Sudoku.cpp $(FRAMEWORK)
	g++ -Wall -g -o $@ $(filter %.cpp,$^) -DVERBOSE

SendMoreMoney: SendMoreMoney.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Zebra: Zebra.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Queens: Queens.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 
Queens.g: Queens.cpp $(FRAMEWORK)
	g++ -Wall -g -o $@ $(filter %.cpp,$^) -DVERBOSE

Strimko: Strimko.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Crossword: Crossword.cpp $(FRAMEWORK)
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Nonogram: Nonogram.cpp $(FRAMEWORK) RunLength.h Automaton.h
	g++ -Wall -O3 -o $@ $(filter %.cpp,$^) 

Nonogram.g: Nonogram.cpp $(FRAMEWORK) RunLength.h Automaton.h
	g++ -Wall -g -o $@ $(filter %.cpp,$^) -DVERBOSE
