// SlidePot.cpp
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Student names: Tim Reynolds
// Last modification date: 5/5/2020

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	volatile int nop;
	SYSCTL_RCGCADC_R |= 0x0001; // activate ADC0
	nop = 0;
	nop = 0;
	SYSCTL_RCGCGPIO_R |= 0x08; // activate clock for Port D
	nop = 0;
	nop = 0;
	GPIO_PORTD_DIR_R &= ~0x04; // make PD2 input
	GPIO_PORTD_AFSEL_R |= 0x04; // enable alternate function on PD2
	GPIO_PORTD_DEN_R &= ~0x04; // disable digital I/O on PD2
	GPIO_PORTD_AMSEL_R |= 0x04; // enable analog on PD2
	ADC0_PC_R &= ~0xF;
	ADC0_PC_R |= 0x1; // max 125k samples/sec
	ADC0_SSPRI_R = 0x0123; // sequencer 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008; // disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000; // seq3 is software trigger
	ADC0_SSMUX3_R &= ~0x000F;
	ADC0_SSMUX3_R += 5; // set channel = 5
	ADC0_SSCTL3_R = 0x0006; // no TS0, D0, yes IE0 END0
	ADC0_IM_R &= ~0x0008; // disable SS3 interrupts
	ADC0_ACTSS_R |= 0x0008; // enable sample sequencer 3
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	uint32_t result;
	ADC0_PSSI_R = 0x0008; // initiate SS3
	while ((ADC0_RIS_R &0x08) == 0) {}; // wait for conversion done
	result = ADC0_SSFIFO3_R & 0xFFF; // read result
	ADC0_ISC_R = 0x0008; // acknowledge completion
	return result;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
	this->data = 0; // initialize data to 0
	this->distance = 0; // initialize distance to 0
	this->flag = 0; // initialize flag to 0
	this->slope = m; // set slope to m (165)
	this->offset = b; // set offset to b (20)
}

void SlidePot::Save(uint32_t n){
	// 1) save ADC sample into private variable
// 2) calculate distance from ADC, save into private variable
// 3) set semaphore flag = 1
	this->data = n; // save ADC sample into data
	this->distance = Convert(n); // convert ADC sample and save into distance
	this->flag = 1; // set flag to 1
}
uint32_t SlidePot::Convert(uint32_t n){
	// use calibration data to convert ADC sample to distance
	// distance = (slope*data+offset)/4096
  uint32_t dist = (((this->slope * n) / 4096) + this->offset); // distance = (165*data+20)/4096
	
	// convert distances into pixel distances on screen: (180-20) is range of slidepot distance values, and (105-18) is pixel range. 160/87 ~~ 2
	uint32_t pix = (dist/2) + 10;
	if (pix < 18) { // if pixel value for x coordinate of reticle is below minimum, set it to minimum of 18
		pix = 18;
	}
	if (pix > 105) { // if pixel value for x coordinate of reticle is above maximum, set it to maximum of 106
		pix = 105;
	}
	return pix;
}

void SlidePot::Sync(void){
	// 1) wait for semaphore flag to be nonzero
  // 2) set semaphore flag to 0
	while (this->flag == 0) { // 1)
	}
	this->flag = 0; // 2)
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  // return last calculated ADC sample
  return this->data;
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
	//*** students write this ******
  // return last calculated distance in 0.001cm
  return this->distance;
}


