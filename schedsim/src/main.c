#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_INPUT "--input="
#define FLAG_ALGO  "--algorithm="
#define FLAG_QUANT "--quantum="

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *algo_name = NULL;
    int quantum = 50; // Default value if not provided

    for (int i = 1; i < argc; i++) {

        // 1. Check for --input=
        if (strncmp(argv[i], FLAG_INPUT , strlen(FLAG_INPUT)) == 0) {
            input_file = argv[i] + 8;
        } 
        // 2. Check for --algorithm=
        else if (strncmp(argv[i], FLAG_ALGO, strlen(FLAG_ALGO)) == 0) {
            algo_name = argv[i] + 12;
        } 
        // 3. Check for --quantum=
        else if (strncmp(argv[i], FLAG_QUANT, strlen(FLAG_QUANT)) == 0) {
            quantum = atoi(argv[i] + 10);
        }
    }

    if (!input_file || !algo_name) {
        fprintf(stderr, "Error: Missing required arguments.\n");
        fprintf(stderr, "Usage: %s %s<file> %s<name> [%s<n>]\n", 
                argv[0], FLAG_INPUT, FLAG_ALGO, FLAG_QUANT);
        return 1;
    }

    printf("Running %s on %s (Quantum: %d)\n", algo_name, input_file, quantum);

    // int count = 0;
    // Process *list = loadProcesses(input_file, &count);
    // run_scheduler(algo_name, list, count, quantum);

    return 0;
}