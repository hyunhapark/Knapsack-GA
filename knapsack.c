#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "rand.h"

#define ISWAP(a,b) int tmp = (a);(a)=(b);(b)=tmp

static struct item *items = NULL;
int **population = NULL;
int **temp_population = NULL;

static long generation = 0;
static int population_size = 0;
static int individual_len = 0;
static int capa_weight = 0;
static double prob_crossover = 0;
static double prob_mutation = 0;

static void sys_init(int, int);
static void read_input(FILE *fp);
static inline int *copy_individual(int *dest, const int *src);
static inline int get_individual_fitness(const int *individual);

static void
sys_init(int _individual_len, int _population_size) {
	int i=0;
	individual_len = _individual_len;
	population_size = _population_size;
	prob_crossover = DEFAULT_PROB_CROSSOVER;
	prob_mutation = DEFAULT_PROB_MUTATION;
	items = (struct item *) calloc ( sizeof(struct item), individual_len+1 );
	population = (int **) calloc ( sizeof(int *),
			population_size );
	temp_population = (int **) calloc ( sizeof(int *),
			population_size );
	for (i=0; i<population_size; i++){
		temp_population[i] = (int *) calloc (individual_len+1, sizeof(int));
	}
	generation = 0;
}

static void
read_input(FILE *fp) {
	int i = 0;
	char *buf = (char *) calloc (MAX_INPUT_LINE_SIZE+1, 1);
	fgets (buf, MAX_INPUT_LINE_SIZE, fp); // 0/1 knapsack problem specification
	fgets (buf, MAX_INPUT_LINE_SIZE, fp); // (Empty Line)
	fgets (buf, MAX_INPUT_LINE_SIZE, fp); // Total weight capacity: 00000
	sscanf (buf, "Total weight capacity: %d\n", &capa_weight);
	fgets (buf, MAX_INPUT_LINE_SIZE, fp); // (Empty Line)
	fgets (buf, MAX_INPUT_LINE_SIZE, fp); // item	weight	profit

	for (i=0; i<individual_len; i++) {
		fgets (buf, MAX_INPUT_LINE_SIZE, fp);
		sscanf (buf, "%d\t%d\t%d\n", &items[i+1].id,
				&items[i+1].weight, &items[i+1].profit);
	}
	free (buf);
}

int **
get_population() {
	int i=0, j=0;
	population = (int **) calloc (population_size, sizeof(int *));
	for (i=0; i<population_size; i++){
		int *individual = (int *) calloc (individual_len+1, sizeof(int));
		for (j=1; j<=individual_len; j++){
			individual[j] = get_irand(0,1);
		}
		population[i] = individual;
	}
	return population;
}

void
print_population() {
	int i=0, j=0;
	for (i=0; i<population_size; i++) {
		printf("[I #%03d] ", i);
		for(j=1;j<=individual_len; j++) {
			printf("%d",population[i][j]);
		}
		int fit = get_individual_fitness(population[i]);
		printf(" (%d)\n", fit);
	}
}

static inline int *
copy_individual(int *dest, const int *src) {
	int i=0;
	for (i=1; i<=individual_len; i++) {
		dest[i] = src[i];
	}
	return dest;
}

static inline int
get_individual_fitness(const int *individual) {
	int i=0;
	int weight_sum=0, fitness=0;
	for (i=1; i<=individual_len; i++) {
		weight_sum += individual[i]? items[i].weight: 0;
		if(weight_sum>capa_weight) {
			fitness = FITNESS_PENALTY;
			break;
		}
		fitness += individual[i]? items[i].profit: 0;
	}
	return fitness;
}

void
get_fitness(const int **population, int *max, int *avg) {
	int i=0, fitness_max=0;
	long long fitness_sum = 0;
	for (i=0; i<population_size; i++) {
		int fitness = get_individual_fitness(population[i]);
		fitness_sum += fitness;
		fitness_max = fitness_max>fitness? fitness_max: fitness;
	}
	/* Return. */
	*max = fitness_max;
	*avg = fitness_sum/population_size;
}

void
selection(const int **population, int **next) {
	int i=0;
#ifdef ENABLE_TOURNAMENT_SELECTION
	for (i=0; i<population_size; i++){
		int random = get_irand(0, population_size-1);
		if ( get_individual_fitness(population[i])
				< get_individual_fitness(population[random]) ) {
			copy_individual(next[i], population[random]);
		}
	}
#else
	int cnt=0;
	while (cnt < population_size) {
		long long fitness_sum=0;
		for (i=0; i<population_size; i++) {
			fitness_sum += get_individual_fitness(population[i]);
		}
		double rw_portion = 1./fitness_sum;
		double random = get_drand();
		fitness_sum = 0;
		for (i=0; i<population_size; i++) {
			int fitness = get_individual_fitness(population[i]);
			fitness_sum += fitness;
			if (fitness_sum * rw_portion > random) {
				copy_individual (next[cnt++], population[i]);
				break;
			}
		}
		if(i==population_size)
			copy_individual (next[cnt++], population[population_size-1]);
	}
#endif
}

void
crossover(int **next) {
	int i=0, j=0;
	/* Shuffling. */
	int **tmp = (int **) calloc (population_size+1, sizeof(int *));
	for (i=0; i<population_size; i++) {
		tmp[i] = copy_individual ( (int *) calloc (individual_len+1, sizeof(int)), next[i]);
	}
	int *bucket = (int *) calloc (population_size, sizeof(int));
	int cnt=0;
	while(cnt<population_size) {
		int random = get_irand (0, population_size-1);
		if(!bucket[random])	{
			bucket[random]=1;
			copy_individual (next[cnt++], tmp[random]);
		}
	}
	free (bucket);
	for (i=0; i<population_size; i++) {
		free (tmp[i]);
	}
	free (tmp);
	/* End Shuffling. */

#ifdef ENABLE_UNIFORM_CROSSOVER
	/* Uniform cross over. */
	for (i=0; i<population_size/2; i++){
		for(j=1; j<=individual_len; j++){
			if (get_drand() < .5){
				int aux = next[i][j];
				next[i][j] = next[i+population_size/2][j];
				next[i+population_size/2][j] = aux;
			}
		}
	}

#else
#ifdef ENABLE_THREEPOINT_CROSSOVER
	/* 3-point cross over. */
	for (i=0; i<population_size/2; i++){
		if (get_drand() <= prob_crossover){
			int position[3];
			j = 0;
			while(j < 3){
				position[j] = get_irand (1, individual_len-1);
				int k;
				/* Check duplicated randoms. */
				for(k=0; k<j; k++)  if (position[k] == position[j]) break;
				if(k<j) continue; /* Case of breaked from above loop. */
				j++;
			}
			/* Sorting (Bubble Sort) */
			if(position[0]>position[1]) {ISWAP(position[0], position[1]);}
			if(position[1]>position[2]) {ISWAP(position[1], position[2]);}
			if(position[0]>position[1]) {ISWAP(position[0], position[1]);}

			/* Crossover */
			for (j=position[0]+1; j<=position[1]; j++)
				{ISWAP(next[i][j], next[i+population_size/2][j]);}
			for (j=position[2]+1; j<=individual_len; j++)
				{ISWAP(next[i][j], next[i+population_size/2][j]);}
		}
	}

#else
	/* 1-point cross over. */
	for (i=0; i<population_size/2; i++){
		if (get_drand() <= prob_crossover){
			int position = get_irand(1, individual_len-1);
			/* Crossover */
			for(j=position+1; j<=individual_len; j++)
				{ISWAP(next[i][j], next[i+population_size/2][j]);}
		}
	}
#endif
#endif
}

void
mutation_cpy(int **population/*Dest*/, const int **next/*Src*/) {
	int i=0, j=0;
	for (i=0; i<population_size; i++){
		for (j=1; j<=individual_len; j++){
			population[i][j] = (get_drand() < prob_mutation)?
					(next[i][j]^1)/*Invert*/: next[i][j];
		}
	}
}

int
main(int argc, char **argv) {
	int i=0;
	int fitness_max=0, fitness_avg=0;
	long run_iter_cnt=0;

	if(argc>=2 && strcmp(argv[1], "DEFAULT")) {
		run_iter_cnt = strtol(argv[1], NULL, 10);
		if(run_iter_cnt==0){
			printf("Usage : %s [iteration count]\n", argv[0]);
			return 1;
		}
	} else {
		run_iter_cnt = DEFAULT_RUN_ITER_CNT;
	}

	sys_init (DEFAULT_INDIVIDUAL_LEN, DEFAULT_POPULATION_SIZE);
	read_input (stdin);
	population = get_population ();

	printf("Gen\tMax\tAvg\n");
	get_fitness ((const int **)population, &fitness_max, &fitness_avg);
	while ( generation <= run_iter_cnt ) {
		printf("%ld\t%d\t%d\n", generation, fitness_max, fitness_avg);
		//print_population(); // For Debug. XXX
		selection ((const int **)population, temp_population);
		crossover (temp_population);
		mutation_cpy (population, (const int **)temp_population);
		generation++;
		get_fitness ((const int **)population, &fitness_max, &fitness_avg);
	}
	print_population();

	/* Free heap. */
	for(i=0; i<population_size; i++)
		free(population[i]);
	free(population);
	for(i=0; i<population_size; i++)
		free(temp_population[i]);
	free(temp_population);

	return 0;
}

