
void GameLogic(Assignment07* A, float Ar, glm::mat4& ViewPrj, glm::mat4& World) {
	// The procedure must implement the game logic  to move the character in third person.
	// Input:
	// <Assignment07 *A> Pointer to the current assignment code. Required to read the input from the user
	// <float Ar> Aspect ratio of the current window (for the Projection Matrix)
	// Output:
	// <glm::mat4 &ViewPrj> the view-projection matrix of the camera
	// <glm::mat4 &World> the world matrix of the object
	// Parameters
	// Camera FOV-y, Near Plane and Far Plane
	const float FOVy = glm::radians(45.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 100.f;

	// Player starting point
	const glm::vec3 StartingPosition = glm::vec3(-41.50, 0.0, -19.5);

	// Camera target height and distance
	const float camHeight = 0.25;
	const float camDist = 1.5;
	// Camera Pitch limits
	const float minPitch = glm::radians(-8.75f);
	const float maxPitch = glm::radians(60.0f);
	const float minYaw = glm::radians(-360.0f);
	const float maxYaw = glm::radians(360.0f);
	// Rotation and motion speed
	const float ROT_SPEED = glm::radians(120.0f);
	const float MOVE_SPEED = 2.0f;

	// Integration with the timers and the controllers
	// returns:
	// <float deltaT> the time passed since the last frame
	// <glm::vec3 m> the state of the motion axes of the controllers (-1 <= m.x, m.y, m.z <= 1)
	// <glm::vec3 r> the state of the rotation axes of the controllers (-1 <= r.x, r.y, r.z <= 1)
	// <bool fire> if the user has pressed a fire button (not required in this assginment)
	float deltaT;
	glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
	bool fire = false;
	A->getSixAxis(deltaT, m, r, fire);

	// Game Logic implementation
	// Current Player Position - statc variables make sure that their value remain 
	// unchanged in subsequent calls to the procedure
	static glm::vec3 Pos = StartingPosition;

	//--- World Matrix ---
	static float objYaw = 0.0f;
	objYaw += ROT_SPEED * r.z * deltaT;
	objYaw = objYaw > maxYaw ? objYaw - maxYaw : (objYaw < minYaw 
		? objYaw + maxYaw : objYaw); // avoid overflows

	glm::quat movementDirection = glm::quat(glm::vec3(0, objYaw, 0));
	m.z *= -1;
	m *= MOVE_SPEED * deltaT;
	glm::vec4 charMovement = glm::mat4(movementDirection) * glm::vec4(m, 1);
	Pos += glm::vec3(charMovement);
	Pos.y = Pos.y < 0.0f ? 0.0f : Pos.y; // avoid character going underground

	World = glm::translate(glm::mat4(1.0), Pos) *
		glm::mat4(glm::quat(glm::vec3(0,objYaw,0)));


	//--- View-Projection Matrix ---
	static float camYaw = 0.0f;
	static float camPitch = 0.0f;
	const float DAMPING = 10.0f;

	camPitch += ROT_SPEED * r.x * deltaT;
	camPitch = camPitch < minPitch ? minPitch : (camPitch > maxPitch ? maxPitch : camPitch); 

	camYaw += ROT_SPEED * r.y * deltaT;
	camYaw = camYaw > maxYaw ? camYaw - maxYaw : (camYaw < minYaw ? camYaw + maxYaw : camYaw);

	const glm::vec3 CAMBASE = glm::vec3(0, camHeight, camDist);
	glm::quat cameraDirection = glm::quat(glm::vec3(camPitch, -camYaw, 0));

	static glm::vec3 oldCam = glm::vec3(glm::mat4(cameraDirection) * glm::vec4(CAMBASE, 1)) + Pos;
	glm::vec3 newCam = glm::vec3(glm::mat4(cameraDirection) * glm::vec4(CAMBASE, 1)) + Pos;
	
	// Damping
	glm::vec3 cam = oldCam * exp(-DAMPING * deltaT) + newCam * (1 - exp(-DAMPING * deltaT));
	oldCam = cam;

	glm::vec3 u = glm::vec3(0, 1, 0);
	glm::mat4 View = glm::lookAt(cam, Pos, u);
	glm::mat4 Proj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
	Proj[1][1] *= -1;

	ViewPrj = Proj * View;
	//-------------------------------
}
