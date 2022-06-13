#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MASTER 0
  
void initArray(int * data, const int size)  
{	
	int i = 0;
	for(i = 0; i <= size; i++){
		// create an data of ones since result is easy to determine, i.e. equal to size
		data[i] = i;
	}
}
  
int main(int argc, char* argv[])
{
	const int ArraySize = 6;
	int data[ArraySize];
	
	// initialise the data, we can always initialise our variable before MPI init
	initArray(data, ArraySize);
	// create placeholders that we will use later
	int nprocesses = 0, currentProcess = 0, sum=0;
	int index = 0, i = 0;
	
	// Start MPI
	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &currentProcess); // get current process ID
	MPI_Comm_size(MPI_COMM_WORLD, &nprocesses); //get number of processes	
		
	// We will be using scatter and reduction. Scatter basically split our array into chunks and distributes to all processes
	// Reduce basically aggregates all out local sums
	/*int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
               MPI_Comm comm)
	*/
	int localSum = 0;
	int chunksize = ArraySize / nprocesses;
	int partialSum[nprocesses];
	int chunks[chunksize]; // we will create chunk array for each process
	
	MPI_Scatter(data, chunksize, MPI_INT, chunks, chunksize, MPI_INT, 0, MPI_COMM_WORLD);
	// At this points our chunks have been set, we can do the operation for each process
	for(i = 0; i < chunksize; i++){
		localSum += chunks[i];
	}
	// Once every process has done the summation we can reduce the result for each process
	 /*
		int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
				   MPI_Op op, int root, MPI_Comm comm)
	*/
	MPI_Reduce(&localSum, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(currentProcess == MASTER){
		// we need to first deal with the case of imbalance workload
		int ignoredChunks = ArraySize%nprocesses;
		// we use the ignored chunks to sum last elements that were not added
		for(i = ArraySize-ignoredChunks; i < ArraySize; i++){
			sum += data[i];
		}
		
		printf("%s\t: %d\n", "The total sum is", sum);
	}
	
	MPI_Finalize(); // Clean up on exit
}