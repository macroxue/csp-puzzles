HEADERS=$(shell ls *.h)
PUZZLES=$(HEADERS:%.h=%)

all: Encode $(PUZZLES)

Encode: Encode.cpp
	g++ -o $@ $<

$(PUZZLES): %: %.h
	Create.sh $<

clean:
	rm -f Encode $(PUZZLES)
