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

	int size, rank, i = 1, position;
	int dest = 0, src = 0, tag = 1;
	double start_t, end_t;
	MPI_Status status;
	char pack_buff[10000];

	/*-----------------bsend variables and PACK_SIZE x messages Buffer size--------------------------*/

	MPI_Comm comm = MPI_COMM_WORLD;
	int s1; // pack_size handler

	char *buf, *bbuf;
	int bufsize, bsize;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//create MPI datatype for struct
	int clientAtributeCount = 4;
	int blocklenArr[clientAtributeCount] = {1, 1, 80, 80};
	MPI_Aint dispArr[clientAtributeCount];
	MPI_Datatype types[clientAtributeCount] = {MPI_INT, MPI_DOUBLE, MPI_CHAR, MPI_CHAR};
	MPI_Datatype mpiCLientType;

	//fill displacement array
	dispArr[0] = offsetof(client_s, clientID);
	dispArr[1] = offsetof(client_s, balance);
	dispArr[2] = offsetof(client_s, name);
	dispArr[3] = offsetof(client_s, surname);

	MPI_Type_create_struct(clientAtributeCount, blocklenArr, dispArr, types, &mpiCLientType);
	MPI_Type_commit(&mpiCLientType);
	
	//struct pools size calc
	MPI_Pack_size(1, mpiCLientType, comm, &s1);

	//bufer memory allocation
	bufsize = MPI_BSEND_OVERHEAD + s1;
	buf = (char *)malloc(bufsize);
	MPI_Buffer_attach(buf, bufsize);

	if (size < 2)
	{
		fprintf(stderr, "Requires at least two processes.\n");
		exit(-1);
	}
	//-------------------------------------------------------------------------------------------------
	start_t = MPI_Wtime();
	if (!rank)
	{
		position = 0;
		//printf("\nProcess 0\n");
		client_s clientToSend;
		clientToSend.clientID = 1;
		clientToSend.balance = 2940.25;
		sprintf(clientToSend.name, "Borys");
		sprintf(clientToSend.surname, "Szyc");

		MPI_Pack(&clientToSend, 1, mpiCLientType, pack_buff, 1000, &position, MPI_COMM_WORLD);

		while (i < size)
		{
			MPI_Bsend(pack_buff, position, MPI_PACKED, i, 0, MPI_COMM_WORLD);
			i++;
		}
	}
	else
	{
		char recv_buff[10000];
		client_s rcvClient;
		MPI_Recv(recv_buff, 10000, MPI_PACKED, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Unpack(recv_buff, 10000, &position, &rcvClient, 1, mpiCLientType, MPI_COMM_WORLD);
		//printf("process %d received client from process 0 client name %s \n", rank, rcvClient.name);
	}
	end_t = MPI_Wtime();
	
	//-----------------------------------------------------------------------------------------------------+
	MPI_Buffer_detach( &bbuf, &bsize );
	MPI_Finalize();

	if (rank == 0)
	{
		printf("PROCESS TIME %lf\n\n", end_t - start_t);
	}
	return 0;
}