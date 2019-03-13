#pragma once
#include <mpi.h>	// MPI functions
#include <iostream>	// standard input/output streams
#include <cstdlib>	// random number functions
#include <time.h>	// for seeding the random generator with the current time
#include <fstream>	// writing file outputs
#include <string>	// for coverting variables to string before writing to file

using namespace std;

	// domain.cpp functions
void init_oscillator(bool* grid, int grid_width, int row, int col);
void init_random(bool* grid, int size);
void init_stat_corners(bool* grid, int grid_width, int grid_height);



	// these are the processor types called in main
void main_processor(int p, int id = 0);
void general_processor(int p, int id);