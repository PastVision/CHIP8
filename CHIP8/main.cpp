#include <iostream>
#include "SDL.h"
#include "chip8.h"


const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int SCALE = 10;

bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}
	window = SDL_CreateWindow("CHIP8 Emulator v1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE, SDL_WINDOW_VULKAN);
	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		return false;
	}
	return true;
}

void drawDisplay(SDL_Renderer* renderer, uint8_t* gfx) {
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	uint32_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
	for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
		for (int x = 0; x < DISPLAY_WIDTH; ++x) {
			pixels[y * DISPLAY_WIDTH + x] = gfx[y * DISPLAY_WIDTH + x] ? 0x00FF00FF : 0x000000FF;
		}
	}
	SDL_UpdateTexture(texture, NULL, pixels, DISPLAY_WIDTH * sizeof(uint32_t));
	SDL_RenderClear(renderer);
	SDL_Rect destRect = { 0, 0, DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE };
	SDL_RenderCopy(renderer, texture, NULL, &destRect);
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
	std::cout << "CHIP8 Emulator v1" << std::endl;
	CHIP8 chip8;
	char* romPath = argv[1];
	if (argv[1] == nullptr) {
		std::cerr << "Usage: " << argv[0] << " <path_to_rom>" << std::endl;
		return 1;
	}
	chip8.loadRom(romPath);
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	if (!initSDL(window, renderer)) {
		return 2;
	}
	SDL_Event event;
	bool running = true;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}
		uint8_t keyState[16] = { 0 };
		const uint8_t* keys = SDL_GetKeyboardState(NULL);
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
		for (int i = 0; i < 10; ++i) { // Emulate 10 cycles for smoother timing
			chip8.emulateCycle();
		}
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
	return 0;
}