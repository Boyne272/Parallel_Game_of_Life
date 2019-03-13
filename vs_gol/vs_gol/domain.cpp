#include "Header.h"
#include "GOL_grid.h"

	// basic 50% chance random fill
void init_random(bool* grid, int size) {
	for (int i = 0; i < size; i++)
		grid[i] = rand() % 2;
}

	// place a stick oscillator or centered at row, col
void init_oscillator(bool* grid, int grid_width, int row, int col) {
	for (int i : {row * grid_width + col, 
				  row * grid_width + col + 1,
				  row * grid_width + col - 1})
		grid[i] = true;
}

	// place a 2x2 (stationary block) in each corner
void init_stat_corners(bool* grid, int grid_width, int grid_height) {

	for (int row : {0, 0, grid_height - 2, grid_height - 2})
		for (int col : {0, grid_width - 2, 0, grid_width - 2}) {
			grid[row * grid_width + col] = true;
			grid[row * grid_width + (col + 1)] = true;
			grid[(row + 1) * grid_width + col] = true;
			grid[(row + 1) * grid_width + (col + 1)] = true;
		}

}