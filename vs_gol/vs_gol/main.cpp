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
	init_random(subgrid.grid, subgrid.height, subgrid.width);

	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif

		// set the config file
	if (id == 0) {
		subgrid.create_config();

		#ifdef to_time  // time if wanted
				double start = MPI_Wtime();
		#endif // to_time
	}

	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif

		// save the inital states
	subgrid.save_state();


	for (int i = 0; i < iterations; i++) {

		#ifdef synch
			MPI_Barrier(MPI_COMM_WORLD);
		#endif

		subgrid.send_receive();
		subgrid.iterate();
		subgrid.save_state();

	}


	#ifdef to_time  // print the time taken
	if (id == 0) {
			double time_taken = MPI_Wtime() - start;
			cout << "Time to run: " << time_taken << "s\n";
	}
	#endif  // to_time	

	// exit program
	MPI_Finalize();

}
