#!/bin/sh

time Crossword/Crossword
time SendMoreMoney/SendMoreMoney
time Zebra/Zebra
time Queens/Queens 200 1
time Fiver/Fiver 20
for INPUT in Sudoku/sudoku.in*; do
    time Sudoku/Sudoku < $INPUT
done
for INPUT in Strimko/strimko.in*; do
    time Strimko/Strimko < $INPUT
done
for INPUT in Nonogram/nonogram.in*; do
    time Nonogram/Nonogram < $INPUT
done
for INPUT in Kakuro/kakuro.in*; do
    time Kakuro/Kakuro < $INPUT
done
for INPUT in Numberlink/numberlink.in[1-5]; do
    time Numberlink/Numberlink < $INPUT
done

