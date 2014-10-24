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

void LINX_Initialize() {
    USBUART_Start(0u, USBUART_5V_OPERATION);
    while(!USBUART_GetConfiguration());
    USBUART_CDC_Init();
    
    #ifdef LINX_DEBUG
        DEBUG_UART_Start();
        DEBUG_UART_PutString("\033[2J");
        DEBUG_UART_PutString("LINX firmware started in debug mode\r\n");
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
    uint8 response_data[LINX_RESPONSE_DATA_SIZE];
    uint8 response_data_len = 0;
    
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
        case 0x04:
            #ifdef LINX_DEBUG
                DEBUG_UART_PutString("Get API (Firmware) Version\r\n");
            #endif
            
            response_data_len = 3;
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
