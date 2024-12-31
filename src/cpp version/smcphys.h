#ifndef SMCPHYS_H
#define SMCPHYS_H

#include <raylib.h>
#include <raymath.h>

class Player {
private:
	static constexpr float JUMP_VELOCITY= 8.4f;
	static constexpr float FRICTION= 0.546f;
	static constexpr float FRICTION_AIR= 0.91f;
	static constexpr float AIR_ACCEL_MULTIPLIER= 0.2f;
	static constexpr float GRAVITY= 1.6f;
	static constexpr float AIR_DRAG= 0.98f;
	static constexpr float VERTICAL_DRAG_THRESHOLD= 0.1f;
	static constexpr float PLAYER_HEIGHT= 1.62f;
	static constexpr float PLAYER_CROUCH_HEIGHT= 1.54f;
	static constexpr float SENSIVITY= 0.002f;

	Vector3 position;
	Vector3 velocity;
	Vector3 angles;
	bool onGround;
	float height;
	Camera3D camera;

	void ApplyFriction(){
		float speed= Vector2Length((Vector2){velocity.x, velocity.z});

		if(speed < 0.01f){
			velocity= (Vector3){0};
			return;
		}

		velocity.x*= FRICTION;
		velocity.z*= FRICTION;
	}

	void ApplyGravity(){
		if(!onGround){
			velocity.y-= GRAVITY;
			velocity.y*= AIR_DRAG;

			velocity.x*= FRICTION_AIR;
			velocity.z*= FRICTION_AIR;

			if(fabs(velocity.y) < VERTICAL_DRAG_THRESHOLD){
				velocity.y= 0.0f;
			}
		}
		else {
			velocity.y= 0;
		}
	}

	void UpdateCamera(){
		camera.position= position;
		Vector3 forward;
		forward.x= cosf(angles.x) * cosf(angles.y);
		forward.y= sinf(angles.x);
		forward.z= cosf(angles.x) * sinf(angles.y);
		
		camera.target= Vector3Add(camera.position, forward);
	}

	void ProcessInput(bool is_tick20){
		Vector2 mouseDelta= GetMouseDelta();
		float sensitivity= SENSIVITY;
		float acclerationMultiplier= 1.0f;
		bool is_going_forward= false;

		angles.y+= mouseDelta.x * sensitivity;
		angles.x= Clamp(angles.x - mouseDelta.y * sensitivity, -PI/2, PI/2);

		Vector3 forward= {
			cosf(angles.y),
			0,
			sinf(angles.y)
		};
		Vector3 right= {
			sinf(angles.y),
			0,
			-cosf(angles.y)
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
				height= PLAYER_CROUCH_HEIGHT;
			}
			else {
				height= PLAYER_HEIGHT;
			}

			if(IsKeyDown(KEY_SPACE) && onGround){
				velocity.y= JUMP_VELOCITY;

				if(acclerationMultiplier== 1.3f){
					forward= Vector3Scale(forward, 4.0f);
					velocity.x+= forward.x;
					velocity.z+= forward.z;
				}

				onGround= false;
			}

			Vector3 playerAcceleration= {0};
			if(!Vector3Equals(wishvel, (Vector3){0,0,0})){
				if(onGround){
					wishvel= Vector3Normalize(wishvel);

					playerAcceleration.x= 20 * 0.098 * acclerationMultiplier;
					playerAcceleration.z= 20 * 0.098 * acclerationMultiplier;
					
					wishvel= Vector3Multiply(wishvel, playerAcceleration);
				}
				else {
					wishvel= Vector3Normalize(wishvel);

					playerAcceleration.x= 20 * 0.098 * AIR_ACCEL_MULTIPLIER * acclerationMultiplier;
					playerAcceleration.z= 20 * 0.098 * AIR_ACCEL_MULTIPLIER * acclerationMultiplier;
					
					wishvel= Vector3Multiply(wishvel, playerAcceleration);
				}
			}

			velocity= Vector3Add(velocity, wishvel);
		}
	}

public:
	Player(){
		position= (Vector3){ 0.0f, PLAYER_HEIGHT, 0.0f };
		velocity= (Vector3){ 0.0f, 0.0f, 0.0f };
		angles= (Vector3){ 0.0f, 0.0f, 0.0f };
		onGround= false;
		height= PLAYER_HEIGHT;

		camera.position= position;
		camera.target= Vector3Add(position, 
			(Vector3){ cosf(angles.y), 0.0f, sinf(angles.y) });
		camera.up= (Vector3){ 0.0f, 1.0f, 0.0f };
		camera.fovy= 75.0f;
		camera.projection= CAMERA_PERSPECTIVE;
	}

	void Update(float deltaTime){
		static float tickAccumulator= 0.0f;
		const float tickRate= 1.0f/20.0f; // 20 ticks per second
		bool is_tick20= false;

		tickAccumulator+= deltaTime;

		if(tickAccumulator >= tickRate){
			is_tick20= true;
			tickAccumulator-= tickRate;
		}
		
		if(position.y<= height){
			position.y= height;
			onGround= true;
		}
		else {
			onGround= false;
		}

		if(is_tick20){
			if(onGround){
				ApplyFriction();
			}
			ApplyGravity();
		}
		ProcessInput(is_tick20);
		
		position= Vector3Add(position, Vector3Scale(velocity, deltaTime));

		UpdateCamera();
	}

	void DrawDebugVector(){
		Vector3 end= Vector3Add(position, velocity);
		Vector3 start= position;
		start.y-= height;
		end.y= position.y -height;
		DrawLine3D(start, end, PINK);
		DrawCube(end, 0.1f, 0.1f, 0.1f, PINK);

		DrawCube((Vector3){position.x,
						  position.y -height,
						  position.z}, 0.1f, 0.1f, 0.1f, PINK);
	}

	const Camera3D& GetCamera() const { return camera; }
};

#endif // SMCPHYS_H