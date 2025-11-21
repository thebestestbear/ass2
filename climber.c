// climber.c - COMP2521 Assignment 2 - Tasks 2,3,4

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "climber.h"
#include "Wall.h"

#define MAX_ROCKS 10000

// Helper: check if two rocks are equal
static bool rockEq(struct rock a, struct rock b) {
    return a.row == b.row && a.col == b.col;
}

// Task 2: Shortest path (fewest rocks) using only one colour
struct path findShortestPath(Wall w, int reach, Colour colour) {
    struct path noPath = {0, NULL};

    int height = WallHeight(w);
    struct rock all[MAX_ROCKS];
    int n = WallGetAllRocks(w, all);

    bool visited[MAX_ROCKS] = {false};
    struct rock parent[MAX_ROCKS];
    int q[MAX_ROCKS], front = 0, rear = 0;

    // Start from all bottom rocks of the given colour
    for (int i = 0; i < n; i++) {
        if (all[i].row == 1 && all[i].colour == colour) {
            q[rear++] = i;
            visited[i] = true;
            parent[i] = all[i];
        }
    }

    int goalIdx = -1;
    while (front < rear) {
        int idx = q[front++];
        struct rock curr = all[idx];

        if (curr.row + reach >= height) {
            goalIdx = idx;
            break;
        }

        struct rock next[MAX_ROCKS];
        int m = WallGetColouredRocksInRange(w, curr.row, curr.col, reach, colour, next);

        for (int j = 0; j < m; j++) {
            if (next[j].row <= curr.row) continue;  // must go up

            for (int k = 0; k < n; k++) {
                if (!visited[k] && rockEq(next[j], all[k])) {
                    visited[k] = true;
                    parent[k] = curr;
                    q[rear++] = k;
                    break;
                }
            }
        }
    }

    if (goalIdx == -1) return noPath;

    // Reconstruct path
    struct rock path[MAX_ROCKS];
    int len = 0;
    struct rock cur = all[goalIdx];
    while (true) {
        path[len++] = cur;
        if (cur.row == 1) break;
        bool found = false;
        for (int i = 0; i < n; i++) {
            if (rockEq(parent[i], cur)) {
                cur = all[i];
                found = true;
                break;
            }
        }
        if (!found) break;
    }

    struct path p;
    p.numRocks = len;
    p.rocks = malloc(len * sizeof(struct rock));
    for (int i = 0; i < len; i++) {
        p.rocks[i] = path[len - 1 - i];
    }
    return p;
}

// Task 3: Minimum total energy path (any colour allowed)
struct path findMinEnergyPath(Wall w, int reach, int energyCosts[NUM_COLOURS]) {
    struct path noPath = {0, NULL};

    int height = WallHeight(w);
    struct rock all[MAX_ROCKS];
    int n = WallGetAllRocks(w, all);

    int dist[MAX_ROCKS];
    struct rock prev[MAX_ROCKS];
    bool done[MAX_ROCKS] = {false};

    for (int i = 0; i < n; i++) dist[i] = INT_MAX;

    // Initialize from bottom
    for (int i = 0; i < n; i++) {
        if (all[i].row == 1) {
            dist[i] = energyCosts[all[i].colour];
            prev[i] = all[i];  // self
        }
    }

    while (true) {
        int best = -1;
        int bestDist = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!done[i] && dist[i] < bestDist) {
                bestDist = dist[i];
                best = i;
            }
        }
        if (best == -1) break;
        done[best] = true;

        struct rock curr = all[best];
        if (curr.row + reach >= height) {
            // Reconstruct path
            struct rock path[MAX_ROCKS];
            int len = 0;
            int idx = best;
            while (idx < n) {
                path[len++] = all[idx];
                if (all[idx].row == 1) break;
                struct rock p = prev[idx];
                bool found = false;
                for (int j = 0; j < n; j++) {
                    if (rockEq(p, all[j])) {
                        idx = j;
                        found = true;
                        break;
                    }
                }
                if (!found) break;
            }

            struct path p;
            p.numRocks = len;
            p.rocks = malloc(len * sizeof(struct rock));
            for (int i = 0; i < len; i++) {
                p.rocks[i] = path[len - 1 - i];
            }
            return p;
        }

        struct rock next[MAX_ROCKS];
        int m = WallGetRocksInRange(w, curr.row, curr.col, reach, next);

        for (int j = 0; j < m; j++) {
            if (next[j].row <= curr.row) continue;
            for (int k = 0; k < n; k++) {
                if (rockEq(next[j], all[k])) {
                    int cost = energyCosts[all[k].colour];
                    int newDist = dist[best] + cost;
                    if (newDist < dist[k]) {
                        dist[k] = newDist;
                        prev[k] = curr;
                    }
                    break;
                }
            }
        }
    }

    return noPath;
}

// Task 4: Minimum number of turns (moves + rests), with energy recovery on rest
struct path findMinTurnsPath(Wall w, int reach, int energyCosts[NUM_COLOURS], int maxEnergy) {
    struct path noPath = {0, NULL};

    int height = WallHeight(w);
    struct rock all[MAX_ROCKS];
    int n = WallGetAllRocks(w, all);

    // State: (rock index, current energy)
    typedef struct { int rock; int energy; int prev; } State;
    State parent[MAX_ROCKS][1001];
    bool visited[MAX_ROCKS][1001] = {false};

    for (int i = 0; i < MAX_ROCKS; i++)
        for (int j = 0; j <= maxEnergy; j++)
            parent[i][j].prev = -1;

    // Queue item: rock index, energy, steps
    typedef struct { int rock; int energy; int steps; } QItem;
    QItem queue[2000000];
    int front = 0, rear = 0;

    // Start from bottom rocks
    for (int i = 0; i < n; i++) {
        if (all[i].row == 1) {
            int cost = energyCosts[all[i].colour];
            if (cost <= maxEnergy) {
                int e = maxEnergy - cost;
                queue[rear++] = (QItem){i, e, 1};
                visited[i][e] = true;
                parent[i][e] = (State){i, maxEnergy, -1};
            }
        }
    }

    QItem goal = {-1, -1, INT_MAX};

    while (front < rear) {
        QItem curr = queue[front++];

        if (curr.steps >= goal.steps) continue;

        struct rock r = all[curr.rock];
        if (r.row + reach >= height) {
            if (curr.steps < goal.steps) goal = curr;
            continue;
        }

        // Rest: restore full energy
        if (curr.energy < maxEnergy) {
            int newE = maxEnergy;
            if (!visited[curr.rock][newE]) {
                visited[curr.rock][newE] = true;
                parent[curr.rock][newE] = (State){curr.rock, curr.energy, curr.steps};
                queue[rear++] = (QItem){curr.rock, newE, curr.steps + 1};
            }
        }

        // Move
        struct rock next[MAX_ROCKS];
        int m = WallGetRocksInRange(w, r.row, r.col, reach, next);

        for (int j = 0; j < m; j++) {
            if (next[j].row <= r.row) continue;
            for (int k = 0; k < n; k++) {
                if (rockEq(next[j], all[k])) {
                    int cost = energyCosts[all[k].colour];
                    if (curr.energy >= cost) {
                        int newE = curr.energy - cost;
                        if (!visited[k][newE]) {
                            visited[k][newE] = true;
                            parent[k][newE] = (State){curr.rock, curr.energy, curr.steps};
                            queue[rear++] = (QItem){k, newE, curr.steps + 1};
                        }
                    }
                    break;
                }
            }
        }
    }

    if (goal.rock == -1) return noPath;

    // Reconstruct path with rest actions
    struct rock path[20000];
    int len = 0;
    int rock = goal.rock;
    int energy = goal.energy;

    while (rock != -1) {
        path[len++] = all[rock];

        State p = parent[rock][energy];
        if (p.prev != -1 && p.rock == rock && p.energy == maxEnergy) {
            // This was a rest
            struct rock rest = all[rock];
            rest.row = rest.col = -1;  // marker (will be printed as "Rest")
            path[len++] = rest;
        }
        rock = p.rock;
        energy = p.energy;
    }

    struct path result;
    result.numRocks = len;
    result.rocks = malloc(len * sizeof(struct rock));
    for (int i = 0; i < len; i++) {
        result.rocks[i] = path[len - 1 - i];
    }
    return result;
}
