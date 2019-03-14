#include "Header.h"
#include "GOL_grid.h"

	// constructor to setup the grid and its memory
GOL_grid::GOL_grid(int m, int n, int id, int px, int py) :
	height(m), width(n), size(m*n), id(id), px(px), py(py) {

		// set the memory
	this->life_grid = new bool[this->size];
	this->adj_grid = new int[this->size];

		// initalise grids to be zero everywhere
	for (int i = 0; i < this->size; i++)
		adj_grid[i] = life_grid[i] = 0;

		// create the send MPI_datatypes
	this->create_MPIrow();
	this->create_MPIcol();

		// open the save file
	time_t now;
	time(&now);
	char date[26];
	ctime_s(date, 26, &now);
	date[7] = date[10] = date[13] = date[16] = '-';
	this->filename = string(date).substr(4, 15) + "-" + to_string(id) + ".csv";
	this->file.open(filename, ofstream::out);

		// check file opened correctly
	if (!this->file.is_open()) {
		cout << "Error: could not open " << filename << "\n";
		cout.flush();
	}

		// put in the header
	this->ss	<< "# simulation ran on " << date
				<< "# id, width, height, px, py\n"
				<< this->id << "," << this->width << "," << this->height << "," 
				<< this->px << "," << this->py << "\n";
	this->file << this->ss.rdbuf();
}


	// deconstructor to clear memory and close file
GOL_grid::~GOL_grid() {
	delete[] this->life_grid;
	delete[] this->adj_grid;
	this->file.close();
}


	// print the life boolan grid (same as below with different ptr)
void GOL_grid::print_life() {
		
		// print a header
	cout << this->id << "-life grid (" << this->height << "," << this->width << ")\n";
	
	for (int i = 0; i < this->size; i++) {
		if ((i % this->width) == 0) cout << "\n";	// print new line 
		cout << "\t" << this->life_grid[i];			// print data
	}

	cout << "\n";
	cout.flush();
}


	// print the life boolan grid (same as above with different ptr)
void GOL_grid::print_adj() {

	// print a header
	cout << this->id << "-adj grid (" << this->height << "," << this->width << ")\n";

	for (int i = 0; i < this->size; i++) {
		if ((i % this->width) == 0) cout << "\n";	// print new line 
		cout << "\t" << this->adj_grid[i];			// print data
	}

	cout << "\n";
	cout.flush();
}


	// create the MPI datatype for sending a row of this grid
void GOL_grid::create_MPIrow() {
	
	const int len = this->width;

		// length and type of each peice of data
	int * block_lengths = new int[len];
	MPI_Datatype * typelist = new MPI_Datatype[len];
	for (int i = 0; i < len; i++) {
		block_lengths[i] = 1;
		typelist[i] = MPI_INT;
	}

		// buffer address and first element address
	MPI_Aint address, add_start;
	MPI_Get_address(this->life_grid, &add_start);

	
		// find the MPI friendly offsets
	MPI_Aint * offsets = new MPI_Aint[len];
	for (int i = 0; i < len; i++) {
		MPI_Get_address(this->life_grid + i, &address);
		offsets[i] = address - add_start;
	}

	MPI_Type_create_struct(len, block_lengths, offsets, typelist, &(this->Row));
	MPI_Type_commit(&(this->Row));

	// cleanup
	delete[] block_lengths;
	delete[] offsets;
	delete[] typelist;
}


	// create the MPI datatype for sending a column of this grid 
	// (same as create_MPIrow but with a different indexing)
void GOL_grid::create_MPIcol() {

	const int len = this->height;
	const int skip = this->width;

	// length and type of each peice of data
	int * block_lengths = new int[len];
	MPI_Datatype * typelist = new MPI_Datatype[len];
	for (int i = 0; i < len; i++) {
		block_lengths[i] = 1;
		typelist[i] = MPI_INT;
	}

	// buffer address and first element address
	MPI_Aint address, add_start;
	MPI_Get_address(this->life_grid, &add_start);


	// find the MPI friendly offsets
	MPI_Aint * offsets = new MPI_Aint[len];
	for (int i = 0; i < len; i++) {
		MPI_Get_address(this->life_grid + i * skip, &address);
		offsets[i] = address - add_start;
	}

	MPI_Type_create_struct(len, block_lengths, offsets, typelist, &(this->Row));
	MPI_Type_commit(&(this->Row));

	// cleanup
	delete[] block_lengths;
	delete[] offsets;
	delete[] typelist;
}


	// update the adj grid to count the cells currently alive 
void GOL_grid::count_local() {
	// rather than looping over all cells and using if statments
	// I do a seperate for loop for each type of cell (interior, left boundary, etc . . .)
	// to prevent using if statments to check what I already would know
	// this does make the code rather long and tedious

		// all interior cells
	for (int row = 1; row < this->height - 1; row++)
		for (int col = 1; col < this->width - 1; col++) {
			if (this->life_grid[row * this->width + col]) {  // if this cell is alive
				this->adj_grid[(row - 1) * this->width + (col - 1)] += 1;	// top-left cell
				this->adj_grid[(row - 1) * this->width + col]		+= 1;	// top cell
				this->adj_grid[(row - 1) * this->width + (col + 1)] += 1;	// top-right cell
				this->adj_grid[row * this->width + (col + 1)]		+= 1;	// right cell
				this->adj_grid[(row + 1) * this->width + (col + 1)] += 1;	// bot-right cell
				this->adj_grid[(row + 1) * this->width + col]		+= 1;	// bot cell
				this->adj_grid[(row + 1) * this->width + (col - 1)]	+= 1;	// bot-left cell
				this->adj_grid[row * this->width + (col - 1)]		+= 1;	// left cell
			}
		}

		// left wall cells
	for (int row = 1; row < this->height - 1; row++) {
		const int col = 0;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[(row - 1) * this->width + col]		+= 1;	// top cell
			this->adj_grid[(row - 1) * this->width + (col + 1)] += 1;	// top-right cell
			this->adj_grid[row * this->width + (col + 1)]		+= 1;	// right cell
			this->adj_grid[(row + 1) * this->width + (col + 1)] += 1;	// bot-right cell
			this->adj_grid[(row + 1) * this->width + col]		+= 1;	// bot cell
			}
	}

		// right wall cells
	for (int row = 1; row < this->height - 1; row++) {
		const int col = this->width - 1;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[(row - 1) * this->width + (col - 1)] += 1;	// top-left cell
			this->adj_grid[(row - 1) * this->width + col]		+= 1;	// top cell
			this->adj_grid[(row + 1) * this->width + col]		+= 1;	// bot cell
			this->adj_grid[(row + 1) * this->width + (col - 1)]	+= 1;	// bot-left cell
			this->adj_grid[row * this->width + (col - 1)]		+= 1;	// left cell
		}
	}

		// top wall cells
	for (int col = 1; col < this->width - 1; col++) {
		const int row = 0;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[row * this->width + (col + 1)]		+= 1;	// right cell
			this->adj_grid[(row + 1) * this->width + (col + 1)] += 1;	// bot-right cell
			this->adj_grid[(row + 1) * this->width + col]		+= 1;	// bot cell
			this->adj_grid[(row + 1) * this->width + (col - 1)]	+= 1;	// bot-left cell
			this->adj_grid[row * this->width + (col - 1)]		+= 1;	// left cell
		}
	}

		// bottom wall cells
	for (int col = 1; col < this->width - 1; col++) {
		const int row = this->height - 1;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[(row - 1) * this->width + (col - 1)] += 1;	// top-left cell
			this->adj_grid[(row - 1) * this->width + col]		+= 1;	// top cell
			this->adj_grid[(row - 1) * this->width + (col + 1)] += 1;	// top-right cell
			this->adj_grid[row * this->width + (col + 1)]		+= 1;	// right cell
			this->adj_grid[row * this->width + (col - 1)]		+= 1;	// left cell
		}
	}

		// top-left diagonal
	{
		const int row = 0;
		const int col = 0;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[row * this->width + (col + 1)]		+= 1;	// right cell
			this->adj_grid[(row + 1) * this->width + (col + 1)] += 1;	// bot-right cell
			this->adj_grid[(row + 1) * this->width + col]		+= 1;	// bot cell
		}
	}

		// top-right diagonal
	{
		const int row = 0;
		const int col = this->width - 1;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[(row + 1) * this->width + col]		+= 1;	// bot cell
			this->adj_grid[(row + 1) * this->width + (col - 1)]	+= 1;	// bot-left cell
			this->adj_grid[row * this->width + (col - 1)]		+= 1;	// left cell
		}
	}

		// bot-left diagonal
	{
		const int row = this->height - 1;
		const int col = 0;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[(row - 1) * this->width + col]		+= 1;	// top cell
			this->adj_grid[(row - 1) * this->width + (col + 1)] += 1;	// top-right cell
			this->adj_grid[row * this->width + (col + 1)]		+= 1;	// right cell
		}
	}


	// bot-right diagonal
	{
		const int row = this->height - 1;
		const int col = this->width - 1;
		if (this->life_grid[row * this->width + col]) {  // if this cell is alive
			this->adj_grid[(row - 1) * this->width + (col - 1)] += 1;	// top-left cell
			this->adj_grid[(row - 1) * this->width + col]		+= 1;	// top cell
			this->adj_grid[row * this->width + (col - 1)]		+= 1;	// left cell
		}
	}

}


	// upadte the life grid to the cells that should be alive now
void GOL_grid::update_life() {

		// for all cells
	for (int i = 0; i < this->size; i++) {
		
		if (this->adj_grid[i] == 3)  // if 3 around you are still alive or revived
			this->life_grid[i] = true;

		else if ((this->adj_grid[i] == 2) && life_grid[i])  // if 2 you are still alive
			this->life_grid[i] = true;

		else  // else you die or stay dead
			this->life_grid[i] = false;

			// reset the adj grid count
		this->adj_grid[i] = 0;
	}
}


void GOL_grid::save_state() {
	for (int r = 0; r < this->height; r++) {
		
		this->ss << this->life_grid[r * this->width];  // write the first
		for (int c = 1; c < this->width; c++) 
			this->ss << "," << this->life_grid[r * this->width + c];
		this->ss << "\n";
	}
	this->file << ss.rdbuf();
}