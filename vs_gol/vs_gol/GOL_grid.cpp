#include "Header.h"
#include "GOL_grid.h"
#define to_print
//#define synch

	// finds the optimal partitioning with the requierment that
	// every section has matching edge neighbours
void GOL_grid::find_partitions() {

	int comm_len; // the length of edges we need to communicate

	// split into strips as a defualt
	if (tot_x >= tot_y) {  // if x is longer, split along x direction
		n_x = n_p;
		n_y = 1;
	}
	else {
		n_x = 1;
		n_y = n_p;
	}

	// this is the length of internal edges
	comm_len = n_x * tot_y + n_y * tot_x;

	// for every factor of np
	for (int i = 2; i < int(sqrt(n_p) + 1); i++)
		if (n_p % i == 0) {

			const int fac1 = i;
			const int fac2 = n_p / i;

			// if the new comm_len would be less than the old
			if (fac1 * tot_y + fac2 * tot_x < comm_len) {
				n_x = fac1;
				n_y = fac2;
				comm_len = n_x * tot_y + n_y * tot_x;
			}

			// do the same check for the other way around
			if (fac2 * tot_y + fac1 * tot_x < comm_len) {
				n_x = fac2;
				n_y = fac1;
				comm_len = n_x * tot_y + n_y * tot_x;
			}
		}

		// find the row and col of this id in the domain
	my_row = id / n_x;
	my_col = id % n_x;
}


void GOL_grid::find_dimensions() {

		// workout the cells dimensions
	width = tot_x / n_x;
	height = tot_y / n_y;

		// find the remainders
	const int x_rem = tot_x % n_x;
	const int y_rem = tot_y % n_y;

		// equally distribute the remainders
	if (x_rem > my_row)
		width++;
	if (x_rem > my_col)
		height++;

		// pad the width and height to include buffer regions
	height += 2;
	width  += 2;
	size = width * height;


	#ifdef to_print
		cout << id << "-dims(" << height << ", " << width << ")\n";
		cout.flush();
	#endif
}


void GOL_grid::find_neighbours() {

		// order [TL, T, TR, L, R, BL, B, BR]
	const int rows[8] = { my_row - 1, my_row - 1, my_row - 1,
						  my_row, my_row,
						  my_row + 1, my_row + 1, my_row + 1 };
	const int cols[8] = { my_col - 1, my_col, my_col + 1,
						  my_col - 1, my_col + 1,
						  my_col - 1, my_col, my_col + 1 };

		// for every neighbour
	for (int i = 0; i < 8; i++) {

		// set to be periodic by default
		neighbours[i] = (rows[i] % n_y) * n_x + (cols[i] % n_x);

			// if non-periodic and outside the grid, set to -1
		if (!periodic) {
			if ((rows[i] < 0) || (rows[i] >= n_y))
				neighbours[i] = -1;
			else if ((cols[i] < 0) || (cols[i] >= n_x))
				neighbours[i] = -1;
		}
	}
}


	// create the MPI datatype for sending a row of this grid
void GOL_grid::create_MPIrows() {
	
		// find the width that we want to send
	const int len = this->width - 2;

		// length and type of each peice of data
	int * block_lengths = new int[len];
	MPI_Datatype * typelist = new MPI_Datatype[len];
	for (int i = 0; i < len; i++) {
		block_lengths[i] = 1;
		typelist[i] = MPI_BYTE;
	}

		// buffer address and first element address
	MPI_Aint address, add_start;
	MPI_Get_address(this->grid, &add_start);

		// find MPI friendly addresses
	MPI_Aint * offsets = new MPI_Aint[len];
	for (int i = 0; i < len; i++) {
		MPI_Get_address(this->grid + i, &address);
		offsets[i] = address - add_start;
	}

	MPI_Type_create_struct(len, block_lengths, offsets, typelist, &(this->Row));
	MPI_Type_commit(&(this->Row));
		
		// cleanup
	delete[] offsets;
	delete[] block_lengths;
	delete[] typelist;
}


	// create the MPI datatype for sending a column of this grid 
	// (same as create_MPIrow but with a different indexing)
void GOL_grid::create_MPIcols() {

	const int len = this->height - 2;

		// length and type of each peice of data
	int * block_lengths = new int[len];
	MPI_Datatype * typelist = new MPI_Datatype[len];
	for (int i = 0; i < len; i++) {
		block_lengths[i] = 1;
		typelist[i] = MPI_BYTE;
	}

		// buffer address and first element address
	MPI_Aint address, add_start;
	MPI_Get_address(this->grid, &add_start);

	MPI_Aint * offsets = new MPI_Aint[len];
	for (int i = 0; i < len; i++) {
		MPI_Get_address(this->grid + i * width, &address);
		offsets[i] = address - add_start;
	}

	MPI_Type_create_struct(len, block_lengths, offsets, typelist, &(this->Col));
	MPI_Type_commit(&(this->Col));

		// cleanup
	delete[] offsets;
	delete[] block_lengths;
	delete[] typelist;
}


void GOL_grid::find_targets() {
		
		// communcation order [TL, T, TR, L, R, BL, B, BR]

		// corners for the send targets
	send_targs[0] = grid + width + 1;			// top left corn
	send_targs[1] = grid + width + 1;			// top row
	send_targs[2] = grid + 2*width - 2;			// top right corn
	send_targs[3] = grid + width + 1;			// left col
	send_targs[4] = grid + 2*width - 2;			// right col
	send_targs[5] = grid + size - 2*width + 1;	// bot left corn
	send_targs[6] = grid + size - 2*width + 1;	// bot row
	send_targs[7] = grid + size - width - 2;	// bot right corn


		// corners for the receive targets
	recv_targs[0] = grid + size - 1;			// bot right corn
	recv_targs[1] = grid + size - width + 1;	// bot row
	recv_targs[2] = grid + size - width;		// bot left corn
	recv_targs[3] = grid + 2*width - 1;			// right col
	recv_targs[4] = grid + width;				// left col
	recv_targs[5] = grid + width - 1;			// top right corn
	recv_targs[6] = grid + 1;					// top row
	recv_targs[7] = grid;						// top left corn
}


	// constructor to setup the grid and its memory
GOL_grid::GOL_grid(int id, int nprocs, int total_width, int total_height, bool periodic, 
	string save_directory) : id(id), n_p(nprocs), tot_x(total_width), tot_y(total_height), 
	periodic(periodic), directory(save_directory) {

	#ifdef to_print
		cout << id << "-setting\n";
		cout.flush();
	#endif

		// setup the domain partition
	this->find_partitions();
	this->find_dimensions();
	this->find_neighbours();
	this->find_targets();

		// set the memory and initalise grid to be zero everywhere
	this->grid		= new bool[this->size];
	this->tmp_grid	= new bool[this->size];
	for (int i = 0; i < this->size; i++)
		this->grid[i] = 0;

		// create the send MPI_datatypes
	this->create_MPIrows();
	this->create_MPIcols();

}


	// deconstructor to clear memory and close file
GOL_grid::~GOL_grid() {
	delete[] this->grid;
}


	// create the config file and test save directory exists
void GOL_grid::create_config() {
		
		// find the current date as a directory name
	time_t now;
	time(&now);
	char date[26];
	ctime_s(date, 26, &now);

		// what to put in the config file
	ss << "Date \t\t\t" << date
		<< "Processors \t\t" << n_p << "\n"
		<< "Dimensions \t\t(" << tot_x << ", " << tot_y << ")\n"
		<< "Partitions \t\t(" << n_x << ", " << n_y << ")\n";

		// create a config file & check it opened
	file.open(directory + "config.txt", ofstream::out);
	assert(file.is_open() && "could not find the given directory");
	
		// write and close
	file << ss.rdbuf();
	file.close();

		// print
	cout << id << "-saved(config)\n";
	cout.flush();
}


	// print the life boolan grid (same as below with different ptr)
void GOL_grid::print() {
		
		// print a header
	cout << this->id << "-life grid (" << this->height << "," << this->width << ")\n";
	
	for (int i = 0; i < this->size; i++) {
		if ((i % this->width) == 0) cout << "\n";	// print new line 
		cout << "\t" << this->grid[i];			// print data
	}

	cout << "\n";
	cout.flush();
}



	// upadte the life grid to the cells that should be alive now
void GOL_grid::iterate() {

		// increament the iteration counter
	iteration++;

	#ifdef to_print
		cout << id << "-iterating(" << iteration << ")\n";
		cout.flush();
	#endif

	int cnt;

		// for every point in the grid excluding the paddings
	for (int row = 1; row < height - 1; row++)
		for (int col = 1; col < width - 1; col++) {

				// find the adjasent indexs
			const int index = row * width + col;

			//const int adjs[8] = {	index - width - 1,  // top left
			//						index - width,		// top
			//						index - width + 1,	// top right
			//						index - 1,			// left
			//						index + 1,			// right
			//						index + width - 1,	// bot left
			//						index + width,		// bot
			//						index + width + 1};	// bot right

			const int adjs[8] = {	(row - 1) * width + (col - 1),  // top left
									(row - 1) * width + (col    ),	// top
									(row - 1) * width + (col + 1),	// top right
									(row    ) * width + (col - 1),	// left
									(row    ) * width + (col + 1),	// right
									(row + 1) * width + (col - 1),	// bot left
									(row + 1) * width + (col    ),	// bot
									(row + 1) * width + (col + 1)};	// bot right
			
				// count the living adjasents
			cnt = 0;
			for (int i = 0; i < 8; i++)
				cnt += grid[ adjs[i] ];

			if (cnt != 0) {
				cout << row << "\t" << col << "\t" << cnt << '\n';
				cout.flush();
			}

				// find out where is alive
			const bool revive = (cnt == 3);
			const bool stay_alive = ((cnt == 2) && (grid[index]));

				// update the tmp_grid
			if (revive || stay_alive)
				tmp_grid[index] = true;
			else
				tmp_grid[index] = false;
		}

		// swap the grids
	bool* ptr = grid;
	grid = tmp_grid;
	tmp_grid = ptr;

}


	// save the current state in a new file
void GOL_grid::save_state() {

	#ifdef to_print
		cout << id << "-saving(" << iteration << ")\n";
		cout.flush();
	#endif

	for (int row = 1; row < height - 1; row++) {

			// write the first row element without a comma
		const int index = row * width + 1;
		this->ss << grid[index];

			// write every other column
		for (int col = 2; col < width - 1; col++) {
			const int index = row * width + col;
			this->ss << "," << grid[index];
		}
			// new start next row on new line
		this->ss << "\n";
	}

		// create the file for this iteration
	const string name = to_string(iteration) + "-" + to_string(id) + ".csv";
	file.open(directory + name, ofstream::out);

		// write the string stream
	file << ss.rdbuf();

		// close file
	file.close();

	#ifdef to_print
		cout << id << "-saved(" << name << ")\n";
		cout.flush();
	#endif
}


	
void GOL_grid::send_receive() {

	// assume non-periodic for now
	// neighbour order [TL, T, TR, L, R, BL, B, BR]
	// send order [TL, TR, BL, BR]


	MPI_Request* reqs[8];
	MPI_Request dummy_req;
	MPI_Datatype types[8] = { MPI_BYTE, this->Row, MPI_BYTE , this->Col, this->Col,
							  MPI_BYTE, this->Row, MPI_BYTE };

	for (int i = 0; i < 8; i++) {
		const int targ = neighbours[i];
		if (targ != -1) {

			#ifdef to_print
				cout << id << "-sending(" << i << " to " << targ << ")\n";
				//cout << id << "-value(" << (bool)send_targs[i] << ")\n";
				cout.flush();
			#endif

			//MPI_Isend(send_targs[i], 1, types[i], targ, i, MPI_COMM_WORLD, &dummy_req);
			MPI_Isend(grid, 1, types[i], targ, i, MPI_COMM_WORLD, &dummy_req);

			#ifdef to_print
				cout << id << "-receiving(" << i << " from " << neighbours[i] << ")\n";
				cout.flush();
			#endif

			//MPI_Irecv(recv_targs[i], 1, types[i], targ, i, MPI_COMM_WORLD, reqs[i]);
			MPI_Irecv(grid + i, 1, MPI_BYTE, targ, i, MPI_COMM_WORLD, reqs[i]);

		}
		else {

			#ifdef to_print
				cout << id << "-skipping(" << i << ")\n";
				cout.flush();
			#endif

				// send a dud message to yourself to mark request as complete
			//MPI_Isend(send_targs[i], 1, MPI_BYTE, id, i, MPI_COMM_WORLD, reqs[i]);
			//MPI_Request_free(reqs[i]);
		}

		#ifdef synch
			MPI_Barrier(MPI_COMM_WORLD);
		#endif
	}

	//#ifdef to_print
	//	cout << id << "-sending(0)\n";
	//	cout.flush();
	//#endif
	//	// send top left corner recv bot right corner
	//i = 0;
	//t = 7;  // tag is always the recv_target index
	//MPI_Isend(send_targs[i], 1, MPI_BYTE, neighbours[0], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, MPI_BYTE, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//#ifdef to_print
	//	cout << id << "-sending(1)\n";
	//	cout.flush();
	//#endif
	//	// send top row rev bot row
	//i = 0;
	//t = 6;
	//MPI_Isend(send_targs[i], 1, this->Row, neighbours[1], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, this->Row, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//
	//#ifdef to_print
	//	cout << id << "-sending(2)\n";
	//	cout.flush();
	//#endif
	//	// send top right corner recv bot left corner 
	//i = 1;
	//t = 5;
	//MPI_Isend(send_targs[i], 1, MPI_BYTE, neighbours[2], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, MPI_BYTE, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//
	//#ifdef to_print
	//	cout << id << "-sending(3)\n";
	//	cout.flush();
	//#endif
	//	// send left column recv right column
	//i = 0;
	//t = 4;
	//MPI_Isend(send_targs[i], 1, this->Col, neighbours[3], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, this->Col, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//
	//#ifdef to_print
	//	cout << id << "-sending(4)\n";
	//	cout.flush();
	//#endif
	//	// send right column recv left column
	//i = 1;
	//t = 3;
	//MPI_Isend(send_targs[i], 1, this->Col, neighbours[4], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, this->Col, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//
	//#ifdef to_print
	//	cout << id << "-sending(5)\n";
	//	cout.flush();
	//#endif
	//	// send bot left corner recv top right corner
	//i = 2;
	//t = 2;
	//MPI_Isend(send_targs[i], 1, MPI_BYTE, neighbours[5], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, MPI_BYTE, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//		
	//#ifdef to_print
	//	cout << id << "-sending(6)\n";
	//	cout.flush();
	//#endif
	//	// send bot row recv top row
	//i = 2;
	//t = 1;
	//MPI_Isend(send_targs[i], 1, this->Row, neighbours[6], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, this->Row, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);
	//		
	//#ifdef to_print
	//	cout << id << "-sending(7)\n";
	//	cout.flush();
	//#endif
	//	// send bot right corner recv top left corner
	//i = 3;
	//t = 0;
	//MPI_Isend(send_targs[i], 1, MPI_BYTE, neighbours[7], t, MPI_COMM_WORLD, &dummy_req);
	//MPI_Irecv(recv_targs[t], 1, MPI_BYTE, neighbours[t], t, MPI_COMM_WORLD, reqs[t]);

	#ifdef to_print
		cout << id << "-waiting\n";
		cout.flush();
	#endif

	MPI_Waitall(8, reqs[0], MPI_STATUSES_IGNORE);

	#ifdef to_print
		cout << id << "-received\n";
		cout.flush();
	#endif

}