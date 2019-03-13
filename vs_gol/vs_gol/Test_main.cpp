#include "Header.h"
#include "GOL_grid.h"

	// here I am testing a single GOL grid with no communication sink boundaries
void main_processor(int p, int id) {

		// create an example 10 x 12 grid
	GOL_grid test(10, 12, 0);

	//init_random(test.life_grid, test.size);
	init_oscillator(test.life_grid, test.width, 1, 1);
	init_oscillator(test.life_grid, test.width, 5, 5);
	init_stat_corners(test.life_grid, test.width, test.height);
	
	
	cout << "Ititial setup:\n";
	test.print_life();
	test.print_adj();

		// do an iteration
	cout << "In iteration 1:\n";
	test.count_local();
	test.print_adj();

	cout << "In Post Iteration 1:\n";
	test.update_life();
	test.print_life();
	test.print_adj();

		// do another iteration
	cout << "In iteration 2:\n";
	test.count_local();
	test.print_adj();

	cout << "In Post Iteration 2:\n";
	test.update_life();
	test.print_life();
	test.print_adj();
}