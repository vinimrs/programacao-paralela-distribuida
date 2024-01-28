#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int size = 11;
  int threads[] = {1, 2, 4, 8, 12, 16, 32, 64, 128, 256, 512};

  FILE *arquivo_csv = fopen("resultados.csv", "w");
  if (arquivo_csv == NULL) {
      fprintf(stderr, "Erro ao abrir o arquivo CSV.\n");
      return 1;
  }

  double elapsed_time_seq = 3.94;

  fprintf(arquivo_csv, "Threads,Real Time(s),User Time(s),Sys Time(s),Speedup\n");


  // Execução com múltiplas threads
  for (int i = 0; i < size; i++) {
    printf("%d\t", threads[i]);
    fflush(stdout);
    char comando[100];
    sprintf(comando, "make NUM=%d time_temp", threads[i]);
    system(comando);
    FILE *temp_output = fopen("temp_output.txt", "r");
    double real_time_ref, user_time_ref, sys_time_ref;
    fscanf(temp_output, "%lfuser %lfsystem 0:%lfelapsed", &user_time_ref, &sys_time_ref, &real_time_ref);
    fclose(temp_output);
    fprintf(arquivo_csv, "%d,%.6f,%.6f,%.6f,%.6f\n", threads[i],real_time_ref, user_time_ref, sys_time_ref, elapsed_time_seq/real_time_ref);
  }

  fclose(arquivo_csv);
  remove("temp_output.txt");

  return 0;
}