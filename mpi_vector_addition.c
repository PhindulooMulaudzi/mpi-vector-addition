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
		data[i] = 1;
	}
}
  
int main(int argc, char* argv[])
{
	const int ArraySize = 5000;
	int data[ArraySize];
	
	// initialise the data, we can always initialise our variable before MPI init
	initArray(data, ArraySize);
	// create placeholders that we will use later
	int nprocesses, currentProcess, sum;
	
	// Start MPI
	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &currentProcess); // get current process ID
	MPI_Comm_size(MPI_COMM_WORLD, &nprocesses); //get number of processes	
		
	// we will use our master thread to initilise the data and initiate the program
	if(currentProcess == MASTER){
		// Basic idea of a summation is to split the data to let everyprocess have values to sum
		// Lets first determine the workload or chunk size for each process
		// Size of our data is shared among all the available processors
		int chunksize = ArraySize / nprocesses;
		int index = 0, i = 0;
		
		// We can only parallelise if we have more than one processor
		if(nprocesses > 1){
			// Lets distribute the workload based on the chunksize
			// Use loop to send the messsage to all the processor 
			// We also leave out the last processor in case the work load in uneven
			/*MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)*/
			for (i = 1; i < nprocesses - 1; i++){
				// index for chunks to be sent start at index of size chunk
				// we leave value from 0:chunksize for the MASTER
				index = i*chunksize;
				// send the chunk sizes, this can change so we need to communicate it
				MPI_Send(&chunksize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				// send the actual data
				MPI_Send(&data[index], chunksize, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
		}
		
		// we now  need to get the remaining chunks that may have been ignore at nprocesses-1 
		index = i*chunksize; // i was changed in loop if we have > 1 rank else it is 0
		int ignoredChunks = ArraySize - index;
		
		MPI_Send(&ignoredChunks, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		MPI_Send(&data[index], ignoredChunks, MPI_INT, i, 0, MPI_COMM_WORLD);
		
		// We can now do the summation for the workload left for master i.e. 0 - chunksize
		sum = 0;
		// Remember that we have MASTER 0-chunksize
		for(i = 0; i < chunksize; i++){
			sum += data[i];
		}
		
		
		//The master is also responsible for recieving back the partial sum
		//Lets get them and aggregate them
		int partialSum = 0;
		
		// we need a loop to recieve from all the nprocesses that are send back answer
		for(i = 1; i < nprocesses; i++){
			//MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,MPI_Comm comm, MPI_Status *status)
			MPI_Recv(&partialSum, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			int sender = status.MPI_SOURCE;
			
			// aggregate the partial sums into global sum
			sum +=partialSum;
		}
		// prints the final sum of array
        printf("Sum of array is : %d\n", sum);
	}
	else{
		// lets hand the slave condition, remember they are being passed around workload chunks
		int localChunkSize = 0;
		
		// recieve value of localChunk and assign it
		MPI_Recv(&localChunkSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		
		//create array to reacieve local chunk data for current workload
		int chunks[localChunkSize];
		
		// recieve workload chunks
		MPI_Recv(&chunks, localChunkSize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		int localSum = 0, i = 0;
		
		// carry out the partial sums
		for(i = 0; i < localChunkSize; i++){
			localSum += chunks[i];			
		}
		
		// send back local sum
		MPI_Send(&localSum, 1, MPI_INT, 0,0, MPI_COMM_WORLD);
	}
	
	MPI_Finalize(); // Clean up on exit
}