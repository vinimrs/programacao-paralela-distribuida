CC=gcc
OUT=amd
OUTOMP=amd_omp
IN=amd.in
IN100=graph.in

all: amd

amd: amd.c
	$(CC) -o $(OUT) amd.c

amd_omp: amd_threads.c
	$(CC) amd_threads.c -o $(OUTOMP) -fopenmp

amd_omp_debug: amd_threads.c
	$(CC) -o $(OUTOMP).debug amd_threads.c -DDEBUG=1 -Wall -fopenmp

time: $(OUTOMP) $(OUT)
	time ./$(OUT) < ./$(IN100) && time ./$(OUTOMP) < ./$(IN100)

random_graph: random_graph.c
	gcc random_graph.c -o random_graph

debug: amd.c
	$(CC) -o $(OUT).debug amd.c -DDEBUG=1 -Wall

clean:
	rm -f $(OUT) $(OUT).debug