#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define GRID_SIZE 50            // Size of the simulation grid
#define FIRE_THRESHOLD 3        // Minimum input to trigger firing
#define REFRACTORY_PERIOD 5     // Steps a neuron remains inactive after firing
#define RANDOM_FIRE_CHANCE 2    // Chance for spontaneous firing (percent)
#define MAX_ITERATIONS 500      // Maximum simulation steps

// Neuron states
#define STATE_INACTIVE 0
#define STATE_FIRING 1
#define STATE_REFRACTORY 2

// Console colors (ANSI escape codes)
#define COLOR_FIRING "\033[41m  \033[0m"    // Red block for firing neurons
#define COLOR_REFRACTORY "\033[44m  \033[0m" // Blue block for refractory neurons
#define COLOR_INACTIVE "\033[40m  \033[0m"  // Black block for inactive neurons
#define CURSOR_MOVE(y, x) printf("\033[%d;%dH", (y), (x))

typedef struct {
    int state;                   // 0 = inactive, 1 = firing, 2 = refractory
    int refractory_counter;      // Counter for refractory period
    int activation_energy;       // Activation level from neighbors
} Neuron;

Neuron grid[GRID_SIZE][GRID_SIZE];          // Current grid
Neuron previous_grid[GRID_SIZE][GRID_SIZE]; // Previous grid for change tracking

// Utility to clear the console and set the cursor to the top-left
void initialize_console() {
    printf("\033[2J"); // Clear screen
    printf("\033[H");  // Move cursor to the top-left
}

// Initialize the grid with neurons in random states
void initialize_grid() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j].state = STATE_INACTIVE; // Start inactive
            grid[i][j].refractory_counter = 0;
            grid[i][j].activation_energy = 0;
            previous_grid[i][j] = grid[i][j];  // Initialize previous grid
        }
    }

    // Seed random initial firing neurons
    for (int i = 0; i < GRID_SIZE / 5; i++) {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        grid[y][x].state = STATE_FIRING;
    }
}

// Update activation energy for neighbors of a firing neuron
void spread_activation(int x, int y) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue; // Skip the current neuron
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                grid[ny][nx].activation_energy++;
            }
        }
    }
}

// Update the grid state for the next step
void update_grid() {
    Neuron temp_grid[GRID_SIZE][GRID_SIZE];

    // Copy the current grid to a temporary grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            temp_grid[i][j] = grid[i][j];
        }
    }

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            Neuron *current = &grid[i][j];
            if (current->state == STATE_FIRING) {
                // Neuron fires and enters refractory state
                temp_grid[i][j].state = STATE_REFRACTORY;
                temp_grid[i][j].refractory_counter = REFRACTORY_PERIOD;

                // Spread activation to neighbors
                spread_activation(j, i);
            } else if (current->state == STATE_REFRACTORY) {
                // Decrease refractory counter
                temp_grid[i][j].refractory_counter--;
                if (temp_grid[i][j].refractory_counter <= 0) {
                    temp_grid[i][j].state = STATE_INACTIVE; // Return to inactive
                }
            } else if (current->state == STATE_INACTIVE) {
                // Check if neuron should fire
                if (current->activation_energy >= FIRE_THRESHOLD || rand() % 100 < RANDOM_FIRE_CHANCE) {
                    temp_grid[i][j].state = STATE_FIRING;
                }
                temp_grid[i][j].activation_energy = 0; // Reset activation energy
            }
        }
    }

    // Copy temp grid back to main grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = temp_grid[i][j];
        }
    }
}

// Display the grid, updating only changed neurons
void display_grid() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Update only if the state has changed
            if (grid[i][j].state != previous_grid[i][j].state) {
                CURSOR_MOVE(i + 1, j * 2 + 1); // Move to the correct position
                if (grid[i][j].state == STATE_FIRING) {
                    printf(COLOR_FIRING);
                } else if (grid[i][j].state == STATE_REFRACTORY) {
                    printf(COLOR_REFRACTORY);
                } else {
                    printf(COLOR_INACTIVE);
                }
            }
            // Update the previous grid to match the current state
            previous_grid[i][j] = grid[i][j];
        }
    }
    fflush(stdout); // Ensure all output is displayed
}

int main() {
    srand(time(NULL));

    // Initialize the grid and console
    initialize_console();
    initialize_grid();

    // Run the simulation
    for (int step = 0; step < MAX_ITERATIONS; step++) {
        display_grid();       // Update the display
        update_grid();        // Update the simulation state
        usleep(100000);       // Pause for visualization (adjust for speed)
    }

    printf("\nSimulation complete.\n");
    return 0;
}
