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
  - post_proc.py holds the methods for combining multiple processors csv files. Examples of using this code in the post_processing.ipynb notebook and some example post processed data is in local_random/ including a \_config.txt that holds the runs parameters and a \_all.html animation of the results
  - analyse_time.ipynb plots the time results from times.csv files which are also stored in this directory

* HPC code is in CX2/ specifically this has a copy of all code, a makefile for gcc compilation on linux and a pbs script for batch solving on CX2. An animation for these results are seen in 00000000000000000000000 but the raw data is too large to upload. The time data can be seen in results

## Notes to the Examiner
There is a short explain of code features in the report so I suggest reading that first before looking at the code. All code is commented and as normal here is appropirate meme:

<img src="cx2_meme.jpg" alt="CX2 waiting meme">
