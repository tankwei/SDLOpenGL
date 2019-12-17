#ifndef GAME_H
#define GAME_H

#include "dll.h"

#include <memory>
#include "SkyBox.h"
#include <util/model.h>

#include "AnimationShader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Transform.h"
#include <util/camera.h>
#include "Light.h"
//#include "ObjLoader.h"
//#include "ObjLoader2.h"
#include "md2Model.h"
#include "Character.h"
#include "AnimatedEntity.h"
#include <sdl_util/Renderer.h>

class SkyBox;
class Map;

namespace game
{
	class Game;

	typedef std::shared_ptr<Game> GamePtr;

	class Game
	{
	public:
		static  GamePtr create();//GAME_DLL

		virtual ~Game();
		void Init(unsigned int screen_width, unsigned int screen_height, Renderer *appRenderer,const char *base_path);
		void Render();
		void gameProcessKeyboard(Camera_Movement direction, float deltaTime);
		void gameProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
		void activeCamera(int active);
		void setGameKeyCodes(int scancode,int keyCodeScanCode);
	private:
		unsigned int activeCam; // 1: game cam, 2: free cam
		unsigned int keycodes[SDL_NUM_SCANCODES];
		unsigned int scr_width;
		unsigned int scr_height;
		SkyBox skyBox;
		// camera
		Map *map;
		//CLoad3DS *m_3ds;
		Model *ourModel;
		Shader *spaceFlightModelShader;
		Model *ourModel_space;
		Shader *spaceFlightModelShader2;
		//md2File *md2_file;
		//md2Model::md2_model_t*	model[2];
		//md2Model *md2_model;
		Shader *m_program;

		AnimationShader *md2Shader;
		
	
		Texture *container_texture;

		md2Model *player_model;
		md2Model *enemy_model;
		md2Model *pknight_weapon_model;
		md2Model *enemyWeapon_model;
		md2Model *cube_model;


		Camera gameCam;
		Camera freeCam;

		/* create a REntity, the final representation of any renderable object in the game */
		Character player;

		/* NPCs */
		Character npc;

		std::vector<Character*> characters;

		Weapon player_weapon;
		Weapon enemey_weapon;

		Character cube;
		/*cube.setModel(&cube_model);
		cube.setTexture(&container_texture);*/
	private:
		void processCharacter(Character& character);
		void processCamera();
	};
};

#endif // !GAME_H