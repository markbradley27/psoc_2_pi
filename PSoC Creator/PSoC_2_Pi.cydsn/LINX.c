/**
* \file LINX.c
* \brief TODO
*
* Version 1.1.2 TODO
*
* \author Mark Bradley
*
* \bug TODO
*
*
* Copyright Embedit Electronics
* 
*/

#ifndef LINX_H
    #include "LINX.h"
#endif

#include <project.h>
#include <stdio.h>
#include "mem1.h"

void LINX_Initialize() {
    // Start the USBUART
    USBUART_Start(0u, USBUART_5V_OPERATION);
    while(!USBUART_GetConfiguration());
    USBUART_CDC_Init();
    
    // Start EEPROM
    EEPROM_Start();
    
    #ifdef LINX_DEBUG
        // Start the debug UART
        DEBUG_UART_Start();
        DEBUG_UART_PutString("\033[2J");
        DEBUG_UART_PutString("LINX firmware started in debug mode\r\n");
    #endif
    
    // Start both VDACs
    #ifdef CY_VDAC8_VDAC8_1_H
        readData(VDAC0_CONTROL, 0x00, 0x00, 0x00);
    #endif
    #ifdef CY_VDAC8_VDAC8_2_H
        readData(VDAC1_CONTROL, 0x00, 0x00, 0x00);
    #endif
}

bool LINX_GetCommand(uint8 *command) {
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("Getting new LINX command\r\n");
        uint8 debug_str[LINX_DEBUG_STR_SIZE];
        uint8 debug_str_len;
        uint8 i;
    #endif
    
    uint8 command_len;
    
    // Get command bytes
    command_len = USBUART_GetAll(command);
    
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("Received command:");
        for (i = 0; i < command_len; ++i) {
            debug_str_len = sprintf(debug_str, " %x", command[i]);
            DEBUG_UART_PutArray(debug_str, debug_str_len);
        }
        DEBUG_UART_PutString("\r\n");
    #endif
    
    // Verify SoF
    if (command[0] != 0xFF) {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tSoF FAILED\r\n");
        #endif
        
        return false;
    }
    
    // Verify checksum
    if (command[command_len - 1] != LINX_CalculateChecksum(command, command_len - 1)) {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tChecksum FAILED\r\n");
        #endif
        
        return false;
    }
    
    // Verify command length
    if (command[1] != command_len) {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tCommand length FAILED\r\n");
        #endif
        
        return false;
    }
    
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("\tCommand validated\r\n");
    #endif
    return true;
}

uint8 LINX_CalculateChecksum(uint8 *buffer, uint8 buffer_len) {
    uint8 checksum = 0;
    uint8 i;
    
    for (i = 0; i < buffer_len; ++i) {
        checksum += buffer[i];
    }
    
    return checksum;
}

void LINX_ProcessCommand(uint8 *command, uint8 *response) {
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("\tProcessing LINX command: ");
        uint8 debug_str[LINX_DEBUG_STR_SIZE];
        uint8 debug_str_len;
    #endif
    
    uint16 cmd = command[4] << 8 | command[5];
    uint8 i;
    
    uint8 status = L_OK;
    uint8 response_data[LINX_RESPONSE_DATA_BUFFER_SIZE];
    uint8 response_data_len = 0;
    
    // TODO: Not great that these are declared even whent they're not used, switches are weird though...
    // TODO: Switch to snake_case when testing read N channels
    uint8 responseBitsRemaining;
    uint8 dataBitsRemaining;
    
    switch(cmd) {
        // Sync
        case 0x00:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Sync\r\n");
            #endif
            
            // Do nothing, will default to responding with L_OK status and no data
            break;
        
        // Get device ID
        case 0x03:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get device ID\r\n");
            #endif
            
            response_data_len = 2;
            response_data[0] = LINX_DEVICE_FAMILY;
            response_data[1] = LINX_DEVICE_ID;
            break;
        
        // Get LINX API Version
        // Untested
        case 0x04:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get API (Firmware) Version\r\n");
            #endif
            
            response_data_len = 4;
            response_data[0] = FIRMWARE_VER_MAJOR;
            response_data[1] = FIRMWARE_VER_MINOR;
            response_data[2] = FIRMWARE_VER_SUBMINOR;
            response_data[3] = FIRMWARE_VER_BUILD;
            break;
        
        // Get max baud rate
        case 0x05:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get max baud rate\r\n");
            #endif
            
            response_data_len = 4;
            response_data[0] = (LINX_MAX_BAUD_RATE >> 24) & 0xFF;
            response_data[1] = (LINX_MAX_BAUD_RATE >> 16) & 0xFF;
            response_data[2] = (LINX_MAX_BAUD_RATE >> 8) & 0xFF;
            response_data[3] = LINX_MAX_BAUD_RATE & 0xFF;
            break;
        
        // Set Baud Rate
        // Untested
        case 0x06:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Set Baud Rate\r\n");
            #endif
            
            // So far, this is a bit of a hack
            // I'm not quite sure how the USBUART baud rate actually
            // works, but it doesn't seem like it gets set the same
            // way a normal UART's does
            // For now, the firmware always responds saying it set
            // the baud rate to the max
            
            // TODO: Follow up on possible typo in documentation
            // Says status is 8th byte, but it's usually the 4th
            response_data_len = 4;
            response_data[0] = (LINX_MAX_BAUD_RATE >> 24) & 0xFF;
            response_data[1] = (LINX_MAX_BAUD_RATE >> 16) & 0xFF;
            response_data[2] = (LINX_MAX_BAUD_RATE >> 8) & 0xFF;
            response_data[3] = LINX_MAX_BAUD_RATE & 0xFF;            
            break;
            
        // Get DIO Channels
        case 0x08:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get DIO Channels\r\n");
            #endif
            
            // Port 2
            #ifdef CY_PINS_GPIO_2_0_H
                response_data[response_data_len] = 20;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_1_H
                response_data[response_data_len] = 21;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_2_H
                response_data[response_data_len] = 22;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_3_H
                response_data[response_data_len] = 23;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_4_H
                response_data[response_data_len] = 24;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_5_H
                response_data[response_data_len] = 25;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_6_H
                response_data[response_data_len] = 26;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_2_7_H
                response_data[response_data_len] = 27;
                ++response_data_len;
            #endif
            
            // Port 4
            #ifdef CY_PINS_GPIO_4_0_H
                response_data[response_data_len] = 40;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_1_H
                response_data[response_data_len] = 41;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_2_H
                response_data[response_data_len] = 42;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_3_H
                response_data[response_data_len] = 43;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_4_H
                response_data[response_data_len] = 44;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_5_H
                response_data[response_data_len] = 45;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_6_H
                response_data[response_data_len] = 46;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_4_7_H
                response_data[response_data_len] = 47;
                ++response_data_len;
            #endif
            
            // Port 5
            #ifdef CY_PINS_GPIO_5_0_H
                response_data[response_data_len] = 50;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_1_H
                response_data[response_data_len] = 51;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_2_H
                response_data[response_data_len] = 52;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_3_H
                response_data[response_data_len] = 53;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_4_H
                response_data[response_data_len] = 54;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_5_H
                response_data[response_data_len] = 55;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_6_H
                response_data[response_data_len] = 56;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_5_7_H
                response_data[response_data_len] = 57;
                ++response_data_len;
            #endif
            
            // Port 6
            #ifdef CY_PINS_GPIO_6_0_H
                response_data[response_data_len] = 60;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_1_H
                response_data[response_data_len] = 61;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_2_H
                response_data[response_data_len] = 62;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_3_H
                response_data[response_data_len] = 63;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_4_H
                response_data[response_data_len] = 64;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_5_H
                response_data[response_data_len] = 65;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_6_H
                response_data[response_data_len] = 66;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_6_7_H
                response_data[response_data_len] = 67;
                ++response_data_len;
            #endif
            
            // Port 12
            #ifdef CY_PINS_GPIO_12_0_H
                response_data[response_data_len] = 120;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_1_H
                response_data[response_data_len] = 121;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_2_H
                response_data[response_data_len] = 122;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_3_H
                response_data[response_data_len] = 123;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_4_H
                response_data[response_data_len] = 124;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_5_H
                response_data[response_data_len] = 125;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_6_H
                response_data[response_data_len] = 126;
                ++response_data_len;
            #endif
            #ifdef CY_PINS_GPIO_12_7_H
                response_data[response_data_len] = 127;
                ++response_data_len;
            #endif
            
            break;
            
        // Get AI Channels
        // Incomplete
        case 0x09:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get AI Channels\r\n");
            #endif
            
            #ifdef CY_ADC_SAR_Seq_1_H
                response_data_len = 10;
                for (i = 0; i < 10; ++i) {
                    response_data[i] = i;
                }
            #endif
            
            break;
            
        // Get AO Channels
        // Incomplete
        case 0x0A:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get AO Channels\r\n");
            #endif
            
            #ifdef CY_VDAC8_VDAC8_1_H
                response_data[response_data_len] = 0x00;
                ++response_data_len;
            #endif
            
            #ifdef CY_VDAC8_VDAC8_2_H
                response_data[response_data_len] = 0x01;
                ++response_data_len;
            #endif
            
            break;
            
        // Get PWM Channels
        // Incomplete
        case 0x0B:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get PWM Channels\r\n");
            #endif
            
            #ifdef CY_PWM_PWM_1_H
                response_data[response_data_len] = 0x00;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_2_H
                response_data[response_data_len] = 0x01;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_3_H
                response_data[response_data_len] = 0x02;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_4_H
                response_data[response_data_len] = 0x03;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_5_H
                response_data[response_data_len] = 0x04;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_6_H
                response_data[response_data_len] = 0x05;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_7_H
                response_data[response_data_len] = 0x06;
                ++response_data_len;
            #endif
            #ifdef CY_PWM_PWM_8_H
                response_data[response_data_len] = 0x07;
                ++response_data_len;
            #endif
            
            break;
            
        // Get QE Channels
        // Untested
        case 0x0C:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get QE Channels\r\n");
            #endif
            
            #ifdef CY_QUADRATURE_DECODER_QuadDec_0_H
                response_data[response_data_len] = 0x00;
                ++response_data_len;
            #endif
            
            break;
            
        // Get UART Channels
        // Incomplete
        case 0x0D:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get UART Channels\r\n");
            #endif
            
            break;
            
        // Get I2C Channels
        // Incomplete
        case 0x0E:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get I2C Channels\r\n");
            #endif
            
            break;
            
        // Get SPI Channels
        // Incomplete
        case 0x0F:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get SPI Channels\r\n");
            #endif
            
            break;
            
        // Get CAN Channels
        // Incomplete
        case 0x10:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get CAN Channels\r\n");
            #endif
            
            break;
            
        // Set Device User ID
        // Untested
        case 0x12:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Set Device User ID\r\n");
            #endif
                
            // Device User ID is stoerd in bytes (0, 0) and (0, 1) of the EEPROM
            i = 0;  // TODO: Added this to get rid of an error when LINX_DEBUG is undefined
                    // Figure out why it's needed
            cystatus first = EEPROM_ByteWrite(command[6], 0, 0);
            cystatus second = EEPROM_ByteWrite(command[7], 0, 1);
            
            if(first == CYRET_LOCKED || first == CYRET_UNKNOWN || second == CYRET_LOCKED || second == CYRET_UNKNOWN) {
                status = L_UNKNOWN_ERROR;
            }
            
            break;
            
        // Get Device User ID
        // Untested
        case 0x13:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get Device User ID\r\n");
            #endif
            
            response_data_len = 2;
            response_data[0] = *((uint8 *)CYDEV_EE_BASE);
            response_data[1] = *((uint8 *)(CYDEV_EE_BASE + 1));
            
            break;
            
        // Get device name
        case 0x24:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get device name\r\n");
            #endif
            
            // TODO: Maybe use a define for the name?
            // Not sure how that works with strings
            response_data_len = sprintf(response_data, "RPiSoC");
            
            break;
            
        // Digital Write
        case 0x41:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Digital Write\r\n");
            #endif
            
            // For each pin
            for (i = 0; i < command[6]; ++i) {
                uint8 port = command[7 + i] / 10;
                uint8 pin = command[7 + i] % 10;
                bool value = (command[7 + command[6] + (i / 8)] >> (i % 8)) & 0x01;
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\tPort: %u", port);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                    debug_str_len = sprintf(debug_str, " Pin: %u", pin);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                    debug_str_len = sprintf(debug_str, " Value: %u\r\n", value);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // Set as output
                uint8 addr = GPIO_REGISTER;
                uint8 cmd = 0x03;               // Set drive mode command
                uint16 dat = 0x0000;
                dat |= (value) & 0x01;
                dat |= (pin << 1) & 0x0E;       // Set pin
                dat |= (port << 4) & 0xF0;      // Set port
                dat |= (0x07 << 8) & 0xF00;     // Set to strong drive
                uint32 result = 0x00;
                readData(addr, cmd, dat, &result);

                // Write value
                cmd = 0x01;                     // Write command
                readData(addr, cmd, dat, &result);
                
            }
            
            break;
            
        // Digital Read
        case 0x42:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Digital Read\r\n");
            #endif
            
            for (i = 0; i < (command[1] - 7); ++i) {
                uint8 port = command[6 + i] / 10;
                uint8 pin = command[6 + i] % 10;
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\tPort: %u", port);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                    debug_str_len = sprintf(debug_str, " Pin: %u\r\n", pin);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // Set as input
                uint8 addr = GPIO_REGISTER;
                uint8 cmd = 0x03;               // Set drive mode command
                uint16 dat = 0x0000;
                dat |= (pin << 1) & 0x0E;       // Set pin
                dat |= (port << 4) & 0xF0;      // Set port
                dat |= (0x02 << 8) & 0xF00;     // Set to digital high impedance
                uint32 result = 0x00;
                readData(addr, cmd, dat, &result);
                
                // Read value
                cmd = 0x00;                     // Read command
                readData(addr, cmd, dat, &result);
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\tResult: %x\r\n", result);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                if ((i % 8) == 0) {
                    response_data[i / 8] = 0x00;
                    ++response_data_len;
                }
                response_data[i / 8] |= (((result >> pin) & 0x01) << (7 - (i % 8)));
            }
            
            break;
            
        // Get AI Reference
        // Untested
        case 0x61:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get AI Reference\r\n");
            #endif
            
            response_data_len = 4;
            response_data[0] = (LINX_AI_REF_UVOLTS >> 24) & 0xFF;
            response_data[1] = (LINX_AI_REF_UVOLTS >> 16) & 0xFF;
            response_data[2] = (LINX_AI_REF_UVOLTS >> 8) & 0xFF;
            response_data[3] = LINX_AI_REF_UVOLTS & 0xFF;
                
            break;
            
        // Analog Read
        case 0x64:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Analog Read\r\n");
            #endif
            
            response_data_len = 1;
            response_data[0] = LINX_AI_BITS;
            
            responseBitsRemaining = 8;
            dataBitsRemaining = LINX_AI_BITS;
            response_data[response_data_len] = 0x00;
            
            for (i = 0; i < (command[1] - 7); ++i) {
                uint8 pin = command[6 + i];
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\t\tPin: %u\r\n", pin);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // Read pin
                uint8 addr = ANALOG_IN_REGISTER;
                uint8 cmd = 0x00;
                uint16 dat = 0x0000;
                uint32 result;
                readData(addr, cmd, dat, &result);
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\t\tResult: %x\r\n", result);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // Pack response bits
                while (dataBitsRemaining > 0) {
                    response_data[response_data_len] |= ((result >> (LINX_AI_BITS - dataBitsRemaining)) << (8 - responseBitsRemaining));
                    
                    if (responseBitsRemaining > dataBitsRemaining) {
                        responseBitsRemaining -= dataBitsRemaining;
                        dataBitsRemaining = 0;
                    }
                    else {
                        dataBitsRemaining = dataBitsRemaining - responseBitsRemaining;
                        ++response_data_len;
                        responseBitsRemaining = 8;
                        response_data[response_data_len] = 0x00;
                    }
                }
            }
            
            response_data_len = responseBitsRemaining == 0 ? response_data_len + 1 : response_data_len + 2;
            
            break;
            
        // Analog write
        // TODO: generalize for varying-bit values, add logic for value unpacking if not using
        // an 8-bit DAC, or just make sure the hardware doesn't support higher than 8 bits
        case 0x65:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Analog write\r\n");
            #endif
            
            for (i = 0; i < command[6]; ++i) {
                uint8 pin = command[7 + i];
                uint8 value = command[7 + command[6] + i];
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\tPin: %u Value: %u\r\n", pin, value);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                uint8 addr = VDAC0_CONTROL + pin;   // Choose which VDAC
                uint8 cmd = 0x04;                   // Set value command
                uint16 dat = value;                 // Set value
                uint32 result = 0x00;
                readData(addr, cmd, dat, &result);
            }
            
            break;
            
        // PWM Set Duty Cycle
        case 0x83:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("PWM Set Duty Cycle\r\n");
            #endif
            
            for (i = 0; i < command[6]; ++i) {
                uint8 pin = command[7 + i];
                uint8 value = command[7 + command[6] + i];
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str,"\t\tPin: %u Value: %u\r\n", pin, value);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // TODO: Softcode with #defines?
                uint16 cmp = (uint32)value * (uint32)60000 / (uint32)255;
                uint8 addr;
                uint32 result;
                switch(pin) {
                    case 0: addr = PWM_REGISTER0; break;
                    case 1: addr = PWM_REGISTER1; break;
                    case 2: addr = PWM_REGISTER2; break;
                    case 3: addr = PWM_REGISTER3; break;
                    case 4: addr = PWM_REGISTER4; break;
                    case 5: addr = PWM_REGISTER5; break;
                    case 6: addr = PWM_REGISTER6; break;
                    case 7: addr = PWM_REGISTER7; break;
                }
                
                // Start PWM channel
                readData(addr, 0x00, cmp, &result);
                
                // Set PWM compare value
                readData(addr, 0x0E, cmp, &result);
            }
            
            break;
            
        // QE Read
        // Untested
        case 0xA2:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("QE Read\r\n");
            #endif
            
            response_data_len = 1;
            response_data[0] = LINX_QE_BITS;
            
            responseBitsRemaining = 8;
            dataBitsRemaining = LINX_QE_BITS;
            response_data[response_data_len] = 0x00;
            
            for (i = 0; i < (command[1] - 7); ++i) {
                uint8 channel = command[6 + i];
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\t\tChannel: %u\r\n", channel);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // Read QE
                int32 result = QuadDec_0_GetCounter();
                // TODO: Remove, debugging
                result = -29;
                
                #ifdef LINX_DEBUG
                    debug_str_len = sprintf(debug_str, "\t\t\tResult: %x\r\n", result);
                    DEBUG_UART_PutArray(debug_str, debug_str_len);
                #endif
                
                // Pack response bits
                while (dataBitsRemaining > 0) {
                    response_data[response_data_len] |= ((result >> (LINX_QE_BITS - dataBitsRemaining)) << (8 - responseBitsRemaining));
                    
                    if (responseBitsRemaining > dataBitsRemaining) {
                        responseBitsRemaining -= dataBitsRemaining;
                        dataBitsRemaining = 0;
                    }
                    else {
                        dataBitsRemaining = dataBitsRemaining - responseBitsRemaining;
                        // TODO: I don't think this is right...
                        if(dataBitsRemaining > 0) {
                            ++response_data_len;
                            response_data[response_data_len] = 0x00;
                        }
                        responseBitsRemaining = 8;
                        
                        debug_str_len = sprintf(debug_str, "\t\t\t\tdataBitsRemaining: %u, last packed response byte: %u\r\n", dataBitsRemaining, response_data[response_data_len-1]);
                        DEBUG_UART_PutArray(debug_str, debug_str_len);
                    }
                }
            }
            
            //response_data_len = responseBitsRemaining == 0 ? response_data_len + 1 : response_data_len + 2;
            if (responseBitsRemaining > 0) { ++response_data_len; }

            break;
            
        // Unsupported command
        default:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Unsupported\r\n");
            #endif
            
            status = L_FUNCTION_NOT_SUPPORTED;
            break;
    }
    
    // Build response
    response[0] = 0xFF;
    response[1] = 6 + response_data_len;
    response[2] = command[2];
    response[3] = command[3];
    response[4] = status;
    for (i = 0; i < response_data_len; ++i) {
        response[5 + i] = response_data[i];
    }
    response[5 + response_data_len] = LINX_CalculateChecksum(response, 5 + response_data_len);
    
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("\tGenerated response:");
        for (i = 0; i < response[1]; ++i) {
            debug_str_len = sprintf(debug_str, " %x", response[i]);
            DEBUG_UART_PutArray(debug_str, debug_str_len);
        }
        DEBUG_UART_PutString("\r\n");
    #endif
}

void LINX_SendResponse(uint8 *response) {
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("\tSending LINX response\r\n");
    #endif
    
    USBUART_PutData(response, response[1]);
}

/* [] END OF FILE */
