#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mpi.h"

#define DIETAG 2

// Solves the minimum distance between all pairs of vertices
void md_all_pairs (uint32_t* dists, uint32_t v) {
  uint32_t k, i, j;

  for (k = 0; k < v; ++k) {
    for (i = 0; i < v; ++i) {
      for (j = 0; j < v; ++j) {
        uint32_t intermediary = dists[i*v+k] + dists[k*v+j];
        // Checks for overflows
        if ((intermediary >= dists[i*v+k])&&(intermediary >= dists[k*v+j])&&(intermediary < dists[i*v+j]))
            dists[i*v+j] = dists[i*v+k] + dists[k*v+j];
      }
    }
  }

}

/* Computes the average minimum distance between all pairs of vertices with a path connecting them */
void amd (uint32_t* dists, uint32_t v) {
  uint32_t i, j;
	uint32_t infinity = v*v;
	uint32_t smd = 0; 	//sum of minimum distances
	uint32_t paths = 0; //number of paths found
	uint32_t solution = 0;

  for (i = 0; i < v; ++i) {
    for (j = 0; j < v; ++j) {
      // We only consider if the vertices are different and there is a path
      if ((i != j) && (dists[i*v+j] < infinity)) {
        smd += dists[i*v+j];
        paths++;
      }
    }
  }

	solution = smd / paths;
	printf("%d\n", solution);

}

/* Debug function (not to be used when measuring performance)*/
void debug (uint32_t* dists, uint32_t v) {
    uint32_t i, j;
	uint32_t infinity = v*v;

  for (i = 0; i < v; ++i) {
    for (j = 0; j < v; ++j) {
      if (dists[i*v+j] > infinity) printf("%7s", "inf");
      else printf ("%7u", dists[i*v+j]);
    }
    printf("\n");
  }
}

// Main program - reads input, calls FW, shows output
int main (int argc, char* argv[]) {

  int rank, num_procs, slice, ypoloipo = 0, n = 200;

  MPI_Status status;
	int resultI = MPI_Init(&argc,&argv);
	if (resultI != MPI_SUCCESS) {
		fprintf(stderr,"Erro iniciando MPI: %d\n",resultI);
		MPI_Abort(MPI_COMM_WORLD, resultI);
	}
  // Get number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  // Get own rank
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Compute slice size and ypoloipo
  ypoloipo = n % (num_procs - 1); // -1 because the master doesn't do any work
  slice = (n-ypoloipo) / (num_procs - 1); // -1 because the master doesn't do any work
  
  //Reads input
  //First line: v (number of vertices) and e (number of edges)
  uint32_t v, e;
  scanf("%u %u", &v, &e);

  if(rank == 0) {

    int disable = 0;

    //Allocates distances matrix (w/ size v*v) i
    //and sets it with max distance and 0 for own vertex
    uint32_t* dists = malloc(v*v*sizeof(uint32_t));
    memset(dists, UINT32_MAX, v*v*sizeof(uint32_t));
    uint32_t i;
    for ( i = 0; i < v; ++i ) dists[i*v+i] = 0;

    //Reads edges from file and sets them in the distance matrix
    uint32_t source, dest, cost;
    for ( i = 0; i < e; ++i ){
        scanf("%u %u %u", &source, &dest, &cost);
        if (cost < dists[source*v+dest]) dists[source*v+dest] = cost;
    }

    for(int i = 1; i < num_procs; i++) {
      MPI_Send(&dists[(i-1)*slice*v], slice*v, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    do {
      MPI_Recv(&dists[(disable)*slice*v], slice*v, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      disable++;
    } while(disable < num_procs - 1);

    //Computes and prints the final solution
    amd(dists, v);

      
#if DEBUG
	debug(dists, v);
#endif

  } else {
    uint32_t* dists = malloc(slice*v*sizeof(uint32_t));
    memset(dists, UINT32_MAX, slice*v*sizeof(uint32_t));
    uint32_t i;
    for ( i = 0; i < slice; ++i ) dists[i*v+i] = 0;

    for(int i = 1; i < num_procs; i++) {
      MPI_Recv(&dists[(i-1)*slice*v], slice*v, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    //Computes the minimum distance for all pairs of vertices
    md_all_pairs(dists, v);

    for(int i = 1; i < num_procs; i++) {
      MPI_Send(&dists[(i-1)*slice*v], slice*v, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();

  return 0;
}
