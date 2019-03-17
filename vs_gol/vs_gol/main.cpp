#include "Header.h"
#include "GOL_grid.h"

//#define synch		// synch the processes at every stage to assist with dubugging
#define CX1			// turn on saving run-time to times.csv (as well as _congif.txt)

int main(int argc, char *argv[]) {

	// MPI overhead
	int id, p;
	MPI_Init(&argc, &argv);				// startup mpi
	MPI_Comm_rank(MPI_COMM_WORLD, &id);	// find this processors ID
	MPI_Comm_size(MPI_COMM_WORLD, &p);  // find the number of processors
	srand(time(NULL) + id * 10);		// seed the random generator


		// unpack input parameters and throw errors if not enough given
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

		// set inital conditions
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


		// set the config file and start timing
	double start;
	if (id == 0) {
		subgrid.create_config();
		start = MPI_Wtime();
	}

		// save the inital states
	subgrid.save_state();


	#ifdef synch
		MPI_Barrier(MPI_COMM_WORLD);
	#endif


		// iterate
	for (int i = 0; i < iterations; i++) {

		#ifdef synch
			MPI_Barrier(MPI_COMM_WORLD);
		#endif

		subgrid.send_receive();
		subgrid.iterate();
		subgrid.save_state();

	}


		// append additional information to _config.txt
	if (id == 0) {

			// print the time taken
		double time_taken = MPI_Wtime() - start;
		cout << "Time to run: " << time_taken << "s\n";
		cout.flush();

			// write iterations and time taken to _config.txt
		ofstream file;
		file.open(save_directory + "_config.txt", ofstream::app);
		file << "Iterations \t\t" << iterations << "\n"
			 << "Run time \t\t" << time_taken << "\n";
		file.close();

			// record the time in the current directory times.csv file
		#ifdef CX1
			file.open("times.csv", ofstream::app);
			file << p << "," << width << "," << height << "," << iterations << ","
				<< periodic << "," << time_taken << "\n";
			file.close();
		#endif // CX1

	}

	// exit program
	MPI_Finalize();

}
