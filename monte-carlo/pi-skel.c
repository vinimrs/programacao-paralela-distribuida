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
#include <time.h>

#define N_THREADS 8
#define SAMPLES 2<<25


// Variável atômica. Pode ser incrementada sem mutex, usando opearação específica
// _Atomic hits = 0;
// Incremento: __atomic_fetch_add (& total, parcial, __ATOMIC_SEQ_CST);


// funcao da thread: o que passar como parâmetro?
void *
calcula(void *arg)
{
	long *pont_amostras = (long *) arg;
	long amostras = *pont_amostras;
	long n_threads = (long) N_THREADS;

	double x, y;  // coordenadas do ponto
	long soma_parcial = 0;

	long quantidade = amostras / n_threads;


    unsigned short *mystate = (unsigned short *)malloc(sizeof(unsigned short));
    // Utilizamos a operação XOR para obter uma semente semi-aleatoria
    *mystate = time(NULL) ^ getpid() ^ pthread_self();

	for (long i=0; i < quantidade; i++) {
		x = erand48(mystate);
		y = erand48(mystate);

		// verifica se coordenadas do ponto estão dentro do círculo
		if(x*x+y*y <= 1.0)
		 	soma_parcial++;
	}	

	pthread_exit((void *) soma_parcial);
}


int
main(int argc, char **argv) 
{ 
	int  status;
	long amostras;
	long hits = 0;

	double pi;

	int num_threads = N_THREADS;
	// vetor de pthread_t para salvamento dos identificadores das threads
	pthread_t *threads; 

	if(argc > 1)
		amostras = atoi(argv[1]);
	else
		amostras = (long) SAMPLES;

	// passando o número de threads como argumanto para a função main...

	if(argc > 2) {
		num_threads = atoi(argv[2]);
		if(num_threads <= 0) {
			printf("Númreo de threads inválido...\n");
			return(EXIT_FAILURE);
		}
	} 
	// aloca memória para o vetor de thread IDs
	threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));	

	// criacao das threads
	for (int i=0; i < num_threads; i++) {

		status = pthread_create(&threads[i], NULL, calcula, (void *)&amostras );

		if (status) {
			printf("Falha da criacao da thread %d (%d)\n", i, status);
			return (EXIT_FAILURE);
		}
	}

	// loop de espera pelo término da execução das threads
	for (int i=0; i < num_threads; i++) {

		long *soma_parcial = (long *) malloc(sizeof(long));
		// join recebendo a soma parcial de cada thread
		status = pthread_join(threads[i], (void **)soma_parcial );

		if (status) {
			printf("Erro em pthread_join (%d)\n",status);
			break;
		}

		hits += *soma_parcial;
	}

	printf("hits: %ld\n",hits);

 	pi=(double) (4 * (double)((double)hits / (double)amostras));

	printf("Pi: %.16f\n",pi);

	return(0);
} 

