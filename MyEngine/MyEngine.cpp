// MyEngine.cpp: definisce il punto di ingresso dell'applicazione.
//

#include "MyEngine.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


using namespace std;

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
};

void cleanup(SDLState &state);

int main(int argc, char* argv[])
{
	SDLState state;
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to initialize SDL", nullptr);
		return 1;
	}
	int width = 800;
	int height = 600;
	state.window = SDL_CreateWindow("MyEngine", width, height, 0);
	if (!state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to create window", nullptr);
		cleanup(state);
		return 1;
	}

	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if(!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to create renderer", nullptr);
		cleanup(state);
		return 1;
	}
	bool running = true;
	while (running) {

		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}
		//Perform drawing operations
		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		SDL_RenderClear(state.renderer);
		SDL_RenderPresent(state.renderer);
	}

	cleanup(state);
	return 0;
}

void cleanup(SDLState &state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}
