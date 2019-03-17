#pragma once
#include "Header.h"

class GOL_grid {
public:

		// constructor to setup the grid and its memory
	GOL_grid(int id, int nprocs, int total_width, int total_height,
			bool periodic, string save_directory);

		// deconstructor to clear dynamic memory
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
	void find_partitions();		// finds the optimal regular grid partitioning for domain
	void find_dimensions();		// find the padded dimensions of this processors subgrid
	void find_neighbours();		// find the neighbours to message (changes if periodic or not)

		// iterate
	void send_receive();	// send boundary data to adjasent processors
	void iterate();			// iterate the life grid

		// communcations
	int neighbours[8];		// message order [TL, T, TR, L, R, BL, B, BR]
	int send_offset[8];		// the offsets from grid pointer for each send message
	int recv_offset[8];		// the offsets from grid pointer for each recv message
	void find_targets();	// find the send and recv offsets
	MPI_Datatype Row, Col;	// datatypes for sending rows and columns
	void create_MPIrows();	// create datatype for sending an interior row
	void create_MPIcols();	// create datatype for sending an interior column

		// misc
	void print();	// print the current subgrid state

		// savefile
	string directory;		// the location to save results
	stringstream ss;		// buffer for writing to a file
	ofstream file;			// file vairable to save with
	void create_config();	// create the config file and test save directory exists
	void save_state();		// save the current state in a new file

};