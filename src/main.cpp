#include <raylib.h>

int main() 
{   
	InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "quake movement");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);
	MaximizeWindow();
	SetTargetFPS(144);

	while (!WindowShouldClose())
	{   
		BeginDrawing();
			ClearBackground(BLACK);
		EndDrawing();
	}

	CloseWindow();
}