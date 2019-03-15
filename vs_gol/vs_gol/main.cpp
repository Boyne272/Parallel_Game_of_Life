#include "Header.h"
#include "GOL_grid.h"
//#define to_time
#define synch

int main(int argc, char *argv[]) {

	// MPI overhead
	int id, p;
	MPI_Init(&argc, &argv);				// startup mpi
	MPI_Comm_rank(MPI_COMM_WORLD, &id);	// find this processors ID
	MPI_Comm_size(MPI_COMM_WORLD, &p);  // find the number of processors
	srand(time(NULL) + id * 10);		// seed the random generator


		// unpack input parameters if they were passed
	assert(argc == 5 && "a save directory must be passed");
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int iterations = atoi(argv[3]);
	string save_directory = argv[4];

	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif

		// setup the subgrid
	GOL_grid subgrid(id, p, width, height, false, save_directory);

	//init_random(subgrid.grid, subgrid.height, subgrid.width);

	//blinker_1(subgrid.grid, subgrid.width, 4, 4);
	//blinker_2(subgrid.grid, subgrid.width, 12, 4);
	//still_1(subgrid.grid, subgrid.width, 4, 12);
	//still_2(subgrid.grid, subgrid.width, 12, 12);

	glider(subgrid.grid, subgrid.width, 5, 5);
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
		ofstream file;
		file.open(save_directory + "config.txt", ofstream::app);
		file << "Iterations \t\t" << iterations << "\n"
			 << "Run time \t\t" << time_taken << "\n";
		file.close();

		#ifdef print
			cout << "Time to run: " << time_taken << "s\n";
			cout.flush();
		#endif
	}

	// exit program
	MPI_Finalize();

}
