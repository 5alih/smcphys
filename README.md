
# smcphys.h

A single-header library that replicates Minecraft's movement physics in C/C++. Built with raylib for rendering and input handling.

## Features
- Accurate Minecraft-like movement physics
- Ground/air acceleration
- Jump mechanics 
- Configurable constants
- Built-in debug visualization
- 20 tick rate synchronization
- Simple integration
- Minimal dependencies (only raylib)

## Usage
1. Include raylib and smcphys.h in your project
2. Initialize a Player
3. Call UpdatePlayer() in your game loop
4. Use the Player's camera for rendering

#### C version:
```c
#include "smcphys.h"

int main() {
    InitWindow(800, 600, "Example");
    
    Player player;			// Create the player
    InitPlayer(&player);	// Initialize player
    
    while (!WindowShouldClose()) {
        UpdatePlayer(&player, GetFrameTime());	// Update the player
        
        BeginDrawing();
            BeginMode3D(player.camera);			// Use player camera
                // Your 3D rendering
            EndMode3D();
        EndDrawing();
    }
}
```

#### C++ version:
```c
#include "smcphys.h"

int main() {
    InitWindow(800, 600, "Example");
    
    Player player;			// Create the player
    InitPlayer(&player);	// Initialize player
    
    while (!WindowShouldClose()) {
        UpdatePlayer(&player, GetFrameTime());	// Update the player
        
        BeginDrawing();
            BeginMode3D(player.camera);			// Use player camera
                // Your 3D rendering
            EndMode3D();
        EndDrawing();
    }
}
```

## Controls
- WASD: Movement
- Space: Jump
- Mouse: Look around
- Left Control: Sprint
- Left Shift: Sneak


## Debug Tools
- `DrawDebugVector(&player)`: Visualizes velocity vector
- Built-in velocity magnitude display
- Ground state indicator

## Dependencies
- raylib (for rendering, input, and math)

## License
MIT License