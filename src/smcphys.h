#ifndef SMCPHYS_H
#define SMCPHYS_H

#include <raylib.h>
#include <raymath.h>

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
	if(!player->onGround){
		player->velocity.y-= GRAVITY;
		player->velocity.y*= AIR_DRAG;

		player->velocity.x*= FRICTION_AIR;
		player->velocity.z*= FRICTION_AIR;

		if(fabs(player->velocity.y) < VERTICAL_DRAG_THRESHOLD){
			player->velocity.y= 0.0f;
		}
	}
	else{
		player->velocity.y= 0;
	}
}

void UpdateCamera(Player *player){
	player->camera.position= player->position;
	Vector3 forward;
	forward.x= cosf(player->angles.x) *cosf(player->angles.y);
	forward.y= sinf(player->angles.x);
	forward.z= cosf(player->angles.x) *sinf(player->angles.y);
	
	player->camera.target= Vector3Add(player->camera.position, forward);
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
				forward= Vector3Scale(forward, 4.0f);
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

void UpdatePlayer(Player* player, float deltaTime){
	static float tickAccumulator= 0.0f;
	const float tickRate= 1.0f/20.0f; // 20 ticks per second
	bool is_tick20= false;

	tickAccumulator+= deltaTime;

	if(tickAccumulator>= tickRate){
		is_tick20= true;
		tickAccumulator-= tickRate;
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

    DrawCube( (Vector3){player->position.x,
                        player->position.y -player->height,
                        player->position.z}, 0.1f, 0.1f, 0.1f, PINK);
}

#endif // SMCPHYS_H