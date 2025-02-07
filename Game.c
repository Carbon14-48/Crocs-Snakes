#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #include <mmsystem.h>
    #define CLEAR "cls"
    void sleep(unsigned milliseconds) {
        Sleep(milliseconds);
    }
#else
    #include <unistd.h>
    #include <termios.h>
    #define CLEAR "clear"
    int _getch(void) {
        struct termios oldattr, newattr;
        int ch;
        tcgetattr(STDIN_FILENO, &oldattr);
        newattr = oldattr;
        newattr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
        return ch;
    }
#endif 
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
#define ROWS 20
#define COLS 30

#define ROWS 20
#define COLS 30

typedef enum { 
    SAFE_LAND,
    THORNS,
    CROCODILE,
    SNAKE,
    FOOD,
    GUN,
    PIERCING_POINT,
    BULLET,
    HEALTH_PACK
} CellType;

typedef struct Node {
    int x, y;
    CellType type;
    int lastShootTime;
    int health;
    struct Node *up, *down, *left, *right;
} Node;

typedef struct InventoryItem {
    char name[20];
    int quantity;
    struct InventoryItem *next;
} InventoryItem;
typedef struct QueueNode {
    Node *position;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front, *rear;
} Queue;

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
Queue crocodileQueue;

void initCrocodileMovement(Node* graph[ROWS][COLS]) {

    initQueue(&crocodileQueue);

    // More dynamic movement pattern
    enqueue(&crocodileQueue, graph[4][8]);
    enqueue(&crocodileQueue, graph[4][9]);
    enqueue(&crocodileQueue, graph[5][9]);
    enqueue(&crocodileQueue, graph[5][8]);
    enqueue(&crocodileQueue, graph[6][8]);
    enqueue(&crocodileQueue, graph[6][7]);
}
void moveCrocodile(Node* graph[ROWS][COLS]) {
    static Node *currentCrocodilePos = NULL; // Keep track of last position

    // Get the next position from the queue
    Node *newPos = dequeue(&crocodileQueue);
    if (newPos == NULL) return;

    // Clear old position if it exists
    if (currentCrocodilePos != NULL) {
        currentCrocodilePos->type = SAFE_LAND;
    }

    // Move to the new position
    newPos->type = CROCODILE;
    currentCrocodilePos = newPos; // Update last known position

    // Re-enqueue to maintain loop movement
    enqueue(&crocodileQueue, newPos);
}

// Stack implementation to store last safe positions
typedef struct StackNode {
    Node* position;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
} Stack;

typedef struct {
    char name[20];
    Node *position;
    int health;
    int score;
    InventoryItem *inventory;
    int hasGun;
    Stack lastSafePositions;
    char message[100]; // Message to display
} Player;

// Stack functions
void stackInit(Stack *stack) {
    stack->top = NULL;
}

void stackPush(Stack *stack, Node *position) {
    StackNode *newNode = (StackNode*)malloc(sizeof(StackNode));
    newNode->position = position;
    newNode->next = stack->top;
    stack->top = newNode;
}

Node* stackPop(Stack *stack) {
    if (stack->top == NULL) return NULL;
    StackNode *topNode = stack->top;
    Node *position = topNode->position;
    stack->top = stack->top->next;
    free(topNode);
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

void pushSafePosition(Player *player) {
    if (player->position->type == SAFE_LAND) {
        stackPush(&player->lastSafePositions, player->position);
    }
}

Node* popSafePosition(Player *player) {
    return stackPop(&player->lastSafePositions);
}

void initGraph(Node* graph[ROWS][COLS], Player *player) {
    srand(time(0));

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            graph[i][j] = createNode(i, j, (rand() % 10 < 2) ? THORNS : SAFE_LAND);
        }
    }

    graph[4][8]->type = CROCODILE;
    graph[4][8]->health = 2;
    graph[10][15]->type = SNAKE;
    graph[10][15]->health = 3;
    graph[8][11]->type = THORNS;
    graph[12][12]->type = FOOD;
    graph[5][5]->type = GUN;
    graph[7][11]->type = PIERCING_POINT;
    graph[3][7]->type = HEALTH_PACK;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i > 0) graph[i][j]->up = graph[i-1][j];
            if (i < ROWS-1) graph[i][j]->down = graph[i+1][j];
            if (j > 0) graph[i][j]->left = graph[i][j-1];
            if (j < COLS-1) graph[i][j]->right = graph[i][j+1];
        }
    }
    player->score=0;
    player->position = graph[1][1];
    player->health = 100;
    player->inventory = NULL;
    player->hasGun = 0;
    stackInit(&player->lastSafePositions); // Initialize the stack
    pushSafePosition(player); // Push the initial safe position
    strcpy(player->message, ""); // Initialize the message
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
            case PIERCING_POINT:
                printf(MAGENTA "! " RESET); // Point perçant en magenta
                break;
            case BULLET:
                printf(CYAN "* " RESET); // Balle en cyan
                break;
            case HEALTH_PACK:
                printf(WHITE "H " RESET); // Pack de santé en blanc
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
void snakeShoot(Node* graph[ROWS][COLS], Player *player, Node* snake) {
    int directions[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}}; // up, down, left, right
    int playerDx = player->position->x - snake->x;
    int playerDy = player->position->y - snake->y;
    int chosenDir = 0;

    // Choose direction closest to player
    if (abs(playerDx) > abs(playerDy)) {
        chosenDir = playerDx > 0 ? 1 : 0;  // down : up
    } else {
        chosenDir = playerDy > 0 ? 3 : 2;  // right : left
    }

    Node *bulletPos = snake;
    int dx = directions[chosenDir][0];
    int dy = directions[chosenDir][1];

    while (1) {
        int newX = bulletPos->x + dx;
        int newY = bulletPos->y + dy;

        if (newX < 0 || newX >= ROWS || newY < 0 || newY >= COLS) break;

        if (graph[newX][newY] == player->position) {
            player->health -= 15;
            strcpy(player->message, "🤕 Touché par le serpent !");
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


void shootBullet(Player *player, Node *graph[ROWS][COLS], char direction) {
    InventoryItem *current = player->inventory;
    while (current) {
        if (strcmp(current->name, "Gun") == 0) {
            if (current->quantity <= 0) {
                strcpy(player->message, "❌ Pas de munitions !");
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
            if (graph[newX][newY]->type == CROCODILE || graph[newX][newY]->type == SNAKE) {
                graph[newX][newY]->health--;
                if (graph[newX][newY]->health <= 0) {
                    if (graph[newX][newY]->type == CROCODILE) {
                        strcpy(player->message, "🎯 Crocodile tué ! +100 points !");
                        player->score += 100;
                    } else {
                        strcpy(player->message, "🎯 Serpent tué ! +75 points !");  // Increased points for harder kill
                        player->score += 75;
                    }
                    graph[newX][newY]->type = SAFE_LAND;
                } else {
                    if (graph[newX][newY]->type == CROCODILE) {
                        strcpy(player->message, "🎯 Crocodile touché ! Encore un coup !");
                    } else {
                        sprintf(player->message, "🎯 Serpent touché ! Encore %d coups !", graph[newX][newY]->health);
                    }
                }
            } else {
                strcpy(player->message, "💥 La balle a heurté un obstacle !");
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
        strcpy(player->message, "💊 Vous avez utilisé un pack de santé !");
    } else {
        strcpy(player->message, "Vous n'avez pas de pack de santé !");
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
    if (newPos->type == THORNS) {
    strcpy(player->message, "🚧 Vous ne pouvez pas vous déplacer ici !");
    player->health -= 10;
#ifdef _WIN32
    Beep(800, 300); // Joue un son (Windows uniquement)
#endif


    } else if (newPos->type == CROCODILE || newPos->type == SNAKE) {
        strcpy(player->message, "🦊 Vous avez rencontré un ennemi !");
        player->health -= 20;
    } else {
        player->position = newPos;
        pushSafePosition(player); // Store the new safe position
        if (newPos->type == GUN) {
        strcpy(player->message, "🔫 Vous avez trouvé des munitions !");
        addInventoryItem(player, "Bullets");
        newPos->type = SAFE_LAND;
        player->hasGun = 1;
    } else if (newPos->type == FOOD) {
            strcpy(player->message, "🍖 Vous avez trouvé de la nourriture !");
            player->health += 20;
            addInventoryItem(player, "Food");
            newPos->type = SAFE_LAND;
        } else if (newPos->type == HEALTH_PACK) {
            strcpy(player->message, "💊 Vous avez trouvé un pack de santé !");
            addInventoryItem(player, "Health Pack");
            newPos->type = SAFE_LAND;
        } else {
            strcpy(player->message, "");
        }
    }

    // Check for death
    if (player->health <= 0) {
        strcpy(player->message, "💀 Vous êtes mort !");
        player->position = popSafePosition(player); // Respawn at the last safe position
        player->health = 50; // Reset health
    }
}

void dangerWarning(Player *player, Node *graph[ROWS][COLS]) {
    int px = player->position->x;
    int py = player->position->y;
    for (int i = px - 5; i <= px + 5; i++) {
        for (int j = py - 5; j <= py + 5; j++) {
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                if (graph[i][j]->type == CROCODILE || graph[i][j]->type == SNAKE) {
                    strcpy(player->message, "⚠️ Danger détecté à proximité !");
                    return;
                }
            }
        }
    }
}
void handleSnakeShooting(Node* graph[ROWS][COLS], Player* player, int* cooldown) {
    if ((*cooldown)++ >= 1) {  // Adjust cooldown if needed
        Node* snake = NULL;
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (graph[i][j]->type == SNAKE) {
                    snake = graph[i][j];
                    break;
                }
            }
            if (snake) break;
        }
        if (snake) snakeShoot(graph, player, snake);
        *cooldown = 0;
    }
}
void gameLoop(Node* graph[ROWS][COLS], Player *player) {
    char input;
    while (1) {
        handleSnakeShooting(graph, player, &snakeShootCooldown);
        moveCrocodile(graph);
        displayGraph(graph, player);
        printf("PV: %d\n", player->health);
        printf("[z] Haut, [s] Bas, [q] Gauche, [d] Droite, [f] Tirer, [i] Inventaire, [u] Utiliser pack de sante, [x] Quitter\n");
        input = _getch();
        if (input == 'x') break;
        if (input == 'f') {
            printf("Direction du tir ? [z] Haut, [s] Bas, [q] Gauche, [d] Droite\n");
            char shootDirection = _getch();
            shootBullet(player, graph, shootDirection);
        }
        else if (input == 'i') displayInventory(player);
        else if (input == 'u') useHealthPack(player);
        else movePlayer(player, input, graph);
        
        dangerWarning(player, graph); // Check for danger
        usleep(200000);
    }

    printf("\nFin du jeu. Merci d'avoir joué !\n");
}

int main() {
    Node* graph[ROWS][COLS];
    Player player;
    strcpy(player.name, "Héros");

    initGraph(graph, &player);
    initCrocodileMovement(graph);
    
    gameLoop(graph, &player); // Appel de la boucle principale du jeu

    return 0;
}
