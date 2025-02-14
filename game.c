#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

// Platform-specific includes
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #include <mmsystem.h>
    #define CLEAR "cls"
    #define msleep(x) Sleep(x)
#else
    #include <termios.h>
    #include <unistd.h>
    #include <stdio.h>
    #define CLEAR "clear"
    #define msleep(x) usleep((x) * 1000)

    // Linux implementation of _getch
    int _getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif


// Constants
#define ROWS 20
#define COLS 20
#define MAX_CROCODILES 4
#define MAX_SNAKES 2
#define SMALL_MAP 0
#define BIG_MAP 1
#define ENEMY_EASY 0
#define ENEMY_HARD 1
#define POWER_WEAK 0
#define POWER_STRONG 1
#define MAX_NAME_LENGTH 20
#define MAX_MESSAGE_LENGTH 10

// ANSI Color codes
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define BOLD    "\x1b[1m"
#define UNDERLINE "\x1b[4m"

// Type definitions
typedef enum {
    SAFE_LAND,
    THORNS,
    WALL,
    CROCODILE,
    SNAKE,
    FOOD,
    GUN,
    BULLET,
    AXE,
    HEALTH_PACK,
    PORTAL,
    BOSS,
    CHECKPOINT
} CellType;

// Forward declarations of structures
typedef struct Node Node;
typedef struct InventoryItem InventoryItem;
typedef struct QueueNode QueueNode;
typedef struct Queue Queue;
typedef struct StackNode StackNode;
typedef struct Stack Stack;
typedef struct Player Player;
typedef struct GameConfig GameConfig;
typedef struct DifficultyNode DifficultyNode;
typedef struct Snake Snake;
typedef struct Crocodile Crocodile;

// Structure definitions
struct Node {
    int x, y;
    CellType type;
    int lastShootTime;
    int health;
    Node *up, *down, *left, *right;
};

struct InventoryItem {
    char name[20];
    int quantity;
    InventoryItem *next;
};

struct QueueNode {
    Node *position;
    QueueNode *next;
};

struct Queue {
    QueueNode *front, *rear;
};

typedef struct CheckpointNode {
    Node* position;
    struct CheckpointNode* next;
} CheckpointNode;

typedef struct CheckpointStack {
    CheckpointNode* top;
    int size;  // Keep track of size for the 3-checkpoint limit
} CheckpointStack;

struct Player {
    char name[20];
    Node *position;
    int health;
    int score;
    InventoryItem *inventory;
    int hasGun;
    char message[100];
    int readyForBoss;
    CheckpointStack checkpoints;  // New field for checkpoint stack
};
typedef struct Boss {
    Node* position;
    int health;
    int attackCooldown;
    int phaseNumber;  // For different attack patterns
    int isActive;
    int moveCooldown;
} Boss;

struct GameConfig {
    int mapSize;
    int enemyCount;
    int enemyPower;
    const char* mapData;
    int snakeHealth;
    int crocodileHealth;
    int snakeDamage;
    int crocodileDamage;
    int snakeCount;
    int crocodileCount;
};

struct DifficultyNode {
    char* prompt;
    DifficultyNode* left;
    DifficultyNode* right;
    int level;
};

struct Snake {
    Node* position;
    int health;
    int shootCooldown;
};

struct Crocodile {
    Node* position;
    Queue movementQueue;
};
typedef struct HighScoreNode {
    char name[MAX_NAME_LENGTH];
    int score;
    struct HighScoreNode *next;
} HighScoreNode;

HighScoreNode *head = NULL;
// Function prototypes
// Node management
Node* createNode(int x, int y, CellType type);
void initGraphFromMap(Node* graph[ROWS][COLS], Player *player, const char* map);

// Queue operations
void initQueue(Queue *queue);
void enqueue(Queue *queue, Node *position);
Node* dequeue(Queue *queue);

// Stack operations
void handleCheckpoint(Player* player, Node* currentNode);
void pushCheckpoint(CheckpointStack* stack, Node* position);
Node* popCheckpoint(CheckpointStack* stack);
void clearCheckpointStack(CheckpointStack* stack);

// Inventory management
void addInventoryItem(Player *player, const char *itemName);
int removeInventoryItem(Player *player, const char *itemName);
void displayInventory(Player *player);
void useHealthPack(Player *player);

// Player actions
void movePlayer(Player *player, char direction, Node *graph[ROWS][COLS]);
void shootBullet(Player *player, Node *graph[ROWS][COLS], char direction);
void breakThorns(Player *player, Node *graph[ROWS][COLS], char direction);

// Enemy management
void initCrocodiles(void);
void setupCrocodiles(Node* graph[ROWS][COLS], GameConfig* config);
void moveAllCrocodiles(Node* graph[ROWS][COLS], Player* player);
void checkCrocodileAttack(Node* crocodileNode, Player* player);
void cleanupCrocodiles(void);
void initializeBoss(Node* graph[ROWS][COLS], Player* player, const char* bossMap);
void moveBoss(Node* graph[ROWS][COLS], Player* player);
void bossAttackPattern(Node* graph[ROWS][COLS], Player* player);





void initSnakes(void);
void setupSnakes(Node* graph[ROWS][COLS], GameConfig* config);
void snakeShoot(Node* graph[ROWS][COLS], Player* player, Snake* snake);
void handleAllSnakesShooting(Node* graph[ROWS][COLS], Player* player);
void cleanupSnakes(void);

// Game setup and control
DifficultyNode* createDifficultyNode(char* prompt, int level);
DifficultyNode* buildDifficultyTree(void);
GameConfig* getDifficultyChoices(DifficultyNode* root);
void freeDifficultyTree(DifficultyNode* root);
void initializeGame(Node* graph[ROWS][COLS], Player* player, GameConfig* config);
void gameLoop(Node* graph[ROWS][COLS], Player *player);
void displayGraph(Node* graph[ROWS][COLS], Player *player);
void dangerWarning(Player *player, Node *graph[ROWS][COLS]);

// Global variables
GameConfig* config;
Crocodile crocodiles[MAX_CROCODILES];
Snake snakes[MAX_SNAKES];
int activeCrocodiles;
int activeSnakes;

//HIGH Score
void addHighScore(const char *name, int score);
void displayHighScores();



Boss boss;

void initQueue(Queue *queue) {
    queue->front = queue->rear = NULL;
}

void enqueue(Queue *queue, Node *position) {
    QueueNode *newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->position = position;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
        return;
    }

    queue->rear->next = newNode;
    queue->rear = newNode;
}

Node* dequeue(Queue *queue) {
    if (queue->front == NULL) return NULL;

    QueueNode *temp = queue->front;
    Node *position = temp->position;
    queue->front = queue->front->next;

    if (queue->front == NULL) queue->rear = NULL;

    free(temp);
    return position;
}



Node* createNode(int x, int y, CellType type) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->x = x;
    newNode->y = y;
    newNode->type = type;
    newNode->up = newNode->down = newNode->left = newNode->right = NULL;
    return newNode;
}

void initCheckpointStack(CheckpointStack* stack) {
    stack->top = NULL;
    stack->size = 0;
}

// Push checkpoint to stack
void pushCheckpoint(CheckpointStack* stack, Node* position) {
    // Create new checkpoint node
    CheckpointNode* newNode = (CheckpointNode*)malloc(sizeof(CheckpointNode));
    newNode->position = position;
    newNode->next = stack->top;

    if (stack->size >= 3) {
        CheckpointNode* current = stack->top;

        // Find second-to-last node
        while (current->next->next != NULL) {
            current = current->next;
        }

        // Free last node and set new last node's next to NULL
        free(current->next);
        current->next = NULL;
    }else {
    stack->size++;
    }


    // Add new checkpoint at top
    stack->top = newNode;
}

// Pop checkpoint from stack
Node* popCheckpoint(CheckpointStack* stack) {
    if (stack->top == NULL) {
        return NULL;
    }

    CheckpointNode* temp = stack->top;
    Node* position = temp->position;
    stack->top = temp->next;
    free(temp);
    stack->size--;

    return position;
}

// Function to clear checkpoint stack (for cleanup)
void clearCheckpointStack(CheckpointStack* stack) {
    while (stack->top != NULL) {
        CheckpointNode* temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    stack->size = 0;
}

// Function to handle checkpoint discovery
void handleCheckpoint(Player* player, Node* currentNode) {
    pushCheckpoint(&player->checkpoints, currentNode);
    currentNode->type = SAFE_LAND;  // Replace checkpoint with safe land
    strcpy(player->message, " Checkpoint sauvegarde! ");
}

int returnToLastCheckpoint(Player* player) {
    Node* lastCheckpoint = popCheckpoint(&player->checkpoints);
    if (lastCheckpoint != NULL) {
        player->position = lastCheckpoint;
        strcpy(player->message, " Retour au dernier checkpoint! ");
        return 1; // Checkpoint available
    } else {
        strcpy(player->message, " Aucun checkpoint disponible! ");
        return 0; // No checkpoint available
    }
}


// Function to create a new difficulty node
DifficultyNode* createDifficultyNode(char* prompt, int level) {
    DifficultyNode* node = (DifficultyNode*)malloc(sizeof(DifficultyNode));
    node->prompt = prompt;
    node->left = NULL;
    node->right = NULL;
    node->level = level;
    return node;
}

// Function to build the difficulty selection tree
DifficultyNode* buildDifficultyTree() {
    // Level 0: Map Size
    DifficultyNode* root = createDifficultyNode("Choose map size:\n1. Small Map\n2. Big Map", 0);

    // Level 1: Enemy Count
    root->left = createDifficultyNode("Choose enemy count:\n1. Few Enemies\n2. Many Enemies", 1);
    root->right = createDifficultyNode("Choose enemy count:\n1. Few Enemies\n2. Many Enemies", 1);

    // Level 2: Enemy Power
    root->left->left = createDifficultyNode("Choose enemy power:\n1. Weak\n2. Strong", 2);
    root->left->right = createDifficultyNode("Choose enemy power:\n1. Weak\n2. Strong", 2);
    root->right->left = createDifficultyNode("Choose enemy power:\n1. Weak\n2. Strong", 2);
    root->right->right = createDifficultyNode("Choose enemy power:\n1. Weak\n2. Strong", 2);

    return root;
}

// Function to traverse the tree and get user choices
GameConfig* getDifficultyChoices(DifficultyNode* root) {
    config = (GameConfig*)malloc(sizeof(GameConfig));
    DifficultyNode* current = root;
    int choice;

    // Initialize maps
    const char* smallMap =
        "+++++++++++++++\n"
        "+P C  G      A+\n"
        "+++++++++++#+++\n"
        "+A            +\n"
        "+           H +\n"
        "++#++++++++++++\n"
        "+ A    #    C +\n"
        "+      #      +\n"
        "+      #   A  +\n"
        "+      #      +\n"
        "+F  A  #      +\n"
        "+++++++++++#+++\n"
        "+ C         ##+\n"
        "+  F   A   # O+\n"
        "+++++++++++++++";

    const char* bigMap =
        "++++++++++++++++++++\n"
        "+P  C A     G    A +\n"
        "+++++++++++++#++++++\n"
        "+  A       #    H  +\n"
        "+           #####  +\n"
        "++##++++++++++++++++\n"
        "+     A         C  +\n"
        "+           F      +\n"
        "+  A        #####  +\n"
        "+H    #    A       +\n"
        "+#+++#++++#+++ +++++\n"
        "+  C    A          +\n"
        "+ A         ###    +\n"
        "++++++#+++#+++++++++\n"
        "+                A +\n"
        "+  ##       F      +\n"
        "+      A          +\n"
        "+   H         #####+\n"
        "+         #### # O +\n"
        "++++++++++++++++++++";


    // Level 0: Map Size
    printf("%s\n", current->prompt);
    scanf("%d", &choice);
    config->mapSize = (choice == 2) ? BIG_MAP : SMALL_MAP;
    config->mapData = (config->mapSize == BIG_MAP) ? bigMap : smallMap;
    current = (choice == 2) ? current->right : current->left;

    // Level 1: Enemy Count
    printf("%s\n", current->prompt);
    scanf("%d", &choice);
    config->enemyCount = (choice == 2) ? ENEMY_HARD : ENEMY_EASY;
    current = (choice == 2) ? current->right : current->left;

    // Level 2: Enemy Power
    printf("%s\n", current->prompt);
    scanf("%d", &choice);
    config->enemyPower = (choice == 2) ? POWER_STRONG : POWER_WEAK;

    // Configure game parameters based on choices
    if (config->enemyCount == ENEMY_EASY) {
        config->snakeCount = 1;
        config->crocodileCount = 2;
    } else {
        config->snakeCount = 2;
        config->crocodileCount = 4;
    }

    if (config->enemyPower == POWER_WEAK) {
        config->snakeHealth = 2;
        config->crocodileHealth = 2;
        config->snakeDamage = 15;
        config->crocodileDamage = 10;
    } else {
        config->snakeHealth = 3;
        config->crocodileHealth = 3;
        config->snakeDamage = 20;
        config->crocodileDamage = 15;
    }

    return config;
}

// Function to free the difficulty tree
void freeDifficultyTree(DifficultyNode* root) {
    if (root == NULL) return;
    freeDifficultyTree(root->left);
    freeDifficultyTree(root->right);
    free(root);
}


void initCrocodiles() {
    for (int i = 0; i < MAX_CROCODILES; i++) {
        crocodiles[i].position = NULL;
        initQueue(&crocodiles[i].movementQueue);
    }
}
void setupCrocodiles(Node* graph[ROWS][COLS], GameConfig* config) {

    activeCrocodiles = config->crocodileCount;

    // Initialize all crocodiles
    initCrocodiles();

    // Define spawn points
    struct SpawnPoint {
        int x, y;
    };
    struct SpawnPoint spawnPoints[4];

if (config->mapSize == 1) {
    struct SpawnPoint temp[] = {
        {3, 4},   // First crocodile
        {11, 4},  // Second crocodile
        {7, 17}, // Third crocodile (hard mode only)
        {15, 8}  // Fourth crocodile (hard mode only)
    };
    // Copy values into spawnPoints
    for (int i = 0; i < 4; i++) {
        spawnPoints[i] = temp[i];
    }
} else {
    struct SpawnPoint temp[] = {
        {3, 3},   // First crocodile
        {12, 3}, // Second crocodile
        {3, 7},   // Third crocodile (hard mode only)
        {12, 8}  // Fourth crocodile (hard mode only)
    };
    // Copy values into spawnPoints
    for (int i = 0; i < 4; i++) {
        spawnPoints[i] = temp[i];
    }
}

    // Spawn crocodiles at predetermined points
    for(int i = 0; i < activeCrocodiles; i++) {
        if (graph[spawnPoints[i].x][spawnPoints[i].y]->type == SAFE_LAND) {
            graph[spawnPoints[i].x][spawnPoints[i].y]->type = CROCODILE;
            graph[spawnPoints[i].x][spawnPoints[i].y]->health = config->crocodileHealth;

            crocodiles[i].position = graph[spawnPoints[i].x][spawnPoints[i].y];

            // Setup movement pattern
            enqueue(&crocodiles[i].movementQueue, crocodiles[i].position);
            if (crocodiles[i].position->right)
                enqueue(&crocodiles[i].movementQueue, crocodiles[i].position->right);
            if (crocodiles[i].position->right && crocodiles[i].position->right->down)
                enqueue(&crocodiles[i].movementQueue, crocodiles[i].position->right->down);
            if (crocodiles[i].position->down)
                enqueue(&crocodiles[i].movementQueue, crocodiles[i].position->down);
        }
    }
}


void moveAllCrocodiles(Node* graph[ROWS][COLS], Player* player) {
    for (int i = 0; i < activeCrocodiles; i++) {
        // Skip if crocodile is dead
        if (!crocodiles[i].position || crocodiles[i].position->type != CROCODILE) {
            continue;
        }

        Node* newPos = dequeue(&crocodiles[i].movementQueue);
        if (newPos == NULL) {
            // Reset queue if empty
            initQueue(&crocodiles[i].movementQueue);
            enqueue(&crocodiles[i].movementQueue, crocodiles[i].position);
            continue;
        }

        // Move crocodile to new position
        if (newPos->type == SAFE_LAND) {
            crocodiles[i].position->type = SAFE_LAND;
            newPos->type = CROCODILE;
            newPos->health = crocodiles[i].position->health;
            crocodiles[i].position = newPos;

            // Re-add position to queue for continuous movement
            enqueue(&crocodiles[i].movementQueue, newPos);

            // Check for player attack
            checkCrocodileAttack(newPos, player);
        }
    }
}

void checkCrocodileAttack(Node* crocodileNode, Player* player) {
    if (!crocodileNode || crocodileNode->type != CROCODILE) return;

    // Calculate distance to player
    int dx = abs(crocodileNode->x - player->position->x);
    int dy = abs(crocodileNode->y - player->position->y);

    // Attack if player is adjacent
    if (dx <= 1 && dy <= 1) {
        int damage = config->crocodileDamage;
        player->health -= damage;
        strcpy(player->message, " Un crocodile vous a attaque !");

        if (player->health <= 0) {
            strcpy(player->message, " Vous etes mort a cause d'un crocodile !");
            returnToLastCheckpoint(player);
            player->health = 50;
        }
    }
}
void cleanupCrocodiles() {
    for (int i = 0; i < MAX_CROCODILES; i++) {
        while (crocodiles[i].movementQueue.front != NULL) {
            dequeue(&crocodiles[i].movementQueue);
        }
    }
}



void initGraphFromMap(Node* graph[ROWS][COLS], Player *player, const char* map) {
    int row = 0, col = 0;

    // Initialize the graph with safe land
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            graph[i][j] = createNode(i, j, SAFE_LAND);
        }
    }

    // Parse the map string
    for (int i = 0; map[i] != '\0'; i++) {
        if (map[i] == '\n') {
            row++;
            col = 0;
            continue;
        }

        // Set the type of the current cell based on the map character
        switch (map[i]) {
            case '+': graph[row][col]->type = WALL; break;
            case '#': graph[row][col]->type = THORNS; break;
            case 'P':
                graph[row][col]->type = SAFE_LAND;
                player->position = graph[row][col]; // Set player position
                break;
            case 'G': graph[row][col]->type = GUN; break;
            case 'A': graph[row][col]->type = AXE; break;
            case 'H': graph[row][col]->type = HEALTH_PACK; break;
            case 'F': graph[row][col]->type = FOOD; break;
            case 'O': graph[row][col]->type = PORTAL; break;
            case 'B': graph[row][col]->type = BOSS; break;
            case 'C': graph[row][col]->type = CHECKPOINT; break;
            default: graph[row][col]->type = SAFE_LAND; break;
        }
        col++;
    }


    // Link nodes to their neighbors
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i > 0) graph[i][j]->up = graph[i - 1][j];
            if (i < ROWS - 1) graph[i][j]->down = graph[i + 1][j];
            if (j > 0) graph[i][j]->left = graph[i][j - 1];
            if (j < COLS - 1) graph[i][j]->right = graph[i][j + 1];
        }
    }

    // Initialize player properties
    player->health = 100;
    player->score = 0;
    player->inventory = NULL;
    player->hasGun = 0;
    initCheckpointStack(&player->checkpoints);
    strcpy(player->message, "");
}
void displayGraph(Node* graph[ROWS][COLS], Player *player) {
    system(CLEAR);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
           if (graph[i][j] == player->position) {
            printf(BOLD GREEN "P " RESET); // Joueur en gras et vert

        }
        else{
        switch (graph[i][j]->type) {
            case CROCODILE:
                printf(RED "C " RESET); // Crocodile en rouge
                break;
            case SNAKE:
                printf(YELLOW "S " RESET); // Serpent en jaune
                break;
            case THORNS:
                printf("# ");
                break;
            case FOOD:
                printf(GREEN "F " RESET); // Nourriture en vert
                break;
            case GUN:
                printf(BLUE "G " RESET); // Arme en bleu
                break;
            case WALL:
                printf(BLUE "W " RESET); // Walls in blue
                break;
            case AXE:
                printf(MAGENTA "^ " RESET); // Axe in magenta
                break;
            case BULLET:
                printf(CYAN "* " RESET); // Balle en cyan
                break;
            case HEALTH_PACK:
                printf(WHITE "H " RESET); // Pack de santé en blanc
                break;
            case PORTAL:
                printf(BOLD CYAN "O " RESET); // Portal in cyan
                break;
            case BOSS:
                printf(BOLD RED "B " RESET);  // Boss in bold red
                break;
            case CHECKPOINT:
                printf(BOLD GREEN "C " RESET); // Checkpoint in bold green
                break;
            default:
                printf(". "); // Terrain normal
                break;
        }
    }
    }
    printf("\n");
}
    printf("Score: %d | PV: %d\n", player->score, player->health);
    printf("%s\n", player->message); // Afficher le message
}


void initSnakes() {
    for (int i = 0; i < MAX_SNAKES; i++) {
        snakes[i].position = NULL;
        snakes[i].health = 3;
        snakes[i].shootCooldown = 0;
    }
}

void setupSnakes(Node* graph[ROWS][COLS], GameConfig* config) {
    activeSnakes = config->snakeCount;

    // Initialize all snakes
    initSnakes();

    // Define spawn points
    struct SpawnPoint {
        int x, y;
    };
    struct SpawnPoint spawnPoints[2];

    if (config->mapSize == 1) {
    struct SpawnPoint temp[] = {
        {8, 4},  // First snake
        {15, 14}    // Second snake (hard mode only)
    };
    // Copy values into spawnPoints
    for (int i = 0; i < 2; i++) {
        spawnPoints[i] = temp[i];
    }
    } else {
    struct SpawnPoint temp[] = {
        {8, 4},  // First snake (example for different map size)
        {8, 10}    // Second snake (hard mode only)
    };
    // Copy values into spawnPoints
    for (int i = 0; i < 2; i++) {
        spawnPoints[i] = temp[i];
    }
}
    // Spawn snakes at predetermined points
    for(int i = 0; i < activeSnakes; i++) {
        if (graph[spawnPoints[i].x][spawnPoints[i].y]->type == SAFE_LAND) {
            graph[spawnPoints[i].x][spawnPoints[i].y]->type = SNAKE;
            graph[spawnPoints[i].x][spawnPoints[i].y]->health = config->snakeHealth;
            snakes[i].position = graph[spawnPoints[i].x][spawnPoints[i].y];
        }
    }
}

// Modified snake shooting function to work with multiple snakes
void snakeShoot(Node* graph[ROWS][COLS], Player* player, Snake* snake) {
    if (!snake->position || snake->position->type != SNAKE) return;

    int directions[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}}; // up, down, left, right
    int playerDx = player->position->x - snake->position->x;
    int playerDy = player->position->y - snake->position->y;
    int chosenDir = 0;

    // Choose direction closest to player
    if (abs(playerDx) > abs(playerDy)) {
        chosenDir = playerDx > 0 ? 1 : 0;  // down : up
    } else {
        chosenDir = playerDy > 0 ? 3 : 2;  // right : left
    }

    Node* bulletPos = snake->position;
    int dx = directions[chosenDir][0];
    int dy = directions[chosenDir][1];

    while (1) {
        int newX = bulletPos->x + dx;
        int newY = bulletPos->y + dy;

        if (newX < 0 || newX >= ROWS || newY < 0 || newY >= COLS) break;

        if (graph[newX][newY] == player->position) {
             int damage = config->snakeDamage;
            player->health -= damage;
            strcpy(player->message, " Touch par un serpent !");
            break;
        }

        if (graph[newX][newY]->type != SAFE_LAND) break;

        bulletPos = graph[newX][newY];
        bulletPos->type = BULLET;
        displayGraph(graph, player);
        usleep(50000);
        bulletPos->type = SAFE_LAND;
    }
}

void handleAllSnakesShooting(Node* graph[ROWS][COLS], Player* player) {
    for (int i = 0; i < activeSnakes; i++) {
        if (!snakes[i].position || snakes[i].position->type != SNAKE) continue;

        if (snakes[i].shootCooldown++ >= 3) {  // Shoot every 3 turns
            snakeShoot(graph, player, &snakes[i]);
            snakes[i].shootCooldown = 0;
        }
    }
}

void cleanupSnakes() {
    for (int i = 0; i < MAX_SNAKES; i++) {
        snakes[i].position = NULL;
    }
}
const char* bossMap =
        "+++++++++++++++\n"
        "+ P           +\n"
        "+          #  +\n"
        "+    ++       +\n"
        "+  ##     B   +\n"
        "+           H +\n"
        "+  ##         +\n"
        "+     F    ## +\n"
        "+++++++++++++++\n";


void initializeBoss(Node* graph[ROWS][COLS], Player* player, const char* bossMap) {
    // Reset the graph for boss arena
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (graph[i][j] != NULL) {
                free(graph[i][j]);
                graph[i][j] = NULL;
            }
        }
    }

    // Initialize new map
    initGraphFromMap(graph, player, bossMap);

    // Initialize boss

    boss.health = 100;
    boss.attackCooldown = 0;
    boss.moveCooldown = 0;
    boss.phaseNumber = 1;
    boss.isActive = 1;


    // Find boss starting position (marked as 'B' in the map)
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (graph[i][j]->type == BOSS) {
                boss.position = graph[i][j];
                break;
            }
        }
    }
    if (boss.position == NULL) {
        printf("Error: Boss position not found!\n");
        exit(1);
    }
}
void shootAtPlayer(Node* graph[ROWS][COLS], Player* player) {
    int dx = player->position->x - boss.position->x;
    int dy = player->position->y - boss.position->y;

    // Normalize direction
    int dirX = (dx != 0) ? dx / abs(dx) : 0;
    int dirY = (dy != 0) ? dy / abs(dy) : 0;

    Node* bulletPos = boss.position;
    while (1) {
        int newX = bulletPos->x + dirX;
        int newY = bulletPos->y + dirY;

        if (newX < 0 || newX >= ROWS || newY < 0 || newY >= COLS) break;

        if (graph[newX][newY] == player->position) {
            player->health -= 10;
            strcpy(player->message, " Le boss vous a touche avec son attaque a distance !");
            break;
        }

        if (graph[newX][newY]->type != SAFE_LAND) break;

        bulletPos = graph[newX][newY];
        bulletPos->type = BULLET;
        displayGraph(graph, player);
        usleep(50000);
        bulletPos->type = SAFE_LAND;
    }
}
void moveBoss(Node* graph[ROWS][COLS], Player* player) {
    if (!boss.isActive || boss.moveCooldown > 0) {
        boss.moveCooldown--;
        return;
    }

    int dx = player->position->x - boss.position->x;
    int dy = player->position->y - boss.position->y;

    // Determine the direction to move
    int dirX = (dx != 0) ? dx / abs(dx) : 0;
    int dirY = (dy != 0) ? dy / abs(dy) : 0;

    // Try to move in the preferred direction
    int newX = boss.position->x + dirX;
    int newY = boss.position->y + dirY;

    if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS) {
        if (graph[newX][newY]->type == SAFE_LAND) {
            // Move the boss
            boss.position->type = SAFE_LAND;  // Clear the old position
            boss.position = graph[newX][newY];
            boss.position->type = BOSS;  // Mark the new position
        }
    }

    // Reset the move cooldown
    boss.moveCooldown = 2;  // Adjust this value to control movement speed
}

void bossAttackPattern(Node* graph[ROWS][COLS], Player* player) {
    if (!boss.isActive || boss.attackCooldown > 0) {
        boss.attackCooldown--;
        return;
    }

    // Update phase based on health more frequently
    if (boss.health <= 30) boss.phaseNumber = 3;
    else if (boss.health <= 60) boss.phaseNumber = 2;
    else boss.phaseNumber = 1;

    switch (boss.phaseNumber) {
        case 1: // Direct attack - more aggressive
            if (abs(boss.position->x - player->position->x) < 3 &&
                abs(boss.position->y - player->position->y) < 3) {
                player->health -= 20;
                strcpy(player->message, " Le boss vous a attaque !");
            }
            boss.attackCooldown = 1; // Reduced cooldown
            break;

        case 2: // Ranged attack - more frequent
            shootAtPlayer(graph, player);
            boss.attackCooldown = 2;
            break;

        case 3: // Final phase - more deadly
            if (rand() % 2 == 0) {
                if (abs(boss.position->x - player->position->x) < 4 &&
                    abs(boss.position->y - player->position->y) < 4) {
                    player->health -= 25;
                    strcpy(player->message, " Le boss vous a porte un coup devastateur !");
                }
            } else {
                shootAtPlayer(graph, player);
            }
            boss.attackCooldown = 2;
            break;
    }
}


void initializeGame(Node* graph[ROWS][COLS], Player* player, GameConfig* config) {
    // Initialize map
    initGraphFromMap(graph, player, config->mapData);

    // Setup enemies with custom parameters
    for (int i = 0; i < config->crocodileCount; i++) {
        crocodiles[i].position = NULL;
        initQueue(&crocodiles[i].movementQueue);
    }

    for (int i = 0; i < config->snakeCount; i++) {
        snakes[i].position = NULL;
        snakes[i].health = config->snakeHealth;
        snakes[i].shootCooldown = 0;
    }

    // Update game parameters
    activeSnakes = config->snakeCount;
    activeCrocodiles = config->crocodileCount;

    // Call setup functions with the new configuration
    setupCrocodiles(graph, config);
    setupSnakes(graph, config);
}
void breakThorns(Player *player, Node *graph[ROWS][COLS], char direction) {
    Node *targetNode = NULL;

    // Determine target node based on direction
    if (direction == 'z' && player->position->up) targetNode = player->position->up;
    if (direction == 's' && player->position->down) targetNode = player->position->down;
    if (direction == 'q' && player->position->left) targetNode = player->position->left;
    if (direction == 'd' && player->position->right) targetNode = player->position->right;

    // Check if target node exists and is thorns
    if (!targetNode || targetNode->type != THORNS) {
        strcpy(player->message, " Aucunes epines a casser dans cette direction !");
        return;
    }

    // Check for axe in inventory
    InventoryItem *current = player->inventory;
    while (current) {
        if (strcmp(current->name, "Axe") == 0 && current->quantity > 0) {
            removeInventoryItem(player, "Axe");
            targetNode->type = SAFE_LAND;
            strcpy(player->message, " Epines cassees avec la hache !");
            return;
        }
        current = current->next;
    }

    strcpy(player->message, " Vous n'avez pas de hache !");
}


void shootBullet(Player *player, Node *graph[ROWS][COLS], char direction) {
    // Check for gun/ammo
    InventoryItem *current = player->inventory;
    while (current) {
        if (strcmp(current->name, "Gun") == 0) {
            if (current->quantity <= 0) {
                strcpy(player->message, " Pas de munitions !");
                return;
            }
            current->quantity--;
            break;
        }
        current = current->next;
    }

    Node *bulletPos = player->position;
    int dx = 0, dy = 0;

    if (direction == 'z') dx = -1;
    else if (direction == 's') dx = 1;
    else if (direction == 'q') dy = -1;
    else if (direction == 'd') dy = 1;

    while (1) {
        int newX = bulletPos->x + dx;
        int newY = bulletPos->y + dy;

        if (newX < 0 || newX >= ROWS || newY < 0 || newY >= COLS) break;
        if (graph[newX][newY]->type != SAFE_LAND) {
            // Handle hitting different types of enemies
            if (graph[newX][newY]->type == CROCODILE ||
                graph[newX][newY]->type == SNAKE ||
                graph[newX][newY] == boss.position) {  // Add boss check here

                graph[newX][newY]->health--;

                // Handle different enemy types
                if (graph[newX][newY] == boss.position) {
                    boss.health -= 10;
                    strcpy(player->message, " Vous avez touche le boss !");
                    if (boss.health <= 0) {
                        strcpy(player->message, " Le boss a ete vaincu !");
                        player->score += 500;
                    }
                }
                else if (graph[newX][newY]->health <= 0) {
                    if (graph[newX][newY]->type == CROCODILE) {
                        strcpy(player->message, " Crocodile tue ! +100 points !");
                        player->score += 100;
                    } else {
                        strcpy(player->message, " Serpent tue ! +75 points !");
                        player->score += 75;
                    }
                    graph[newX][newY]->type = SAFE_LAND;
                } else {
                    if (graph[newX][newY]->type == CROCODILE) {
                        strcpy(player->message, " Crocodile touche ! Encore un coup !");
                    } else {
                        sprintf(player->message, " Serpent touche ! Encore %d coups !",
                               graph[newX][newY]->health);
                    }
                }
            } else {
                strcpy(player->message, " La balle a heurté un obstacle !");
            }
            break;
        }

        bulletPos = graph[newX][newY];
        bulletPos->type = BULLET;
        displayGraph(graph, player);
        usleep(100000);
        bulletPos->type = SAFE_LAND;
    }
}
void addInventoryItem(Player *player, const char *itemName) {
    InventoryItem *current = player->inventory;

    // Check if item already exists
    while (current) {
        if (strcmp(current->name, itemName) == 0) {
            current->quantity++;
            return;
        }
        current = current->next;
    }

    // If item doesn't exist, create new item
    InventoryItem *newItem = (InventoryItem*)malloc(sizeof(InventoryItem));
    strcpy(newItem->name, itemName);
    newItem->quantity = 1;
    newItem->next = player->inventory;
    player->inventory = newItem;
}
int removeInventoryItem(Player *player, const char *itemName) {
    InventoryItem *current = player->inventory;
    InventoryItem *previous = NULL;

    while (current) {
        if (strcmp(current->name, itemName) == 0) {
            if (current->quantity > 1) {
                current->quantity--;
                return 1;
            } else {
                // Remove the item completely
                if (previous) {
                    previous->next = current->next;
                } else {
                    player->inventory = current->next;
                }
                free(current);
                return 1;
            }
        }
        previous = current;
        current = current->next;
    }
    return 0;
}

void displayInventory(Player *player) {
    printf("Inventaire:\n");
    InventoryItem *current = player->inventory;
    while (current) {
        printf("- %s\n", current->name);
        current = current->next;
    }
    printf("Appuyez sur une touche pour continuer...\n");
    _getch();
}

void useHealthPack(Player *player) {
    if (removeInventoryItem(player, "Health Pack")) {
        player->health += 50;
        strcpy(player->message, " Vous avez utilise un pack de sante !");
    } else {
        strcpy(player->message, "Vous n'avez pas de pack de sante !");
    }
}

void movePlayer(Player *player, char direction, Node *graph[ROWS][COLS]) {
    Node *newPos = player->position;
    if (direction == 'z' && player->position->up) newPos = player->position->up;
    if (direction == 's' && player->position->down) newPos = player->position->down;
    if (direction == 'q' && player->position->left) newPos = player->position->left;
    if (direction == 'd' && player->position->right) newPos = player->position->right;

    if (newPos == player->position) {
        return; // No movement
    }
    if (newPos->type == WALL) {
        strcpy(player->message, " Vous ne pouvez pas traverser les murs !");
        return;
    }

    if (newPos->type == THORNS) {
    strcpy(player->message, " Vous ne pouvez pas vous deplacer ici !");
    player->health -= 10;
#ifdef _WIN32
    Beep(800, 300); // Joue un son (Windows uniquement)
#endif


    } else if (newPos->type == CROCODILE || newPos->type == SNAKE) {
        strcpy(player->message, " Vous avez rencontre un ennemi !");
        player->health -= 20;
    } else {
        player->position = newPos;
        if (newPos->type == GUN) {
        strcpy(player->message, " Vous avez trouve des munitions !");
        addInventoryItem(player, "Bullets");
        newPos->type = SAFE_LAND;
        player->hasGun = 1;
    }
        else if (newPos->type == AXE) {
            strcpy(player->message, " Vous avez trouve une hache !");
            addInventoryItem(player, "Axe");
            newPos->type = SAFE_LAND;
    }   else if (newPos->type == FOOD) {
            strcpy(player->message, " Vous avez trouve de la nourriture !");
            player->health += 20;
            addInventoryItem(player, "Food");
            newPos->type = SAFE_LAND;
    }    else if (newPos->type == HEALTH_PACK) {
            strcpy(player->message, " Vous avez trouve un pack de sante !");
            addInventoryItem(player, "Health Pack");
            newPos->type = SAFE_LAND;
        }else if (newPos->type == CHECKPOINT) {
                handleCheckpoint(player, newPos);
        }else {
            strcpy(player->message, "");
        }
    }

    // Check for death
     if (player->health <= 0) {
        strcpy(player->message, " Vous etes mort !");
        if (!returnToLastCheckpoint(player)) {
            // No checkpoints left, end the game
            strcpy(player->message, " Vous n'avez plus de checkpoints ! Game Over !");
            player->health = 0; // Ensure health is 0 to trigger game over
            return;
        }
        player->health = 50; // Reset health after respawning
    }
}


void dangerWarning(Player *player, Node *graph[ROWS][COLS]) {
    int px = player->position->x;
    int py = player->position->y;
    for (int i = px - 5; i <= px + 5; i++) {
        for (int j = py - 5; j <= py + 5; j++) {
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                if (graph[i][j]->type == CROCODILE || graph[i][j]->type == SNAKE) {
                    strcpy(player->message, " Danger detecte a proximite !");
                    return;
                }
            }
        }
    }
}

void gameLoop(Node* graph[ROWS][COLS], Player *player) {
    char input;

    while (1) {
            if (player->health <= 0) {
            strcpy(player->message, " Game Over - Vous avez ete vaincu !");
            printf("\nAppuyez sur une touche pour quitter...\n");
            _getch();
            return;
            }

        if (boss.isActive && boss.health <= 0) {
            strcpy(player->message, " Felicitations ! Vous avez vaincu le boss !");
            player->score += 500;
            printf("\nAppuyez sur une touche pour quitter...\n");
            _getch();
            return;
        }
        if (boss.isActive) {
            bossAttackPattern(graph, player);
             moveBoss(graph, player);
        } else {
            handleAllSnakesShooting(graph, player);
            moveAllCrocodiles(graph, player);
        }
         displayGraph(graph, player);
          if (player->position->type == PORTAL) {
            system(CLEAR);
            printf("\n" BOLD CYAN "🌀 Vous avez atteint le portail!" RESET "\n");
            printf("Que souhaitez-vous faire?\n");
            printf("1. Entrer dans l'arene du boss\n");
            printf("2. Continuer d'explorer la carte actuelle\n");
            printf("\nVotre choix (1 ou 2): ");

            char choice = _getch();
            if (choice == '1') {
                player->readyForBoss = 1;
                return;
            }
        }



        if (boss.isActive) printf("PV: %d | Boss PV: %d\n", player->health,  boss.health);
        printf("[z] Haut, [s] Bas, [q] Gauche, [d] Droite, [f] Tirer, [i] Inventaire, [u] Utiliser pack de sante, [c] casser, [x] Quitter\n");

        input = _getch();
        switch (input) {
            case 'x':
                return;
            case 'f': {
                printf("Direction du tir ? [z] Haut, [s] Bas, [q] Gauche, [d] Droite\n");
                char shootDirection = _getch();
                shootBullet(player, graph, shootDirection);
                break;
            }
            case 'i':
                displayInventory(player);
                break;
            case 'u':
                useHealthPack(player);
                break;
            case 'c': {
                printf("Direction des epines ? [z] Haut, [s] Bas, [q] Gauche, [d] Droite\n");
                char breakDirection = _getch();
                breakThorns(player, graph, breakDirection);
                break;
            }
            case 'r':
            returnToLastCheckpoint(player);
            break;
            default:
                movePlayer(player, input, graph);
                break;
        }

        dangerWarning(player, graph);
        usleep(200000);
    }
}
void addHighScore(const char *name, int score) {
    HighScoreNode *newNode = (HighScoreNode*)malloc(sizeof(HighScoreNode));
    strcpy(newNode->name, name);
    newNode->score = score;
    newNode->next = NULL;

    if (head == NULL || head->score < score) {
        newNode->next = head;
        head = newNode;
    } else {
        HighScoreNode *current = head;
        while (current->next != NULL && current->next->score >= score) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

void displayHighScores() {
    system(CLEAR);
    printf("\n" BOLD " HIGH SCORES 🏆" RESET "\n\n");
    HighScoreNode *current = head;
    int rank = 1;
    while (current != NULL && rank <= 10) {  // Show only top 10 scores
        printf("%d. %s: %d\n", rank, current->name, current->score);
        current = current->next;
        rank++;
    }
    printf("\nAppuyez sur une touche pour continuer...\n");
    _getch();
}


int playAgain() {
    system(CLEAR);
    printf("\nVoulez-vous rejouer ? (o/n): ");
    char choice = _getch();
    return (choice == 'o' || choice == 'O');
}

int main() {
    char playerName[MAX_NAME_LENGTH];
    int continueGame = 1;

    while (continueGame) {
        system(CLEAR);
        printf("\n" BOLD " NOUVEAU JEU " RESET "\n\n");
        printf("Entrez votre nom (max %d caracteres): ", MAX_NAME_LENGTH - 1);
        scanf("%s", playerName);

        Node* graph[ROWS][COLS];
        Player player;
        strcpy(player.name, playerName);

        DifficultyNode* difficultyTree = buildDifficultyTree();
        GameConfig* gameConfig = getDifficultyChoices(difficultyTree);

        // Initialize game with selected configuration
        initializeGame(graph, &player, gameConfig);

        // Main game loop
        gameLoop(graph, &player);

        // Boss battle
        if (player.readyForBoss && player.health > 0) {
            system(CLEAR);
            printf("\n" BOLD YELLOW " Preparez-vous pour le combat final !" RESET "\n");
            printf("Appuyez sur une touche pour continuer...\n");
            _getch();

            initializeBoss(graph, &player, bossMap);
            gameLoop(graph, &player);
        }

        // Add score to high scores
        addHighScore(player.name, player.score);

        // Display final score and high scores
        system(CLEAR);
        printf("\n" BOLD " Partie terminée !" RESET "\n");
        printf("Score final: %d\n\n", player.score);
        printf("Appuyez sur une touche pour voir les meilleurs scores...\n");
        _getch();

        displayHighScores();

        // Cleanup
        cleanupCrocodiles();
        cleanupSnakes();
        freeDifficultyTree(difficultyTree);
        free(gameConfig);

        // Ask to play again
        continueGame = playAgain();
    }

    // Final cleanup of high scores list
    while (head != NULL) {
        HighScoreNode *temp = head;
        head = head->next;
        free(temp);
    }

    return 0;
}
