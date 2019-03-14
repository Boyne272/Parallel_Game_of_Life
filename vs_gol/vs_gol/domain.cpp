#include "Header.h"
#include "GOL_grid.h"

// -------------------- Ititalisation Functions --------------------

	// basic 50% chance random fill
void init_random(bool* grid, int rows, int cols) {
	for (int i = 1; i < rows; i++)
		for (int j = 1; j < cols; j++)
			grid[i * rows + j] = rand() % 2;
}

//	// place a stick oscillator or centered at row, col
//void init_oscillator(bool* grid, int grid_width, int row, int col) {
//	for (int i : {row * grid_width + col, 
//				  row * grid_width + col + 1,
//				  row * grid_width + col - 1})
//		grid[i] = true;
//}
//
//	// place a 2x2 (stationary block) in each corner
//void init_stat_corners(bool* grid, int grid_width, int grid_height) {
//
//	for (int row : {0, 0, grid_height - 2, grid_height - 2})
//		for (int col : {0, grid_width - 2, 0, grid_width - 2}) {
//			grid[row * grid_width + col] = true;
//			grid[row * grid_width + (col + 1)] = true;
//			grid[(row + 1) * grid_width + col] = true;
//			grid[(row + 1) * grid_width + (col + 1)] = true;
//		}
//
//}


// -------------------- Partition Functions --------------------

	// given the wight, height and number of processors
	// finds the optimal partitioning with the requierment that
	// every section has only 4 neighbours and the input grid
	// (deals with any input parameters, primes, etc. all ok)
//void find_partitions(int x, int y, int np, int &nx, int &ny) {
//
//	int comm_len;
//
//		// split into strips as a defualt
//	if (x >= y) {  // if x is longer, split along x direction
//		nx = np;
//		ny = 1;
//	}
//	else {
//		nx = 1;
//		ny = np;
//	}
//
//		// this is the length of internal edges
//	comm_len = nx * y + ny * x;
//
//		// for every factor of np
//	for (int i = 2; i < int(sqrt(np) + 1); i++)
//		if (np % i == 0) {
//			
//			const int fac1 = i;
//			const int fac2 = np / i;
//
//				// if the new comm_len would be less than the old
//			if (fac1 * y + fac2 * x < comm_len) {
//				nx = fac1;
//				ny = fac2;
//				comm_len = nx * y + ny * x;
//			}
//
//				// do the same check for the other way around
//			if (fac2 * y + fac1 * x < comm_len) {
//				nx = fac2;
//				ny = fac1;
//				comm_len = nx * y + ny * x;
//			}
//		}
//}


	// given info about the grid this finds a specific IDs dimensions
//void find_dims(int id, int x, int y, int nx, int ny, int &dim_x, int &dim_y) {
//
//		// check the input makes sense
//	assert(id >= 0 && "ID is negative");
//	assert(id < (nx*ny) && "ID is out of grid");
//
//		// find the row and col
//	const int my_row = id / nx;
//	const int my_col = id % nx;
//
//		// workout the cells dimensions
//	dim_x = x / nx;
//	dim_y = y / ny;
//
//		// equally distribute the x remainder
//	const int x_rem = x % nx;
//	if (x_rem > my_row)
//		dim_x++;
//
//		// equally distribute the x remainder
//	const int y_rem = y % ny;
//	if (x_rem > my_col)
//		dim_y++;
//
//
//}


//void find_neighbours(int id, int nx, int ny, int* neighbours, bool periodic) {
//
//		// find the row and col
//	const int my_row = id / nx;
//	const int my_col = id % nx;
//	int cnt = 0;
//
//		// for every neighbour
//	for (int row : {my_row - 1, my_row, my_row + 1})
//		for (int col : {my_col - 1, my_col, my_col + 1})
//			if (row != col) {
//
//					// set to be periodic by default
//				neighbours[cnt] = (row % ny) * nx + (col % nx);
//
//					// if non-periodic and outside the grid, set to -1
//				if (!periodic) {
//					if ((row < 0) || (row >= ny))
//						neighbours[cnt] = -1;
//					else if ((col < 0) || (col >= nx))
//						neighbours[cnt] = -1;
//				}
//
//				cnt += 1;
//		}
//
//}
