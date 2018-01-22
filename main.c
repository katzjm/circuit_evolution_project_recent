/* 
 * Module for executing the program
 *
 * Author: Josh Katz
 */

#include <stdlib.h>
#include <stdio.h>

#include "Configs.h"
#include "Cvode_Utils.h"
#include "Population.h"

void Usage() {
  printf("./evolver\n");
  printf("./evolver configuration_file_name\n");
  printf("Evolves a set of reactions to fit data or a function.\n");
  printf("A file listing configuration parameters or data can be provided\n");
  printf("otherwise a default run is executed\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc > 2) {
    Usage();
  }

  // Configure this run according to client provided parameters
  Config c;
  if (argc == 1) {
    Configure(&c, NULL);
  } else {
    Configure(&c, argv[1]);
  }

  // Set up Cvode
  N_Vector concentrations = GetNewNVector(&c);
  CvodeData cvode_data = { NULL, concentrations };
  UserData user_data = { NULL, &c };
  SetUpCvodeFirstRun(&cvode_data, &user_data);

  Population pop;
  SetFirstGeneration(&pop, &c, &cvode_data, &user_data);
  for (int i = 0; i < c.max_num_generations; i++) {
    if (BestFitness(&pop) < c.fit_threshold) {
      break;
    }

    if (i % c.output_interval == 0) {
      char buf[256];
      GetSmallStatus(&pop, buf);
      printf("Generation: %d, %s\n", i, buf);
    }

    SetNextGeneration(&pop, &c, &cvode_data, &user_data);
  }

  char buf[256];
  GetLargeStatus(&pop, buf);
  printf("%s", buf);

  KillPopulation(&pop);
  return EXIT_SUCCESS;
}
