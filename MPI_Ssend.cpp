#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <cstddef>

typedef struct //structure to pass between processors
{
	int clientID;
	double balance;
	char name[80];
	char surname[80];
} client_s;

int main(int argc, char *argv[])
{

	int size, rank, i = 1;
	double start_t, end_t;
	MPI_Status status;

	MPI_Init(&argc, &argv);

	//create MPI datatype for struct
	int clientAtributeCount = 4;
	int blocklenArr[clientAtributeCount] = {1, 1, 80, 80};
	MPI_Aint dispArr[clientAtributeCount];
	MPI_Datatype types[clientAtributeCount] = {MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR};
	MPI_Datatype mpiCLientType;

	//fill displacement array
	dispArr[0] = offsetof(client_s, clientID);
	dispArr[1] = offsetof(client_s, balance);
	dispArr[2] = offsetof(client_s, name);
	dispArr[3] = offsetof(client_s, surname);

	MPI_Type_create_struct(clientAtributeCount, blocklenArr, dispArr, types, &mpiCLientType);
	MPI_Type_commit(&mpiCLientType);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (size < 2)
	{
		fprintf(stderr, "Requires at least two processes.\n");
		exit(-1);
	}

	start_t = MPI_Wtime();
	if (rank == 0)
	{
		client_s clientToSend;
		clientToSend.clientID = 1;
		clientToSend.balance = 2940.25;
		sprintf(clientToSend.name, "Borys");
		sprintf(clientToSend.surname, "Szyc");

		while (i < size)
		{
			MPI_Ssend(&clientToSend, 1, mpiCLientType, i, 0, MPI_COMM_WORLD);
			i++;
		}
	}
	else
	{
		client_s rcvClient;
		MPI_Comm communicator;
		MPI_Recv(&rcvClient, 1, mpiCLientType, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//printf("process %d received client from process 0\n", rank);
	}
	end_t = MPI_Wtime();

	MPI_Finalize();

	if (rank == 0)
	{
		printf("PROCESS TIME %lf\n\n", end_t - start_t);
	}
	return 0;
}