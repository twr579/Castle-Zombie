// put implementations for functions, explain how it works
// Tim Reynolds
// 5/4/2020
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "DAC.h"

// **************Input_Init*********************
// Initialize two inputs, called once to initialize the digital ports
// Input: none 
// Output: none
void Input_Init(void){
	volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x10; // enable Port E clock
	delay = 0; // wait
	delay = 0;
	GPIO_PORTE_DEN_R |= 0x03; // enable PE0-3
	GPIO_PORTE_DIR_R &= ~0x03; // set PE0-3 as inputs
}

// **************Input_In*********************
// Read inputs 
// Input: none 
// Output: 0 to 3 depending on buttons
//   0x01 is just Button0, 0x02 is just Button1
//   bit n is set if button n is pressed
uint32_t Input_In(void){
  return GPIO_PORTE_DATA_R & 0x03; // will return 1, or 2 depending on which button is pressed
}

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x02; // enable Port B clock
	delay = 0; // wait
	GPIO_PORTB_DEN_R |= 0x0F; // enable PB0-3
	GPIO_PORTB_DIR_R |= 0x0F; // set PB0-3 as outputs
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R = data; // put 4-bit data in the Port B data register
}
