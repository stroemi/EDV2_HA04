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

double f(double x)
{
    return (1.0 / (1.0 + x*x));
}

int main(int argc, char *argv[])
{
    double PI25 = 3.141592653589793238462643;
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
