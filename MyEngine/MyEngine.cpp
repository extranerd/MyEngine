// MyEngine.cpp: definisce il punto di ingresso dell'applicazione.
//

#include "MyEngine.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


using namespace std;

void cleanup(SDL_Window* win);

int main(int argc, char* argv[])
{
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to initialize SDL", nullptr);
		return 1;
	}
	int width = 800;
	int height = 600;
	SDL_Window* win = SDL_CreateWindow("MyEngine", width, height, 0);
	if (!win) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to create window", nullptr);
		cleanup(win);
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

	}

	cleanup(win);
	return 0;
}

void cleanup(SDL_Window* win)
{
	SDL_DestroyWindow(win);
	SDL_Quit();
}
