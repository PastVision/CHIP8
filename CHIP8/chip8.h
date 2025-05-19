#pragma once
#include <cstdint>

class CHIP8
{
public:
	bool drawFlag; // Flag to indicate if the screen needs to be redrawn
	uint8_t gfx[64 * 32]; // Graphics (64x32 pixels) 1 bit per pixel
	
	CHIP8();
	void loadRom(const char* filename); // Open a ROM file and load it into memory
	void setKeys(uint8_t* keyState); // Set keyboard
	void emulateCycle(); // Emulate one cycle of the CHIP-8 CPU
	uint8_t getDelay(); // Getter for DelayTimer
	void setDelay(uint8_t newValue); // Setter for DelayTimer
	void setSound(uint8_t newValue); // Setter for SoundTimer

private:
	uint8_t memory[4096]; // Memory (4KB)
	uint8_t V[16]; // V registers (V0 to VF) 8 bit each
	uint16_t I; // Index register 16 bit
	uint16_t PC; // Program counter 16 bit
	uint16_t stack[16]; // Stack 16 bit 16 levels
	uint8_t SP; // Stack pointer 8 bit
	uint8_t delay_timer; // Delay timer 8 bit
	uint8_t sound_timer; // Sound timer 8 bit
	uint8_t keypad[16]; // Keypad (0x0 to 0xF) 16 keys
	uint16_t opcode; // Current opcode 16 bit

	void fetchOpcode(); // Fetch the current opcode
	void executeOpcode(); // Execute the current opcode
	void loadFont(); // Load font during initialize
	void initialize(); // Initialize the CHIP-8 system
};
