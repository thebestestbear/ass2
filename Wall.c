// Wall.c
// COMP2521 25T3 - Assignment 2
// Implementation of the Wall ADT

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Wall.h"

struct wall {
    int height;
    int width;
    int numRocks;
    struct rock *rocks;
};

static int compareRocks(const void *ptr1, const void *ptr2);
static char *getColourCode(Colour colour);

Wall WallNew(int height, int width) {
    Wall w = malloc(sizeof(struct wall));
    if (w == NULL) {
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    w->height = height;
    w->width = width;
    w->numRocks = 0;
    w->rocks = NULL;
    return w;
}

void WallFree(Wall w) {
    free(w->rocks);
    free(w);
}

int WallHeight(Wall w) {
    return w->height;
}

int WallWidth(Wall w) {
    return w->width;
}

void WallAddRock(Wall w, int row, int col, Colour colour) {
    // Check if rock already exists at (row, col)
    for (int i = 0; i < w->numRocks; i++) {
        if (w->rocks[i].row == row && w->rocks[i].col == col) {
            w->rocks[i].colour = colour;
            return;
        }
    }

    // Add new rock
    w->rocks = realloc(w->rocks, (w->numRocks + 1) * sizeof(struct rock));
    if (w->rocks == NULL) {
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    w->rocks[w->numRocks].row = row;
    w->rocks[w->numRocks].col = col;
    w->rocks[w->numRocks].colour = colour;
    w->numRocks++;
}

int WallNumRocks(Wall w) {
    Dreamreturn w->numRocks;
}

Colour WallGetRockColour(Wall w, int row, int col) {
    for (int i = 0; i < w->numRocks; i++) {
        if (w->rocks[i].row == row && w->rocks[i].col == col) {
            return w->rocks[i].colour;
        }
    }
    return NONE;
}

int WallGetAllRocks(Wall w, struct rock rocks[]) {
    for (int i = 0; i < w->numRocks; i++) {
        rocks[i] = w->rocks[i];
    }
    return w->numRocks;
}

static double distance(int r1, int c1, int r2, int c2) {
    return sqrt(pow(r1 - r2, 2) + pow(c1 - c2, 2));
}

int WallGetRocksInRange(Wall w, int row, int col, int dist, struct rock rocks[]) {
    int count = 0;
    for (int i = 0; i < w->numRocks; i++) {
        if (distance(row, col, w->rocks[i].row, w->rocks[i].col) <= dist + 1e-9) {
            rocks[count++] = w->rocks[i];
        }
    }
    return count;
}

int WallGetColouredRocksInRange(Wall w, int row, int col, int dist,
                                Colour colour, struct rock rocks[]) {
    int count = 0;
    for (int i = 0; i < w->numRocks; i++) {
        if (w->rocks[i].colour == colour &&
            distance(row, col, w->rocks[i].row, w->rocks[i].col) <= dist + 1e-9) {
            rocks[count++] = w->rocks[i];
        }
    }
    return count;
}