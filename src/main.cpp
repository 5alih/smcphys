#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string>
#include <iostream>

#define PLAYER_SPEED 5.612f
#define JUMP_VELOCITY 8.93f
#define FRICTION 0.8f
#define GRAVITY 31.1f

#define PLAYER_HEIGHT 1.625f
#define SENSIVITY 0.002f

typedef struct {
	Vector3 position;
	Vector3 velocity;
	Vector3 angles;
	bool onGround;
	Camera3D camera;
} Player;

void UpdateCamera(Player *player);

void InitPlayer(Player* player) 
{
	player->position= (Vector3){ 0.0f, PLAYER_HEIGHT, 0.0f };
	player->velocity= (Vector3){ 0.0f, 0.0f, 0.0f };
	player->angles= (Vector3){ 0.0f, 0.0f, 0.0f };
	player->onGround= false;
	
	player->camera.position= player->position;
	player->camera.target= Vector3Add(player->position, 
		(Vector3){ cosf(player->angles.y), 0.0f, sinf(player->angles.y) });
	player->camera.up= (Vector3){ 0.0f, 1.0f, 0.0f };
	player->camera.fovy= 75.0f;
	player->camera.projection= CAMERA_PERSPECTIVE;
}

void ProcessInput(Player* player, float deltaTime) {
	Vector2 mouseDelta= GetMouseDelta();
	float sensitivity= SENSIVITY;
	
	player->angles.y+= mouseDelta.x *sensitivity;
	player->angles.x= Clamp(player->angles.x -mouseDelta.y *sensitivity, -PI/2, PI/2);

	Vector3 forward= {
		cosf(player->angles.y),
		0,
		sinf(player->angles.y)
	};
	Vector3 right= {
		sinf(player->angles.y),
		0,
		-cosf(player->angles.y)
	};

	Vector3 wishvel= {0};
	if(IsKeyDown(KEY_W)) wishvel= Vector3Add(wishvel, forward);
	if(IsKeyDown(KEY_S)) wishvel= Vector3Subtract(wishvel, forward);
	if(IsKeyDown(KEY_A)) wishvel= Vector3Add(wishvel, right);
	if(IsKeyDown(KEY_D)) wishvel= Vector3Subtract(wishvel, right);

	if(!Vector3Equals(wishvel, (Vector3){0,0,0})){
		wishvel= Vector3Normalize(wishvel);
		wishvel= Vector3Scale(wishvel, PLAYER_SPEED);
	}

	// Handle jumping
	if(IsKeyDown(KEY_SPACE) && player->onGround){
		player->velocity.y= JUMP_VELOCITY;
		player->onGround= false;
	}

	player->velocity.x= wishvel.x;
	player->velocity.z= wishvel.z;

	// if (!Vector3Equals(wishvel, (Vector3){0,0,0})) {
	// 	Vector3 wishdir = Vector3Normalize(wishvel);
	// 	float wishspeed = PLAYER_SPEED;

	// 	if (player->onGround) {
	// 		Accelerate(player, wishdir, wishspeed, PLAYER_ACCELERATION, deltaTime);
	// 	} else {
	// 		AirAccelerate(player, wishdir, wishspeed, deltaTime);
	// 	}
	// }
}

void ApplyFriction(Player* player, float deltaTime) {
	float speed= Vector3Length(player->velocity);

	if(speed < 0.1f){
		player->velocity= (Vector3){0};
		return;
	}
	//float control= (speed < STOP_SPEED) ? STOP_SPEED : speed;
	float drop= speed *FRICTION *deltaTime;

	float newspeed= speed -drop;
	if(newspeed < 0) newspeed = 0;

	newspeed/= speed;
	player->velocity= Vector3Scale(player->velocity, newspeed);
}

void ApplyGravity(Player* player, float deltaTime){
	static float oldPosition= 0;
	static bool didPrint= false;
	static float timer= 0;

	if(!player->onGround){
		player->velocity.y-= GRAVITY *deltaTime;

		timer++;
		if(oldPosition< player->position.y){
			oldPosition= player->position.y;
		}
		else if(!didPrint && oldPosition!= PLAYER_HEIGHT && oldPosition>= player->position.y){
			didPrint= true;
			std::cout<< "jump height: "<< oldPosition -PLAYER_HEIGHT<< std::endl;
		}
	}
	else{
		player->velocity.y= 0;
		didPrint= false;
		if(timer!= 0)
			std::cout<< "timer: "<< timer/GetFPS() << std::endl;
		timer= 0;
	}
}

void UpdatePlayer(Player* player, float deltaTime) {
	ProcessInput(player, deltaTime);
	
	if (player->onGround) {
		ApplyFriction(player, deltaTime);
	}
	
	ApplyGravity(player, deltaTime);
	
	player->position= Vector3Add(player->position, 
		Vector3Scale(player->velocity, deltaTime));
	
	if(player->position.y<= PLAYER_HEIGHT){
		player->position.y= PLAYER_HEIGHT;
		player->onGround= true;
	}
	else{
		player->onGround= false;
	}
	UpdateCamera(player);
}

void UpdateCamera(Player *player){
	player->camera.position = player->position;
	Vector3 forward;
	forward.x= cosf(player->angles.x) * cosf(player->angles.y);
	forward.y= sinf(player->angles.x);
	forward.z= cosf(player->angles.x) * sinf(player->angles.y);
	
	player->camera.target = Vector3Add(player->camera.position, forward);
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
		UpdatePlayer(&player, deltaTime);

		// UpdateCamera(&player.camera, CAMERA_CUSTOM);

		BeginDrawing();
			ClearBackground(BLACK);
			
			BeginMode3D(player.camera);
				DrawGrid(100, 2.0f);
				DrawCube( (Vector3){0, 1, 0} , 1.0f, 2.0f, 1.0f, YELLOW);
				DrawCube( (Vector3){0, 0.5, 1} , 1.0f, 1.0f, 1.0f, DARKBLUE);

			EndMode3D();

			DrawFPS(10, 10);

			char velocity[32];
			std::sprintf(velocity, "%.1f", Vector3Length(player.velocity));
			DrawText(velocity, 10, 90, 20, YELLOW);

			DrawText(std::to_string((int)player.position.x).c_str(), 10, 30, 20, WHITE);
			std::sprintf(velocity, "%.3f", player.position.y);
			DrawText(velocity, 10, 50, 20, WHITE);
			DrawText(std::to_string((int)player.position.z).c_str(), 10, 70, 20, WHITE);
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}