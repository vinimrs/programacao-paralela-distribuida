#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Gera um número aleatório entre min e max
int random_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <num_vertices> <num_edges>\n", argv[0]);
        return 1;
    }

    int num_vertices = atoi(argv[1]);
    int num_edges = atoi(argv[2]);

    if (num_vertices <= 0 || num_edges <= 0) {
        fprintf(stderr, "Número de vértices e arestas deve ser maior que zero.\n");
        return 1;
    }

    srand(time(NULL));

    // Nome do arquivo de saída
    const char* filename = "graph.in";

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo para escrita.\n");
        return 1;
    }

    fprintf(file, "%d %d\n", num_vertices, num_edges);

    // Gera arestas aleatórias
    for (int i = 0; i < num_edges; ++i) {
        int source = random_range(0, num_vertices - 1);
        int dest = random_range(0, num_vertices - 1);

        // Certifique-se de que source e dest sejam diferentes
        while (dest == source) {
            dest = random_range(0, num_vertices - 1);
        }

        int weight = random_range(1, 10); // Peso aleatório entre 1 e 10

        fprintf(file, "%d %d %d\n", source, dest, weight);
    }

    fclose(file);

    printf("Grafo gerado com sucesso e salvo em %s.\n", filename);

    return 0;
}
