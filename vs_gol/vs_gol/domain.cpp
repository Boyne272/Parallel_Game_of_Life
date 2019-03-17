#include "Header.h"
#include "GOL_grid.h"

// -------------------- Ititalisation Functions --------------------

	// basic 50% chance random fill
void init_random(bool* grid, int rows, int cols) {
	for (int i = 1; i < rows-1; i++)
		for (int j = 1; j < cols-1; j++)
			grid[i * cols + j] = rand() % 2;
}

	// place a stick oscillator at row, col
void blinker_1(bool* grid, int grid_width, int row, int col) {
	for (int i : {row * grid_width + col, 
				  row * grid_width + col + 1,
				  row * grid_width + col - 1})
		grid[i] = true;
}


	// place a block oscillator at row, col
void blinker_2(bool* grid, int grid_width, int row, int col) {
	for (int i : {row * grid_width + col,
				  row * grid_width + col + 1,
				 (row + 1) * grid_width + col,
				 (row + 2) * grid_width + col + 3,
				 (row + 3) * grid_width + col + 3,
				 (row + 3) * grid_width + col + 2})
		grid[i] = true;
}

	// place a still block at row, col
void still_1(bool* grid, int grid_width, int row, int col) {
	for (int i : {row * grid_width + col,
				  row * grid_width + col + 1,
				 (row + 1) * grid_width + col,
				 (row + 1) * grid_width + col + 1})
		grid[i] = true;
}

	// place a still cross at row, col
void still_2(bool* grid, int grid_width, int row, int col) {
	for (int i : {(row - 1) * grid_width + col,
				  (row + 1) * grid_width + col,
				   row * grid_width + col + 1,
				   row * grid_width + col - 1})
		grid[i] = true;
}


	// place a glider to check boundaries
void glider(bool* grid, int grid_width, int row, int col) {
	for (int i : {(row - 1) * grid_width + col - 1,
				  (row + 1) * grid_width + col,
				  (row + 1) * grid_width + col - 1,
				   row * grid_width + col,
				   row * grid_width + col + 1})
	grid[i] = true;
}

	// fill each corner to check communcations
void fill_corns(bool* grid, int grid_width, int grid_height) {
	grid[grid_width + 1] = true;
	grid[2*grid_width - 2] = true;
	grid[(grid_width - 2) * grid_height + 1] = true;
	grid[(grid_width - 1) * grid_height - 2] = true;
}