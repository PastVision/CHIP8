#include "chip8.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

CHIP8::CHIP8() {
	initialize();
}

void CHIP8::initialize() {
	opcode = 0; // Point to start of memory
	PC = 0x200; // Leaving Interpreter space
	I = 0; // Index Register = 0
	SP = 0; // Point to base of stack
	memset(memory, 0, sizeof(memory));
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(gfx, 0, sizeof(gfx));
	memset(keypad, 0, sizeof(keypad));
	delay_timer = 0;
	sound_timer = 0;
	drawFlag = false;
	loadFont();
}

void CHIP8::loadRom(const char* filename) {
	std::ifstream rom(filename, std::ios::binary | std::ios::in);
	if (!rom) exit(1);
	rom.seekg(0, std::ios::end);
	std::streamsize romSize = rom.tellg();
	rom.seekg(0, std::ios::beg);
	rom.read((char*)(memory + 0x200), romSize);
	std::cout << "Loaded ROM: " << filename << " Size: " << romSize << " bytes" << std::endl;
}

uint8_t CHIP8::getDelay() {
	return delay_timer;
}

void CHIP8::setDelay(uint8_t newValue) {
	delay_timer = newValue;
}

void CHIP8::setSound(uint8_t newValue) {
	sound_timer = newValue;
}

void CHIP8::setKeys(uint8_t* keyState) {
	for(int i = 0; i < 16; i++) {
		keypad[i] = keyState[i];
	}
}

void CHIP8::fetchOpcode(){
	opcode = memory[PC] << 8 | memory[PC + 1]; // Fetch opcode from memory
	PC += 2; // Increment program counter by 2 bytes
}

void CHIP8::executeOpcode() { // Decode and execute opcode - Reference: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x00FF)
		{
		case 0x00E0:
			memset(gfx, 0, sizeof(gfx));
			drawFlag = true;
			break;
		case 0x00EE:
			--SP;
			PC = stack[SP];
			break;
		default:
			break;
		}
		break;
	case 0x1000:
		PC = opcode & 0x0FFF;
		break;
	case 0x2000:
		stack[SP] = PC;
		++SP;
		PC = opcode & 0x0FFF;
		break;
	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			PC += 2;
		break;
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			PC += 2;
		break;
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			PC += 2;
		break;
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		break;
	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		break;
	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0004:
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0005:
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0006:
			V[(opcode & 0x0F00) >> 8] >>= V[(opcode & 0x00F0) >> 4];
			break;
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			break;
		case 0x000E:
			V[(opcode & 0x0F00) >> 8] <<= V[(opcode & 0x00F0) >> 4];
			break;
		default:
			break;
		}
		break;
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			PC += 2;
		break;
	case 0xA000:
		I = opcode & 0x0FFF;
		break;
	case 0xB000:
		PC = V[0] + (opcode & 0x0FFF);
		break;
	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
		break;
	case 0xD000: {
		uint8_t x = V[(opcode & 0x0F00) >> 8];
		uint8_t y = V[(opcode & 0x00F0) >> 4];
		uint8_t height = opcode & 0x000F;
		V[0xF] = 0;
		for (int row = 0; row < height; ++row) {
			uint8_t sprite = memory[I + row];
			for (int col = 0; col < 8; ++col) {
				if ((sprite & (0x80 >> col)) != 0) {
					int px = (x + col) % 64;
					int py = (y + row) % 32;
					int idx = px + (py * 64);
					if (gfx[idx] == 1)
						V[0xF] = 1;
					gfx[idx] ^= 1;
				}
			}
		}
		drawFlag = true;
		break;
	}
	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E:
			if (keypad[V[(opcode & 0x0F00) >> 8]])
				PC += 2;
			break;
		case 0x00A1:
			if (!keypad[V[(opcode & 0x0F00) >> 8]])
				PC += 2;
			break;
		default:
			break;
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF){
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = getDelay();
			break;
		case 0x000A: {
			bool keyPressed = false;
			for (int i = 0; i < 16; ++i) {
				if (keypad[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					keyPressed = true;
				}
			}
			if (!keyPressed)
				PC -= 2;
		}
			break;
		case 0x0015:
			setDelay(V[(opcode & 0x0F00) >> 8]);
			break;
		case 0x0018:
			setSound(V[(opcode & 0x0F00) >> 8]);
			break;
		case 0x001E:
			I += V[(opcode & 0x0F00) >> 8];
			break;
		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 5;
			break;
		case 0x0033: {
			uint8_t value = V[(opcode & 0x0F00) >> 8];
			memory[I] = value / 100;
			memory[I + 1] = (value / 10) % 10;
			memory[I + 2] = value % 10;
		}
			break;
		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];
			break;
		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];
			break;
		default:
			break;
		}
		break;
	default:
		break; // Unrecognized opcode
	} // Decode the opcode
}

void CHIP8::emulateCycle() {
	fetchOpcode(); // Fetch opcode from memory
	executeOpcode(); // Execute the opcode
	if (delay_timer > 0) {
		--delay_timer; // Decrement delay timer
	}
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			// Play Beep here (if sound is enabled)
		}
		--sound_timer; // Decrement sound timer
	}
}

void CHIP8::loadFont() {
	uint8_t fontSet[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	memcpy(memory, fontSet, 80); // Copy Font Set to mmemory in place of interpreter
}