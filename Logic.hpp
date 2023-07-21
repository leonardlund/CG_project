
void GameLogic(Project* A, float Ar, glm::mat4& ViewPrj, glm::mat4& World, glm::vec3& ViewPosition, float& dollAngle, int& gameState) {
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
	const glm::vec3 StartingPosition = glm::vec3(-50, 0.0, 0);

	// Camera target height and distance
	const float camHeight = 1;
	const float camDist = 2;
	// Camera Pitch limits
	const float minPitch = glm::radians(-30.0f);
	const float maxPitch = glm::radians(20.0f);
	const float minYaw = glm::radians(-360.0f);
	const float maxYaw = glm::radians(360.0f);
	// Rotation and motion speed
	const float ROT_SPEED = glm::radians(120.0f);
	const float MOVE_SPEED = 2.0f;

	static float objYaw = -1.7f;

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
	
	objYaw -= ROT_SPEED * r.y * deltaT;
	objYaw = objYaw > maxYaw ? objYaw - maxYaw : (objYaw < minYaw 
		? objYaw + maxYaw : objYaw); // avoid overflows

	glm::quat movementDirection = glm::quat(glm::vec3(0, objYaw, 0));
	m.z *= -1;
	m *= MOVE_SPEED * deltaT;
	glm::vec4 charMovement = glm::mat4(movementDirection) * glm::vec4(m, 1);
	Pos += glm::vec3(charMovement);
	Pos.y = 0; // our character can't fly or go underground

	World = glm::translate(glm::mat4(1.0), Pos) *
		glm::mat4(glm::quat(glm::vec3(0,objYaw,0)));

	
	//--- View-Projection Matrix ---
	static float camYaw = 1.5f;
	static float camPitch = 0.0f;
	const float DAMPING = 10.0f;

	camPitch += ROT_SPEED * r.x * deltaT;
	camPitch = camPitch < minPitch ? minPitch : (camPitch > maxPitch ? maxPitch : camPitch); 
	camYaw = -objYaw;


	const glm::vec3 CAMBASE = glm::vec3(0, camHeight, camDist);
	glm::quat cameraDirection = glm::quat(glm::vec3(camPitch, -camYaw, 0));

	static glm::vec3 oldCam = glm::vec3(glm::mat4(cameraDirection) * glm::vec4(CAMBASE, 1)) + Pos;
	glm::vec3 newCam = glm::vec3(glm::mat4(cameraDirection) * glm::vec4(CAMBASE, 1)) + Pos;
	
	// Damping
	glm::vec3 cam = oldCam * exp(-DAMPING * deltaT) + newCam * (1 - exp(-DAMPING * deltaT));
	oldCam = cam;
	ViewPosition = cam;

	glm::vec3 u = glm::vec3(0, 1, 0);
	glm::mat4 View = glm::lookAt(cam, Pos + glm::vec3(0,0.5,0), u);
	glm::mat4 Proj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
	Proj[1][1] *= -1;

	ViewPrj = Proj * View;

	// ----- DOLL rotation ---------
	static float dolldirection = glm::radians(180.0f);
	const float dollSpeed = glm::radians(50.0f);
	static float time = 0;
	const float timeSpeed = 1.5f;
	float coef1 = 2.0f;
	float coef2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float coef3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float coef4 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float coef5 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	time += deltaT * timeSpeed;
	dolldirection += deltaT * dollSpeed * (coef1*cos(time)+ coef2*cos(2*time) + coef3*cos(3*time) + coef4*cos(4*time) + coef5*cos(5*time) + 0.5);
	dollAngle = dolldirection;

	float epsilon = glm::radians(10.0f);

	glm::vec3 firstVector = glm::vec3(cos(dolldirection + epsilon), 0, sin(dolldirection + epsilon));
	glm::vec3 secondVector = glm::vec3(cos(dolldirection - epsilon), 0, sin(dolldirection - epsilon));
	
	glm::vec3 firstNormal = glm::normalize(glm::cross(firstVector, glm::vec3(0, 1, 0)));
	glm::vec3 secondNormal = glm::normalize(glm::cross(secondVector, glm::vec3(0, 1, 0)));

	float firstCheck = glm::dot(firstNormal, Pos);
	float secondCheck = glm::dot(secondNormal, Pos);
	glm::vec3 zeroVec = glm::vec3(0.0f);

	static bool wasFire = false;
	bool handleFire = (wasFire && (!fire));
	wasFire = fire;
	bool startPlaying = false;

	switch (gameState) {
	case 0: // initial state - show splash screen
		if (handleFire) {
			gameState = 1;	// Start playing
			startPlaying = true;
		}
		break;
	case 1: 
		if (firstCheck <= 0 && secondCheck >= 0 && (r != zeroVec || m != zeroVec)) {
			gameState = 2;	// Lose
			Pos = StartingPosition;
		}
		if (Pos.x > -1.5) { // Win
			gameState = 3;
			Pos = StartingPosition;
		}
		if (startPlaying) {
			dolldirection = glm::radians(180.f);
			startPlaying = false;
			coef2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			coef3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			coef4 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			coef5 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		}
		break;
	case 2: // Lose
		if (handleFire) {
			gameState = 1;	// play again
			startPlaying = true;
		}
	case 3: // Win
		if (handleFire) {
			gameState = 1;	// play again
			startPlaying = true;
		}
	}

}
