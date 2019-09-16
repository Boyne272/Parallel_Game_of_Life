## Parallel Game Of Life with MPI

Using MPI the famous cellular automaton (game of life) model has been implemented in a scalable manner. It has been tested on up to 128 processors on the imperial CX1 computer cluster with runtime improvements of 80%.

12th to 17th March 2019

By Richard Boyne (GitHub:Boyne272, CID: 01057503)

## Repsoitory Structure
* C++ code is kept in a visual studio project within vs\_gol/ with the raw .cpp and .h files in vs\_gol/vs\_gol/
  - GOL\_grid.h GOL\_grid.cpp contains the class which implements the game of life simulation
  - domain.cpp holds a few functions for filling the initial grid either randomly or with known structures for testing (e.g. a glider)
  - main.cpp controls each processor and initialises the GOL\_grid class
  - Header.h holds the dependencies
  - note various compiler #define statements allows for changes to the code, such as CX1 option for saving runtimes to a separate csv file

* All results are in results/
  - post_proc.py holds the methods for combining multiple processors csv files. Examples of using this code in the post_processing.ipynb notebook and some example post-processed data is in local_random/ including a \_config.txt that holds the runs parameters and a \_all.html animation of the results
  - analyse_time.ipynb plots the time results from times.csv files which are also stored in this directory

* HPC code is in CX2/ specifically this has a copy of all code, a makefile for gcc compilation on Linux and a pbs script for batch solving on CX2. No data other than run times from CX2 was kept due to its size (around 5~10 GB per simulation). The raw time data can be seen in the results directory.

## Appropriate Meme:

<img src="meme.jpg" alt="waiting meme">
