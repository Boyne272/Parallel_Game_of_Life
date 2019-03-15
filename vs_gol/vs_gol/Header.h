#pragma once
#include <mpi.h>	// MPI functions
#include <iostream>	// standard input/output streams
#include <cstdlib>	// random number functions
#include <time.h>	// for seeding the random generator with the current time
#include <fstream>	// writing file outputs
#include <sstream>	// for a buffer when writing to file
#include <string>	// for coverting variables to string
#include <assert.h> // for debugging

using namespace std;

	// domain.cpp functions
void init_random(bool* grid, int rows, int cols);
void blinker_1(bool* grid, int grid_width, int row, int col);
void blinker_2(bool* grid, int grid_width, int row, int col);
void still_1(bool* grid, int grid_width, int row, int col);
void still_2(bool* grid, int grid_width, int row, int col);
