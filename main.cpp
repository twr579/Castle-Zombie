// main.cpp
// Runs on LM4F120/TM4C123
// Tim Reynolds
// Last Modified: 5/4/2020

/* 
 */
// *******Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// move button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Sound.h"
#include "print.h"

SlidePot my(165,20);

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

// Creating a struct for the Sprite.
typedef enum {dead,alive} status_t;
struct sprite{
  uint32_t x;      // x coordinate
  uint32_t y;      // y coordinate
  const unsigned short *image; // ptr->image
  status_t life;            // dead/alive
};          
typedef struct sprite sprite_t;

sprite_t zomb1s = {33, 125, smallzombie, dead};
sprite_t zomb2s = {54, 125, smallzombie, dead};
sprite_t zomb3s = {75, 125, smallzombie, dead};
sprite_t zomb1m = {20, 152, smallzombie, dead};
sprite_t zomb2m = {50, 152, smallzombie, dead};
sprite_t zomb3m = {80, 152, smallzombie, dead};
sprite_t smallt = {33, 98, smalltorch, dead};
sprite_t mediumt = {18, 101, mediumtorch, dead};
sprite_t larget = {3, 105, largetorch, alive};
sprite_t heart1 = {42, 16, heart, alive};
sprite_t heart2 = {50, 16, heart, alive};
sprite_t heart3 = {58, 16, heart, alive};

typedef enum {English, Spanish} Language_t;
Language_t myL;
typedef enum {LANGUAGE, TITLE, PRESS, SCORE, LIVES, GO, ZOMBIES, GAMEOVER} phrase_t;
const char Language_English[] ="English";
const char Language_Spanish[] ="Espa\xA4ol";
const char Title_English[] ="\x7B\x7C\x7D\x7E\x7F\x80 \x81\x82\x83\x84\x85\x86";
const char Title_Spanish[] ="\x7B\x7C\x7D\x7E\x87\x7F\x7F\x88 \x81\x82\x83\x84\x85";
const char Press_English[] ="PRESS";
const char Press_Spanish[] ="PRESIONA";
const char Score_English[] ="SCORE:";
const char Score_Spanish[] ="PUNTAJE:";
const char Lives_English[] ="LIVES:";
const char Lives_Spanish[] ="VIDAS:";
const char Go_English[] ="GO!";
const char Go_Spanish[] ="\xADVAMOS!";
const char Zombies_English[] ="ZOMBIES!";
const char Zombies_Spanish[] ="\xADZOMBIS!";
const char GameOver_English[] ="\x89\x8A\x83\x86 \x82\x8B\x86\x8C";
const char GameOver_Spanish[] ="\x8D\x8E\x86\x89\x82 \x8F\x86\x8C\x83\x85\x90\x8A\x91\x82";
const char *Phrases[8][2]={
	{Language_English,Language_Spanish},
  {Title_English,Title_Spanish},
  {Press_English,Press_Spanish},
	{Score_English,Score_Spanish},
	{Lives_English,Lives_Spanish},
	{Go_English,Go_Spanish},
	{Zombies_English,Zombies_Spanish},
	{GameOver_English,GameOver_Spanish},
};



volatile uint32_t flag; // semaphore for interrupt syncing
uint8_t hearts = 3; // start with 3 hearts
uint32_t score = 0; // initialize score to 0
uint8_t move = 0; // 'move' gamemode initially disabled
uint8_t zombie = 0; // 'zombie' gamemode initially disabled
int32_t count; // count for Timer1 interrupt

// If move or zombie are enabled, this interrupt handler will update the status of sprites. It can also be used as a timer.
void graphics(void){
	GPIO_PORTF_DATA_R ^= 0x02; // toggle PF1
	flag = 1; // set flag
	if (move == 1) { // if player is moving
		if (count % 10 == 0) {
		Sound_Footstep(); // play footstep sound
		score += 10; // increment score by 10
		if (smallt.life == alive) { // if small torch is alive, set it to dead and set the next torch, medium, to alive
			smallt.life = dead;
			mediumt.life = alive;
			larget.life = dead;
		} else if (mediumt.life == alive) { // if medium torch is alive, set it to dead and set the next torch, large, to alive
			smallt.life = dead;
			mediumt.life = dead;
			larget.life = alive;
		} else { // if large torch is alive, set it to dead and set the next torch, small, to alive
			larget.life = dead;
			mediumt.life = dead;
			smallt.life = alive;
		}
	}
}	else if (zombie == 1) { // if there are zombies
		if (count == 50 || count == 25) {
			Sound_Zombie(); // play zombie sound at 0s and 2.5s
		}
		if (count <= 25) { // zombies get closer after 2.5s
			if (zomb1s.life == alive) { // set zombie 1 to medium
				zomb1s.life = dead;
				zomb1m.life = alive;
			}
			if (zomb2s.life == alive) { // set zombie 2 to medium
				zomb2s.life = dead;
				zomb2m.life = alive;
			}
			if (zomb3s.life == alive) { // set zombie 3 to medium
				zomb3s.life = dead;
				zomb3m.life = alive;
			}
		}
	}
	
	if (count <= 0) {
			NVIC_DIS0_R = 1<<21; // disable interrupt
		}
	count--; // decrement counter
}

// This interrupt handler updates ADC value every 0.1sec as the slide pot moves
void SysTick_Handler(void) {
	GPIO_PORTF_DATA_R ^= 0x04; // toggle PF2
	my.Save(ADC_In()); // update ADC value
}

void SysTick_Init(unsigned long period) { 
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0xA0000000; // priority 5          
  NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
}

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
	volatile int nop;
	SYSCTL_RCGCGPIO_R |= 0x20; // enable Port F clock
	nop = 0;
	GPIO_PORTF_DEN_R |= 0x0E; // enable digital for PF1, PF2, PF3
	GPIO_PORTF_DIR_R |= 0x0E; // set PF1, PF2, PF3 as outputs
}

// This select language screen will wait for user input and set the language based on the button they press
void selectLanguage(void) {
	ST7735_SetCursor(1, 1);
	ST7735_SetTextColor(ST7735_WHITE);
	ST7735_OutString((char*)"Select Language:");
	ST7735_DrawBitmap(1, 38, a, 11, 11); // a button sprite
	ST7735_SetCursor(3, 3);
	ST7735_OutString((char*)Phrases[LANGUAGE][English]); // "english"
	ST7735_DrawBitmap(1, 78, b, 11, 11); // b button sprite
	ST7735_SetCursor(3, 7);
	ST7735_OutString((char*)Phrases[LANGUAGE][Spanish]); // "espanol"

	while (GPIO_PORTE_DATA_R == 0) {}; // wait for button press
	while (GPIO_PORTE_DATA_R == 0x01) {
		myL = English; // Button0 pressed = English
	}
	while (GPIO_PORTE_DATA_R == 0x02) {
		myL = Spanish; // Button1 pressed = Spanish
	}
}

// This title screen will wait for the user to press the 'A' button before starting the game
void titleScreen(void) {
	ST7735_FillScreen(ST7735_BLACK); // clear screen
	ST7735_SetCursor(1, 1);
	ST7735_SetTextColor(ST7735_RED);
	ST7735_OutString((char*)Phrases[TITLE][myL]); // print game title
	ST7735_SetCursor(1, 3);
	ST7735_OutString((char*)Phrases[PRESS][myL]); // print "press"
	ST7735_DrawBitmap(60, 38, a, 11, 11); // print a button sprite
	ST7735_DrawBitmap(53, 140, mediumzombie, 29, 79); // print medium zombie in bottom middle of screen
	while (GPIO_PORTE_DATA_R != 0x01) {}; // wait for button press
	while (GPIO_PORTE_DATA_R == 0x01) {}; // wait for button release
}

// This function will perform the 'move' part of the game
void movetime(void) {
	ST7735_FillScreen(ST7735_BLACK); // clear screen
	ST7735_SetCursor(13, 1);
	ST7735_SetTextColor(ST7735_RED);
	ST7735_OutString((char*)Phrases[GO][myL]); // print "go" message
	ST7735_SetCursor(1, 0);
	ST7735_SetTextColor(ST7735_WHITE);
	ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
	LCD_OutDec(score);
	ST7735_SetCursor(1, 1);
	ST7735_OutString((char*)Phrases[LIVES][myL]); // print lives
	if (heart1.life == alive) { // print heart 1 if alive
		ST7735_DrawBitmap(heart1.x, heart1.y, heart1.image, 7, 7);
	}
	if (heart2.life == alive) { // print heart 2 if alive
		ST7735_DrawBitmap(heart2.x, heart2.y, heart2.image, 7, 7);
	}
	if (heart3.life == alive) { // print heart 3 if alive
		ST7735_DrawBitmap(heart3.x, heart3.y, heart3.image, 7, 7);
	}
	ST7735_DrawBitmap(2, 157, movebackground, 124, 138); // print move background
	larget.life = alive;
	ST7735_DrawBitmap(larget.x, larget.y, larget.image, 14, 49); // print large torch
	flag = 0; // initialize flag to 0
	count = Random5() * 10; // random count value of 50, 60, 70, 80, 90, or 100
	NVIC_EN0_R = 1<<21; // enable interrupts for Timer1
	while (count > 0) { 
		while(((GPIO_PORTE_DATA_R & 0x02) == 0x02) && count > 0) { // while button held
			move = 1; // set player move flag to update torch locations
			ST7735_SetCursor(1, 0);
			ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
			LCD_OutDec(score);
			int counter = 10; // only update torch locations every second (10 * 0.1s = 1s)
			while (counter > 0) {
			NVIC_EN0_R = 1<<21; // reenable interrupts in case count reaches 0
			while (flag == 0) {}; // wait for interrupt
			flag = 0; // reset flag to 0
			counter--; // decrement counter
			}
			ST7735_DrawBitmap(2, 157, movebackground, 124, 138); // print background
			if (smallt.life == alive) { // print small torch if alive
				ST7735_DrawBitmap(smallt.x, smallt.y, smallt.image, 13, 30);
			}
			if (mediumt.life == alive) { // print medium torch if alive
				ST7735_DrawBitmap(mediumt.x, mediumt.y, mediumt.image, 14, 36);
			}
			if (larget.life == alive) { // print large torch if alive
				ST7735_DrawBitmap(larget.x, larget.y, larget.image, 14, 49);
			}
		}
		move = 0; // disable move interrupts when button is not being held
	}
	while (larget.life == dead) { // if large torch is not on the screen, reenable interrupts until it is
		move = 1;
		NVIC_EN0_R = 1<<21; // reenable interrupt to print torches until large torch is on screen
		ST7735_SetCursor(1, 0);
		ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
		LCD_OutDec(score);
		int counter = 10; // only update torch locations every second
		while (counter > 0) {
		NVIC_EN0_R = 1<<21;
		while (flag == 0) {}; // wait for interrupt
			flag = 0; // reset flag
			counter--; // decrement counter
		}
			ST7735_DrawBitmap(2, 157, movebackground, 124, 138); // draw background
		if (mediumt.life == alive) { // print medium torch if alive
				ST7735_DrawBitmap(mediumt.x, mediumt.y, mediumt.image, 14, 36);
			}
		}
	move = 0; // disable "move" section of Timer1 interrupt
	ST7735_SetCursor(1, 0);
	ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
	LCD_OutDec(score);
	ST7735_DrawBitmap(larget.x, larget.y, larget.image, 14, 49); // print large torch
}

// This function will perform the 'zombie' part of the game
void zombietime(void) {
	ST7735_FillScreen(ST7735_BLACK); // clear screen
	ST7735_SetCursor(12, 1);
	ST7735_SetTextColor(ST7735_RED);
	ST7735_OutString((char*)Phrases[ZOMBIES][myL]); // print "zombies" message
	ST7735_SetCursor(1, 0);
	ST7735_SetTextColor(ST7735_WHITE);
	ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
	LCD_OutDec(score);
	ST7735_SetCursor(1, 1);
	ST7735_OutString((char*)Phrases[LIVES][myL]); // print lives
	if (heart1.life == alive) { // print heart 1 if alive
		ST7735_DrawBitmap(heart1.x, heart1.y, heart1.image, 7, 7);
	}
	if (heart2.life == alive) { // print heart 2 if alive
		ST7735_DrawBitmap(heart2.x, heart2.y, heart2.image, 7, 7);
	}
	if (heart3.life == alive) { // print heart 3 if alive
		ST7735_DrawBitmap(heart3.x, heart3.y, heart3.image, 7, 7);
	}
	ST7735_DrawBitmap(2, 157, zombiebackground, 124, 138); // print zombie background
	flag = 0; // initialize flag to 0
	count = 50; // 5 seconds
	uint8_t zombinit = Random7(); // random number between 1-7
	if ((zombinit & 0x01) == 1) { // zombie 1 represented by bit 0 of zombinit
		zomb1s.life = alive;
	}
	if ((zombinit & 0x02)>>1 == 1) { // zombie 2 represented by bit 1 of zombinit
		zomb2s.life = alive;
	}
	if ((zombinit & 0x04)>>2 == 1) { // zombie 3 represented by bit 2 of zombinit
		zomb3s.life = alive;
	}
	zombie = 1; // enable 'zombie' gamemode for Timer1 interrupt
	uint8_t previous = 0; // previous input value initially 0
	NVIC_EN0_R = 1<<21; // enable interrupts for Timer1
	while (((zomb1s.life == alive || zomb2s.life == alive || zomb3s.life == alive) || (zomb1m.life == alive || zomb2m.life == alive || zomb3m.life == alive)) && count > 0) {
		ST7735_SetCursor(1, 0);
		ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
		LCD_OutDec(score);
		while (flag == 0) {}; // wait for interrupt
		flag = 0; // reset flag to 0
		ST7735_DrawBitmap(2, 157, zombiebackground, 124, 138); // print background
		if (zomb1s.life == alive) { // print zombie 1 if alive and small
			ST7735_DrawBitmap(zomb1s.x, zomb1s.y, smallzombie, 19, 46);
		}
		if (zomb2s.life == alive) { // print zombie 2 if alive and small
			ST7735_DrawBitmap(zomb2s.x, zomb2s.y, smallzombie, 19, 46);
		}
		if (zomb3s.life == alive) { // print zombie 3 if alive and small
			ST7735_DrawBitmap(zomb3s.x, zomb3s.y, smallzombie, 19, 46);
		}
		if (zomb1m.life == alive) { // print zombie 1 if alive and medium
			ST7735_DrawBitmap(zomb1m.x, zomb1m.y, mediumzombie, 29, 79);
		}
		if (zomb2m.life == alive) { // print zombie 2 if alive and medium
			ST7735_DrawBitmap(zomb2m.x, zomb2m.y, mediumzombie, 29, 79);
		}
		if (zomb3m.life == alive) { // print zombie 3 if alive and medium
			ST7735_DrawBitmap(zomb3m.x, zomb3m.y, mediumzombie, 29, 79);
		}
		
		my.Sync(); // wait for SysTick interrupt to update distance value
		ST7735_DrawBitmap(my.Distance(), 85, reticle, 5, 1); // draw horizontal component of reticle
		ST7735_DrawBitmap(my.Distance() + 2, 87, reticle, 1, 5); // draw vertical component of reticle
		uint8_t input = GPIO_PORTE_DATA_R; // read button input
		if (input == 0x01 && (previous == 0)) { // initial press of 'A' button
			Sound_Gun(); // play gunshot sound effect
			if (zomb1s.life == alive && (my.Distance() > 35) && (my.Distance() < 43)) { // small zombie 1 valid kill
				zomb1s.life = dead;
				score += 100;
			}
			if (zomb2s.life == alive && (my.Distance() > 56) && (my.Distance() < 64)) { // small zombie 2 valid kill
				zomb2s.life = dead;
				score += 100;
			}
			if (zomb3s.life == alive && (my.Distance() > 77) && (my.Distance() < 85)) { // small zombie 3 valid kill
				zomb3s.life = dead;
				score += 100;
			}
			if (zomb1m.life == alive && (my.Distance() > 24) && (my.Distance() < 36)) { // medium zombie 1 valid kill
				zomb1m.life = dead;
				score += 50;
			}
			if (zomb2m.life == alive && (my.Distance() > 54) && (my.Distance() < 66)) { // medium zombie 2 valid kill
				zomb2m.life = dead;
				score += 50;
			}
			if (zomb3m.life == alive && (my.Distance() > 84) && (my.Distance() < 96)) { // medium zombie 3 valid kill
				zomb3m.life = dead;
				score += 50;
			}
		}
		previous = input; // current input will be previous input in next loop iteration
	}
	NVIC_DIS0_R = 1<<21; // disable interrupt if hasn't been disabled already
	zombie = 0; // set 'zombie' gamemode to false
	if ((zomb1m.life == alive || zomb2m.life == alive || zomb3m.life == alive)) { // if any zombies have not beed killed
		Sound_Zombie(); // play zombie sound effect
		ST7735_DrawBitmap(2, 157, bigzombie, 124, 138); // draw big zombie on screen
		count = 20; // 2 second delay carried out by Timer1
		NVIC_EN0_R = 1<<21;
		while (count > 0) {};
		hearts--; // decrement hearts if not all zombies killed
		if (hearts == 2) {
			heart3.life = dead; // first heart lost
		} else if (hearts == 1) {
			heart2.life = dead; // second heart lost
		} else {
			heart1.life = dead; // final heart lost
		}
	}
	zomb1m.life = dead; // reset all zombies to dead
	zomb2m.life = dead;
	zomb3m.life = dead;
}


int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(10);
  Output_Init();
  SysTick_Init(8000000); // 10 Hz
  Timer1_Init(&graphics,8000000); // 10 Hz
	Sound_Init(); // initialize sound, with DAC and input buttons
	ADC_Init(); // initialize ADC
	PortF_Init(); // initialize Port F LED outputs
	
	selectLanguage(); // language select screen
	titleScreen(); // title screen
		
	EnableInterrupts();
	
	while (hearts != 0) { // while still alive
		
	// move mode
	movetime();
	
	// zombie mode
	zombietime();
	}
	ST7735_FillScreen(ST7735_BLACK); // clear screen
	ST7735_SetCursor(1, 0);
	ST7735_SetTextColor(ST7735_WHITE);
	ST7735_OutString((char*)Phrases[SCORE][myL]); // print score
	LCD_OutDec(score);
	ST7735_SetCursor(1, 1);
	ST7735_SetTextColor(ST7735_RED);
	ST7735_OutString((char*)Phrases[GAMEOVER][myL]); // print game over
	ST7735_DrawBitmap(2, 157, bigzombie, 124, 138); // print big zombie on game over screen
}




