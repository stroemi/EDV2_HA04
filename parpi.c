#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define PI25 3.141592653589793238462643
#define World MPI_COMM_WORLD

/* erster MPI-Prozess ist Master */
#define Master 0

/* message types */
#define GiveH   1
#define GiveIL  2
#define GiveIR  3
#define ReturnP 4

/* Error Codes */
#define NotANumber      2

double f(double x)
{
    return (1.0 / (1.0 + x*x));
}


/* 
   Berechnet Fläche im Interval [left,right[
   Dabei werden die Balkenhöhen hoch( )gestapelt
*/

double calculate_bars_height(double h, int left, int right) 
{
    double hoeh = 0.0;
    int i;
    if (left <= right) { 
        for (i=left; i<=right; i++)
        {
            hoeh += f(
                    h * ((double)i - 0.5)
                        );
        }
    } else {
        /* leeres Interval */
        return 0.0;
    }

    return hoeh;
}


int main(int argc, char *argv[])
{
    int process_count, interval_count, me;
    MPI_Status status;

    /* MPI hochfahren */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(World, &process_count);
    /* Wer bin ich, und wenn ja wie viele? */
    MPI_Comm_rank(World, &me);

    double h;
    int left, right;
    double pi_part, tmp, pi;
    int p; /* Laufvariable für prozesse */
    int intervals_per_process;


    /*========================== Devide  ============================================*/
    /*

        |                              |
        |                              |
        |0     1       2       3      4| left-right
        |                              |
        |      |       |       |       |
      0 -------------------------------- 1
                     0.5
    */
    if (me == Master) {

        printf("Wieviele Intervalle? ");
        scanf("%d", &interval_count); 

        if (interval_count <= 0) {
            printf("Intervalanzahl muss schon eine ganze, positive Zahl sein\n");
            exit(NotANumber);
        }

        h = 1.0 / (double) interval_count;

        intervals_per_process = (int) ceil( 
                (double) interval_count / 
                (double) process_count    );

        for (p=1; p < process_count; p++) {

            /* erstmal rein mathematisch */
            left = p * intervals_per_process + 1;
            right = left + intervals_per_process - 1;

            /* Obere Grenze des Integrationsgebiets nicht überschreiten */
            if (right > interval_count) {
                right = interval_count;
            }

            MPI_Send(&h, 1, MPI_DOUBLE, p, GiveH, World);
            MPI_Send(&left, 1, MPI_INT, p, GiveIL, World);
            MPI_Send(&right, 1, MPI_INT, p, GiveIR, World);
        }  /* each process */
        
    } else { /* in slave */
        MPI_Recv(&h, 1, MPI_DOUBLE, Master, GiveH, World, &status);
        MPI_Recv(&left, 1, MPI_INT, Master, GiveIL, World, &status);
        MPI_Recv(&right, 1, MPI_INT, Master, GiveIR, World, &status);
    }
    /*========================== Calculate ============================================*/

    if (me == Master) {
        pi_part = calculate_bars_height(h, 1, intervals_per_process);
    } else { /* I am slave */
        pi_part = calculate_bars_height(h, left, right);
    }

    /*========================== Conquer ============================================*/
    
    if (me == Master) {
        pi = pi_part;
        for (p=1; p < process_count; p++) {
            MPI_Recv(&tmp, 1, MPI_DOUBLE, p, ReturnP, World, &status);
            pi += tmp;
        }
        pi *= 4.0 * h;
        printf("Pi ist näherungsweis %.16f, die Abweichung ist %.16f\n",
                pi, fabs(PI25 - pi)
                );
    } else { /* I am slave */
        MPI_Send(&pi_part, 1, MPI_DOUBLE, Master, ReturnP, World);
    }

    /*========================== Hanging the King ============================================*/

    MPI_Finalize();
	
    return 0;
}
