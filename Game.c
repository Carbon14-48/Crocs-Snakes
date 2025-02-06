#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
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
    struct Node *up, *down, *left, *right;
} Node;

typedef struct InventoryItem {
    char name[20];
    struct InventoryItem *next;
} InventoryItem;

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
    graph[10][15]->type = SNAKE;
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
            if (graph[i][j] == player->position)
                printf(BOLD GREEN "P " RESET); // Joueur en gras et vert
            else if (graph[i][j]->type == CROCODILE)
                printf(RED "C " RESET); // Crocodile en rouge
            else if (graph[i][j]->type == SNAKE)
                printf(YELLOW "S " RESET); // Serpent en jaune
            else if (graph[i][j]->type == THORNS)
                printf("# ");
            else if (graph[i][j]->type == FOOD)
                printf(GREEN "F " RESET); // Nourriture en vert
            else if (graph[i][j]->type == GUN)
                printf(BLUE "G " RESET); // Arme en bleu
            else if (graph[i][j]->type == PIERCING_POINT)
                printf(MAGENTA "! " RESET); // Point perçant en magenta
            else if (graph[i][j]->type == BULLET)
                printf(CYAN "* " RESET); // Balle en cyan
            else if (graph[i][j]->type == HEALTH_PACK)
                printf(WHITE "H " RESET); // Pack de santé en blanc
            else
                printf(". ");
        }
        printf("\n");
    }
    printf("%s\n", player->message); // Afficher le message
}


void shootBullet(Player *player, Node *graph[ROWS][COLS], char direction) {
    if (!player->hasGun) {
        strcpy(player->message, "❌ Vous n'avez pas d'arme !");
        return;
    }

    Node *bulletPos = player->position;
    int dx = 0, dy = 0;

    // Définir la direction du tir
    if (direction == 'z') dx = -1; // Haut
    else if (direction == 's') dx = 1;  // Bas
    else if (direction == 'q') dy = -1; // Gauche
    else if (direction == 'd') dy = 1;  // Droite

    // Tirer jusqu'à heurter un obstacle
    while (1) {
        int newX = bulletPos->x + dx;
        int newY = bulletPos->y + dy;

        if (newX < 0 || newX >= ROWS || newY < 0 || newY >= COLS) break; // Sortie du terrain
        if (graph[newX][newY]->type != SAFE_LAND) {
            if (graph[newX][newY]->type == CROCODILE || graph[newX][newY]->type == SNAKE) {
                strcpy(player->message, "🎯 L'ennemi a été touché !");
                graph[newX][newY]->type = SAFE_LAND;
            } else {
                strcpy(player->message, "💥 La balle a heurté un obstacle !");
            }
            break;
        }

        // Faire avancer la balle
        bulletPos = graph[newX][newY];
        bulletPos->type = BULLET;
        displayGraph(graph, player);
        usleep(100000);
        bulletPos->type = SAFE_LAND;
    }
}
void addInventoryItem(Player *player, const char *itemName) {
    InventoryItem *newItem = (InventoryItem*)malloc(sizeof(InventoryItem));
    strcpy(newItem->name, itemName);
    newItem->next = player->inventory;
    player->inventory = newItem;
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
    InventoryItem *current = player->inventory;
    InventoryItem *previous = NULL;
    while (current) {
        if (strcmp(current->name, "Health Pack") == 0) {
            player->health += 50;
            strcpy(player->message, "💊 Vous avez utilisé un pack de santé !");
            
            // Remove health pack from inventory
            if (previous) {
                previous->next = current->next;
            } else {
                player->inventory = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
    strcpy(player->message, "Vous n'avez pas de pack de santé !");
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
    } else if (newPos->type == CROCODILE || newPos->type == SNAKE) {
        strcpy(player->message, "🦊 Vous avez rencontré un ennemi !");
        player->health -= 20;
    } else {
        player->position = newPos;
        pushSafePosition(player); // Store the new safe position
        if (newPos->type == GUN) {
            strcpy(player->message, "🔫 Vous avez trouvé une arme !");
            player->hasGun = 1;
            addInventoryItem(player, "Gun");
            newPos->type = SAFE_LAND;
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
void gameLoop(Node* graph[ROWS][COLS], Player *player) {
    char input;
    while (1) {
        displayGraph(graph, player);
        printf("PV: %d\n", player->health);
        printf("[z] Haut, [s] Bas, [q] Gauche, [d] Droite, [f] Tirer, [i] Inventaire, [u] Utiliser pack de santé, [x] Quitter\n");
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
    }

    printf("\nFin du jeu. Merci d'avoir joué !\n");
}

int main() {
    Node* graph[ROWS][COLS];
    Player player;
    strcpy(player.name, "Héros");

    initGraph(graph, &player);
    
    gameLoop(graph, &player); // Appel de la boucle principale du jeu

    return 0;
}
