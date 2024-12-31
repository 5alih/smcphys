#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string>
#include <iostream>

#define JUMP_VELOCITY 8.4f
#define FRICTION 0.546f
#define FRICTION_AIR 0.91f
#define AIR_ACCEL_MULTIPLIER 0.2f
#define GRAVITY 1.6f
#define AIR_DRAG 0.98f
#define VERTICAL_DRAG_THRESHOLD 0.1f

#define PLAYER_HEIGHT 1.62f
#define PLAYER_CROUCH_HEIGHT 1.54f
#define SENSIVITY 0.002f

typedef struct {
	Vector3 position;
	Vector3 velocity;
	Vector3 angles;
	bool onGround;
	float height;
	Camera3D camera;
} Player;

void InitPlayer(Player* player) 
{
	player->position= (Vector3){ 0.0f, PLAYER_HEIGHT, 0.0f };
	player->velocity= (Vector3){ 0.0f, 0.0f, 0.0f };
	player->angles= (Vector3){ 0.0f, 0.0f, 0.0f };
	player->onGround= false;
	player->height= PLAYER_HEIGHT;

	player->camera.position= player->position;
	player->camera.target= Vector3Add(player->position, 
		(Vector3){ cosf(player->angles.y), 0.0f, sinf(player->angles.y) });
	player->camera.up= (Vector3){ 0.0f, 1.0f, 0.0f };
	player->camera.fovy= 75.0f;
	player->camera.projection= CAMERA_PERSPECTIVE;
}

void ApplyFriction(Player* player){
	float speed= Vector2Length( (Vector2){player->velocity.x, player->velocity.z} );

	if(speed < 0.01f){
		player->velocity= (Vector3){0};
		return;
	}

	player->velocity.x*= FRICTION;
	player->velocity.z*= FRICTION;
}

void ApplyGravity(Player* player){
	static float oldPosition= 0;
	static bool didPrint= false;
	static bool isFirstTime= true;
	static Vector3 firstPos= {0};

	static float jumpStartTime= 0;

	if(!player->onGround){
		if(isFirstTime){
			std::cout<< "Velocity: "<< Vector3Length(player->velocity)<< std::endl;
			std::cout<< "Horizontal Velocity: "<< Vector2Length( (Vector2){player->velocity.x, player->velocity.z} )<< std::endl;

			firstPos= player->position;
			isFirstTime= false;
			jumpStartTime= GetTime();
		}

		player->velocity.y-= GRAVITY;
		player->velocity.y*= AIR_DRAG;

		player->velocity.x*= FRICTION_AIR;
		player->velocity.z*= FRICTION_AIR;

		if(fabs(player->velocity.y) < VERTICAL_DRAG_THRESHOLD){
			player->velocity.y = 0.0f;
		}

		if(oldPosition< player->position.y){
			oldPosition= player->position.y;
		}
		else if(!didPrint && oldPosition!= player->height && oldPosition>= player->position.y){
			didPrint= true;
			std::cout<< "jump height: "<< oldPosition -player->height<< std::endl;
		}
	}
	else{
		player->velocity.y= 0;
		didPrint= false;
		if(!isFirstTime){
			std::cout<< "Duration: "<< GetTime() -jumpStartTime << std::endl;
			std::cout<< "Distance: "<< Vector3Distance(firstPos, player->position) +0.5f<< std::endl;
			std::cout<< "____________________________________________________"<< std::endl;
		}	
		isFirstTime= true;
	}
}

void UpdateCamera(Player *player){
	player->camera.position = player->position;
	Vector3 forward;
	forward.x= cosf(player->angles.x) * cosf(player->angles.y);
	forward.y= sinf(player->angles.x);
	forward.z= cosf(player->angles.x) * sinf(player->angles.y);
	
	player->camera.target = Vector3Add(player->camera.position, forward);
}

void ProcessInput(Player* player, bool is_tick20){
	Vector2 mouseDelta= GetMouseDelta();
	float sensitivity= SENSIVITY;
	float acclerationMultiplier= 1.0f;
	bool is_going_forward= false;

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

	if(is_tick20){
		Vector3 wishvel= {0};
		if(IsKeyDown(KEY_W)){
			wishvel= Vector3Add(wishvel, forward);
			is_going_forward= true;
		}
		if(IsKeyDown(KEY_S)) wishvel= Vector3Subtract(wishvel, forward);
		if(IsKeyDown(KEY_A)) wishvel= Vector3Add(wishvel, right);
		if(IsKeyDown(KEY_D)) wishvel= Vector3Subtract(wishvel, right);

		if(IsKeyDown(KEY_LEFT_CONTROL) && is_going_forward){
			acclerationMultiplier= 1.3f;
		}
		else if(IsKeyDown(KEY_LEFT_SHIFT)){
			acclerationMultiplier= 0.3f;
			player->height= PLAYER_CROUCH_HEIGHT;
		}
		else{
			player->height= PLAYER_HEIGHT;
		}

		if(IsKeyDown(KEY_SPACE) && player->onGround){
			player->velocity.y= JUMP_VELOCITY;

			if(acclerationMultiplier== 1.3f){
				forward= Vector3Scale(forward, 4.0f);	//boost 4.5f
				player->velocity.x+= forward.x;
				player->velocity.z+= forward.z;
			}

			player->onGround= false;
		}

		Vector3 playerAcceleration= {0};
		if(!Vector3Equals(wishvel, (Vector3){0,0,0})){

			if(player->onGround){
				wishvel= Vector3Normalize(wishvel);


				playerAcceleration.x= 20 *0.098 *acclerationMultiplier;
				playerAcceleration.z= 20 *0.098 *acclerationMultiplier;
				
				wishvel= Vector3Multiply(wishvel, playerAcceleration);
			}
			else{
				wishvel= Vector3Normalize(wishvel);

				playerAcceleration.x= 20 *0.098 *AIR_ACCEL_MULTIPLIER *acclerationMultiplier;
				playerAcceleration.z= 20 *0.098 *AIR_ACCEL_MULTIPLIER *acclerationMultiplier;
				
				wishvel= Vector3Multiply(wishvel, playerAcceleration);
			}
		}

		player->velocity= Vector3Add(player->velocity, wishvel);
	}
}

void UpdatePlayer(Player* player, float deltaTime) {
	// Keep track of accumulated time between ticks
	static float tickAccumulator= 0.0f;
	const float tickRate= 1.0f/20.0f; // 20 ticks per second
	bool is_tick20= false;

	// Accumulate time
	tickAccumulator+= deltaTime;

	// Check if enough time has passed for a tick
	if(tickAccumulator>= tickRate) {
		is_tick20= true;
		tickAccumulator-= tickRate; // Subtract tick time but keep remainder
	}
	
	if(player->position.y<= player->height){
		player->position.y= player->height;
		player->onGround= true;
	}
	else{
		player->onGround= false;
	}

	if(is_tick20){
		if(player->onGround){
			ApplyFriction(player);
		}
		ApplyGravity(player);
	}
	ProcessInput(player, is_tick20);
	
	player->position= Vector3Add(player->position, Vector3Scale(player->velocity, deltaTime));

	UpdateCamera(player);
}

void DrawDebugVector(Player* player){
	Vector3 end= Vector3Add(player->position, player->velocity);
	Vector3 start= player->position;
	start.y-= player->height;
	end.y= player->position.y -player->height;
	DrawLine3D(start, end, PINK);
	DrawCube(end, 0.1f, 0.1f, 0.1f, PINK);
}

int main(){
	InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "minecraft movement");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetExitKey(KEY_NULL);
	MaximizeWindow();
	SetTargetFPS(240);
	DisableCursor();
	
	Player player;
	InitPlayer(&player);
	
	float counter= 0;
	float frameCounter= 0;
	float avgVelocity= 0;
	float avgVelocityHorizontal= 0;
	float totalVeclocity= 0;
	float totalVeclocityHorizontal= 0;
	while(!WindowShouldClose())
	{
		float deltaTime= GetFrameTime(); //		/20.0f
		UpdatePlayer(&player, deltaTime);
		
		counter+= deltaTime;
		frameCounter++;
		totalVeclocity+= Vector3Length(player.velocity);
		totalVeclocityHorizontal+= Vector2Length( (Vector2){player.velocity.x, player.velocity.z} );
		if(counter>= 3.0f){
			avgVelocity= totalVeclocity/frameCounter;
			avgVelocityHorizontal= totalVeclocityHorizontal/frameCounter;
			counter= 0;
			frameCounter= 0;
			totalVeclocity= 0;
			totalVeclocityHorizontal= 0;
		}

		BeginDrawing();
			ClearBackground(BLACK);
			
			BeginMode3D(player.camera);
				DrawGrid(100, 1.0f);
				DrawCube( (Vector3){0, 1, 0} , 1.0f, 2.0f, 1.0f, YELLOW);
				DrawCube( (Vector3){0, 0.5, 1} , 1.0f, 1.0f, 1.0f, DARKBLUE);
				DrawCube( (Vector3){player.position.x, 
									player.position.y -player.height,
									player.position.z}, 0.1f, 0.1f, 0.1f, PINK);

				// DrawDebugVector(&player);
			EndMode3D();

			DrawFPS(10, 10);

			char velocity[32];
			std::sprintf(velocity, "%.3f", Vector3Length(player.velocity));
			DrawText(velocity, 10, 30, 20, GOLD);

			std::sprintf(velocity, "%.3f", Vector2Length( (Vector2){player.velocity.x, player.velocity.z} ));
			DrawText(velocity, 10, 50, 20, PURPLE);

			std::sprintf(velocity, "%.3f", avgVelocity);
			DrawText(velocity, 10, 70, 20, (Color){105, 86, 24, 255} );

			std::sprintf(velocity, "%.3f", avgVelocityHorizontal);
			DrawText(velocity, 10, 90, 20, (Color){83, 63, 96, 255} );
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}