#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define GRID_SIZE 50          // Size of the simulation grid
#define GROWTH_PROBABILITY 50 // Probability of spreading per step
#define DECAY_TIME 15         // Lifespan of mycelium
#define NUTRIENT_DECAY_TIME 10 // Lifespan of nutrients
#define REGENERATION_RATE 5   // Probability (percent) of nutrients regenerating
#define DISTURBANCE_RATE 15    // Probability (percent) of disturbances
#define MAX_ITERATIONS 10000   // Maximum simulation steps

// ANSI color codes for visualization
#define RESET "\033[0m"
#define GREEN "\033[42m  "  // Mycelium
#define YELLOW "\033[43m  " // Spore
#define DARK_GRAY "\033[40m  " // Empty space
#define CYAN "\033[46m  "   // Nutrients
#define RED "\033[41m  "    // Decayed cells
#define CURSOR_MOVE(y, x) printf("\033[%d;%dH", (y), (x))

typedef struct {
    int state;  // 0 = empty, 1 = spore, 2 = mycelium, 3 = decayed, 4 = nutrient
    int age;    // Age of the cell
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE];          // Current simulation grid
Cell previous_grid[GRID_SIZE][GRID_SIZE]; // Previous state for change tracking

// Initialize the console for visual output
void initialize_console() {
    printf("\033[2J");  // Clear the screen
    CURSOR_MOVE(0, 0);  // Move cursor to the top-left
}

// Print a single cell if it has changed
void print_cell(int x, int y) {
    if (grid[y][x].state == previous_grid[y][x].state) {
        return;  // No change, no need to redraw
    }

    CURSOR_MOVE(y + 1, x * 2 + 1);  // Move to the correct cell position
    if (grid[y][x].state == 0) {
        printf(DARK_GRAY);
    } else if (grid[y][x].state == 1) {
        printf(YELLOW);
    } else if (grid[y][x].state == 2) {
        printf(GREEN);
    } else if (grid[y][x].state == 3) {
        printf(RED);
    } else if (grid[y][x].state == 4) {
        printf(CYAN);
    }
    printf(RESET);
    fflush(stdout);
}

void update_grid_realistic() {
    Cell temp_grid[GRID_SIZE][GRID_SIZE];

    // Copy the current grid to a temporary grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            temp_grid[i][j] = grid[i][j];
        }
    }

    for (int i = 1; i < GRID_SIZE - 1; i++) {
        for (int j = 1; j < GRID_SIZE - 1; j++) {
            Cell current = grid[i][j];

            switch (current.state) {
                case 1: {  // Spore
                    // Look for nutrients nearby to germinate
                    int nutrient_found = 0;
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            if (grid[i + dx][j + dy].state == 4) {
                                nutrient_found = 1;
                                break;
                            }
                        }
                        if (nutrient_found) break;
                    }

                    if (nutrient_found || rand() % 100 < GROWTH_PROBABILITY) {
                        temp_grid[i][j].state = 2;  // Germinate into mycelium
                        temp_grid[i][j].age = 0;
                    }
                    break;
                }
                case 2: {  // Mycelium
                    temp_grid[i][j].age++;

                    // Spread organically to nearby empty cells
                    int directions[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
                    for (int d = 0; d < 4; d++) {
                        int nx = i + directions[d][0];
                        int ny = j + directions[d][1];
                        if (grid[nx][ny].state == 0 && rand() % 100 < GROWTH_PROBABILITY) {
                            temp_grid[nx][ny].state = 1;  // New spore
                        }
                    }

                    // Decay if lifespan exceeded or no nutrients nearby
                    int nutrient_nearby = 0;
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            if (grid[i + dx][j + dy].state == 4) {
                                nutrient_nearby = 1;
                                break;
                            }
                        }
                        if (nutrient_nearby) break;
                    }

                    if (!nutrient_nearby || temp_grid[i][j].age > DECAY_TIME) {
                        temp_grid[i][j].state = 3;  // Transition to decayed
                        temp_grid[i][j].age = 0;
                    }
                    break;
                }
                case 3: {  // Decayed
                    temp_grid[i][j].age++;
                    if (temp_grid[i][j].age > NUTRIENT_DECAY_TIME) {
                        temp_grid[i][j].state = 4;  // Convert to nutrients
                        temp_grid[i][j].age = 0;
                    }
                    break;
                }
                case 4: {  // Nutrients
                    if (rand() % 100 < REGENERATION_RATE) {
                        temp_grid[i][j].state = 4;  // Chance to stay nutrient
                    } else {
                        temp_grid[i][j].state = 0;  // Decay to empty
                    }
                    break;
                }
            }
        }
    }

    // Add random disturbances or spores
    if (rand() % 100 < DISTURBANCE_RATE) {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        temp_grid[y][x].state = 0;  // Clear random cell
    }

    if (rand() % 100 < 5) {  // Add random spores
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        if (temp_grid[y][x].state == 0) {
            temp_grid[y][x].state = 1;  // New spore
        }
    }

    // Update grid and redraw only changed cells
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (temp_grid[i][j].state != grid[i][j].state) {
                grid[i][j] = temp_grid[i][j];
                print_cell(j, i);  // Redraw changed cell
            }
            previous_grid[i][j] = grid[i][j];
        }
    }
}



int main() {
    srand(time(NULL));

    // Initialize the grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j].state = 0;  // All cells start empty
            grid[i][j].age = 0;
            previous_grid[i][j].state = 0;  // Match the initial state
            previous_grid[i][j].age = 0;
        }
    }

    // Place initial spores
    grid[GRID_SIZE / 2][GRID_SIZE / 2].state = 1;  // Center spore

    // Run the simulation
    initialize_console();

    for (int step = 0; step < MAX_ITERATIONS; step++) {
        update_grid_realistic();
        usleep(100000);  // Pause for visualization
    }

    printf("\nSimulation complete!\n");
    return 0;
}
