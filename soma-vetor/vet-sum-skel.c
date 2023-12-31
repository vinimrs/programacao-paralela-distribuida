/* 
 ** Universidade Federal de São Carlos
 ** Departamento de Computação
 ** Prof. Hélio Crestana Guardia
 */

/*
 ** Programa : soma dos elementos de um vetor
 ** Comando: 
 ** Objetivo: observar passagem de valores para função da thread.
              Nesse caso, usa-se vetor em que cada thread preenche 
              os resultados da sua operação. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <limits.h> 
#include <time.h>


#define NELEM (1<<30)
#define N_THREADS 20


// Variável atômica. Pode ser incrementada sem mutex, usando opearação específica
// _Atomic double total = 0;
// Incremento: __atomic_fetch_add (& total, parcial, __ATOMIC_SEQ_CST);
double *_vet;

typedef struct thread_data {
   long init;
   long end;
} thread_data;


// Funcao da thread: o que passar como parâmetro?
void *
soma(void *arg)
{

   	thread_data *ptdata=(thread_data*) arg;
	long init = ptdata->init;
	long end = ptdata->end;

	double* soma_parcial = (double*) malloc(sizeof(double));

	for (long i=init; i < end; i++) {
		*soma_parcial += _vet[i];
	}	

	pthread_exit((void *) soma_parcial);

}



int
main(int argc, char *argv[])
{
	int i, status;
	double sum;
	long int nelem;
	unsigned int seedp;

	int num_threads = N_THREADS;
	// vetor de pthread_t para salvamento dos identificadores das threads
	pthread_t *threads; 

	if(argc > 1)
		nelem = atoi(argv[1]);
	else
		nelem = NELEM;

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

	// alocação do vetor
	_vet = (double *)malloc(nelem * sizeof(double));

	if (!_vet) {
		perror("Erro na alocacao do vetor de elementos.");
		return EXIT_FAILURE;
	}

	// gerar sempre a mesma sequência de valores? 
	srand(time(NULL));

	// atribuição de valores (0<val<=1) aos elementos do vetor 
	// Atenção para a função de geração de valores aleatórios: rand_r é 'thread safe'
	for(i=0; i < nelem; i++) 
		// _vet[i] = (float)((float)rand() / (float)RAND_MAX);
		_vet[i] = (float)((float)rand_r(&seedp) / (float)RAND_MAX);


	long quantidade_para_thread = (long) NELEM / num_threads;
	// printf("quantidade elementos para thread %ld", quantidade_para_thread); 

	thread_data **vetor_tdata = (thread_data **) malloc(num_threads * sizeof(thread_data));

	for (int i = 0; i < num_threads; i++) {
		vetor_tdata[i] = (thread_data *) malloc(sizeof(thread_data));
		vetor_tdata[i]->init = quantidade_para_thread * i;
		vetor_tdata[i]->end = vetor_tdata[i]->init + quantidade_para_thread; 
	}

	// Loop de criacao das threads
	for (int i=0; i < num_threads; i++) {
		// printf("tdata.init %ld, tdata.end %ld, i = %d\n", vetor_tdata[i]->init, vetor_tdata[i]->end, i);

		status = pthread_create(&threads[i], NULL, soma, (void *) vetor_tdata[i] );

		if (status) {
			printf("Falha da criacao da thread %d (%d)\n", i, status);
			return (EXIT_FAILURE);
		}
	}


	for (int i=0; i < num_threads; i++) {

		// double *soma_parcial = (double *) malloc(sizeof(double));
		void * result;

		// join recebendo a soma parcial de cada thread
		status = pthread_join(threads[i], &result );

		if (status) {
			printf("Erro em pthread_join (%d)\n",status);
			break;
		}

		sum += *(double*)result;
	}

	printf("Soma: %f\n",sum);

	// libera o vetor de ponteiros para as threads
	free(threads);

	// libera o vetor de valores
	free(_vet);

	for (i = 0; i < num_threads; ++i) {
		free(vetor_tdata[i]);
	}

	return(0);
}



