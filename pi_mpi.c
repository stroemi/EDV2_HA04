/*********************************************************
   pi.c - Berechnung von Pi durch Integration von
   f(x) = 4/(1 + x**2)     
**********************************************************/
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#define MASTER			0

#define H_TRANSFER		1
#define IU_TRANSFER		2
#define IO_TRANSFER		3
#define PIPART_TRANSFER 4

double f(double x)
{
    return (1.0 / (1.0 + x*x));
}

int main(int argc, char *argv[])
{
int processCount, identity, iu, io;
double PI25 = 3.141592653589793238462643;
double h;

/* MPI: Prozess-Bearbeitung beginnen */
MPI_Init(&argc, &argv);

/* MPI: Bestimmen der Prozess-Anzahl */
MPI_Comm_size(MPI_COMM_WORLD, &processCount);

/* MPI: Bestimmen des Rechnernames auf dem der aktuelle Prozess laeuft */
MPI_Comm_rank(MPI_COMM_WORLD, &identity);

/* MPI: Statusvariable */
MPI_Status status;

if (identity == MASTER)
{
	int intervalCount, intervalChunk, neededProcesses, targetProcess, iuc, ioc;

	printf("Geben Sie bitte die Anzahl der Intervalle ein: ");
	fflush(stdout);
	scanf("%d", &intervalCount); 
	h = 1.0 / (double) intervalCount;
	intervalChunk = (int) ceil((double) intervalCount / (double) processCount);
	neededProcesses = (int) ceil((double) intervalCount / (double) intervalChunk);
	
	/* Uebermitteln von h sowie i_u und i_o der einzelnen Prozesse */	
	for (targetProcess = 0; targetProcess < processCount; targetProcess++) 
	{
		/* h Uebertragen */
		if (targetProcess != MASTER)
		{
			MPI_Send(&h, 1, MPI_DOUBLE, targetProcess, H_TRANSFER, MPI_COMM_WORLD);
		}
		
		/* Berechnen der individuellen Berechnungsschranken */
		if (targetProcess < (neededProcesses - 1))
		{
			iuc = targetProcess * intervalChunk + 1;
			ioc = (targetProcess + 1) * intervalChunk;
		}
		else if (targetProcess == (neededProcesses - 1))
		{
			iuc = targetProcess * intervalChunk + 1;
			ioc = intervalCount;
		}
		else
		{
			iuc = 0;
			ioc = 0;
		}
		/* Senden von iu und io */
		if (targetProcess == MASTER)
		{
			iu = iuc;
			io = ioc;
		}
		else
		{
			/* Senden von iu und io */
			MPI_Send(&iuc, 1, MPI_INT, targetProcess, IU_TRANSFER, MPI_COMM_WORLD);
			MPI_Send(&ioc, 1, MPI_INT, targetProcess, IO_TRANSFER, MPI_COMM_WORLD);
		}			
	}	
}
else
{
	/* Empfangen von h */
	 MPI_Recv(&h, 1, MPI_DOUBLE, MASTER, H_TRANSFER, MPI_COMM_WORLD, &status);
	
	/* Empfangen von iu */
	MPI_Recv(&iu, 1, MPI_INT, MASTER, IU_TRANSFER, MPI_COMM_WORLD, &status);
	
	/* Empfangen von io */
	MPI_Recv(&io, 1, MPI_INT, MASTER, IO_TRANSFER, MPI_COMM_WORLD, &status);	
}

/* Individuelle Berechnungen */
double partpi = 0.;
int i;
if (iu != 0)
{
	for (i = iu; i <= io ; i++)
	{
		partpi += h * f(h * ((double) i - 0.5));
	}
}
	
/* Uebermitteln der Ergebnisse */		
if (identity == MASTER)
{
	double pi = 0., tmp;
	int targetProcess;
	
	/* Empfangen der Partialsummen */
	for (targetProcess = 0; targetProcess < processCount; targetProcess++) 
	{
		if (targetProcess == MASTER)
		{
			pi += partpi;
		}
		else
		{
			/* Teilsumme empfangen */
			 MPI_Recv(&tmp, 1, MPI_DOUBLE, targetProcess, PIPART_TRANSFER, MPI_COMM_WORLD, &status);
			 pi += tmp;
		}
	}
	pi *= 4.;
	/* Ausgabe von Pi */
	printf("Pi ist naeherungsweise %.16f, die Abweichung ist %.16f\n", pi, fabs(pi - PI25));
}
else
{
	/* Senden der Partialsumme */
	MPI_Send(&partpi, 1, MPI_DOUBLE, MASTER, PIPART_TRANSFER, MPI_COMM_WORLD);
}

/* MPI: Prozess-Bearbeitung beenden */
MPI_Finalize();
}

