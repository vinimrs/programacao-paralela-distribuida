all: 
	make compile && make run

compile: pi-skel.c pi-skel_seq.c
	gcc -Wall -pthread pi-skel.c -o pi-skel.o && \
	gcc -Wall pi-skel_seq.c -o pi-skel_seq.o && \
	gcc -Wall -fopenmp pi-skel_omp.c -o pi-skel_omp.o

run: 
	time ./pi-skel.o && time ./pi-skel_seq.o && time ./pi-skel_omp.o
