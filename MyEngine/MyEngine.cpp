// MyEngine.cpp: definisce il punto di ingresso dell'applicazione.
//

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>
#include <format>

#include "gameobject.h"


using namespace std;

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logW, logH;
	const bool* keys;

	SDLState() : keys(SDL_GetKeyboardState(nullptr)) {}
};


const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState {

	std::array<std::vector<GameObject>, 2> layers;
	int playerIndex;

	GameState() {

		playerIndex = -1;

	}

	GameObject& player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; }

};

struct Resources {
	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUNNING = 1;
	std::vector<Animation> playerAnims;

	std::vector<SDL_Texture*> textures;
	SDL_Texture *texIdle, *texRun, *texBrick, *texGround;

	SDL_Texture* loadTexture(SDL_Renderer *renderer, const std::string& filepath) {
		SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	void load(SDLState& state) {
		playerAnims.resize(5);
		playerAnims[ANIM_PLAYER_IDLE] = Animation(3, 1.6f);
		playerAnims[ANIM_PLAYER_RUNNING] = Animation(5, 0.6f);

		texIdle = loadTexture(state.renderer, "assets/idle.png");
		texRun = loadTexture(state.renderer, "assets/running.png");
		texBrick = loadTexture(state.renderer, "assets/Brick.png");
		texGround = loadTexture(state.renderer, "assets/ground.png");
	}
	void unload() {
		for (SDL_Texture* tex : textures)
			SDL_DestroyTexture(tex);
	}
};

void cleanup(SDLState& state);
bool initialize(SDLState& state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime);
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);
void handleKeyInput(const SDLState& state, GameState& gs, GameObject& obj, SDL_Scancode key, bool keyDown);


int main(int argc, char* argv[])
{
	SDLState state;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 320;

	if (!initialize(state))
		return 1;

	// load game assets
	Resources res;
	res.load(state);
	//setup game data
	GameState gs;
	createTiles(state, gs, res);

	uint64_t prevTime = SDL_GetTicks();

	bool running = true;
	while (running) {
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f;
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					state.width = event.window.data1;
					state.height = event.window.data2;
					break;
				case SDL_EVENT_KEY_DOWN:
					handleKeyInput(state, gs, gs.player(), event.key.scancode, true);
					break;
				case SDL_EVENT_KEY_UP:
					handleKeyInput(state, gs, gs.player(), event.key.scancode, false);
					break;
				}
				
		}
		//update objects
		for (auto& layer : gs.layers)
		{

			for (GameObject& obj : layer) { update(state, gs, res, obj, deltaTime); if (obj.currentAnimation != -1) obj.animations[obj.currentAnimation].step(deltaTime); }


		}

		//Perform drawing operations
		SDL_SetRenderDrawColor(state.renderer, 49, 178, 204, 255);
		SDL_RenderClear(state.renderer);

		//draw objects
		for (auto &layer : gs.layers)
		{

			for (GameObject& obj : layer) { drawObject(state, gs, obj, deltaTime); }


		}

		//debug info
		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		SDL_RenderDebugText(state.renderer, 5, 5, std::format("State: {}", static_cast<int>(gs.player().data.player.state)).c_str());
		

		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
	}
	res.unload();
	cleanup(state);
	return 0;
}

bool initialize(SDLState& state) {
	bool initSuccess = true;
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to initialize SDL", nullptr);
		initSuccess = false;
	}
	state.window = SDL_CreateWindow("MyEngine", state.width, state.height, SDL_WINDOW_RESIZABLE);
	if (!state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to create window", nullptr);
		cleanup(state);
		initSuccess = false;
	}

	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to create renderer", nullptr);
		cleanup(state);
		initSuccess = false;
	}

	//configure presentation

	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	return initSuccess;

}
void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}

void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime) {

	const float spriteSize = 32;
	float srcX = obj.currentAnimation != -1
		? obj.animations[obj.currentAnimation].currentFrame() * spriteSize : 0.0f;

	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = spriteSize,
		.h = spriteSize
	};

	SDL_FRect dst{
		.x = obj.position.x,
		.y = obj.position.y,
		.w = spriteSize,
		.h = spriteSize
	};
	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
}
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
	if(obj.dynamic)
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	if (obj.type == ObjectType::player) {
		
		float currDir = 0;
		if (state.keys[SDL_SCANCODE_A])
			currDir += -1;
		if (state.keys[SDL_SCANCODE_D])
			currDir += 1;
		if (currDir != 0) obj.direction = currDir;

		switch (obj.data.player.state) {

		case PlayerState::idle: {
			if (currDir) { 
				obj.data.player.state = PlayerState::running; 
			}
			else { 
				if (obj.velocity.x) {
					const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
					float amount = factor * obj.acceleration.x * deltaTime;	
					if (std::abs(obj.velocity.x) < std::abs(amount)) obj.velocity.x = 0;
					else obj.velocity.x += amount;
				}
			}
			obj.texture = res.texIdle;
			obj.currentAnimation = res.ANIM_PLAYER_IDLE;
			break;
		}
		case PlayerState::running: { 
			if (currDir == 0) { obj.data.player.state = PlayerState::idle; }
			obj.texture = res.texRun;
			obj.currentAnimation = res.ANIM_PLAYER_RUNNING;
			break;
		}
		case PlayerState::jumping: {

			obj.texture = res.texRun;
			obj.currentAnimation = res.ANIM_PLAYER_RUNNING;

			break;

		}
		}
		
		//add acc
		obj.velocity += currDir * obj.acceleration * deltaTime;
		if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
			obj.velocity.x = obj.maxSpeedX * currDir;
		}
		
		

	}
	//add vel
	obj.position += obj.velocity * deltaTime;

	bool foundGround = false;

	//handle collision detection
	for(auto &layer : gs.layers)
		for (GameObject& objB : layer) {

			if (&obj != &objB) {
				checkCollision(state, gs, res, obj, objB, deltaTime);
				if (obj.velocity.y >= 0.0f) {
					SDL_FRect sensor{
						.x = obj.position.x + obj.collider.x,
						.y = obj.position.y + obj.collider.y + obj.collider.h,
						.w = obj.collider.w,
						.h = 1
					};
					SDL_FRect rectB{
						.x = objB.position.x + objB.collider.x,
						.y = objB.position.y + objB.collider.y,
						.w = objB.collider.w,
						.h = objB.collider.h
					};
					if (SDL_HasRectIntersectionFloat(&sensor, &rectB)) {
						foundGround = true;
					}
				}
			}

			

		}
	//switching grounded state

	obj.grounded = foundGround;

	if (obj.type == ObjectType::player) {
		// Se tocchiamo terra, usciamo dallo stato di salto
		if (obj.grounded && obj.data.player.state == PlayerState::jumping) {
			obj.data.player.state = PlayerState::running;
		}
	}
}
void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& rectC, GameObject& a, GameObject& b, float deltaTime) {
	if (a.type == ObjectType::player) {
		switch (b.type) {
		case ObjectType::level:
		{
			if (rectC.w < rectC.h) {
				
				if (a.velocity.x > 0 && rectA.x < rectB.x) {
					a.position.x -= rectC.w;
					a.velocity.x = 0;
				}
				else if (a.velocity.x < 0 && rectA.x > rectB.x) {
					a.position.x += rectC.w;
					a.velocity.x = 0;
				}
			}
			else {
				
				float playerCenterY = rectA.y + rectA.h * 0.5f;
				float tileCenterY = rectB.y + rectB.h * 0.5f;

				if (playerCenterY < tileCenterY) {
					
					if (a.velocity.y >= 0) {
						a.position.y -= rectC.h;
						a.velocity.y = 0;
					}
					
				}
				else {
					if (a.velocity.y < 0) {
						a.position.y += rectC.h;
						a.velocity.y = 0;
					}
				}
			}
			break;
		}
		}
	}
}
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject &b, float deltaTime) {

	SDL_FRect rectA{
		.x = a.position.x + a.collider.x, .y = a.position.y + a.collider.y,
		.w = a.collider.w, .h = a.collider.h
	};
	SDL_FRect rectB{
		.x = b.position.x + b.collider.x, .y = b.position.y + b.collider.y,
		.w = b.collider.w, .h = b.collider.h
	};
	SDL_FRect rectC{ 0 };

	if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC)) {

		collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltaTime);

	}
}
void createTiles(const SDLState& state, GameState& gs, const Resources& res) {

	short map[MAP_ROWS][MAP_COLS] = {

		/*
		 1 - Ground
		 2 - Panel
		 3 - Enemy
		 4 - Player
		 5 - Grass
		 6 - Brick
		*/

		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	const auto createObject = [&state](int r, int c, SDL_Texture* tex, ObjectType type)
		{
			GameObject o;
			o.type = type;
			o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
			o.texture = tex;
			o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE };
			return o;

		};
	for (int r = 0; r < MAP_ROWS; r++) {

		for (int c = 0; c < MAP_COLS; c++) {
			switch (map[r][c])
			{
				case 1: //Ground
				{

					GameObject o = createObject(r, c, res.texGround, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(o);
					break;
				}
				case 4: //Player
				{
					GameObject player = createObject(r, c, res.texIdle, ObjectType::player);
					player.data.player = PlayerData();
					player.animations = res.playerAnims;
					player.currentAnimation = res.ANIM_PLAYER_IDLE;
					player.acceleration = glm::vec2(300, 0);
					player.maxSpeedX = 100;
					player.dynamic = true;
					player.collider = { .x = 10, .y = 7, .w = 10, .h = 24 };
					gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
					gs.playerIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;
					break;
				}
			}
		
		}

	}
	assert(gs.playerIndex != -1);

}

void handleKeyInput(const SDLState& state, GameState& gs, GameObject& obj, SDL_Scancode key, bool keyDown) {

	if (obj.type == ObjectType::player) {
		const float JUMP_FORCE = -200.0f;
		switch (obj.data.player.state) {

			case PlayerState::idle:
				if (key == SDL_SCANCODE_SPACE && keyDown) { obj.data.player.state = PlayerState::jumping; obj.velocity.y = JUMP_FORCE; }
				break;
			case PlayerState::running:
				if (key == SDL_SCANCODE_SPACE && keyDown) { obj.data.player.state = PlayerState::jumping; obj.velocity.y = JUMP_FORCE; }
				break;


		}

	}

}