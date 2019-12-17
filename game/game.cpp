#include "game/game.h"
#include "Map.h"
#include <util/filesystem.h>
#include <SDL2/SDL.h>
#include <string>
#include <glm/gtx/vector_angle.hpp>

#define EPSILON 0.5f;

glm::vec3 gameCamPos = glm::vec3(0.0f, 4.0f, 10.0f);
glm::vec3 gameCamFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 gameCamUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 freeCamPos = glm::vec3(0.0f, 0.0f, 30.0f);
glm::vec3 freeCamFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 freeCamUp = glm::vec3(0.0f, 1.0f, 0.0f);

static Camera globalCam;

// pitch and yaw
GLfloat pitch = 0.0f;
GLfloat yaw = 0.0f;
glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

// frame for animation
int frame = 0;


GLfloat game_deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat game_lastFrame = 0.0f;  	// Time of last frame
GLfloat game_deltaAnim = 0.0f;	// timer for animation

bool animIsRunning = false;	// check if animation is currently running



/* checks each Character in the scene if it is bumping into something. */
void collisionDetect(std::vector<Character*> entities) {
	for (int i = 0; i < entities.size(); ++i) {
		for (int j = i + 1; j < entities.size(); ++j) {
			int currentFramesIndex = entities[i]->getModelPtr()->getModelData().currentFrame;//40
			if (entities[i]->getTransform().pos.x < entities[j]->getTransform().pos.x) { // player left from npc
				if (entities[i]->getTransform().pos.x + 0.001f + entities[i]->getBBox().getWidth(currentFramesIndex) / 2 > entities[j]->getTransform().pos.x
					&& entities[i]->getTransform().pos.z < entities[j]->getTransform().pos.z + entities[j]->getBBox().getDepth(40) / 2
					&& entities[i]->getTransform().pos.z > entities[j]->getTransform().pos.z - entities[j]->getBBox().getDepth(40) / 2) {

					entities[i]->setVelocity(0.0f); // no contribution in x direction anymore
					entities[i]->getTransform().pos += -entities[i]->getDirection() * 0.1f; // set back a bit so the model doesn't get stuck in the obstacle
				}
				else {
					entities[i]->setVelocity(0.03f);
				}
			}
			if (entities[i]->getTransform().pos.x > entities[j]->getTransform().pos.x) { // player right from npc
				if (entities[i]->getTransform().pos.x - 0.001f - entities[i]->getBBox().getWidth(currentFramesIndex) / 2 < entities[j]->getTransform().pos.x
					&& entities[i]->getTransform().pos.z < entities[j]->getTransform().pos.z + entities[j]->getBBox().getDepth(40) / 2
					&& entities[i]->getTransform().pos.z > entities[j]->getTransform().pos.z - entities[j]->getBBox().getDepth(40) / 2) {

					entities[i]->setVelocity(0.0f); // no contribution in x direction anymore
					entities[i]->getTransform().pos += -entities[i]->getDirection() * 0.1f; // set back a bit so the model doesn't get stuck in the obstacle
				}
				else {
					entities[i]->setVelocity(0.03f);
				}
			}
			if (entities[i]->getTransform().pos.z > entities[j]->getTransform().pos.z) { // player in front of npc
				if (entities[i]->getTransform().pos.z - 0.001f - entities[i]->getBBox().getDepth(40) / 2 < entities[j]->getTransform().pos.z
					&& entities[i]->getTransform().pos.x < entities[j]->getTransform().pos.x + entities[j]->getBBox().getWidth(40) / 2
					&& entities[i]->getTransform().pos.x > entities[j]->getTransform().pos.x - entities[j]->getBBox().getWidth(40) / 2) {

					entities[i]->setVelocity(0.0f); // no contribution in x direction anymore
					entities[i]->getTransform().pos += -entities[i]->getDirection() * 0.1f; // set back a bit so the model doesn't get stuck in the obstacle
				}
				else {
					entities[i]->setVelocity(0.03f);
				}
			}
			if (entities[i]->getTransform().pos.z < entities[j]->getTransform().pos.z) { // player behind of npc
				if (entities[i]->getTransform().pos.z + 0.001f + entities[i]->getBBox().getDepth(currentFramesIndex) / 2 > entities[j]->getTransform().pos.z
					&& entities[i]->getTransform().pos.x < entities[j]->getTransform().pos.x + entities[j]->getBBox().getWidth(currentFramesIndex) / 2
					&& entities[i]->getTransform().pos.x > entities[j]->getTransform().pos.x - entities[j]->getBBox().getWidth(currentFramesIndex) / 2) {

					entities[i]->setVelocity(0.0f); // no contribution in x direction anymore
					entities[i]->getTransform().pos += -entities[i]->getDirection() * 0.1f; // set back a bit so the model doesn't get stuck in the obstacle
				}
				else {
					entities[i]->setVelocity(0.03f);
				}
			}
		}
	}
}

void computeAI(Character& character, Character& player) {
	glm::vec3 direction = glm::normalize(player.getTransform().pos - character.getTransform().pos);
	character.setDirection(direction);
	character.getTransform().pos += game_deltaTime * character.getVelocity() * character.getDirection();

	float angle = glm::angle(glm::vec3(1.0f, 0.0f, 0.0f), character.getDirection());
	if (character.getDirection().z > 0) {
		angle = glm::angle(glm::vec3(-1.0f, 0.0f, 0.0f), character.getDirection());
		angle += glm::radians(180.0f);
	}
	character.getTransform().rot.y = angle;
}

char *basePath = SDL_GetBasePath();

std::vector<std::string> shader_variables_md2 = { "tposition", "texCoord", "normal", "nextVertexFrame", "nextNormalFrame" };
/* md2 shader for md2 models to animate them */


/* position of the light */
glm::vec3 lightPos(0.0f, 2.0f, 0.0f);

/* replace light stuff with light class (no model) */
Light myLight(lightPos, glm::vec3(1.0f, 1.0f, 1.0f));

/* load MD2 model */


/* pack the characters in a vector, maybe not the best solution for collision detection. Using something more abstract than Character (not only they can collide) would be better */


bool isClosed = false;
//glUniform1i(glGetUniformLocation(lightMapShader.m_program, "diffuseSampler"), 0);
//glUniform1i(glGetUniformLocation(lightMapShader.m_program, "specularSampler"), 1);
SDL_Event event;

/* push vertices of md2 model to gpu */


// scale the bounding boxes. should probably not be here but in an init loop


/* initial setup for characters */


glm::mat4x4 mProjMatrix, mMVP, mModel, mView;
glm::vec3 modelTrans;
float scaleFactor;
float md2deltaTime = 0.0f;
float md2lastFrame = 0.0f;
Camera camera(glm::vec3(0.0f, 4.0f, 10.0f));

namespace game
{
	GamePtr Game::create()
	{
		return std::make_shared<Game>();
	}
	
	Game::~Game()
	{
		delete this->map;
		this->map = NULL;
	}

	void Game::Init(unsigned int screen_width, unsigned int screen_height, Renderer *appRenderer,const char *base_path)
	{
		this->activeCam = 1;
		this->scr_width = screen_width;
		this->scr_height = screen_height;
		this->skyBox.Init();
		this->map = new Map();
		this->map->Init();
		//this->m_3ds = new CLoad3DS();
		//char *base_path = SDL_GetBasePath();

		//this->m_3ds->Init(const_cast<char *>(FileSystem::getPath(base_path, "resources/model/spaceFlight.3ds").c_str()), 0);
		//this->m_3ds->show3ds(0, 0, 0, 0, 100);
		//
		
		this->ourModel_space = new Model(FileSystem::getPath(base_path, "shape/body_all.obj").c_str());
		//resources/model/Model/soldier.md2
		this->ourModel = new Model(FileSystem::getPath(base_path, "shape/head_01.obj").c_str());
		//resources/model/IntergalacticSpaceship/Intergalactic_Spaceship-(Wavefront).obj
		//(const_cast<char *>(FileSystem::getPath(base_path, "resources/model/spaceFlight.3ds").c_str()));
	   //("C:/workspace/StudyOpenGLProgram3DGameCAI/model/Realistic_earth.obj");
		this->spaceFlightModelShader = new Shader(FileSystem::getPath(base_path, "resources/shaderFiles/model_loading_color.vs").c_str(), FileSystem::getPath(base_path, "resources/shaderFiles/model_loading_color.fs").c_str());
		this->spaceFlightModelShader2 = new Shader(FileSystem::getPath(base_path, "resources/shaderFiles/model_loading_color.vs").c_str(), FileSystem::getPath(base_path, "resources/shaderFiles/model_loading_color.fs").c_str());
		
		//this->md2_file = new md2File(2.5f, 512, 512, 0, 100);
		////center this model
		//this->md2_file->Import(FileSystem::getPath(base_path, "resources/model/Model/warrior.md2").c_str(), glm::mat4(0.0f));
		//modelTrans.x = (this->md2_file->m_bb.m_fMaxx + this->md2_file->m_bb.m_fMinx) / 2.0f;
		//modelTrans.y = (this->md2_file->m_bb.m_fMaxy + this->md2_file->m_bb.m_fMiny) / 2.0f;
		//modelTrans.z = (this->md2_file->m_bb.m_fMaxz + this->md2_file->m_bb.m_fMinz) / 2.0f;

		//scaleFactor = 1.0f / fmaxf(this->md2_file->m_bb.m_fMaxx - this->md2_file->m_bb.m_fMinx,
		//	fmaxf(this->md2_file->m_bb.m_fMaxy - this->md2_file->m_bb.m_fMiny, this->md2_file->m_bb.m_fMaxz - this->md2_file->m_bb.m_fMinz));
		//this->md2_model = new md2Model();
		//this->model[0] = this->md2_model->md2_readModel(FileSystem::getPath(base_path, "resources/model/Model/warrior.md2").c_str());
		//this->m_program.LoadShaders(FileSystem::getPath(base_path, "resources/shaderFiles/modelMd2.vs").c_str(),
			//FileSystem::getPath(base_path, "resources/shaderFiles/modelMd2.fs").c_str());
		char basePath[256] = {0};
		SDL_snprintf(basePath,256,"%s%s/",base_path,"resources");
		this->md2Shader = new AnimationShader(FileSystem::getPath(basePath,"opengl_shaders/md2_animation_shader"), shader_variables_md2);
		Texture *enemy_texture = appRenderer->imgLoadTexture(FileSystem::getPath(basePath, "models/Model/soldier.png").c_str());
		Texture *enemy_weapon_texture = appRenderer->imgLoadTexture(FileSystem::getPath(basePath, "models/Model/gun.png").c_str());
		 
		 
		Texture *player_texture = appRenderer->imgLoadTexture(FileSystem::getPath(basePath, "models/pknight/knight_diffuse_cheetos_1024.png").c_str());
		Texture *player_texture_specular = appRenderer->imgLoadTexture(FileSystem::getPath(basePath, "models/pknight/knight_spec_cheetos_1024.png").c_str());
	     //new Texture(FileSystem::getPath(base_path, "models/Model/warrior.jpg").c_str());
		Texture *player_weapon_texture = appRenderer->imgLoadTexture(FileSystem::getPath(basePath, "models/pknight/w_railgun.png").c_str());
		
		//this->container_texture = new Texture(FileSystem::getPath(base_path, "textures/container2.png"));

		this->player_model = new md2Model(FileSystem::getPath(basePath, "models/pknight/tris.md2"));
		this->enemy_model = new md2Model(FileSystem::getPath(basePath, "models/Model/tris.md2"));

		this->pknight_weapon_model = new md2Model(FileSystem::getPath(basePath, "models/pknight/w_railgun.md2"));
		this->enemyWeapon_model = new md2Model(FileSystem::getPath(basePath, "models/Model/gun.md2"));
		//this->cube_model = new md2Model(FileSystem::getPath(base_path, "models/cube.md2"));

		this->characters = {&player, &npc};

		this->player.setModel(this->player_model);
		player.setTexture(player_texture, DIFFUSE);
		player.setTexture(player_texture_specular, SPECULAR);

		 /* NPCs */
		
		 this->npc.setModel(this->enemy_model);
		 this->npc.setTexture(enemy_texture, DIFFUSE);
		 this->enemey_weapon.setModel(this->enemyWeapon_model);
		 this->enemey_weapon.setTexture(enemy_weapon_texture,DIFFUSE);
		 this->npc.setWeapon(this->enemey_weapon);

		 this->player_weapon.setModel(pknight_weapon_model);
		 this->player_weapon.setTexture(player_weapon_texture, DIFFUSE);
		 this->player.setWeapon(this->player_weapon);

		 /* push vertices of md2 model to gpu */
		 this->player_model->pushGPU();
		 this->enemy_model->pushGPU();
		 pknight_weapon_model->pushGPU();
		 this->enemyWeapon_model->pushGPU();
		 //cube_model->pushGPU();

		 // scale the bounding boxes. should probably not be here but in an init loop
		 player.getBBox().scale(glm::vec3(0.0625f));

		 /* initial setup for characters */
		 player.getTransform().scale = glm::vec3(0.0625f);
		 player.getTransform().rot = glm::vec3(glm::radians(-1.0f * 0.0f), 0.0, 0.0);
		 glm::vec3 playerPos = player.getTransform().getPos();
		 this->npc.getTransform().scale = glm::vec3(0.0625f);
		 npc.getTransform().rot = glm::vec3(glm::radians(-1.0f * 0.0f), 0.0, 0.0);
	     //glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * 90.0f), glm::vec3(1.0, 0.0, 0.0));
		 npc.setPos(glm::vec3(-5.0f, 0.0f, 0.0f));
		 camera.ProcessMouseMovement(0.0, -50);
		 camera.ProcessKeyboard(Camera_Movement::FORWARD, 40);
	}

	void Game::Render()
	{
		unsigned int now_FPS_Timer = SDL_GetTicks();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)this->scr_width / (float)scr_height, 0.1f, 3000.0f);
		this->skyBox.Render(camera,projection);
		glm::mat4 view = camera.GetViewMatrix();
		this->map->Render(camera, projection, view);
		glm::mat4 model = glm::mat4(1.0f);
		//0.0f, -48.3f, -35.0f  -23.0f, -48.15f, -42.0f
		model = glm::translate(model, glm::vec3(0.0,-35.0,0.0)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(5.1f, 5.1f, 5.1f));	// it's a bit too big for our scene, so scale it down
		this->spaceFlightModelShader->use();
		this->spaceFlightModelShader->setMat4("view", view);
		this->spaceFlightModelShader->setMat4("projection", projection);
		this->spaceFlightModelShader->setMat4("model", model);
		this->ourModel->Draw(*(this->spaceFlightModelShader));

		this->spaceFlightModelShader2->use();
		model = glm::mat4(1.0f);
		//0.0f, -48.3f, -35.0f
		model = glm::translate(model, glm::vec3(0.0f, -35.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(5.2f, 5.2f, 5.2f));
		model = glm::rotate(model, glm::radians(-1.0f * 10.0f), glm::vec3(1.0, 0.0, 0.0));
		
		this->spaceFlightModelShader2->setMat4("view", view);
		this->spaceFlightModelShader2->setMat4("projection", projection);
		this->spaceFlightModelShader2->setMat4("model", model);

		this->ourModel_space->Draw(*(this->spaceFlightModelShader2));
		//this->m_3ds->show3ds_temp(this->camera, projection,view);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		//this->m_program.use();

		//this->m_program.setMat4("projection", projection);
		//this->m_program.setMat4("view", view);
		//this->m_program.setMat4("model", model);

		//this->md2_file->Animate(1);
		md2lastFrame = now_FPS_Timer;
		//this->md2_file->UpdateVAO();
		//this->md2_file->Draw();

		// GAME UPDATE ////////////////////////////////////////////////////////////////////

		// calculate new parameters
		glm::vec3 camTarget = freeCamPos + direction;
		freeCam.setView(freeCamPos, camTarget, freeCamUp);
		glm::vec3 camPos = gameCamPos + player.getTransform().pos;
		gameCam.setView(camPos, player.getTransform().pos, gameCamUp);	// will generate a new lookAt matrix
		glm::vec3 camera_camPos = camera.getPosition() + player.getTransform().pos;
		camera.setView(camera_camPos, player.getTransform().pos, camera.getCamUp());	// will generate a new lookAt matrix
		if (1 == activeCam){
			processCharacter(this->player); // update character state based on input
			globalCam = gameCam;
		}
		if (2 == activeCam){
			processCamera();		  // camera movement input
			globalCam = freeCam;
		}
		this->md2Shader->Bind();
		// set character facing direction
		float angle = glm::angle(glm::vec3(1.0f, 0.0f, 0.0f), glm::normalize(player.getDirection()));
		if (player.getDirection().z > 0) { // for 3rd and 4th quadrants
			angle += glm::radians(180.0f);
			if (player.getDirection().x < 0.0f) {
				angle -= glm::radians(90.0f);
			}
			else if (player.getDirection().x > 0.0f) {
				angle += glm::radians(90.0f);
			}
		}
		player.getTransform().rot.y = angle;

		// collision detection. check only for player and one npc for testing purposes, dammit, this part is hard!
		collisionDetect(characters);

		//// check the state of every character
		//for (auto iter = characters.begin(); iter != characters.end(); ++iter) {
		//	if ((*iter)->getHitPoints() <= 0) {
		//		(*iter)->setAnimationState(DEATH1);
		//	}
		//}

		// set animation and apply transform. TODO: pack into function!
		int playerNumFrames = player.getModelPtr()->getModelData().numFrames;
		if (player.getState() == IDLE) {
			player.animate(1, playerNumFrames - 1, 0.009f * game_deltaTime);
			//39
		}
		if (player.getState() == RUN) {
			player.animate(40, 45, 0.009f * game_deltaTime);
			player.getTransform().pos += game_deltaTime * player.getVelocity() * player.getDirection();
		}
		if (player.getState() == ATTACK) {
			player.animate(46, 53, 0.015f * game_deltaTime);
		}
		if (player.getState() == DEATH1) {
			player.animate(178, 183, 0.009f * game_deltaTime);
		}

		static float lightPosX = 0.0f;
		static float lightPosY = 0.0f;
		myLight.getPosition().x = sin(lightPosX) * 5.0f;
		myLight.getPosition().y = sin(lightPosY) * 5.0f;
		lightPosX += 0.02f;
		lightPosY += 0.02f;

		// AI
		//npc3.setVelocity(0.006f);
		//goblin.setVelocity(0.015f);
		//computeAI(npc3, player);
		//computeAI(goblin, player);

		// END AI
		playerNumFrames = npc.getModelPtr()->getModelData().numFrames;
		npc.animate(1, playerNumFrames - 1, 0.009f * game_deltaTime);
		//cube.animate(1, 1, 0.0f);
		// ///////////////////////////////////////////////////////////////////////////////

		// RENDER ////////////////////////////////////////////////////////////////////////
		this->md2Shader->Update(myLight, player, globalCam);
		player.render();
		this->md2Shader->Update(myLight, npc, globalCam);
		npc.render();
		//this->md2Shader->Update(myLight, cube, camera);
		//cube.render();

		// compute new delta time
		GLfloat currentFrame = SDL_GetTicks();
		game_deltaTime = currentFrame - game_lastFrame;
		game_lastFrame = currentFrame;
	}

	void Game::gameProcessKeyboard(Camera_Movement direction,float deltaTime)
	{
		camera.ProcessKeyboard(direction, deltaTime);
	}

	void Game::gameProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
	}

	void Game::activeCamera(int active)
	{
		activeCam = active;
	}

	void Game::setGameKeyCodes(int scancode,int keyCodeScanCode)
	{
		keycodes[scancode] = keyCodeScanCode;
	}

	void Game::processCharacter(Character& character)
	{
		bool keyHit = false;
		int sym = 0;
		Camera_Movement direction = Camera_Movement::FORWARD;
		glm::vec3 newDirection(0.0f, 0.0f, 0.0f);
		if (keycodes[SDL_SCANCODE_D]) {							// right
			character.setAnimationState(RUN);
			newDirection += glm::vec3(0.001f, 0.0f, 0.0f);
			direction = Camera_Movement::RIGHT;
			gameProcessKeyboard(direction, game_deltaTime * 0.01);
			keyHit = true;
		}
		if (keycodes[SDL_SCANCODE_A]) {							// left
			character.setAnimationState(RUN);
			newDirection += glm::vec3(-0.001f, 0.0f, 0.0f);
			keyHit = true;
			direction = Camera_Movement::LEFT;
			gameProcessKeyboard(direction, game_deltaTime * 0.01);
		}
		if (keycodes[SDL_SCANCODE_W]) {							// forward
			character.setAnimationState(RUN);
			newDirection += glm::vec3(0.0f, 0.0f, -0.001f);
			keyHit = true;
			direction = Camera_Movement::FORWARD;
			gameProcessKeyboard(direction, game_deltaTime * 0.01);
		}
		if (keycodes[SDL_SCANCODE_S]) {							// backwards
			character.setAnimationState(RUN);
			newDirection += glm::vec3(0.0f, 0.0f, 0.001f);
			keyHit = true;
			direction = Camera_Movement::BACKWARD;
			gameProcessKeyboard(direction, game_deltaTime * 0.01);
		}
		if (keycodes[SDL_SCANCODE_SPACE]) {
			character.setAnimationState(ATTACK);
			newDirection = character.getDirection();
			keyHit = true;
		}
		if (!keyHit) {
			character.setAnimationState(IDLE);
			newDirection = character.getDirection();
		}

		//newDirection = glm::normalize(newDirection);
		character.setDirection(newDirection);
	}

	// function that moves the camera
	void Game::processCamera() {
		if (keycodes[SDL_SCANCODE_W]) {
			freeCamPos += direction * game_deltaTime * 0.05f;
		}
		if (keycodes[SDL_SCANCODE_S]) {
			freeCamPos -= direction * game_deltaTime * 0.05f;
		}
		if (keycodes[SDL_SCANCODE_A]) {
			freeCamPos -= glm::normalize(glm::cross(direction, freeCamUp)) * game_deltaTime * 0.05f;
		}
		if (keycodes[SDL_SCANCODE_D]) {
			freeCamPos += glm::normalize(glm::cross(direction, freeCamUp)) * game_deltaTime * 0.05f;
		}
		// pitch
		if (keycodes[SDL_SCANCODE_UP]) {
			pitch += 0.05f * game_deltaTime;
			if (pitch > 89.0f)
				pitch = 89.0f;
		}
		if (keycodes[SDL_SCANCODE_DOWN]) {
			pitch -= 0.05f * game_deltaTime;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}
		// yaw
		if (keycodes[SDL_SCANCODE_LEFT]) {
			yaw -= 0.05f * game_deltaTime;
		}
		if (keycodes[SDL_SCANCODE_RIGHT]) {
			yaw += 0.05f * game_deltaTime;
		}

		// update pitch and yaw
		direction.x = glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
		direction.y = glm::sin(glm::radians(pitch));
		direction.z = glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));
	}
}
