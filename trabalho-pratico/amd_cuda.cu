%%writefile cuda.cu
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cuda.h>

// Solves the minimum distance between all pairs of vertices
__global__ void md_all_pairs (uint32_t* dists, uint32_t k, uint32_t v) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;

  if(i < v && j < v) {
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

/* Computes the average minimum distance between all pairs of vertices with a path connecting them
    with CUDA */
__global__ void amd_cuda (uint32_t* dists, uint32_t v, uint32_t* data) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;
	uint32_t infinity = v*v;
  uint32_t val = 1;

  // We only consider if the vertices are different and there is a path
  if ((i != j) && (j < v && i < v) &&
      (dists[i*v+j] != 0) && (dists[i*v+j] < infinity)
      ) {
    atomicAdd(&data[0], dists[i*v+j]);
    atomicAdd(&data[1], val);
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
  uint32_t* d_dists; // pointer to matrix from GPU

  //Reads input
  //First line: v (number of vertices) and e (number of edges)
  uint32_t v, e;
  scanf("%u %u", &v, &e);

  //Allocates distances matrix (w/ size v*v) i
  //and sets it with max distance and 0 for own vertex
  size_t size = v*v*sizeof(uint32_t);
  uint32_t* dists = (uint32_t *) malloc(size);
  memset(dists, UINT32_MAX, size);
  uint32_t i;
  for ( i = 0; i < v; ++i ) dists[i*v+i] = 0;

  //Reads edges from file and sets them in the distance matrix
  uint32_t source, dest, cost;
  for ( i = 0; i < e; ++i ){
    scanf("%u %u %u", &source, &dest, &cost);
    if (cost < dists[source*v+dest]) dists[source*v+dest] = cost;
  }

  // Define number of threads with the number of vertices;
  int num_threads = v > 32 ? 32 : v;
  printf("num_threads = %d\n", num_threads);

  // Malloc of space to matrix in GPU
  cudaMalloc((void **)&d_dists, size);

  // Copy of data from dists to d_dists
  cudaMemcpy(d_dists, dists, size, cudaMemcpyHostToDevice);

  // Defining block and grid
  dim3 block;

  // Standard setup with 252 threads per block
  block.x = 16;
  block.y = 16;
  block.z = 1;

  dim3 grid;
  grid.x = (v + block.x -1) / block.x; // ceil of division
  grid.y = (v + block.y -1) / block.y;
  grid.z = 1;

  //Computes the minimum distance for all pairs of vertices
  for(int k = 0; k < v; ++k) {
    md_all_pairs<<< grid, block >>>(d_dists, k, v);
  }

  // Copy result from gpu to cpu
  cudaMemcpy(dists, d_dists, size, cudaMemcpyDeviceToHost);

#if AMD_PAR == 1
  printf("Computing AMD parallely\n");
  // Computing the final solution
  uint32_t* d_data; // pointer to solution data on GPU
  size_t sizeData = 2 * sizeof(uint32_t);
  uint32_t* data = (uint32_t *) malloc(sizeData);
  data[0] = 0; // sum of minimum distances
  data[1] = 0; // number of paths found

  uint32_t solution = 0;

  cudaMalloc((void **)&d_data, sizeData);

  // Copy of data from data to d_data
  cudaMemcpy(d_data, data, sizeData, cudaMemcpyHostToDevice);

  //Computes the final solution
  amd_cuda<<< grid, block >>>(d_dists, v, d_data);

  cudaMemcpy(data, d_data, sizeData, cudaMemcpyDeviceToHost);

  solution = data[0] / data[1];
  printf("%d\n", solution);

  cudaFree(d_data); 
#else 
  printf("Computing AMD sequentially\n");
  //Computes and prints the final solution
  amd(dists, v);
#endif

#if DEBUG
	debug(dists, v);
#endif

  cudaFree(d_dists);

  return 0;
}