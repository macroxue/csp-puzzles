CONSTRAINTS=Function.h FunctionAC.h OneToOne.h Different.h \
	    Same.h
FRAMEWORK=Problem.h $(CONSTRAINTS) Constraint.h Variable.h Domain.h

PUZZELS=Fiver Sudoku SendMoreMoney Zebra Queens Strimko Crossword

all: $(PUZZELS)

clean:
	rm -f $(PUZZELS)

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
