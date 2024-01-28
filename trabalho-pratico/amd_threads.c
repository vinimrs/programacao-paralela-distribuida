#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>

// https://en.wikipedia.org/wiki/Parallel_all-pairs_shortest_path_algorithm#Parallelization
// Solves the minimum distance between all pairs of vertices
void md_all_pairs (uint32_t* dists, uint32_t v) {
    uint32_t k, i, j;


    // from vertex k, which paths passing through it can be less costly?
    //  for any yes answer, uses the less costly result to the distance from i => j
    // i => k => j < i => j ? 
    for (k = 0; k < v; ++k) {
      // Since row k and column k (the pivots) remain
      // unchanged after step k, the corresponding loop can
      // be executed in parallel. Each iteration of the internal loop updates 
      // only the corresponding entry in the dists array, 
      // which is independent of the other entries.

      #pragma omp parallel for private(i, j) shared(dists)
      for (i = 0; i < v; ++i) {
          if(i != k)
            for (j = 0; j < v; ++j) {
              if(j != k && j != i) {
                uint32_t distanceThroughK = dists[i*v+k] + dists[k*v+j];
                
                // Checks for overflows with the UINT32_MAX
                if ((distanceThroughK >= dists[i*v+k])
                      &&(distanceThroughK >= dists[k*v+j])
                      &&(distanceThroughK < dists[i*v+j])
                    ) {
                  dists[i*v+j] = distanceThroughK;
                }
              }
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

  #pragma omp parallel for private(i, j) reduction(+:smd, paths)
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

    //Reads input 
    //First line: v (number of vertices) and e (number of edges)
    uint32_t v, e;
    scanf("%u %u", &v, &e);

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

    if(getenv("OMP_NUM_THREADS")) {
      printf("OMP_NUM_THREADS defined: %s\n", getenv("OMP_NUM_THREADS"));

    } else {
      printf("OMP_NUM_THREADS not defined\n");
      // Define number of threads with the number of vertices;
      int num_threads = v > 12 ? 12 : v; 
      omp_set_num_threads(num_threads);
      
      printf("num_threads = %d\n", num_threads);
    }

	  //Computes the minimum distance for all pairs of vertices
    md_all_pairs(dists, v);

    //Computes and prints the final solution
    amd(dists, v);

#if DEBUG
	debug(dists, v);
#endif

    return 0;
}