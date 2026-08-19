/* INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <math.h>
#include <time.h>

/* DEFINES */
/* Game Background Properties */
#define GAME_ROWS 22
#define GAME_COLS 120

/* Game Parameters */
#define PI_NUMBER 3.14159265358
#define GRAVITY 0.4

/* Tank Properties */
#define Y_TANKS 19
#define MIN_X_TANK1 0
#define MAX_X_TANK1 24
#define MIN_X_TANK2 85
#define MAX_X_TANK2 111
#define TANK_WIDTH 9
#define TANK_HEIGHT 3

/* Angle and Power lower and upper bounds */
#define MIN_ANGLE 0.0
#define MAX_ANGLE 180.0
#define MIN_POWER 1
#define MAX_POWER 100

#define HIT_DAMAGE 20

/* STATIC GAME STATES */
#define STATE_NEXT_TURN 0
#define STATE_RESET 1
#define STATE_QUIT 2
#define STATE_BACK 3

/* COLLISION RETURN VALUES */
#define COLLISION_NONE 0
#define COLLISION_TANK1 1
#define COLLISION_TANK2 2
#define COLLISION_TERRAIN 3

/* DELAYS DEFINES (IN ms) */
#define DELAY_DISPLAY_SHOT 750
#define DELAY_SHOW_RESULT 3000

/* TRAJECTORY DEFINES */
#define MAX_TRAJECTORY_LEN 1000

/* TYPE DEFS */
typedef struct
{
    int x;
    int y;
} POSITION;

/* Game State structure for storing game elements */
typedef struct
{
    POSITION tank1Pos;
    POSITION tank2Pos;
    POSITION bulletPos;

    int tank1Health;
    int tank2Health;

    int currentTurn;
} GameState;

/* DISPLAY FUNCTIONS */
void printGrid(char grid[GAME_ROWS][GAME_COLS + 1]);
void printStatusBar(int tank1Health, int tank2Health);

/* TANK FUNCTIONS */
bool updateTankPosition(int tankNumber, char direction, int distance, POSITION *pos);
void drawTanks(char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos);

/* CLEAR FUNCTIONS */
void clearTanksFromGrid(char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos);
void clearBulletFromGrid(char grid[GAME_ROWS][GAME_COLS + 1], POSITION bulletPos);
void clearDynamicElements(char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos, POSITION bulletPos);

/* COLLISION FUNCTIONS */
bool checkTerrainCollision(POSITION bulletPos, char grid[GAME_ROWS][GAME_COLS + 1]);
int checkTankCollision(POSITION tank1Pos, POSITION tank2Pos, POSITION bulletPos);
/* checkTankCollision RETURN VALUES:
** 0: COLLISION_NONE  - NO COLLISION WITH TANKS
** 1: COLLISION_TANK1 - COLLISION WITH TANK1
** 2: COLLISION_TANK2 - COLLISION WITH TANK2 */

/* BULLET FUNCTIONS */
void calculateShotParameters(double angle, int power, POSITION tank, int turn, double *vx, double *vy, POSITION *bulletStartPos);
void computeShotPath(POSITION startPos, double vx, double vy, char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos, POSITION *trajectory, int *trajectoryLen, int *collisionType);
void displayShot(POSITION *path, int trajectoryLen, char grid[GAME_ROWS][GAME_COLS + 1], int *lastDrawnBulletIndex);

/* CORE FUNCTION */
/* Handles the core game logic and updates the game state
** game Pointer to the GameState structure containing current game state information & grid is the game background
** handleGameLogic returns an integer indicating the result of game logic processing */
int processPlayerTurn(char grid[GAME_ROWS][GAME_COLS + 1], GameState *gameCurrent);

/* UTILITY FUNCTIONS */
int randomInRange(int a, int b);
void changeTurnNumber(int *turnNumber);
bool positionsAreEqual(POSITION p1, POSITION p2);

int main()
{
    char grid[GAME_ROWS][GAME_COLS + 1] =
        {
            "                  ^                                                                                                     ",
            "                 ^^^                                                                            ^                       ",
            "                ^^^^^                                                                          ^^^                      ",
            "                                                                                              ^^^^^                     ",
            "                                                                                                                        ",
            "                                                                                                                        ",
            "                      ^^                                                                                                ",
            "                     ^^^^                                                                                               ",
            "                    ^^^^^^                                                                                    ^         ",
            "                                                                                                             ^^^        ",
            "                                                                                                                        ",
            "                                                                                                                        ",
            "                                                          ^                                                             ",
            "                                                         ^^^         ^^                                                 ",
            "                                                        ^^^^^       ^^^^                                                ",
            "                                               ^       ^^^^^^^^    ^^^^^^                                               ",
            "                                              ^^^     ^^^^^^^^^^  ^^^^^^^^     ^                                        ",
            "                                   ^^        ^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^  ^^^                                       ",
            "                                  ^^^^     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^                                      ",
            "                                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^                                    ",
            "########################################################################################################################",
            "########################################################################################################################",
        };

    /* Initialize random seed with current time */
    srand(time(NULL));

    /* Define Game States */
    GameState gameCurrent;
    GameState gamePrev;
    GameState gamePrevPrev;

    /* Variables for checking that back feature is used or not */
    bool tank1BackAbilityUsed = false;
    bool tank2BackAbilityUsed = false;

    /* Initialize current game state */
    gameCurrent.currentTurn = 1;
    gameCurrent.tank1Health = 100;
    gameCurrent.tank2Health = 100;
    gameCurrent.tank1Pos = (POSITION){
        .x = randomInRange(MIN_X_TANK1, MAX_X_TANK1), .y = Y_TANKS};
    gameCurrent.tank2Pos = (POSITION){
        .x = randomInRange(MIN_X_TANK2, MAX_X_TANK2), .y = Y_TANKS};
    /* Initialize bullet position to -1, -1 to indicate no bullet is active */
    gameCurrent.bulletPos = (POSITION){
        .x = -1, .y = -1};

    /* At the begining of game, gamePrev and gamePrevPrev set to current */
    gamePrev = gameCurrent;
    gamePrevPrev = gameCurrent;

    /* Main Loop of the Program */
    while (1)
    {
        system("cls");
        printStatusBar(gameCurrent.tank1Health, gameCurrent.tank2Health);
        drawTanks(grid, gameCurrent.tank1Pos, gameCurrent.tank2Pos);
        printGrid(grid);

        if (gameCurrent.tank1Health <= 0)
        {
            printf("--------------------------------------------------------PLAYER 2 WINS-----------------------------------------------------\n");
            Sleep(DELAY_SHOW_RESULT);
            break;
        }
        else if (gameCurrent.tank2Health <= 0)
        {
            printf("--------------------------------------------------------PLAYER 1 WINS-----------------------------------------------------\n");
            Sleep(DELAY_SHOW_RESULT);
            break;
        }

        /* Process Player Turn */
        int gameState = processPlayerTurn(grid, &gameCurrent);

        if (gameState == STATE_QUIT)
        {
            printf("\nQuitting the program...\n");
            break;
        }
        else if (gameState == STATE_RESET)
        {
            /* Reset game state */
            gameCurrent.currentTurn = 1;
            gameCurrent.tank1Health = 100;
            gameCurrent.tank2Health = 100;
            clearDynamicElements(grid, gameCurrent.tank1Pos, gameCurrent.tank2Pos, gameCurrent.bulletPos);
            gameCurrent.tank1Pos.x = randomInRange(MIN_X_TANK1, MAX_X_TANK1);
            gameCurrent.tank2Pos.x = randomInRange(MIN_X_TANK2, MAX_X_TANK2);
            gamePrev = gameCurrent;
            gamePrevPrev = gameCurrent;
            tank1BackAbilityUsed = false;
            tank2BackAbilityUsed = false;
            continue;
        }
        else if (gameState == STATE_NEXT_TURN)
        {
            gamePrevPrev = gamePrev;
            gamePrev = gameCurrent;
            changeTurnNumber(&gameCurrent.currentTurn);
            continue;
        }
        else if (gameState == STATE_BACK)
        {
            int requestingPlayer = gameCurrent.currentTurn;

            if (requestingPlayer == 1 && tank1BackAbilityUsed)
            {
                printf("Ability Is Already Used - Your Turn Is Lost!\n");
                changeTurnNumber(&gameCurrent.currentTurn);
                Sleep(DELAY_SHOW_RESULT);
                continue;
            }
            if (requestingPlayer == 2 && tank2BackAbilityUsed)
            {
                printf("Ability Is Already Used - Your Turn Is Lost!\n");
                changeTurnNumber(&gameCurrent.currentTurn);
                Sleep(DELAY_SHOW_RESULT);
                continue;
            }

            if (requestingPlayer == 1)
            {
                tank1BackAbilityUsed = true;
            }
            else
            {
                tank2BackAbilityUsed = true;
            }

            clearTanksFromGrid(grid, gameCurrent.tank1Pos, gameCurrent.tank2Pos);
            gameCurrent = gamePrevPrev;
            continue;
        }

        gamePrevPrev = gamePrev;
        gamePrev = gameCurrent;
        /* STATE_CONTINUE means loop continues */
    }

    return 0;
}

void printGrid(char grid[GAME_ROWS][GAME_COLS + 1])
{
    printf("|------------------------------------------------------------------------------------------------------------------------|\n");
    for (int i = 0; i < GAME_ROWS; i++)
    {
        printf("|");
        for (int j = 0; j < GAME_COLS; j++)
        {
            printf("%c", grid[i][j]);
        }
        printf("|\n");
    }
    printf("|------------------------------------------------------------------------------------------------------------------------|\n");
}

void printStatusBar(int tank1Health, int tank2Health)
{
    printf("==========================================================================================================================\n");
    printf("|                         [P1] TANK ALPHA  | HEALTH: %3d%%   ||   [P2] TANK BETA  | HEALTH: %3d%%                          |\n", tank1Health, tank2Health);
    printf("==========================================================================================================================\n");
}

bool updateTankPosition(int tankNumber, char direction, int distance, POSITION *pos)
{
    int startX = pos->x;
    int finalX;
    if (direction == 'R')
        finalX = startX + distance;
    else
        finalX = startX - distance;

    /* Check finalX is in the range or not */
    if ((tankNumber == 1) && !((finalX >= MIN_X_TANK1) && (finalX <= MAX_X_TANK1)))
        return false;
    else if ((tankNumber == 2) && !((finalX >= MIN_X_TANK2) && (finalX <= MAX_X_TANK2)))
        return false;
    pos->x = finalX;
    return true;
}

void drawTanks(char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos)
{
    /* Tank Shapes */
    char shapeTank1[TANK_HEIGHT][TANK_WIDTH] = {{' ', ' ', ' ', '_', '_', ' ', ' ', ' ', ' '},
                                                {' ', '_', '|', '_', '_', '|', '_', '/', '/'},
                                                {'|', '_', '_', '_', '_', '_', '_', '_', '|'}};
    char shapeTank2[TANK_HEIGHT][TANK_WIDTH] = {{' ', ' ', ' ', ' ', '_', '_', ' ', ' ', ' '},
                                                {'\\', '\\', '_', '|', '_', '_', '|', '_', ' '},
                                                {'|', '_', '_', '_', '_', '_', '_', '_', '|'}};

    /* Change grid and add tank to it */
    for (int i = (TANK_HEIGHT - 1); i >= 0; i--)
    {
        for (int j = 0; j < TANK_WIDTH; j++)
        {
            int row_tank1 = tank1Pos.y + i - (TANK_HEIGHT - 1);
            int col_tank1 = tank1Pos.x + j;
            int row_tank2 = tank2Pos.y + i - (TANK_HEIGHT - 1);
            int col_tank2 = tank2Pos.x + j;

            grid[row_tank1][col_tank1] = shapeTank1[i][j];
            grid[row_tank2][col_tank2] = shapeTank2[i][j];
        }
    }
}

/* Clears tanks from the grid */
void clearTanksFromGrid(char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos)
{
    for (int i = (TANK_HEIGHT - 1); i >= 0; i--)
    {
        for (int j = 0; j < TANK_WIDTH; j++)
        {
            grid[(tank1Pos.y + i - (TANK_HEIGHT - 1))][tank1Pos.x + j] = ' ';
            grid[(tank2Pos.y + i - (TANK_HEIGHT - 1))][tank2Pos.x + j] = ' ';
        }
    }
}

/* Clears bullet from the grid */
void clearBulletFromGrid(char grid[GAME_ROWS][GAME_COLS + 1], POSITION bulletPos)
{
    if ((bulletPos.x >= 0) && (bulletPos.x < GAME_COLS) &&
        (bulletPos.y >= 0) && (bulletPos.y < GAME_ROWS))
    {
        grid[bulletPos.y][bulletPos.x] = ' ';
    }
}

/* Clears tanks and bullet from the grid */
void clearDynamicElements(char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos, POSITION bulletPos)
{
    clearTanksFromGrid(grid, tank1Pos, tank2Pos);
    if ((bulletPos.x != -1) && (bulletPos.y != -1))
    {
        clearBulletFromGrid(grid, bulletPos);
    }
}

/* Checks collision between bullet and terrain */
bool checkTerrainCollision(POSITION bulletPos, char grid[GAME_ROWS][GAME_COLS + 1])
{
    /* check for frame collision */
    if ((bulletPos.x < 0) || (bulletPos.x >= GAME_COLS) || (bulletPos.y < 0) || (bulletPos.y >= GAME_ROWS))
    {
        return true;
    }

    char temp = grid[bulletPos.y][bulletPos.x];
    if ((temp == '#') || (temp == '^'))
        return true;
    else
        return false;
}

int checkTankCollision(POSITION tank1Pos, POSITION tank2Pos, POSITION bulletPos)
{
    for (int i = (TANK_HEIGHT - 1); i >= 0; i--)
    {
        for (int j = 0; j < TANK_WIDTH; j++)
        {
            int row_tank1 = tank1Pos.y + i - (TANK_HEIGHT - 1);
            int col_tank1 = tank1Pos.x + j;
            if ((bulletPos.x == col_tank1) && (bulletPos.y == row_tank1))
                return COLLISION_TANK1;

            int row_tank2 = tank2Pos.y + i - (TANK_HEIGHT - 1);
            int col_tank2 = tank2Pos.x + j;
            if ((bulletPos.x == col_tank2) && (bulletPos.y == row_tank2))
                return COLLISION_TANK2;
        }
    }
    return COLLISION_NONE;
}

void calculateShotParameters(double angle, int power, POSITION tank, int turn, double *vx, double *vy, POSITION *bulletStartPos)
{
    /* Calculating the start position of the bullet */
    if (turn == 1)
        *bulletStartPos = (POSITION){
            .x = tank.x + TANK_WIDTH, .y = Y_TANKS - 2};
    else
        *bulletStartPos = (POSITION){
            .x = tank.x - 1, .y = Y_TANKS - 2};

    /* Calculating the angle and power which is suitable for using it in the formula */
    double angleInRadian = angle * (PI_NUMBER / 180);
    double powerInFormula = 2.0 + (7.0 * pow((double)power / 100.0, 1.5));
    double cosine = cos(angleInRadian);
    double sinus = sin(angleInRadian);
    /* Calculating velocity in the direction of X and Y axes */
    *vx = ((double)powerInFormula) * cosine;
    *vy = ((double)powerInFormula) * sinus;
}

/* Compute the projectile trajectory using discrete time simulation */
void computeShotPath(POSITION startPos, double vx, double vy, char grid[GAME_ROWS][GAME_COLS + 1], POSITION tank1Pos, POSITION tank2Pos, POSITION *trajectory, int *trajectoryLen, int *collisionType)
{
    double simulationTime = 0.0;
    *trajectoryLen = 0;

    while (*trajectoryLen < MAX_TRAJECTORY_LEN)
    {

        /* Initializing and Calculating the new position */
        POSITION newPos;
        newPos.x = (int)round((vx * simulationTime) + startPos.x);
        newPos.y = (int)round((0.5 * GRAVITY * pow(simulationTime, 2.0)) - (vy * simulationTime) + startPos.y);

        /* check for collision */
        int tankCollisionType = 0;
        if (checkTerrainCollision(newPos, grid) == true)
        {
            *collisionType = COLLISION_TERRAIN;
            break;
        }
        else if ((tankCollisionType = checkTankCollision(tank1Pos, tank2Pos, newPos)) > COLLISION_NONE)
        {
            *collisionType = tankCollisionType;
            break;
        }

        /* Add new position to the path */
        trajectory[*trajectoryLen] = newPos;

        (*trajectoryLen)++;
        simulationTime += 0.1;
    }
}

void displayShot(POSITION *path, int trajectoryLen, char grid[GAME_ROWS][GAME_COLS + 1], int *lastDrawnBulletIndex)
{
    *lastDrawnBulletIndex = -1;
    int animationStep = 0;
    for (int i = 0; i < trajectoryLen; i++)
    {
        if (i == 0)
        {
            if ((path[i].x >= 0) && (path[i].x < GAME_COLS) &&
                (path[i].y >= 0) && (path[i].y < GAME_ROWS))
            {
                grid[path[i].y][path[i].x] = '*';
            }
            *lastDrawnBulletIndex = i;
            system("cls");
            printGrid(grid);
            Sleep(DELAY_DISPLAY_SHOT);
        }
        else
        {
            if (!positionsAreEqual(path[i], path[i - 1]))
            {
                if (animationStep % 6 == 0)
                {
                    clearBulletFromGrid(grid, path[*lastDrawnBulletIndex]);
                    if ((path[i].x >= 0) && (path[i].x < GAME_COLS) &&
                        (path[i].y >= 0) && (path[i].y < GAME_ROWS))
                    {
                        grid[path[i].y][path[i].x] = '*';
                    }
                    *lastDrawnBulletIndex = i;
                    system("cls");
                    printGrid(grid);
                    Sleep(DELAY_DISPLAY_SHOT);
                }
                animationStep++;
            }
            if (i == (trajectoryLen - 1))
            {
                clearBulletFromGrid(grid, path[*lastDrawnBulletIndex]);
                if ((path[i].x >= 0) && (path[i].x < GAME_COLS) &&
                    (path[i].y >= 0) && (path[i].y < GAME_ROWS))
                {
                    grid[path[i].y][path[i].x] = '*';
                }
                *lastDrawnBulletIndex = i;
                system("cls");
                printGrid(grid);
                Sleep(DELAY_DISPLAY_SHOT);
            }
        }
    }
}

/* Handles one complete turn of the current player */
int processPlayerTurn(char grid[GAME_ROWS][GAME_COLS + 1], GameState *currentGame)
{
    char playerCommand;
    printf("(Player %d) Enter Command - L=Left, R=Right, S=Skip, Q=Quit, N=New Game, B=Back: ", currentGame->currentTurn);
    scanf(" %c", &playerCommand);

    if (playerCommand == 'S')
    {
        return STATE_NEXT_TURN;
    }
    else if (playerCommand == 'B')
    {
        return STATE_BACK;
    }
    else if (playerCommand == 'N')
    {
        return STATE_RESET;
    }
    else if (playerCommand == 'Q')
    {
        return STATE_QUIT;
    }
    else if ((playerCommand == 'R') || (playerCommand == 'L'))
    {
        int distance;
        scanf(" %d", &distance);

        clearTanksFromGrid(grid, currentGame->tank1Pos, currentGame->tank2Pos);

        bool moveResult;
        if (currentGame->currentTurn == 1)
            moveResult = updateTankPosition(1, playerCommand, distance, &currentGame->tank1Pos);
        else
            moveResult = updateTankPosition(2, playerCommand, distance, &currentGame->tank2Pos);

        drawTanks(grid, currentGame->tank1Pos, currentGame->tank2Pos);

        if (!moveResult)
        {
            printf("Illegal Move - Your Turn Is Lost!\n");
            Sleep(DELAY_SHOW_RESULT);
            return STATE_NEXT_TURN;
        }

        system("cls");
        printStatusBar(currentGame->tank1Health, currentGame->tank2Health);
        printGrid(grid);

        /* Angle and Power Input */
        printf("(Player [%d]) Enter Firing Angle [0-180]: ", currentGame->currentTurn);
        double angle;
        scanf("%lf", &angle);
        if ((angle < MIN_ANGLE) || (angle > MAX_ANGLE))
        {
            printf("Angle Out Of Range - Your Turn Is Lost!\n");
            Sleep(DELAY_SHOW_RESULT);
            return STATE_NEXT_TURN;
        }
        if (currentGame->currentTurn == 2)
            angle = 180 - angle;

        printf("(Player [%d]) Enter Shot Power [1-100]: ", currentGame->currentTurn);
        int power;
        scanf("%d", &power);
        if ((power < MIN_POWER) || (power > MAX_POWER))
        {
            printf("Power Out Of Range - Your Turn Is Lost!\n");
            Sleep(DELAY_SHOW_RESULT);
            return STATE_NEXT_TURN;
        }

        /* Bullet Part */
        double vx, vy;
        POSITION bulletStartPos;

        if (currentGame->currentTurn == 1)
            calculateShotParameters(angle, power, currentGame->tank1Pos, currentGame->currentTurn, &vx, &vy, &bulletStartPos);
        else
            calculateShotParameters(angle, power, currentGame->tank2Pos, currentGame->currentTurn, &vx, &vy, &bulletStartPos);

        POSITION trajectory[MAX_TRAJECTORY_LEN];
        int trajectoryLen;
        int collisionType;
        int lastDrawnBulletIndex;

        computeShotPath(bulletStartPos, vx, vy, grid, currentGame->tank1Pos, currentGame->tank2Pos, trajectory, &trajectoryLen, &collisionType);
        displayShot(trajectory, trajectoryLen, grid, &lastDrawnBulletIndex);

        /* Show appropriate message based on the collision result */
        switch (collisionType)
        {
        case COLLISION_TERRAIN:
            printf("Shot Terminated!\n");
            break;
        case COLLISION_TANK1:
            currentGame->tank1Health -= HIT_DAMAGE;
            if (currentGame->currentTurn == 1)
                printf("BOOM!!! Friendly Fire\n");
            else
                printf("BOOM!!! Clean Hit On The Enemy\n");
            break;
        case COLLISION_TANK2:
            currentGame->tank2Health -= HIT_DAMAGE;
            if (currentGame->currentTurn == 1)
                printf("BOOM!!! Clean Hit On The Enemy\n");
            else
                printf("BOOM!!! Friendly Fire\n");
            break;
        }

        /* cleat the last printed bullet */
        if ((trajectoryLen > 0) && (lastDrawnBulletIndex >= 0))
        {
            clearBulletFromGrid(grid, trajectory[lastDrawnBulletIndex]);
        }
        Sleep(DELAY_SHOW_RESULT);

        return STATE_NEXT_TURN;
    }
    else
    {
        printf("Invalid Command - Your Turn Is Lost!");
        Sleep(DELAY_SHOW_RESULT);
        return STATE_NEXT_TURN;
    }
}

/* Returns a random integer in the range [a, b] */
int randomInRange(int a, int b)
{
    int randomNumber = (rand() % (b - a + 1)) + a;
    return randomNumber;
}

/* Changes the turn number from 1 to 2 or from 2 to 1 */
void changeTurnNumber(int *turnNumber)
{
    *turnNumber = 3 - *turnNumber;
}

/* Compare two Positions */
bool positionsAreEqual(POSITION p1, POSITION p2)
{
    return (p1.x == p2.x) && (p1.y == p2.y);
}