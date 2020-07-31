#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mtrand.h"
#include "mp_utils.h"

#define ROTATIONS_MAX 4
#define PIPES_MAX 4UL
#define CORNER_PIPES_MAX 2UL
#define PIPE_NORTH 1
#define PIPE_EAST 2
#define PIPE_SOUTH 4
#define PIPE_WEST 8
#define ALL_PIPES (PIPE_NORTH+PIPE_EAST+PIPE_SOUTH+PIPE_WEST)
#define NOT_SET (ALL_PIPES+1)

typedef struct tile_s tile_t;

struct tile_s {
	unsigned long pipes_n;
	int rotation;
	unsigned long rank;
	tile_t *root;
	unsigned long row;
	unsigned long column;
	unsigned long rotations_n;
	int rotations[ROTATIONS_MAX];
	int option;
	unsigned long options_n;
	int options[ROTATIONS_MAX];
	tile_t *last;
	tile_t *next;
};

typedef struct {
	tile_t *tile1;
	int pipe1;
	tile_t *tile2;
	int pipe2;
}
edge_t;

void link_tile(tile_t *, tile_t *, tile_t *);
int start_generator(void);
void set_tile_generator(tile_t *);
void set_edge(edge_t *, unsigned long);
int edge_opened(edge_t *);
tile_t *find_root(tile_t *);
void add_tile_pipe(tile_t *, int);
void rotate_tile(tile_t *);
int start_solver(void);
int set_tile_common(tile_t *, unsigned long, unsigned long);
int check_rotation(unsigned long, unsigned long, int);
void solve(tile_t **);
void set_options(tile_t *);
int set_constraint(tile_t *, int, int);
int check_constraint(int, int, int);
void lock_tile(tile_t *, int);
void unlock_tile(tile_t *);

int generate;
unsigned time0;
unsigned long rows_n, columns_n, tiles_n;
mp_t nodes_n, solutions_n;
tile_t *tiles, *header, **locks;

int main(void) {
	tile_t *tile;
	if (scanf("%lu%lu%d", &rows_n, &columns_n, &generate) != 3 || rows_n == 0UL || columns_n == 0UL) {
		fprintf(stderr, "Invalid grid size or generate flag\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	tiles_n = rows_n*columns_n;
	tiles = malloc(sizeof(tile_t)*(tiles_n+1UL));
	if (!tiles) {
		fprintf(stderr, "Could not allocate memory for tiles\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	header = tiles+tiles_n;
	link_tile(tiles, header, tiles+1);
	for (tile = tiles+1; tile < header; tile++) {
		link_tile(tile, tile-1, tile+1);
	}
	link_tile(tile, tile-1, tiles);
	locks = malloc(sizeof(tile_t *)*tiles_n);
	if (!locks) {
		fprintf(stderr, "Could not allocate memory for locks\n");
		fflush(stderr);
		free(tiles);
		return EXIT_FAILURE;
	}
	if (generate) {
		if (start_generator() < 0) {
			free(locks);
			free(tiles);
			return EXIT_FAILURE;
		}
	}
	else {
		if (start_solver() < 0) {
			free(locks);
			free(tiles);
			return EXIT_FAILURE;
		}
	}
	free(locks);
	free(tiles);
	return EXIT_SUCCESS;
}

void link_tile(tile_t *tile, tile_t *last, tile_t *next) {
	tile->last = last;
	tile->next = next;
}

int start_generator(void) {
	int unique_solution, found;
	unsigned long pipes_min, edges_max, *edge_nums, pipes_n1, i;
	if (scanf("%d%lu", &unique_solution, &pipes_min) != 2 || pipes_min == 0UL || pipes_min > CORNER_PIPES_MAX) {
		fprintf(stderr, "Invalid unique solution flag or minimum number of pipes\n");
		fflush(stderr);
		return -1;
	}
	edges_max = tiles_n*2UL-rows_n-columns_n;
	edge_nums = malloc(sizeof(unsigned long)*edges_max);
	if (!edge_nums) {
		fprintf(stderr, "Could not allocate memory for edge_nums\n");
		fflush(stderr);
		return -1;
	}
	smtrand((unsigned long)time(NULL));
	do {
		unsigned long edges_n, opened;
		for (i = 0UL; i < tiles_n; i++) {
			set_tile_generator(tiles+i);
		}
		edges_n = edges_max;
		for (i = 0UL; i < edges_n; i++) {
			edge_nums[i] = i;
		}
		opened = 0UL;
		while (opened < tiles_n-1UL) {
			unsigned long j;
			edge_t edge;
			j = emtrand(edges_n);
			set_edge(&edge, edge_nums[j]);
			if (!edge_opened(&edge)) {
				tile_t *root1 = find_root(edge.tile1), *root2 = find_root(edge.tile2);
				if (root1 != root2) {
					if (root1->rank < root2->rank) {
						root1->root = root2;
					}
					else if (root1->rank > root2->rank) {
						root2->root = root1;
					}
					else {
						root1->rank++;
						root2->root = root1;
					}
					add_tile_pipe(edge.tile1, edge.pipe1);
					add_tile_pipe(edge.tile2, edge.pipe2);
					opened++;
				}
			}
			edge_nums[j] = edge_nums[--edges_n];
		}
		for (pipes_n1 = 1UL; pipes_n1 < pipes_min; pipes_n1++) {
			unsigned long pipes_n2;
			for (pipes_n2 = pipes_n1; pipes_n2 < PIPES_MAX; pipes_n2++) {
				edges_n = edges_max;
				for (i = 0UL; i < edges_n; i++) {
					edge_nums[i] = i;
				}
				while (edges_n > 0UL) {
					unsigned long j;
					edge_t edge;
					j = emtrand(edges_n);
					set_edge(&edge, edge_nums[j]);
					if (((edge.tile1->pipes_n == pipes_n1 && edge.tile2->pipes_n == pipes_n2) || (edge.tile1->pipes_n == pipes_n2 && edge.tile2->pipes_n == pipes_n1)) && !edge_opened(&edge)) {
						add_tile_pipe(edge.tile1, edge.pipe1);
						add_tile_pipe(edge.tile2, edge.pipe2);
					}
					edge_nums[j] = edge_nums[--edges_n];
				}
			}
		}
		for (i = 0UL; i < tiles_n; i++) {
			rotate_tile(tiles+i);
		}
		if (unique_solution) {
			found = start_solver();
			if (found < 0) {
				free(edge_nums);
				return -1;
			}
		}
		else {
			found = 1;
		}
	}
	while (!found);
	printf("%lu\n%lu\n0\n", rows_n, columns_n);
	for (i = 0UL; i < rows_n; i++) {
		unsigned long j;
		printf("%d", tiles[i*columns_n].rotation);
		for (j = 1UL; j < columns_n; j++) {
			printf(" %d", tiles[i*columns_n+j].rotation);
		}
		puts("");
	}
	fflush(stdout);
	free(edge_nums);
	return 1;
}

void set_tile_generator(tile_t *tile) {
	tile->pipes_n = 0UL;
	tile->rotation = 0;
	tile->rank = 0UL;
	tile->root = tile;
}

void set_edge(edge_t *edge, unsigned long num) {
	if (num < tiles_n-rows_n) {
		unsigned long row = num/(columns_n-1UL);
		edge->tile1 = tiles+row*(columns_n-1UL)+num%(columns_n-1UL)+row;
		edge->pipe1 = PIPE_EAST;
		edge->tile2 = edge->tile1+1;
		edge->pipe2 = PIPE_WEST;
	}
	else {
		edge->tile1 = tiles+num/columns_n*columns_n+num%columns_n-(tiles_n-rows_n);
		edge->pipe1 = PIPE_SOUTH;
		edge->tile2 = edge->tile1+columns_n;
		edge->pipe2 = PIPE_NORTH;
	}
}

int edge_opened(edge_t *edge) {
	return (edge->tile1->rotation & edge->pipe1) && (edge->tile2->rotation & edge->pipe2);
}

tile_t *find_root(tile_t *tile) {
	if (tile->root != tile) {
		tile->root = find_root(tile->root);
	}
	return tile->root;
}

void add_tile_pipe(tile_t *tile, int rotation) {
	tile->pipes_n++;
	tile->rotation += rotation;
}

void rotate_tile(tile_t *tile) {
	int shift = (int)emtrand((unsigned long)ROTATIONS_MAX);
	if (shift > 0) {
		tile->rotation = ((tile->rotation >> (ROTATIONS_MAX-shift)) | (tile->rotation << shift)) & ALL_PIPES;
	}
}

int start_solver(void) {
	int found;
	unsigned long i;
	for (i = 0UL; i < rows_n; i++) {
		unsigned long j;
		for (j = 0UL; j < columns_n; j++) {
			if (!set_tile_common(tiles+i*columns_n+j, i, j)) {
				return -1;
			}
		}
	}
	if (!mp_new(&nodes_n)) {
		return -1;
	}
	if (!mp_new(&solutions_n)) {
		mp_free(&nodes_n);
		return -1;
	}
	time0 = (unsigned)time(NULL);
	solve(locks);
	if (!generate) {
		printf("Runtime %us\n", (unsigned)time(NULL)-time0);
		mp_print("Nodes", &nodes_n);
		mp_print("Solutions", &solutions_n);
		fflush(stdout);
	}
	found = !mp_eq_val(&solutions_n, 0UL) && (!generate || mp_eq_val(&solutions_n, 1UL));
	mp_free(&solutions_n);
	mp_free(&nodes_n);
	return found;
}

int set_tile_common(tile_t *tile, unsigned long row, unsigned long column) {
	int rotation;
	unsigned long i;
	tile->row = row;
	tile->column = column;
	tile->rotations_n = 0UL;
	if (generate) {
		rotation = tile->rotation;
	}
	else {
		if (scanf("%d", &rotation) != 1 || rotation < 0 || rotation > ALL_PIPES) {
			fprintf(stderr, "Invalid tile\n");
			fflush(stderr);
			return 0;
		}
	}
	if (check_rotation(row, column, rotation)) {
		tile->rotations[0] = rotation;
		tile->rotations_n = 1UL;
	}
	for (i = 1UL; i < ROTATIONS_MAX; i++) {
		rotation = ((rotation >> 3) | (rotation << 1)) & ALL_PIPES;
		if (check_rotation(row, column, rotation)) {
			if (tile->rotations_n > 0UL && tile->rotations[0] == rotation) {
				break;
			}
			tile->rotations[tile->rotations_n++] = rotation;
		}
	}
	tile->option = NOT_SET;
	return 1;
}

int check_rotation(unsigned long row, unsigned long column, int rotation) {
	return (row > 0UL || !(rotation & PIPE_NORTH)) && (column < columns_n-1UL || !(rotation & PIPE_EAST)) && (row < rows_n-1UL || !(rotation & PIPE_SOUTH)) && (column > 0UL || !(rotation & PIPE_WEST));
}

void solve(tile_t **start) {
	tile_t **end, *tile_min, *tile;
	if (!mp_inc(&nodes_n)) {
		return;
	}
	if (generate && mp_eq_val(&solutions_n, 2UL)) {
		return;
	}
	if (header->next == header) {
		if (!mp_inc(&solutions_n)) {
			return;
		}
		if (!generate && mp_eq_val(&solutions_n, 1UL)) {
			unsigned long i;
			printf("Runtime %us\n", (unsigned)time(NULL)-time0);
			mp_print("Nodes", &nodes_n);
			for (i = 0UL; i < rows_n; i++) {
				unsigned long j;
				printf("%d", tiles[i*columns_n].option);
				for (j = 1UL; j < columns_n; j++) {
					printf(" %d", tiles[i*columns_n+j].option);
				}
				puts("");
			}
			fflush(stdout);
		}
		return;
	}
	end = start;
	tile_min = header->next;
	set_options(tile_min);
	if (tile_min->options_n == 1UL) {
		*end = tile_min;
		end++;
	}
	for (tile = tile_min->next; tile != header && tile_min->options_n > 0UL; tile = tile->next) {
		set_options(tile);
		if (tile->options_n == 1UL) {
			*end = tile;
			end++;
		}
		if (tile->options_n < tile_min->options_n) {
			tile_min = tile;
		}
	}
	if (tile_min->options_n == 1UL) {
		tile_t **lock;
		for (lock = start; lock < end; lock++) {
			lock_tile(*lock, (*lock)->options[0]);
		}
		solve(end);
		for (lock = end-1; lock >= start; lock--) {
			unlock_tile(*lock);
		}
	}
	else if (tile_min->options_n > 1UL) {
		unsigned long i;
		for (i = 0UL; i < tile_min->options_n; i++) {
			lock_tile(tile_min, tile_min->options[i]);
			solve(end);
			unlock_tile(tile_min);
		}
	}
}

void set_options(tile_t *tile) {
	int constraint_north, constraint_east, constraint_south, constraint_west;
	unsigned long i;
	constraint_north = tile->row > 0UL ? set_constraint(tile-columns_n, PIPE_SOUTH, PIPE_NORTH):0;
	constraint_east = tile->column < columns_n-1UL ? set_constraint(tile+1, PIPE_WEST, PIPE_EAST):0;
	constraint_south = tile->row < rows_n-1UL ? set_constraint(tile+columns_n, PIPE_NORTH, PIPE_SOUTH):0;
	constraint_west = tile->column > 0UL ? set_constraint(tile-1, PIPE_EAST, PIPE_WEST):0;
	tile->options_n = 0UL;
	for (i = 0UL; i < tile->rotations_n; i++) {
		if (check_constraint(constraint_north, tile->rotations[i], PIPE_NORTH) && check_constraint(constraint_east, tile->rotations[i], PIPE_EAST) && check_constraint(constraint_south, tile->rotations[i], PIPE_SOUTH) && check_constraint(constraint_west, tile->rotations[i], PIPE_WEST)) {
			tile->options[tile->options_n++] = tile->rotations[i];
		}
	}
}

int set_constraint(tile_t *tile, int mask, int constraint) {
	return tile->option < NOT_SET ? tile->option & mask ? constraint:0:NOT_SET;
}

int check_constraint(int constraint, int rotation, int mask) {
	return constraint == NOT_SET || (rotation & mask) == constraint;
}

void lock_tile(tile_t *tile, int option) {
	tile->next->last = tile->last;
	tile->last->next = tile->next;
	tile->option = option;
}

void unlock_tile(tile_t *tile) {
	tile->option = NOT_SET;
	tile->last->next = tile;
	tile->next->last = tile;
}
