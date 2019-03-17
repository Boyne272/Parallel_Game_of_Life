
// -------------------------- Header.h --------------------------

#include <mpi.h>	// MPI functions
#include <iostream>	// standard input/output streams
#include <cstdlib>	// random number functions
#include <time.h>	// for seeding the random generator with the current time
#include <fstream>	// writing file outputs
#include <sstream>	// for a buffer when writing to file
#include <string>	// for coverting variables to string
#include <assert.h> // for debugging

using namespace std;

	// domain.cpp functions
void init_random(bool* grid, int rows, int cols);
void blinker_1(bool* grid, int grid_width, int row, int col);
void blinker_2(bool* grid, int grid_width, int row, int col);
void still_1(bool* grid, int grid_width, int row, int col);
void still_2(bool* grid, int grid_width, int row, int col);
void glider(bool* grid, int grid_width, int row, int col);
void fill_corns(bool* grid, int grid_width, int grid_heights);


// -------------------------- GOL_grid.h --------------------------

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
	int send_offset[8];
	int recv_offset[8];
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


// -------------------------- GOL_grid.cpp --------------------------

//#define to_print
//#define to_print_all
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

	#ifdef to_print
		cout << id << "-patritions(" << n_y << ", " << n_x << ")\n";
		cout.flush();
	#endif

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


void GOL_grid::find_neighbours() {

	// order [TL, T, TR, L, R, BL, B, BR]
	// add the number of rows and cols to ensure +ve values for % operator
	//const int rows[8] = { my_row - 1 + n_y, my_row - 1 + n_y, my_row - 1 + n_y,
	//					  my_row + n_y, my_row + n_y,
	//					  my_row + 1 + n_y, my_row + 1 + n_y, my_row + 1 + n_y };
	//const int cols[8] = { my_col - 1 + n_x, my_col + n_x, my_col + 1 + n_x,
	//					  my_col - 1 + n_x, my_col + 1 + n_x,
	//					  my_col - 1 + n_x, my_col + n_x, my_col + 1 + n_x };	
	const int rows[8] = { my_row - 1, my_row - 1, my_row - 1,
						  my_row, my_row,
						  my_row + 1, my_row + 1, my_row + 1 };
	const int cols[8] = { my_col - 1, my_col, my_col + 1,
						  my_col - 1, my_col + 1,
						  my_col - 1, my_col, my_col + 1 };

	// for every neighbour
	for (int i = 0; i < 8; i++) {

		if (periodic) {
			// mod to wrap around (after + row_max/col_max to prevent negative modulos)
			const int mod_row = (rows[i] + n_y) % n_y;
			const int mod_col = (cols[i] + n_x) % n_x;
			neighbours[i] = mod_row * n_x + mod_col;
		}

		// if non-periodic set outside grid pointes to -1
		else {
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
		<< "Dimensions \t\t(" << tot_y << ", " << tot_x << ")\n"
		<< "Partitions \t\t(" << n_y << ", " << n_x << ")\n";

		// create a config file & check it opened
	file.open(directory + "_config.txt", ofstream::out);
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

	
void GOL_grid::send_receive() {

	// assume non-periodic for now
	// neighbour order [TL, T, TR, L, R, BL, B, BR]
	// send order [TL, TR, BL, BR]

	MPI_Request reqs[8];
	MPI_Request dummy_req;
	MPI_Datatype types[8] = { MPI_BYTE, this->Row, MPI_BYTE , this->Col, this->Col,
							  MPI_BYTE, this->Row, MPI_BYTE };

	for (int i = 0; i < 8; i++) {
		const int targ = neighbours[i];
		if (targ != -1) {

			#ifdef to_print_all
				cout << id << "-sending(" << i << " to " << targ << ")\n";
				cout.flush();
			#endif

			MPI_Isend(grid + send_offset[i], 1, types[i], targ, i, MPI_COMM_WORLD, &dummy_req);

			#ifdef to_print_all
				cout << id << "-receiving(" << i << " from " << neighbours[i] << ")\n";
				cout.flush();
			#endif

			MPI_Irecv(grid + recv_offset[i], 1, types[i], targ, 7-i, MPI_COMM_WORLD, reqs + i);

		}
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


// -------------------- domain.cpp --------------------

	// basic 50% chance random fill
void init_random(bool* grid, int rows, int cols) {
	for (int i = 1; i < rows; i++)
		for (int j = 1; j < cols; j++)
			grid[i * rows + j] = rand() % 2;
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


void glider(bool* grid, int grid_width, int row, int col) {
	for (int i : {(row - 1) * grid_width + col - 1,
				  (row + 1) * grid_width + col,
				  (row + 1) * grid_width + col - 1,
				   row * grid_width + col,
				   row * grid_width + col + 1})
	grid[i] = true;
}

void fill_corns(bool* grid, int grid_width, int grid_height) {
	grid[grid_width + 1] = true;
	grid[2*grid_width - 2] = true;
	grid[(grid_width - 2) * grid_height + 1] = true;
	grid[(grid_width - 1) * grid_height - 2] = true;
}



// --------------------- main.cpp --------------------------


//#define synch

int main(int argc, char *argv[]) {

	// MPI overhead
	int id, p;
	MPI_Init(&argc, &argv);				// startup mpi
	MPI_Comm_rank(MPI_COMM_WORLD, &id);	// find this processors ID
	MPI_Comm_size(MPI_COMM_WORLD, &p);  // find the number of processors
	srand(time(NULL) + id * 10);		// seed the random generator


		// unpack input parameters if they were passed
	assert(argc == 6 && "a save directory must be passed");
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int iterations = atoi(argv[3]);
	bool periodic = (bool)atoi(argv[4]);
	string save_directory = argv[5];

	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif


		// setup the subgrid
	GOL_grid subgrid(id, p, width, height, periodic, save_directory);

	init_random(subgrid.grid, subgrid.height, subgrid.width);

	//blinker_1(subgrid.grid, subgrid.width, 4, 4);
	//blinker_2(subgrid.grid, subgrid.width, 12, 4);
	//still_1(subgrid.grid, subgrid.width, 4, 12);
	//still_2(subgrid.grid, subgrid.width, 12, 12);
	//glider(subgrid.grid, subgrid.width, 5, 5);
	//fill_corns(subgrid.grid, subgrid.height, subgrid.width);

	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif

		// set the config file
	double start;
	if (id == 0) {
		subgrid.create_config();
		start = MPI_Wtime();
	}

	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif

		// save the inital states
	subgrid.save_state();

		// iterate
	for (int i = 0; i < iterations; i++) {

		#ifdef synch
			MPI_Barrier(MPI_COMM_WORLD);
		#endif

		subgrid.send_receive();
		subgrid.iterate();
		subgrid.save_state();

	}

		// append information to config
	if (id == 0) {
		double time_taken = MPI_Wtime() - start;
		cout << "Time to run: " << time_taken << "s\n";
		cout.flush();

		ofstream file;
		file.open(save_directory + "_config.txt", ofstream::app);
		file << "Iterations \t\t" << iterations << "\n"
			 << "Run time \t\t" << time_taken << "s\n";
		file.close();
	}

	// exit program
	MPI_Finalize();

}
