all:
	gcc -g knapsack.c rand.c -O2 -Wall -Wextra -Wno-unused-result -o knapsack_rws -DENABLE_THREEPOINT_CROSSOVER
	gcc -g knapsack.c rand.c -O2 -Wall -Wextra -Wno-unused-result -o knapsack_tour -DENABLE_TOURNAMENT_SELECTION -DENABLE_UNIFORM_CROSSOVER
clean:
	rm -f knapsack_rws knapsack_tour ##knapsack_multi
