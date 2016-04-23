#ifndef KNAPSACK_KNAPSACK_H
#define KNAPSACK_KNAPSACK_H

#define DEFAULT_RUN_ITER_CNT    100  /* Total iteration count. */

#define DEFAULT_PROB_CROSSOVER  0.9
#ifdef ENABLE_TOURNAMENT_SELECTION
  #define DEFAULT_PROB_MUTATION 0.005
#else
  #define DEFAULT_PROB_MUTATION 0.01
#endif
#define DEFAULT_POPULATION_SIZE 100
#define DEFAULT_ITEMS_COUNT     500
#define DEFAULT_INDIVIDUAL_LEN  DEFAULT_ITEMS_COUNT
#define FITNESS_PENALTY         0    /* Fitness value if the weight
                                        exceeds the limit. */
#define MAX_INPUT_LINE_SIZE     1024

/* Sturcture of item to put into knapsack. */
struct item{
	int id;
	int weight;
	int profit;
};

int **get_population();
void print_population();
void get_fitness(const int **population, int *max, int *avg);
void selection(const int **population, int **next);
void crossover(int **next);
void mutation_cpy(int **population, const int **next);

#endif
