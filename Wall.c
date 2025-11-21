// Wall.c - COMP2521 Assignment 2 - Task 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Wall.h"

struct wall {
    int height;
    int width;
    int capacity;
    int numRocks;
    struct rock *rocks;
};

Wall WallNew(int height, int width) {
    Wall w = malloc(sizeof(struct wall));
    w->height = height;
    w->width = width;
    w->numRocks = 0;
    w->capacity = 16;
    w->rocks = malloc(w->capacity * sizeof(struct rock));
    return w;
}

void WallFree(Wall w) {
    free(w->rocks);
    free(w);
}

int WallHeight(Wall w) { return w->height; }
int WallWidth(Wall w)  { return w->width; }

int WallNumRocks(Wall w) { return w->numRocks; }

void WallAddRock(Wall w, int row, int col, Colour colour) {
    // Replace if exists
    for (int i = 0; i < w->numRocks; i++) {
        if (w->rocks[i].row == row && w->rocks[i].col == col) {
            w->rocks[i].colour = colour;
            return;
        }
    }

    // Grow array if needed
    if (w->numRocks == w->capacity) {
        w->capacity *= 2;
        w->rocks = realloc(w->rocks, w->capacity * sizeof(struct rock));
    }

    w->rocks[w->numRocks].row = row;
    w->rocks[w->numRocks].col = col;
    w->rocks[w->numRocks].colour = colour;
    w->numRocks++;
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

static double dist(int r1, int c1, int r2, int c2) {
    return sqrt((r1 - r2) * (r1 - r2) + (c1 - c2) * (c1 - c2));
}

int WallGetRocksInRange(Wall w, int row, int col, int d, struct rock rocks[]) {
    int count = 0;
    for (int i = 0; i < w->numRocks; i++) {
        if (dist(row, col, w->rocks[i].row, w->rocks[i].col) <= d + 1e-9) {
            rocks[count++] = w->rocks[i];
        }
    }
    return count;
}

int WallGetColouredRocksInRange(Wall w, int row, int col, int d, Colour c, struct rock rocks[]) {
    int count = 0;
    for (int i = 0; i < w->numRocks; i++) {
        if (w->rocks[i].colour == c && dist(row, col, w->rocks[i].row, w->rocks[i].col) <= d + 1e-9) {
            rocks[count++] = w->rocks[i];
        }
    }
    return count;
}
