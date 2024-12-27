#include <raylib.h>
#include <raymath.h>
#include <math.h>

// Constants matching Quake's values
#define PLAYER_SPEED 6.4f          // sv_maxspeed
#define PLAYER_ACCELERATION 10.0f     // sv_accelerate
#define AIR_ACCELERATION 1.0f         // Also from sv_accelerate but applied differently
#define STOP_SPEED 2.0f            // sv_stopspeed
#define FRICTION 6.0f                // sv_friction
#define GRAVITY 16.0f               // sv_gravity
#define JUMP_FORCE 5.4f            // Based on Quake's jump velocity
#define MAX_CLIP_PLANES 5

typedef struct {
	Vector3 position;
	Vector3 velocity;
	bool onGround;
	Camera3D camera;
} Player;

void InitPlayer(Player *player){
	player->position = (Vector3){0.0f, 2.0f, 0.0f};
	player->velocity = (Vector3){0.0f, 0.0f, 0.0f};
	player->onGround = false;
	player->camera.position = player->position;
	player->camera.target = (Vector3){0.0f, 0.0f, 0.0f};
	player->camera.up = (Vector3){0.0f, 1.0f, 0.0f};
	player->camera.fovy = 90.0f;
	player->camera.projection = CAMERA_PERSPECTIVE;
}



int main(){
	InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "quake movement");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);
	MaximizeWindow();
	SetTargetFPS(144);
	DisableCursor();
	
	Player player;
	InitPlayer(&player);
	
	while(!WindowShouldClose())
	{   
		float deltaTime= GetFrameTime();
		
		BeginDrawing();
			ClearBackground(BLACK);
			
			BeginMode3D(player.camera);
				DrawGrid(50, 5.0f);

			EndMode3D();
			
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}