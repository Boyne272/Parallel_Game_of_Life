#include "Header.h"
#include "GOL_grid.h"

	// here I am testing a single GOL grid with no communication sink boundaries
//void main_processor(int p, int id) {
void test_basic() {

		// create an example 10 x 12 grid
	GOL_grid test(10, 12, 0, 1, 1);

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

	// here I am testing saving on a single GOL grid with no communication sink boundaries
void main_processor(int p, int id) {
//void test_basic() {

	cout << "running\n";

		// create an example 10 x 12 grid
	GOL_grid test(50, 60, 0, 1, 1);

	cout << test.filename;

	init_random(test.life_grid, test.size);
	//init_oscillator(test.life_grid, test.width, 1, 1);
	//init_oscillator(test.life_grid, test.width, 5, 5);
	//init_stat_corners(test.life_grid, test.width, test.height);
	
		// save the inital state
	test.save_state();

	for (int i = 0; i < 1000; i++) {
		test.count_local();
		test.update_life();
		test.save_state();
	}

}