// climber.c
// COMP2521 25T3 - Assignment 2
// Implementation of boulder climbing algorithms

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "climber.h"
#include "Wall.h"

#define MAX_ROCKS 10000

struct path {
    int numRocks;
    struct rock *rocks;
};

// Helper: check if two rocks are the same
static bool sameRock(struct rock a, struct rock b) {
    return a.row == b.row && a.col == b.col;
}

// Helper: reconstruct path from parent array
static struct path reconstructPath(struct rock parent[], int parentCount[], int size, struct rock end) {
    struct path p = {0, NULL};
    struct rock *path = malloc(MAX_ROCKS * sizeof(struct rock));
    int count = 0;

    struct rock current = end;
    while (true) {
        path[count++] = current;
        if (parentCount[current.row * 1000 + current.col] == -1) break;
        current = parent[current.row * 1000 + current.col];
    }

    // Reverse path (from bottom to top)
    p.numRocks = count;
    p.rocks = malloc(count * sizeof(struct rock));
    for (int i = 0; i < count; i++) {
        p.rocks[i] = path[count - 1 - i];
    }

    free(path);
    return p;
}

struct path findShortestPath(Wall w, int reach, Colour colour) {
    int height = WallHeight(w);
    int width = WallWidth(w);

    struct rock allRocks[MAX_ROCKS];
    int totalRocks = WallGetAllRocks(w, allRocks);

    // BFS state
    bool visited[MAX_ROCKS] = {false};
    struct rock parent[MAX_ROCKS];
    int queue[MAX_ROCKS];
    int front = 0, back = 0;

    // Add all bottom rocks (row == 1)
    for (int i = 0; i < totalRocks; i++) {
        if (allRocks[i].row == 1 && allRocks[i].colour == colour) {
            queue[back++] = i;
            visited[i] = true;
            parent[i] = allRocks[i];  // self
        }
    }

    struct rock target = {0};
    bool found = false;
    int targetIdx = -1;

    while (front < back) {
        int currIdx = queue[front++];
        struct rock curr = allRocks[currIdx];

        // Check if we've reached the top
        if (curr.row + reach >= height) {
            target = curr;
            targetIdx = currIdx;
            found = true;
            break;
        }

        // Get reachable rocks of correct colour
        struct rock reachable[MAX_ROCKS];
        int n = WallGetColouredRocksInRange(w, curr.row, curr.col, reach, colour, reachable);

        for (int j = 0; j < n; j++) {
            if (reachable[j].row <= curr.row) continue; // only climb up

            // Find index in allRocks
            for (int k = 0; k < totalRocks; k++) {
                if (sameRock(reachable[j], allRocks[k]) && !visited[k]) {
                    visited[k] = true;
                    parent[k] = curr;
                    queue[back++] = k;
                }
            }
        }
    }

    struct path noPath = {0, NULL};
    if (!found) return noPath;

    // Reconstruct path manually since we don't have parent array indexed by rock
    struct path p = {0, NULL};
    struct rock *path = malloc(MAX_ROCKS * sizeof(struct rock));
    int count = 0;

    struct rock current = target;
    while (true) {
        path[count++] = current;
        if (current.row == 1) break;

        bool foundPrev = false;
        for (int i = 0; i < totalRocks; i++) {
            if (allRocks[i].colour != colour || allRocks[i].row >= current.row) continue;
            if (distance(allRocks[i].row, allRocks[i].col, current.row, current.col) <= reach + 1e-9) {
                if (!foundPrev || allRocks[i].row > path[count-1].row) {
                    path[count-1] = allRocks[i];  // better previous
                    foundPrev = true;
                }
            }
        }
        if (!foundPrev) break;
        current = path[count-1];
        count--;
    }

    // Fix path order
    p.numRocks = count;
    p.rocks = malloc(count * sizeof(struct rock));
    for (int i = 0; i < count; i++) {
        p.rocks[i] = path[count - 1 - i];
    }
    free(path);

    return p;
}

struct path findMinEnergyPath(Wall w, int reach, int energyCosts[NUM_COLOURS]) {
    int height = WallHeight(w);
    struct rock allRocks[MAX_ROCKS];
    int n = WallGetAllRocks(w, allRocks);

    // Dijkstra
    int dist[MAX_ROCKS];
    struct rock prev[MAX_ROCKS];
    bool visited[MAX_ROCKS] = {false};

    for (int i = 0; i < n; i++) dist[i] = INT_MAX;
    
    // Start from bottom rocks
    for (int i = 0; i < n; i++) {
        if (allRocks[i].row == 1) {
            dist[i] = energyCosts[allRocks[i].colour];
            prev[i] = allRocks[i];
        }
    }

    while (true) {
        int u = -1;
        int minDist = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visited[u] = true;

        struct rock curr = allRocks[u];

        if (curr.row + reach >= height) {
            // Found target
            struct path p = {0, NULL};
            struct rock *path = malloc(MAX_ROCKS * sizeof(struct rock));
            int count = 0;
            int idx = u;
            while (true) {
                path[count++] = allRocks[idx];
                if (allRocks[idx].row == 1 && prev[idx].row == allRocks[idx].row) break;
                // Find previous
                for (int j = 0; j < n; j++) {
                    if (sameRock(prev[idx], allRocks[j])) {
                        idx = j;
                        break;
                    }
                }
            }
            p.numRocks = count;
            p.rocks = malloc(count * sizeof(struct rock));
            for (int i = 0; i < count; i++) {
                p.rocks[i] = path[count - 1 - i];
            }
            free(path);
            return p;
        }

        struct rock candidates[MAX_ROCKS];
        int m = WallGetRocksInRange(w, curr.row, curr.col, reach, candidates);

        for (int j = 0; j < m; j++) {
            if (candidates[j].row <= curr.row) continue;
            for (int k = 0; k < n; k++) {
                if (sameRock(candidates[j], allRocks[k])) {
                    int alt = dist[u] + energyCosts[allRocks[k].colour];
                    if (alt < dist[k]) {
                        dist[k] = alt;
                        prev[k] = curr;
                    }
                    break;
                }
            }
        }
    }

    struct path noPath = {0, NULL};
    return noPath;
}

struct path findMinTurnsPath(Wall w, int reach, int energyCosts[NUM_COLOURS], int maxEnergy) {
    // This is a BFS where state is (position, current_energy)
    // Goal: minimize number of moves (turns), while respecting energy + rest

    int height = WallHeight(w);
    struct rock allRocks[MAX_ROCKS];
    int n = WallGetAllRocks(w, allRocks);

    struct state {
        int rockIdx;
        int energy;
        int steps;
    };

    bool visited[MAX_ROCKS][1001] = {false};  // energy up to 1000 is safe
    struct state parent[MAX_ROCKS][1001];
    for (int i = 0; i < MAX_ROCKS; i++)
        for (int j = 0; j <= maxEnergy; j++)
            parent[i][j].rockIdx = -1;

    struct queueNode {
        int rockIdx;
        int energy;
        int steps;
    } queue[1000000];
    int front = 0, rear = 0;

    // Start from bottom
    for (int i = 0; i < n; i++) {
        if (allRocks[i].row == 1) {
            int cost = energyCosts[allRocks[i].colour];
            if (cost <= maxEnergy) {
                queue[rear++] = (struct queueNode){i, maxEnergy - cost, 1};
                visited[i][maxEnergy - cost] = true;
                parent[i][maxEnergy - cost] = (struct state){i, maxEnergy, 0};
            }
        }
    }

    struct queueNode best = {-1, 0, INT_MAX};

    while (front < rear) {
        struct queueNode curr = queue[front++];

        if (curr.steps >= best.steps) continue;

        struct rock r = allRocks[curr.rockIdx];
        if (r.row + reach >= height) {
            if (curr.steps < best.steps) {
                best = curr;
            }
            continue;
        }

        // Option 1: Rest
        if (curr.energy < maxEnergy) {
            int newEnergy = maxEnergy;
            if (!visited[curr.rockIdx][newEnergy]) {
                visited[curr.rockIdx][newEnergy] = true;
                parent[curr.rockIdx][newEnergy] = (struct state){curr.rockIdx, curr.energy, curr.steps};
                queue[rear++] = (struct queueNode){curr.rockIdx, newEnergy, curr.steps + 1};
            }
        }

        // Option 2: Move to reachable rocks
        struct rock next[MAX_ROCKS];
        int m = WallGetRocksInRange(w, r.row, r.col, reach, next);

        for (int j = 0; j < m; j++) {
            if (next[j].row <= r.row) continue;
            for (int k = 0; k < n; k++) {
                if (sameRock(next[j], allRocks[k])) {
                    int cost = energyCosts[allRocks[k].colour];
                    if (curr.energy >= cost) {
                        int newEnergy = curr.energy - cost;
                        if (!visited[k][newEnergy]) {
                            visited[k][newEnergy] = true;
                            parent[k][newEnergy] = (struct state){curr.rockIdx, curr.energy, curr.steps};
                            queue[rear++] = (struct queueNode){k, newEnergy, curr.steps + 1};
                        }
                    }
                    break;
                }
            }
        }
    }

    if (best.rockIdx == -1) {
        struct path noPath = {0, NULL};
        return noPath;
    }

    // Reconstruct path
    struct path p = {0, NULL};
    struct rock *path = malloc(1000 * sizeof(struct rock));
    int count = 0;

    int currIdx = best.rockIdx;
    int currEnergy = best.energy;
    while (currIdx != -1) {
        path[count++] = allRocks[currIdx];

        struct state prev = parent[currIdx][currEnergy];
        if (prev.rockIdx == currIdx) {
            // This was a rest
            struct rock rest = allRocks[currIdx];
            rest.row = rest.col = -1;  // marker
            path[count++] = rest;
        }
        currIdx = prev.rockIdx;
        currEnergy = prev.energy;
    }

    p.numRocks = count;
    p.rocks = malloc(count * sizeof(struct rock));
    for (int i = 0; i < count; i++) {
        p.rocks[i] = path[count - 1 - i];
    }
    free(path);
    return p;
}