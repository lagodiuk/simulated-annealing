#include "canvas.h"
#include "color.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define WIDTH 100
#define HEIGHT 100
#define INITIAL_TEMPERATURE 10
#define FINAL_TEMPERATURE 0.1
#define ALPHA 0.998
#define STEPS_PER_CHANGE 100
#define ANSWER_ENERGY_THRESHOLD 1e-5

typedef
struct {
	Canvas * canvas;
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
	int x1 = getRand(WIDTH);
	int y1 = getRand(HEIGHT);
    Color c1 = get_pixel(x1, y1, m->canvas);

    int x2 = getRand(WIDTH);
	int y2 = getRand(HEIGHT);
    Color c2 = get_pixel(x2, y2, m->canvas);
    
    while((c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)) {
        x2 = getRand(WIDTH);
        y2 = getRand(HEIGHT);
        c2 = get_pixel(x2, y2, m->canvas);
    }
    
    set_pixel(x1, y1, c2, m->canvas);
    set_pixel(x2, y2, c1, m->canvas);
}

void
initializeSolution(memberType * m) {
	m->canvas = new_canvas(WIDTH, HEIGHT);
    int i;
    int j;
    int c;
    Color colors[3] = {rgb(255, 0, 0), rgb(0, 255, 0), rgb(0, 0, 255)};
    for(i = 0; i < WIDTH; ++i) {
        for(j = 0; j < HEIGHT; ++j) {
            c = getRand(3);
            set_pixel(i, j, colors[c], m->canvas);
        }
    }
}

void
calculateEnergy(memberType * m) {
    int diff = 0;
    int dx[4] = {-1, 1,  0, 0};
    int dy[4] = { 0, 0, -1, 1};
    int i;
    int j;
    int k;
    Color c1;
    Color c2;
    for(i = 1; i < WIDTH - 1; ++i) {
        for(j = 1; j < HEIGHT - 1; ++j) {
            c1 = get_pixel(i, j, m->canvas);
            for(k = 0; k < 4; ++k) {
                c2 = get_pixel(i + dx[k], j + dy[k], m->canvas);
                if((c1.r != c2.r) || (c1.g != c2.g) || (c1.b != c2.b))
                    ++diff;
            }
        }
    }
    m->energy = diff;
}

void
displaySolution(memberType * m, char file_name[]) {
    write_png(file_name, m->canvas);
}

void
copySolution(memberType * source, memberType * dest) {
	dest->energy = source->energy;
    if(!dest->canvas) {
        dest->canvas = new_canvas(WIDTH, HEIGHT);
    }
    
    int i;
    int j;
    for(i = 0; i < WIDTH; ++i) {
        for(j = 0; j < HEIGHT; ++j) {
            set_pixel(i, j, get_pixel(i, j, source->canvas), dest->canvas);
        }
    }
}

int 
main() {
	srand(time(NULL));

	memberType current;
	memberType working;
	memberType best;
    
    current.canvas = NULL;
    working.canvas = NULL;
    best.canvas = NULL;

	int step;
	float deltaEnergy;
	float p;

	int useNew;
	int accepted;

	float temperature = INITIAL_TEMPERATURE;

	initializeSolution(&current);
	calculateEnergy(&current);
    
    displaySolution(&current, "start.png");
    
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
					//displaySolution(&best);
				}
			} else {
				copySolution(&current, &working);
			}
		}
		fprintf(log, "%f\t%f\t%f\n", temperature, (float)accepted, (float)best.energy);
   		printf("%f\t%f\t%f\n", temperature, (float)accepted, (float)best.energy);
		if(best.energy < ANSWER_ENERGY_THRESHOLD) {
			break;
		}
		temperature *= ALPHA;
	}

	fclose(log);
    
    displaySolution(&best, "result.png");
	return 0;
}
