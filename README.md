# ACSE 6.2 - Parallel Game Of Life with MPI
12th to 17th Martch 2019

By Richard Boyne CID 01057503

## Repsoitory Structure
* C++ code is kept in a visual studio project within vs\_gol/ with the raw .cpp and .h files in vs\_gol/vs\_gol/
- GOL\_grid.h GOL\_grid.cpp contains the class which implements the game of life simulation
- domain.cpp holds a few functions for filling the intial grid either randomly or with known structures for testing (e.g. a glider)
- main.cpp controls each processor and intialises the GOL\_grid class
- Header.h holds the dependencies
- note vairaous compiler #define statments allows for changes to the code, such as CX1 option for saving runtimes to a seperate csv file

* All results are in results/
- post_proc.py holds the methods for combining multiple processors csv files, this code used in the post_processing.ipynb notebook
- 0000000000000 plots the time results from \*\_times.csv files
- subdirectories hold individual runs data, including a \_config.txt that holds the runs parameters and a \_all.html animation of the results

* HPC code is in CX1/ specifically this has a copy of all code, a makefile for gcc compilation on linux and a pbs script for batch solving on CX1

## Notes to the Examiner
. . .
