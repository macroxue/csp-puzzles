CONSTRAINTS=Function.h FunctionAC.h OneToOne.h Different.h \
	    Same.h Next.h Consecutive.h 
FRAMEWORK=Problem.h $(CONSTRAINTS) Constraint.h Variable.h Domain.h

all: Fiver Sudoku SendMoreMoney Zebra Queens

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
