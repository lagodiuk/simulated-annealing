#include "canvas.h"
#include "color.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define WIDTH 120
#define HEIGHT 90
#define INITIAL_TEMPERATURE 5
#define FINAL_TEMPERATURE 0.1
#define ALPHA 0.998
#define STEPS_PER_CHANGE 100
#define ANSWER_ENERGY_THRESHOLD 1e-5
#define SAME_COLORS_THRESHOLD 1e-5
#define TELEPORT

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

float
potential_energy(Color * c1, Color * c2) {
	float dr = (c1->r - c2->r) / (float) 256;
	float dg = (c1->g - c2->g) / (float) 256;
	float db = (c1->b - c2->b) / (float) 256;
	return dr * dr + dg * dg + db * db;
}

#ifdef TELEPORT
// Swap random points
void
shuffleSolution(memberType * m) {
	int x1 = getRand(WIDTH);
	int y1 = getRand(HEIGHT);
    Color c1 = get_pixel(x1, y1, m->canvas);

    int x2 = getRand(WIDTH);
	int y2 = getRand(HEIGHT);
    Color c2 = get_pixel(x2, y2, m->canvas);
    
    while(potential_energy(&c1, &c2) < SAME_COLORS_THRESHOLD) {
        x2 = getRand(WIDTH);
        y2 = getRand(HEIGHT);
        c2 = get_pixel(x2, y2, m->canvas);
    }
    
    set_pixel(x1, y1, c2, m->canvas);
    set_pixel(x2, y2, c1, m->canvas);
}
#else
// Swap neighbour points
void
shuffleSolution(memberType * m) {
	int x1 = getRand(WIDTH - 1) + 1;
	int y1 = getRand(HEIGHT - 1) + 1;
    Color c1 = get_pixel(x1, y1, m->canvas);
    
    int dx[8] = {-1, 1,  0, 0, -1, -1,  1, 1};
    int dy[8] = { 0, 0, -1, 1, -1,  1, -1, 1};
    
    int i = getRand(4);
    
    int x2 = x1 + dx[i];
	int y2 = y1 + dy[i];
    Color c2 = get_pixel(x2, y2, m->canvas);
    
    set_pixel(x1, y1, c2, m->canvas);
    set_pixel(x2, y2, c1, m->canvas);
}
#endif // TELEPORT

void
initializeSolution(memberType * m) {
	m->canvas = new_canvas(WIDTH, HEIGHT);
    int i;
    int j;
    Color c;
    float rand;
    for(i = 0; i < WIDTH; ++i) {
        for(j = 0; j < HEIGHT; ++j) {
            rand = getRandF();
            if(rand < 0.5)
                c = rgb(getRand(50), getRand(256), getRand(50));
            else if(rand < 0.75)
                c = rgb(getRand(150), getRand(150), getRand(256));
            else
                c = rgb(getRand(256), getRand(150), getRand(150));
            set_pixel(i, j, c, m->canvas);
        }
    }
}

void
calculateEnergy(memberType * m) {
    float diff = 0;
    int dx[8] = {-1, 1,  0, 0, -1, -1,  1, 1};
    int dy[8] = { 0, 0, -1, 1, -1,  1, -1, 1};
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
                diff += potential_energy(&c1, &c2);
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

    int num;
    char file_name[200];
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
                    
                    ++num;
                    if(num % 200 == 0) {
                        sprintf(file_name, "step_%06i.png", (num / 200));
                        displaySolution(&best, file_name);
                    }
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
