#pragma once
#include "Header.h"

class GOL_grid {
public:

		// constructor to assign memory
	GOL_grid(int id, int nprocs, int total_width, int total_height,
			bool periodic, string save_directory);

		// deconstructor to clear memory
	~GOL_grid();

		// domain info (some given, some found)
	int tot_x = -1, tot_y = -1;		// domain dimenaions
	int n_x = -1, n_y = -1;			// number of partitions
	int my_row = -1, my_col = -1;	// subgrid position in domain
	int id = -1, n_p = -1;			// id and number of processors
	bool periodic;				    // is the domain periodic

		// subgrid data
	int width = -1, height = -1, size = -1;
	int iteration = 0;
	bool* grid = nullptr;
	bool* tmp_grid = nullptr;

		// setup functions
	void find_partitions();
	void find_dimensions();
	void find_neighbours();
	void print();

		// iterate
	void send_receive();
	void iterate();

		// communcations
	int neighbours[8];		// order [TL, T, TR, L, R, BL, B, BR]
	bool* send_targs[8];
	bool* recv_targs[8];
	void find_targets();
	MPI_Datatype Row, Col;
	void create_MPIrows();
	void create_MPIcols();

		// savefile
	string directory;
	stringstream ss;
	ofstream file;
	void create_config();
	void save_state();

};