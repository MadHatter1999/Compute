#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h> // For multithreading

#define GRID_SIZE 200            // Size of the simulation grid
#define REGION_SIZE 5            // Size of each region
#define NUM_REGIONS (GRID_SIZE / REGION_SIZE)
#define FIRE_THRESHOLD 3         // Minimum input to trigger firing
#define REFRACTORY_PERIOD 5      // Steps a neuron remains inactive after firing
#define RANDOM_FIRE_CHANCE 2     // Chance for spontaneous firing (percent)
#define MAX_ITERATIONS 500       // Maximum simulation steps
#define NUM_THREADS 4            // Number of threads for parallel processing

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

int active_regions[NUM_REGIONS][NUM_REGIONS]; // Tracks active regions
pthread_mutex_t region_mutexes[NUM_REGIONS][NUM_REGIONS]; // Mutexes for each region

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
        int region_x = x / REGION_SIZE;
        int region_y = y / REGION_SIZE;
        active_regions[region_y][region_x] = 1; // Mark the region as active
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
                int region_x = nx / REGION_SIZE;
                int region_y = ny / REGION_SIZE;
                active_regions[region_y][region_x] = 1; // Mark the region as active
            }
        }
    }
}

// Update a single region
void update_region(int region_x, int region_y) {
    int x_start = region_x * REGION_SIZE;
    int y_start = region_y * REGION_SIZE;
    int x_end = x_start + REGION_SIZE;
    int y_end = y_start + REGION_SIZE;

    pthread_mutex_lock(&region_mutexes[region_y][region_x]);

    int active = 0; // Tracks if the region remains active
    for (int i = y_start; i < y_end; i++) {
        for (int j = x_start; j < x_end; j++) {
            Neuron *current = &grid[i][j];
            if (current->state == STATE_FIRING) {
                // Neuron fires and enters refractory state
                current->state = STATE_REFRACTORY;
                current->refractory_counter = REFRACTORY_PERIOD;
                spread_activation(j, i);
                active = 1;
            } else if (current->state == STATE_REFRACTORY) {
                // Decrease refractory counter
                current->refractory_counter--;
                if (current->refractory_counter <= 0) {
                    current->state = STATE_INACTIVE; // Return to inactive
                }
            } else if (current->state == STATE_INACTIVE) {
                // Check if neuron should fire
                if (current->activation_energy >= FIRE_THRESHOLD || rand() % 100 < RANDOM_FIRE_CHANCE) {
                    current->state = STATE_FIRING;
                    active = 1;
                }
                current->activation_energy = 0; // Reset activation energy
            }
        }
    }

    active_regions[region_y][region_x] = active; // Update activity status
    pthread_mutex_unlock(&region_mutexes[region_y][region_x]);
}

// Thread function to update regions
void *update_regions_thread(void *arg) {
    int thread_id = *(int *)arg;

    for (int i = thread_id; i < NUM_REGIONS; i += NUM_THREADS) {
        for (int j = 0; j < NUM_REGIONS; j++) {
            if (active_regions[i][j]) { // Only update active regions
                update_region(j, i);
            }
        }
    }
    return NULL;
}

// Display the grid
void display_grid() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j].state != previous_grid[i][j].state) {
                CURSOR_MOVE(i + 1, j * 2 + 1);
                if (grid[i][j].state == STATE_FIRING) {
                    printf(COLOR_FIRING);
                } else if (grid[i][j].state == STATE_REFRACTORY) {
                    printf(COLOR_REFRACTORY);
                } else {
                    printf(COLOR_INACTIVE);
                }
                previous_grid[i][j] = grid[i][j]; // Update the previous state
            }
        }
    }
    fflush(stdout);
}

int main() {
    srand(time(NULL));

    // Initialize the grid and console
    initialize_console();
    initialize_grid();

    // Initialize mutexes
    for (int i = 0; i < NUM_REGIONS; i++) {
        for (int j = 0; j < NUM_REGIONS; j++) {
            pthread_mutex_init(&region_mutexes[i][j], NULL);
        }
    }

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Run the simulation
    for (int step = 0; step < MAX_ITERATIONS; step++) {
        // Create threads to update regions
        for (int t = 0; t < NUM_THREADS; t++) {
            thread_ids[t] = t;
            pthread_create(&threads[t], NULL, update_regions_thread, &thread_ids[t]);
        }

        // Wait for all threads to complete
        for (int t = 0; t < NUM_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }

        display_grid(); // Display the grid
        usleep(100000); // Pause for visualization
    }

    printf("\nSimulation complete.\n");
    return 0;
}
