#include <iostream>
#include <SDL3/SDL.h>
#include "chip8.h"


const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int SCALE = 20;

bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}
	window = SDL_CreateWindow("CHIP8 Emulator v1", DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE, SDL_WINDOW_OPENGL);
	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}
	renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		return false;
	}
	return true;
}

void drawDisplay(SDL_Renderer* renderer, uint8_t* gfx) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
		for (int x = 0; x < DISPLAY_WIDTH; ++x) {
			if (gfx[y * DISPLAY_WIDTH + x]) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_FRect rect = { x * SCALE, y * SCALE, SCALE, SCALE };
				SDL_RenderFillRect(renderer, &rect);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

void main(int argc, char *argv[]) {
	std::cout << "CHIP8 Emulator v1" << std::endl;
	CHIP8 chip8;
	const char* romPath = argv[1];
	chip8.loadRom(romPath);
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	if (!initSDL(window, renderer)) {
		return;
	}
	SDL_Event event;
	bool running = true;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}
		uint8_t keyState[16] = { 0 };
		const bool* keys = SDL_GetKeyboardState(NULL);
		keyState[0x1] = keys[SDL_SCANCODE_1];
		keyState[0x2] = keys[SDL_SCANCODE_2];
		keyState[0x3] = keys[SDL_SCANCODE_3];
		keyState[0xC] = keys[SDL_SCANCODE_4];
		keyState[0x4] = keys[SDL_SCANCODE_Q];
		keyState[0x5] = keys[SDL_SCANCODE_W];
		keyState[0x6] = keys[SDL_SCANCODE_E];
		keyState[0xD] = keys[SDL_SCANCODE_R];
		keyState[0x7] = keys[SDL_SCANCODE_A];
		keyState[0x8] = keys[SDL_SCANCODE_S];
		keyState[0x9] = keys[SDL_SCANCODE_D];
		keyState[0xE] = keys[SDL_SCANCODE_F];
		keyState[0xA] = keys[SDL_SCANCODE_Z];
		keyState[0x0] = keys[SDL_SCANCODE_X];
		keyState[0xB] = keys[SDL_SCANCODE_C];
		keyState[0xF] = keys[SDL_SCANCODE_V];
		chip8.setKeys(keyState);
		
		chip8.emulateCycle();
		if(chip8.drawFlag) {
			chip8.drawFlag = false;
			drawDisplay(renderer, chip8.gfx);
		}
		SDL_Delay(16); // ~60 FPS
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	std::cout << "Exiting..." << std::endl;
}