#!/bin/sh

time Crossword
time SendMoreMoney
time Zebra
time Queens 200 1
time Fiver 20
for INPUT in sudoku.in*; do
    time Sudoku < $INPUT
done
for INPUT in strimko.in*; do
    time Strimko < $INPUT
done

