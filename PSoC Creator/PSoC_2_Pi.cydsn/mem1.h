/**
* \file mem1.h
* \brief This file contains the defined register address information for each 
         supported component, and it provides the function prototypes for mem1.c
*
* Version 1.1.2
*
* \author Brian Bradley
*
* \bug No known bugs.
*
*
* Copyright Embedit Electronics
* 
*/

#ifndef MEM1_H
#define MEM1_H
    
#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h"
#include <stdbool.h>
    
#define MAX_32          (0xFFFFFFFF)
#define I2C_BUFFER_SIZE (4u)

#define DELSIG_ADC_CONTROL (0x01) //No 0x00 because the Pi's SPI readbytes function sends 0x00 every time it's called
#define SAR_ADC0_CONTROL (DELSIG_ADC_CONTROL +1)
#define SAR_ADC1_CONTROL (SAR_ADC0_CONTROL +1)


#define VDAC0_CONTROL (SAR_ADC1_CONTROL +1)
#define VDAC1_CONTROL (VDAC0_CONTROL +1)

#define IDAC0_CONTROL (VDAC1_CONTROL +1)
#define IDAC1_CONTROL (IDAC0_CONTROL +1)

#define WAVEDAC_CONTROL (IDAC1_CONTROL +1)

#define PWM_REGISTER0 (WAVEDAC_CONTROL +1)
#define PWM_REGISTER1 (PWM_REGISTER0 +1)
#define PWM_REGISTER2 (PWM_REGISTER1 +1)
#define PWM_REGISTER3 (PWM_REGISTER2 +1)
#define PWM_REGISTER4 (PWM_REGISTER3 +1)
#define PWM_REGISTER5 (PWM_REGISTER4 +1)
#define PWM_REGISTER6 (PWM_REGISTER5 +1)
#define PWM_REGISTER7 (PWM_REGISTER6 +1)

#define GPIO_REGISTER (PWM_REGISTER7 + 1)
#define ANALOG_IN_REGISTER (GPIO_REGISTER + 1)

    
#define RESET_ADDRESS (0xFF)

/*UNDEFINE UNUSED PROTOCOLS*/
//#undef CY_SPIS_SPIS_1_H     /* SPI */
#undef CY_I2C_I2C_1_H       /* I2C */
    
    
void WriteTo_Pi(uint32 dat);
uint32 ReadFrom_Pi(void);

bool readData(uint8 addr, uint8 cmd, uint16 dat, uint32 *result);

void DELSIG_ADC_Control(uint8 cmd, uint16 val);
void SAR0_ADC_Control(uint8 cmd, uint16 val);
void SAR1_ADC_Control(uint8 cmd, uint16 val);
void VDAC0_Control(uint8 cmd, uint16 val);
void VDAC1_Control(uint8 cmd, uint16 val);
void IDAC0_Control(uint8 cmd, uint16 val);
void IDAC1_Control(uint8 cmd, uint16 val);
void WAVEDAC_Control(uint8 cmd, uint16 val);
void Generate_Wave(void);
void PWM_Control_0(uint8 cmd, uint16 val);
void PWM_Control_1(uint8 cmd, uint16 val);
void PWM_Control_2(uint8 cmd, uint16 val);
void PWM_Control_3(uint8 cmd, uint16 val);
void PWM_Control_4(uint8 cmd, uint16 val);
void PWM_Control_5(uint8 cmd, uint16 val);
void PWM_Control_6(uint8 cmd, uint16 val);
void PWM_Control_7(uint8 cmd, uint16 val);
bool GPIO_Control(uint8 cmd, uint16 val, uint32 *result);
bool Analog_Read(uint8 cmd, uint16 dat, uint32 *result);


#endif
/* [] END OF FILE */