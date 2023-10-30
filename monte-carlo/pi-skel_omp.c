/* 
 ** Universidade Federal de São Carlos
 ** Departamento de Computação
 ** Prof. Hélio Crestana Guardia
 */

/*
 ** Programa : 
 ** Comando: 
 ** Objetivo: observar passagem de valores para função da thread.
              Nesse caso, usa-se vetor em que cada thread preenche 
              os resultados da sua operação. 
*/

/* 
 Algoritmo de Monte Carlo para cálculo do valor de pi 
 Dados um círculo de raio 1 inserido em um quadrado de lado 2.
 Relação entre pontos aleatórios que estão dentro do círculo 
 e pontos no quadrado pode ser usada para cálculo de pi.

 Relação entre as áreas: (p*r^2)/(4*r^2)
 Considerando r=1: (p/4)=pontos_no_círculo/total_pontos
 p=4*pontos_no_círculo/total_pontos

 Considerando a geração de valores aleatórios entre 0 e 1 (raio)
 para as coordenadas x e y de um ponto, usa-se a equação do teorema 
 de Pitágoras para determinar se ponto está dentro do círculo ou não:
 z^2=x^2+y^2
*/

#include <unistd.h>
#include <stdio.h> 
#include <pthread.h> 
#include <stdlib.h> 
#include <limits.h> 
#include <omp.h>

#define N_THREADS 32
#define SAMPLES 2<<25


// Variável atômica. Pode ser incrementada sem mutex, usando opearação específica
// _Atomic total = 0;
// Incremento: __atomic_fetch_add (& total, parcial, __ATOMIC_SEQ_CST);


// funcao da thread: o que passar como parâmetro?
void *
calcula(void *arg)
{

	// Como tratar a soma local? Retorna como parâmetro?

	pthread_exit(NULL);
}


int
main(int argc, char **argv) 
{ 
	long amostras;
	long hits = 0;

	double pi;

	// int num_threads = 1;
	// vetor de pthread_t para salvamento dos identificadores das threads
	// pthread_t *threads; 

	if(argc > 1)
		amostras = atoi(argv[1]);
	else
		amostras = SAMPLES;

	double x, y;  // coordenadas do ponto

	// cálculo sequencial
	#pragma omp parallel num_threads(N_THREADS)
	{
		unsigned short *mystate = (unsigned short *)malloc(sizeof(unsigned short));
		// Utilizamos a operação XOR para obter uma semente semi-aleatoria
		*mystate = time(NULL) ^ omp_get_thread_num() ^ pthread_self();

		#pragma omp for reduction(+ : hits) 
		for (long i=0; i < amostras; i++) {
			// Geração de números aleatórios entre 0 e 1, usando a função eradnd48()
			x=erand48(mystate);
			y=erand48(mystate);
			// x=(double)drand48();
			// y=(double)drand48();

			// verifica se coordenadas do ponto estão dentro do círculo
			if(x*x+y*y <= 1.0)
				hits++;
		}	
	}
	

	printf("hits: %ld\n",hits);

 	pi=(double) (4 * (double)((double)hits / (double)amostras));

	printf("Pi: %.16f\n",pi);

	return(0);
} 

