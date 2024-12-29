#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string>
#include <iostream>

#define PLAYER_SPEED 5.612f
#define JUMP_VELOCITY 9.17f
#define PLAYER_ACCELERATION 4.107f
#define FRICTION 0.0001f
#define FRICTION_AIR 0.00001f
#define AIR_ACCEL_MULTIPLIER 1.1f //1.3f
#define GRAVITY 31.55f
#define AIR_DRAG 0.8f
#define VERTICAL_DRAG_THRESHOLD 0.005f

#define PLAYER_HEIGHT 1.62f
#define PLAYER_SNEAK_HEIGHT 1.54f
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
	float acclerationMultiplier= 1.0f;

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

	if(IsKeyDown(KEY_LEFT_CONTROL)) acclerationMultiplier= 1.3f;
	if(IsKeyDown(KEY_LEFT_SHIFT))	acclerationMultiplier= 0.3f;

	Vector3 playerAcceleration= {0};
	if(!Vector3Equals(wishvel, (Vector3){0,0,0})){

		if(player->onGround){
			wishvel= Vector3Normalize(wishvel);
			wishvel= Vector3Scale(wishvel, PLAYER_ACCELERATION);

			playerAcceleration= wishvel;
			playerAcceleration.x*= 10 *acclerationMultiplier *deltaTime;
			playerAcceleration.z*= 10 *acclerationMultiplier *deltaTime;
		}
		else{
			wishvel= Vector3Normalize(wishvel);
			wishvel= Vector3Scale(wishvel, PLAYER_ACCELERATION);

			playerAcceleration= wishvel;
			playerAcceleration.x*= 10 *AIR_ACCEL_MULTIPLIER *acclerationMultiplier *deltaTime;
			playerAcceleration.z*= 10 *AIR_ACCEL_MULTIPLIER *acclerationMultiplier *deltaTime;
		}
	}

	player->velocity.x+= playerAcceleration.x;
	player->velocity.z+= playerAcceleration.z;

	if(IsKeyDown(KEY_SPACE) && player->onGround){
		player->velocity.y= JUMP_VELOCITY;

		if(acclerationMultiplier== 1.3f){
			forward= Vector3Scale(forward, 17.0f);	//boost
			player->velocity.x+= forward.x;
			player->velocity.z+= forward.z;
		}

		player->onGround= false;
	}
}

void ApplyFriction(Player* player, float deltaTime){
	float speed= Vector2Length( (Vector2){player->velocity.x, player->velocity.z} );

	if(speed < 0.01f){
		player->velocity= (Vector3){0};
		return;
	}

	player->velocity.x*= powf(FRICTION, deltaTime);
	player->velocity.z*= powf(FRICTION, deltaTime);
}

void ApplyGravity(Player* player, float deltaTime){
	static float oldPosition= 0;
	static bool didPrint= false;
	static float timer= 0;
	static bool isFirstTime= true;
	static Vector3 firstPos= {0};

	if(!player->onGround){
		player->velocity.y-= GRAVITY *deltaTime;
		player->velocity.y*= powf(AIR_DRAG, deltaTime);
		
		player->velocity.x*= powf(FRICTION_AIR, deltaTime);
		player->velocity.z*= powf(FRICTION_AIR, deltaTime);

        if(fabs(player->velocity.y) < VERTICAL_DRAG_THRESHOLD){
            player->velocity.y = 0.0f;
        }

		if(isFirstTime){
			std::cout<< "Velocity: "<< Vector3Length(player->velocity)<< std::endl;
			std::cout<< "Horizontal Velocity: "<< Vector2Length( (Vector2){player->velocity.x, player->velocity.z} )<< std::endl;

			firstPos= player->position;
			isFirstTime= false;
		}

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
		if(timer!= 0){
			std::cout<< "timer: "<< timer/GetFPS() << std::endl;
			std::cout<< "Distance: "<< Vector3Distance(firstPos, player->position)<< std::endl;
			std::cout<< "____________________________________________________"<< std::endl;
		}
		timer= 0;
		isFirstTime= true;
	}
}

void UpdatePlayer(Player* player, float deltaTime) {
	ProcessInput(player, deltaTime);
	
	if(player->onGround){
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

void DrawVelocityVector(Player* player){
	Vector3 end= Vector3Add(player->position, player->velocity);
	Vector3 start= player->position;
	start.y-= PLAYER_HEIGHT;
	end.y= player->position.y -PLAYER_HEIGHT;
	DrawLine3D(start, end, PINK);
	DrawCube(end, 0.1f, 0.1f, 0.1f, PINK);
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
		float deltaTime= GetFrameTime(); //		/20.0f
		UpdatePlayer(&player, deltaTime);

		// UpdateCamera(&player.camera, CAMERA_CUSTOM);

		BeginDrawing();
			ClearBackground(BLACK);
			
			BeginMode3D(player.camera);
				DrawGrid(100, 1.0f);
				DrawCube( (Vector3){0, 1, 0} , 1.0f, 2.0f, 1.0f, YELLOW);
				DrawCube( (Vector3){0, 0.5, 1} , 1.0f, 1.0f, 1.0f, DARKBLUE);
				DrawCube( (Vector3){player.position.x, 
									player.position.y -PLAYER_HEIGHT,
									player.position.z}, 0.1f, 0.1f, 0.1f, PINK);

//				DrawVelocityVector(&player);
			EndMode3D();

			DrawFPS(10, 10);

			char velocity[32];
			std::sprintf(velocity, "%.3f", Vector3Length(player.velocity));
			DrawText(velocity, 10, 90, 20, YELLOW);

			std::sprintf(velocity, "%.3f", Vector2Length( (Vector2){player.velocity.x, player.velocity.z} ));
			DrawText(velocity, 10, 110, 20, PURPLE);

			DrawText(std::to_string((int)player.position.x).c_str(), 10, 30, 20, WHITE);
			std::sprintf(velocity, "%.3f", player.position.y);
			DrawText(velocity, 10, 50, 20, WHITE);
			DrawText(std::to_string((int)player.position.z).c_str(), 10, 70, 20, WHITE);
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}