#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#define TRUE 1
#define FALSE 0

#define NCURSES TRUE

struct Cell {
	struct Cell * p[8];
	int n;
	int alive;
};

struct Map {
	struct Cell *old;
	struct Cell *map;
	int n;
};

int get_grid_pos(int x, int y, int n) {
	if (x == -1)
		x = n - 1;
	if (x == n)
		x = 0;
	if (y == -1)
		y = n - 1;
	if (y == n)
		y = 0;
	return (x * n) + y;
}



void allocate_map(struct Map *m, int n) {
	m->map = (struct Cell *) malloc(sizeof(struct Cell) * n * n);
	m->old = (struct Cell *) malloc(sizeof(struct Cell) * n * n);
	m->n = n;
	int x, y, c;
	int cx, cy;
	struct Cell * current, *currold;
	for (x = 0; x < n; x++) {
		for (y = 0; y < n; y++) {
			c = get_grid_pos(x, y, n);
			current = &(m->map[c]);
			currold = &(m->old[c]);
			for (cx = -1; cx <= 1; cx++) {
				for (cy = -1; cy <= 1; cy++) {
					if (cx == 0 && cy == 0)
						continue;
					c = get_grid_pos(x + cx, y + cy, n);
					current->p[current->n] = &(m->map[c]);
					currold->p[currold->n] = &(m->old[c]);
					current->n++;
					currold->n++;
				}
			}
		}
	}
}

void print_map(struct Map *m) {
	int x, y, c;
	int n = m->n;
	for (x = 0; x < n; x++) {
		for (y = 0; y < n; y++) {
			c = get_grid_pos(x, y, n);
			printf("%c", (m->map[c].alive == TRUE)?'+':' ');
		}
		printf("\n");
	}
}

void print_map_nc(struct Map *m, int frame) {
	int x, y, c;
	mvprintw(0, 0, "== Frame %d ==", frame);
	for (x = 0; x < m->n; x++) {
		for (y = 0; y < m-> n; y++) {
			c = get_grid_pos(x, y, m->n);
			mvaddch(y + 1, x, (m->map[c].alive == TRUE)?'+':' ');
		}
	}
	refresh();
}

void free_map(struct Map *m) {
	free(m->map);
}

void initial_map(struct Map *m, char *fn) {
	FILE * fp = fopen(fn, "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	if (fp == NULL)
		exit(-1);
	
	int A, B;
	while ((read = getline(&line, &len, fp)) != -1) {
		sscanf(line, "%d %d", &A, &B);
		
		//printf("[%d, %d] :: %s", A, B, line);
		m->map[get_grid_pos(A, B, m->n)].alive = TRUE;
	}
	fclose(fp);
}

int neighbour_count(struct Cell *c) {
	int i, count = 0;
	for (i = 0; i < c->n; i++) {
		if (c->p[i]->alive == TRUE)
			count++;
	}
	return count;
}

void prop_map(struct Map *m) {
	int x, y, c, count;
	struct Cell *curr, *currnew;
	struct Cell *tmp;
	for (x = 0; x < m->n*m->n; x++)
		m->old[x].alive = m->map[x].alive;

	for (x = 0; x < m->n; x++) {
		for (y = 0; y < m->n; y++) {
			c = get_grid_pos(x, y, m->n);
			curr = &(m->old[c]);
			currnew = &(m->map[c]);
			count = neighbour_count(curr);

			if (count < 2)
				currnew->alive = FALSE;
			else if (count > 3)
				currnew->alive = FALSE;
			else if (count == 3)
				currnew->alive = TRUE;
		}
	}
}

int main(int argc, char * argv[]) {
	int n = 32;
	struct Map sys;
	char filename[255] = "glider.gol";
	int nframes = 128, q;
	allocate_map(&sys, n);
	initial_map(&sys, filename);
	if (!NCURSES) {
		for (q = 0; q < nframes; q++) {
			printf("=== %d ===\n", q);
			print_map(&sys);
			prop_map(&sys);
		}
	} else {
		initscr();
		for (q = 0; q < nframes; q++) {
			print_map_nc(&sys, q);
			prop_map(&sys);
			napms(200);
		}
		endwin();
	}

	free_map(&sys);
	return 1;
}
