// put prototypes for public functions, explain what it does
// Tim Reynolds, 5/5/2020
#ifndef __DAC_H__ // do not include more than once
#define __DAC_H__
#include <stdint.h>

// **************Input_Init*********************
// Initialize two inputs, called once to initialize the digital ports
// Input: none 
// Output: none
void Input_Init(void);

// **************Input_In*********************
// Read inputs 
// Input: none 
// Output: 0 to 3 depending on buttons
//   0x01 is just Button0, 0x02 is just Button1
//   bit n is set if button n is pressed
uint32_t Input_In(void);

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void);

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data);

#endif
