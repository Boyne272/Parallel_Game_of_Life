#include "Header.h"
//#define to_time

int main(int argc, char *argv[]) {

	// MPI overhead
	int id, p;
	MPI_Init(&argc, &argv);				// startup mpi
	MPI_Comm_rank(MPI_COMM_WORLD, &id);	// find this processors ID
	MPI_Comm_size(MPI_COMM_WORLD, &p);  // find the number of processors
	srand(time(NULL) + id * 10);		// seed the random generator


		// unpack input parameters if they were passed
	if (argc > 1) {
	}

	// run processor commands
	if (id == 0) {

#ifdef to_time  // time if wanted
		double start = MPI_Wtime();
#endif // to_time

		main_processor(p);  // call main processor

#ifdef to_time  // print the time taken
		double time_taken = MPI_Wtime() - start;
		cout << "Time to run: " << time_taken << "s\n";
#endif  // to_time	
	}
	else {
		general_processor(p, id);
	}

	// exit program
	MPI_Finalize();

}
