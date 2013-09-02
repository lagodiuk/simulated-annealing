//
// Solving N-queens puzzle
// Example from book: "AI Application Programming" by M. Tim Jones (chapter 2)
//

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define SIZE 40
#define INITIAL_TEMPERATURE 100
#define FINAL_TEMPERATURE 0.5
#define ALPHA 0.99
#define STEPS_PER_CHANGE 100
#define ANSWER_ENERGY_THRESHOLD 1e-5

typedef 
int 
solutionType[SIZE];

typedef
struct {
	solutionType solution;
	float energy;
}
memberType;

int
getRand(int max) {
	return rand() % max;
}

float
getRandF() {
	return (float)rand() / (float)RAND_MAX;
}

void
shuffleSolution(memberType * m) {
	int x;
	int y;
	int temp;
	x = getRand(SIZE);
	do {
		y = getRand(SIZE);
	} while (x == y);
	temp = m->solution[x];
	m->solution[x] = m->solution[y];
	m->solution[y] = temp;
}

void
initializeSolution(memberType * m) {
	int i;
	for(i = 0; i < SIZE; ++i) {
		m->solution[i] = i;
	}
	for(i = 0; i < SIZE; ++i) {
		shuffleSolution(m);
	}
}

void
calculateEnergy(memberType * m) {
	int dx[4] = {-1, -1, 1,  1};
	int dy[4] = { 1, -1, 1, -1};

	int collisions = 0;
	
	int i;
	int j;
	int currX;
	int currY;
	
	for(i = 0; i < SIZE; ++i) {
		for(j = 0; j < 4; ++j) {
			currX = i;
			currY = m->solution[i];
			
			currX += dx[j];
			currY += dy[j];
			while((currX >= 0) && (currX < SIZE)) {
				if(m->solution[currX] == currY) {
					++collisions;
				}
				currX += dx[j];
				currY += dy[j];
			}
		}
	}

	m->energy = (float) collisions;
}

void
displaySolution(memberType * m) {
	int i;
	int j;
	for(i = 0; i < SIZE; ++i) {
		for(j = 0; j < SIZE; ++j) {
			if(m->solution[i] == j)
				printf("X");
			else
				printf("O");
		}
		printf("\n");
	}
	printf("Energy is %f\n", m->energy);
}

void
copySolution(memberType * source, memberType * dest) {
	dest->energy = source->energy;
	int i;
	for(i = 0; i < SIZE; ++i) {
		dest->solution[i] = source->solution[i];
	}
}

int 
main() {
	srand(time(NULL));

	memberType current;
	memberType working;
	memberType best;

	int step;
	float deltaEnergy;
	float p;

	copySolution(&current, &best);
	int useNew;
	int accepted;

	float temperature = INITIAL_TEMPERATURE;

	initializeSolution(&current);
	calculateEnergy(&current);

	copySolution(&current, &best);
	copySolution(&current, &working);

	FILE * log = fopen("log.txt", "w");
	fprintf(log, "temperature\taccepted\tbest_energy\n");

	while(temperature > FINAL_TEMPERATURE) {
		accepted = 0;
		for(step = 0; step < STEPS_PER_CHANGE; ++step) {
			useNew = 0;
			
			shuffleSolution(&working);
			calculateEnergy(&working);

			if(working.energy < current.energy) {
				useNew = 1;
			} else {
				deltaEnergy = working.energy - current.energy;
				if(getRandF() < expf(- deltaEnergy / temperature)) {
					useNew = 1;
					++accepted;
				}
			}

			if(useNew) {
				useNew = 0;

				copySolution(&working, &current);
				if(current.energy < best.energy) {
					copySolution(&current, &best);
					displaySolution(&best);
				}
			} else {
				copySolution(&current, &working);
			}
		}
		fprintf(log, "%f\t%f\t%f\n", temperature, (float)accepted, (float)best.energy);
		if(best.energy < ANSWER_ENERGY_THRESHOLD) {
			break;
		}
		temperature *= ALPHA;
	}

	fclose(log);

	return 0;
}
