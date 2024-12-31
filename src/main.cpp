#include "smcphys.h"

int main(){
	InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "smcphys | minecraft movement");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);
	MaximizeWindow();
	SetTargetFPS(240);
	DisableCursor();
	
	Player player;
	InitPlayer(&player);
	
	while(!WindowShouldClose())
	{
		float deltaTime= GetFrameTime();
		UpdatePlayer(&player, deltaTime);

		BeginDrawing();
			ClearBackground(BLACK);
			
			BeginMode3D(player.camera);
				DrawGrid(100, 1.0f);
				DrawCube( (Vector3){0, 1, 0} , 1.0f, 2.0f, 1.0f, YELLOW);
				DrawCube( (Vector3){0, 0.5, 1} , 1.0f, 1.0f, 1.0f, DARKBLUE);

				DrawDebugVector(&player);
			EndMode3D();
			DrawFPS(10, 10);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}