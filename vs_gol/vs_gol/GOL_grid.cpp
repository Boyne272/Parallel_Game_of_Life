#include "Header.h"
#include "GOL_grid.h"

//#define to_print		// print when each processor enters each section
//#define to_print_all	// print explicity what each processor is doing
//#define synch			// synch the processes at every stage
#define CX1				// disable ctime_s in create_config() for gcc compiling


	// finds the optimal regular grid partitioning for domain
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

				// if the new comm_len would be less than the old use it
			if (fac1 * tot_y + fac2 * tot_x < comm_len) {
				n_x = fac1;
				n_y = fac2;
				comm_len = n_x * tot_y + n_y * tot_x;
			}

				// do the same check for flipped axis
			if (fac2 * tot_y + fac1 * tot_x < comm_len) {
				n_x = fac2;
				n_y = fac1;
				comm_len = n_x * tot_y + n_y * tot_x;
			}
		}

	#ifdef to_print
		cout << id << "-patritions(" << n_y << ", " << n_x << ")\n";
		cout.flush();
	#endif

		// find the row and col of this id in the domain
	my_row = id / n_x;
	my_col = id % n_x;

}


	// find the padded dimensions of this processors subgrid
void GOL_grid::find_dimensions() {

		// workout the cells dimensions
	width = tot_x / n_x;
	height = tot_y / n_y;

		// find the remainders
	const int x_rem = tot_x % n_x;
	const int y_rem = tot_y % n_y;

		// equally distribute the remainders
	if (x_rem > my_col)
		width++;
	if (y_rem > my_row)
		height++;

		// pad the width and height to include buffer regions
	height += 2;
	width  += 2;
	size = width * height;

	#ifdef to_print
		cout << id << "-dims(" << height-2 << ", " << width-2 << ")\n";
		//cout << id << "-padd(" << height << ", " << width << ")\n";
		//cout << id << "-remainders(" << x_rem << ", " << y_rem << ")\n";
		cout.flush();
	#endif
}


	// find the neighbours to message (changes if periodic or not)
void GOL_grid::find_neighbours() {

		// message order [TL, T, TR, L, R, BL, B, BR]
	const int rows[8] = { my_row - 1, my_row - 1, my_row - 1,
						  my_row, my_row,
						  my_row + 1, my_row + 1, my_row + 1 };
	const int cols[8] = { my_col - 1, my_col, my_col + 1,
						  my_col - 1, my_col + 1,
						  my_col - 1, my_col, my_col + 1 };

		// for every neighbour
	for (int i = 0; i < 8; i++) {

		if (periodic) {
				// mod to wrap around (after + max to prevent negative modulus)
			const int mod_row = (rows[i] + n_y) % n_y;
			const int mod_col = (cols[i] + n_x) % n_x;
			neighbours[i] = mod_row * n_x + mod_col;
		}

		else {
				// if non-periodic set outside grid pointes to -1
			if ((rows[i] < 0) || (rows[i] >= n_y))
				neighbours[i] = -1;

			else if ((cols[i] < 0) || (cols[i] >= n_x))
				neighbours[i] = -1;

			else 
				neighbours[i] = rows[i] * n_x + cols[i];
		}
	}

	#ifdef to_print
		cout << id << "-neighbours(";
		for (int i = 0; i < 8; i++)
			cout << neighbours[i] << " ";
		cout << ")\n";
	#endif
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

		// create the datatype
	MPI_Type_create_struct(len, block_lengths, offsets, typelist, &(this->Row));
	MPI_Type_commit(&(this->Row));
		
		// cleanup
	delete[] offsets;
	delete[] block_lengths;
	delete[] typelist;
}


	// create MPI col datatype (same as above but with a different indexing)
void GOL_grid::create_MPIcols() {

		// find the width that we want to send
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

		// find MPI friendly addresses
	MPI_Aint * offsets = new MPI_Aint[len];
	for (int i = 0; i < len; i++) {
		MPI_Get_address(this->grid + i * width, &address);
		offsets[i] = address - add_start;
	}

		// create the datatype
	MPI_Type_create_struct(len, block_lengths, offsets, typelist, &(this->Col));
	MPI_Type_commit(&(this->Col));

		// cleanup
	delete[] offsets;
	delete[] block_lengths;
	delete[] typelist;
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


	// deconstructor to clear dynamic memory
GOL_grid::~GOL_grid() {
	delete[] this->grid;
	delete[] this->tmp_grid;
}


	// create the config file and test save directory exists
void GOL_grid::create_config() {
		
		// find the current date for config file
	#ifdef CX1
		time_t now;
		time(&now);
		char date[26];
		ctime_s(date, 26, &now);
	#else
		char date[26] = "not avaibale on CX1 :( \n";
	#endif

		// what to put in the config file
	ss << "Date \t\t\t" << date
		<< "Processors \t\t" << n_p << "\n"
		<< "Dimensions \t\t(" << tot_y << ", " << tot_x << ")\n"
		<< "Partitions \t\t(" << n_y << ", " << n_x << ")\n";

		// create a config file & check it opened
	file.open(directory + "_config.txt", ofstream::out);
	assert(file.is_open() && "could not find the given directory");
	
		// write and close
	file << ss.rdbuf();
	file.close();

		// print
	#ifdef to_print
		cout << id << "-saved(config)\n";
		cout.flush();
	#endif // to_print
}


	// print the current subgrid state
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


	// iterate the life grid
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

				// find the adjasent and current points index
			const int index = row * width + col;
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

	#ifdef to_print_all
		cout << id << "-saving(" << iteration << ")\n";
		cout.flush();
	#endif

		// for every non-padded row
	for (int row = 1; row < height - 1; row++) {

			// write the first row element without a comma
		const int index = row * width + 1;
		this->ss << grid[index];

			// write every other non-padded column
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

		// write the string stream and close the file
	file << ss.rdbuf();
	file.close();

	#ifdef to_print
		cout << id << "-saved(" << name << ")\n";
		cout.flush();
	#endif
}


	// find the offsets that are needed for communcations
void GOL_grid::find_targets() {
		
		// communcation order [TL, T, TR, L, R, BL, B, BR]

		// ofsets to the corners for the send targets
	send_offset[0] = width + 1;				// top left corn
	send_offset[1] = width + 1;				// top row
	send_offset[2] = 2*width - 2;			// top right corn
	send_offset[3] = width + 1;				// left col
	send_offset[4] = 2*width - 2;			// right col
	send_offset[5] = size - 2*width + 1;	// bot left corn
	send_offset[6] = size - 2*width + 1;	// bot row
	send_offset[7] = size - width - 2;		// bot right corn

		// ofsets to the corners for the receive targets
	recv_offset[0] = 0;					// top left corn
	recv_offset[1] = 1;					// top row
	recv_offset[2] = width - 1;			// top right corn
	recv_offset[3] = width;				// left col
	recv_offset[4] = 2*width - 1;		// right col
	recv_offset[5] = size - width;		// bot left corn
	recv_offset[6] = size - width + 1;	// bot row
	recv_offset[7] = size - 1;			// bot right corn
}

	
	// send boundary data to adjasent processors
void GOL_grid::send_receive() {

	// communcations order [TL, T, TR, L, R, BL, B, BR]
	// receive order is the reverse

	MPI_Request reqs[8];
	MPI_Request dummy_req;
	MPI_Datatype types[8] = { MPI_BYTE, this->Row, MPI_BYTE , this->Col, this->Col,
							  MPI_BYTE, this->Row, MPI_BYTE };

		// for every neighbour
	for (int i = 0; i < 8; i++) {

			// if neighbour exists
		if (neighbours[i] != -1) {

			#ifdef to_print_all
				cout << id << "-sending(" << i << " to " << targ << ")\n";
				cout.flush();
			#endif

				// send data to them
			MPI_Isend(grid + send_offset[i], 1, types[i], neighbours[i], 
				i, MPI_COMM_WORLD, &dummy_req);

			#ifdef to_print_all
				cout << id << "-receiving(" << i << " from " << neighbours[i] << ")\n";
				cout.flush();
			#endif

				// wait to recieve from them on the correct communcation tag (reverse order)
			MPI_Irecv(grid + recv_offset[i], 1, types[i], neighbours[i], 
				7-i, MPI_COMM_WORLD, reqs + i);

		}
		
			// no neighbour to send to (sink boudnary)
		else {

			#ifdef to_print_all
				cout << id << "-skipping(" << i << ")\n";
				cout.flush();
			#endif

				// send a dud message to yourself to mark request as complete
			MPI_Isend(nullptr, 0, MPI_BYTE, id, i, MPI_COMM_WORLD, reqs + i);
			MPI_Request_free(reqs + i);

		}

		#ifdef synch
			MPI_Barrier(MPI_COMM_WORLD);
		#endif
	}

	#ifdef to_print_all
		cout << id << "-waiting(" << iteration << ")\n";
		cout.flush();
	#endif

	MPI_Waitall(8, reqs, MPI_STATUSES_IGNORE);

	#ifdef to_print
		cout << id << "-received" << iteration << "\n";
		cout.flush();
	#endif

}