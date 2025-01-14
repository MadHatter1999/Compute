#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 31        // Maze size (must be odd for proper walls/paths)
#define MAX_ITER 1000       // Maximum iterations for the algorithm

// ANSI color codes
#define RESET "\033[0m"     // Reset color
#define RED "\033[41m  "    // Red background for walls
#define GREEN "\033[42m  "  // Green background for visited paths
#define GRAY "\033[40m  "   // Dark gray background for unvisited paths
#define BLUE "\033[44m  "   // Blue background for the goal
#define CURSOR_MOVE(y, x) printf("\033[%d;%dH", (y), (x))  // Move cursor to (y, x)

int grid[GRID_SIZE][GRID_SIZE];         // Maze grid (0 for walls, 1 for paths)
int visited[GRID_SIZE][GRID_SIZE];      // Visited cells during pathfinding
int goal_x = GRID_SIZE - 2;             // X-coordinate of the goal
int goal_y = GRID_SIZE - 2;             // Y-coordinate of the goal

#ifdef _WIN32
#include <windows.h>  // For Sleep on Windows
#else
#include <unistd.h>   // For usleep on POSIX systems
#endif

// Function to clear the console and reset the cursor
void initialize_console() {
#ifdef _WIN32
    system("cls");  // Clear screen on Windows
#else
    printf("\033[2J");  // Clear screen on POSIX
#endif
    CURSOR_MOVE(0, 0);  // Reset cursor to top-left
}

// Generate the maze using a recursive backtracking algorithm
void generate_maze(int x, int y) {
    int directions[4][2] = {{0, 2}, {0, -2}, {2, 0}, {-2, 0}};
    
    // Randomize the directions for unpredictability
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int tempX = directions[i][0], tempY = directions[i][1];
        directions[i][0] = directions[r][0];
        directions[i][1] = directions[r][1];
        directions[r][0] = tempX;
        directions[r][1] = tempY;
    }

    // Carve paths in random directions
    for (int i = 0; i < 4; i++) {
        int nx = x + directions[i][0];
        int ny = y + directions[i][1];

        // Check bounds and ensure the cell is a wall
        if (nx > 0 && ny > 0 && nx < GRID_SIZE - 1 && ny < GRID_SIZE - 1 && grid[nx][ny] == 0) {
            grid[nx][ny] = 1;  // Path
            grid[x + directions[i][0] / 2][y + directions[i][1] / 2] = 1;  // Path between cells
            generate_maze(nx, ny);  // Recursive call
        }
    }
}

// Print a single cell of the maze at a specific position
void print_cell(int y, int x) {
    CURSOR_MOVE(y + 1, x * 2 + 1);  // Move to the correct position
    if (x == goal_x && y == goal_y) {
        printf(BLUE);  // Goal
    } else if (grid[y][x] == 0) {
        printf(RED);  // Wall
    } else if (visited[y][x]) {
        printf(GREEN);  // Visited path
    } else {
        printf(GRAY);  // Unvisited path
    }
    printf(RESET);
    fflush(stdout);  // Force immediate output
}

// Perform pathfinding (DFS) with smooth updates
int find_path(int x, int y) {
    // If we reach the goal, stop
    if (x == goal_x && y == goal_y) {
        visited[y][x] = 1;  // Mark goal as visited
        print_cell(y, x);   // Update the goal cell
        return 1;  // Found the goal
    }

    // Out of bounds or already visited or wall
    if (x <= 0 || y <= 0 || x >= GRID_SIZE - 1 || y >= GRID_SIZE - 1 || visited[y][x] || grid[y][x] == 0) {
        return 0;
    }

    visited[y][x] = 1;  // Mark the cell as visited
    print_cell(y, x);   // Update only the current cell

    #ifdef _WIN32
    Sleep(50);  // Pause for 50ms on Windows
    #else
    usleep(50000);  // Pause for 50ms on POSIX
    #endif

    // Explore neighbors
    if (find_path(x + 1, y)) return 1;  // Down
    if (find_path(x - 1, y)) return 1;  // Up
    if (find_path(x, y + 1)) return 1;  // Right
    if (find_path(x, y - 1)) return 1;  // Left

    // Backtrack if no path is found
    visited[y][x] = 0;
    print_cell(y, x);  // Revert cell
    return 0;
}

int main() {
    srand(time(NULL));

    // Create a grid full of walls
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;  // All cells start as walls
            visited[i][j] = 0;  // No cells visited
        }
    }

    // Generate the maze
    grid[1][1] = 1;  // Start point
    generate_maze(1, 1);
    grid[goal_y][goal_x] = 1;  // Ensure the goal is a valid path

    // Initialize the console and print the maze
    initialize_console();
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            print_cell(i, j);
        }
    }
    printf("\nPress Enter to start pathfinding...\n");
    getchar();  // Wait for user input

    // Perform pathfinding
    if (find_path(1, 1)) {
        printf("\nPathfinding complete! The goal was reached.\n");
    } else {
        printf("\nNo path to the goal exists.\n");
    }
    return 0;
}
