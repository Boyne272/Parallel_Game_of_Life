#pragma once
#include "Header.h"

class GOL_grid {
public:

		// constructor to assign memory
	GOL_grid(int m, int n, int id, int px, int py);
		// deconstructor to clear memory
	~GOL_grid();

		// wholegrid dimensions
	int px = -1;
	int py = -1;
	int id = -1;

		// subgrid dimensions
	int width = -1;
	int height = -1;
	int size = -1;

		// data
	bool* life_grid;	// the grid of alive points
	int* adj_grid;		// to count the number of alive adjasent points
	void print_life(); 	// print the life boolan grid
	void print_adj();  	// print the adjasent integer grid

		// iterate
	//void send_receive();
	//void count_boundaries();
	void count_local();
	void update_life();

		// communcations
	int neighbours[4];
	bool* boundaries[4];
	MPI_Datatype Row, Col;
	void create_MPIrow();
	void create_MPIcol();

		// savefile
	string filename;
	stringstream ss;
	ofstream file;
	void save_state();

};