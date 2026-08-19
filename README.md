### Tank Battle Game

Two player, terminal-based artillery battle coded in C, with influence of classics like Scorched Earth and Worms. The player alternates changing the angle and the power to shoot at their opponent's tank on randomly generated map.

Features
- Turn based game for two local players (Tank Alpha vs Tank Beta)
- Physics-based projectile motion: the trajectory is calculated by firing angle and power and time-based simulation under effect of gravity
- Terrain collision detection and tank collision detection
- Rendering in ASCII grid format with animated projectile movement
- Single use undo command per player (undoes the action to previous state of the game), available once per match
- Health system with damage calculation and win/loss condition check

Controls
- L / R: move tank left/right
- S: skip turn
- B: use back/undo command (once per player)
- N: start new game
- Q: quit

After moving your tank, enter firing angle (0-180) and power (1-100).

Technical Features
- Structures and pointers for managing the game state (GameState, POSITION)
- Time-based simulation of projectile's motion under effect of gravity and initial speed
- Grid-based rendering, redrawing and clearing of tanks and projectiles
- Tracking turn history (gamePrev / gamePrevPrev) to implement single use back command

Build and Run

Need C compiler that can compile code for Windows (uses <windows.h> for Sleep and system("cls")), e.g. MinGW:

```bash
gcc src/main.c -o tank_battle_game -lm
./tank_battle_game
