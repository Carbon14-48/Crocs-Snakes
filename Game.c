#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>
#define MAP_WIDTH 100  // Set this as a fixed large value for the width
// Function to get the terminal height
int getTerminalHeight() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;  // Height of the terminal screen
}
#define MAX_INVENTORY_SIZE 3
// Define terrain types
#define SAFE_LAND 'S'
#define RIVER 'R'
#define CROCODILE 'C'
#define VOID 'V'
#define APPLE 'A'
#define GUN 'G'
#define SWORD 'S'

// Risk levels
#define SAFE 0
#define LOW_RISK 1
#define MEDIUM_RISK 2
#define HIGH_RISK 3

// Node structure to represent each map tile
typedef struct Node {
    char terrain;    // Terrain (S, R, C, V, A, G, S)
    int x, y;        // Position in the grid
    int riskLevel;   // Risk (SAFE, LOW_RISK, MEDIUM_RISK, HIGH_RISK)
    int canWalk;     // 1 if the player can walk here, 0 otherwise
    int hasItem;     // 1 if an item (apple, gun, sword) is here, 0 if not

    struct Node *up, *down, *left, *right;  // Pointers to neighboring nodes
} Node;

// Function to create a new map node
Node* createNode(int x, int y, char terrain, int riskLevel, int canWalk) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->x = x;
    newNode->y = y;
    newNode->terrain = terrain;
    newNode->riskLevel = riskLevel;
    newNode->canWalk = canWalk;
    newNode->hasItem = 0;
    newNode->up = newNode->down = newNode->left = newNode->right = NULL;
    return newNode;
}

// Function to initialize the map with random terrains and items
void initializeMap(Node* map[getTerminalHeight()][MAP_WIDTH]) {
    // Seed the random number generator
    srand(time(NULL));

    for (int i = 0; i < getTerminalHeight(); i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            // Randomly choose terrain
            int terrainChoice = rand() % 7; // Random number between 0 and 6

            char terrain = SAFE_LAND;  // Default is Safe Land
            int riskLevel = SAFE;
            int canWalk = 1;           // Initially, assume player can walk here

            // Assign terrain based on random choice
            switch (terrainChoice) {
                case 0: // Safe Land
                    terrain = SAFE_LAND;
                    riskLevel = SAFE;
                    canWalk = 1;
                    break;
                case 1: // River
                    terrain = RIVER;
                    riskLevel = HIGH_RISK;
                    canWalk = 0;
                    break;
                case 2: // Crocodile
                    terrain = CROCODILE;
                    riskLevel = HIGH_RISK;
                    canWalk = 1;
                    break;
                case 3: // Void
                    terrain = VOID;
                    riskLevel = HIGH_RISK;
                    canWalk = 0;
                    break;
                case 4: // Apple
                    terrain = APPLE;
                    canWalk = 1;
                    break;
                case 5: // Gun
                    terrain = GUN;
                    canWalk = 1;
                    break;
                case 6: // Sword
                    terrain = SWORD;
                    canWalk = 1;
                    break;
            }

            // Create the node and place it in the map
            map[i][j] = createNode(i, j, terrain, riskLevel, canWalk);
        }
    }

    // Connect neighboring nodes (up, down, left, right)
    for (int i = 0; i < getTerminalHeight(); i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (i > 0) map[i][j]->up = map[i-1][j];         // Connect to the node above
            if (i < getTerminalHeight() - 1) map[i][j]->down = map[i+1][j]; // Connect to the node below
            if (j > 0) map[i][j]->left = map[i][j-1];       // Connect to the node on the left
            if (j < MAP_WIDTH - 1) map[i][j]->right = map[i][j+1]; // Connect to the node on the right
        }
    }
}

// Function to display the map (for debugging purposes)
void displayMap(Node* map[getTerminalHeight()][MAP_WIDTH]) {
    for (int i = 0; i < getTerminalHeight(); i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            printf("%c ", map[i][j]->terrain);
        }
        printf("\n");
    }
}
typedef struct item{
    char itemType;
    struct item *next;
}item;
typedef struct{
item *first;
item *last;
int size;
}inventory;
typedef struct Player {
    char name[10];      // player name
    int health;         // player life points
    bool alive;         // if the player is still alive
    int score;          // current player score
    int x, y;           //position in the game world
    inventory *inventory;    // player ressources
}
item* createItem(char itemType) {
    item *newItem = (Item *)malloc(sizeof(item));
    newItem->itemType = itemType;
    newItem->next = NULL;
    return newItem;
}

// Function to add an item to the inventory
void addItem(inventory *inventory, char itemType) {
    if( inventory ==NULL){
        return;
    }
    if (inventory->size > MAX_INVENTORY_SIZE) {
        printf("Inventory is full! Cannot add more items\n");
        return;
    }
      item *newItem = createItem(itemType);
     if (inventory->size == 0) {
       inventory->first = newItem;  
        inventory->last = newItem; 
        inventory->first->next = newItem;
    } else {
        inventory->last->next = newItem;  
        inventory->last = newItem;  
        newItem->next=inventory->first;
    }
}



int main() {
    // Create the map as a 2D array of Node pointers
    Node* map[getTerminalHeight()][MAP_WIDTH];

    // Initialize the map
    initializeMap(map);

    // Display the map to check the structure
    displayMap(map);

    return 0;
}
