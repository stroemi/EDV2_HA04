/**********************************************************************
   pi.c - Berechnung von Pi durch Integration von f(x) = 4/(1 + x**2)     
     
  Variablen:
    pi          Ergebnis
    n           Anzahl der Intervalle
    h           Breite der Intervalle
    x           Mittelpunkt des jeweiligen Intervalls
    f(x)        zu integrierende Funktion
    flaeche     Flaeche als Summe f(x)*h fuer alle Intervalle
    i           Schleifenindex
***********************************************************************/
#include <stdio.h>
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
#define NoIntervalCount 1
#define NotANumber      2

double f(double x)
{
    return (1.0 / (1.0 + x*x));
}


/* 
   Berechnet Fläche im Interval [left,right[
   Dabei werden die Balkenhöhen hoch( )gestapelt
*/

double calculate_bars_height(h, left, right) 
{
    double hoeh = 0.0;
    int i;
    for (i=left; i<=right; i++)
    {
        hoeh += f(
                h * ((double)i - 0.5)
                    );
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
    double pi_part;

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

        h = 1.0 / (double) interval_count;

        if (argc < 2) {
            printf("Bitte geben sie eine Intervalanzahl an\n");
            exit(NoIntervalCount);
        }
        sscanf(argv[1], "%d", &interval_count) 
        if (interval_count <= 0) {
            printf("Intervalanzahl muss schon eine ganze, positive Zahl sein\n");
            exit(NotANumber);
        }

        intervals_per_process = (int) ceil( 
                (double) interval_count / 
                (double) process_count    );

        int p,i;

        for (p=0; p < process_count; p++) {

            /* erstmal rein mathematisch */
            left = p * intervals_per_process + 1;
            right = left + intervals_per_process - 1;

            /* Obere Grenze des Integrationsgebiets nicht überschreiten */
            if (right > interval_count) {
                right = interval_count;
            }

            if (left < right) { /* nicht-leeres Interval */
                if (p == MASTER) {
                    /* TODO hier berechnen, mal kukn */
                } else {
                    MPI_Send(&h, 1, MPI_DOUBLE, target_process, GiveH, World);
                    MPI_Send(&left, 1, MPI_INT, target_process, GiveIL, World);
                    MPI_Send(&right, 1, MPI_INT, target_process, GiveIR, World);
                }
            }
        }  /* each process */
        
    } else { /* in slave */
        MPI_Recv(&h, 1, MPI_DOUBLE, Master, GiveH, World, &status);
        MPI_Recv(&left, 1, MPI_INT, Master, GiveIL, World, &status);
        MPI_Recv(&right, 1, MPI_INT, Master, GiveIR, World, &status);
    }
    /*========================== Calculate ============================================*/

    if (me == MASTER) {
        pi_part = calculate_bars_height(h, 1, intervals_per_process);
    } else { /* i am slave */
        pi_part = calculate_bars_height(h, left, right);
    }

    /*========================== Conquer ============================================*/

    /*========================== Hanging the King ============================================*/
	
    double pi, h, flaeche, x;
    int    i, n;

    printf("Geben Sie die Anzahl der Intervalle ein!\n");
    scanf("%d", &n); 

    h       = 1.0 / (double) n;
    flaeche = 0.0;

    for (i = 1; i <= n; i++)
    {
	x = h * ((double)i - 0.5);
	flaeche += f(x);
    }
    pi = 4.0 * h * flaeche;

    printf("Pi ist naeherungsweise %.16f, die Abweichung ist %.16f\n",
            pi, fabs(pi - PI25));

    return 0;
}
