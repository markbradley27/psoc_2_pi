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

bool LINX_GetCommand(uint8 *command, uint8 *command_len) {
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("Getting new LINX command\r\n");
        uint8 debug_str[LINX_DEBUG_STR_SIZE];
        uint8 debug_str_len;
    #endif
    
    uint8 i;
    
    // Get command bytes
    *command_len = USBUART_GetAll(command);
    
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("Received command:");
        for (i = 0; i < *command_len; ++i) {
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
    else {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tSoF Passed\r\n");
        #endif
    }
    
    // Verify checksum
    if (command[*command_len - 1] != LINX_CalculateChecksum(command, *command_len)) {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tChecksum FAILED\r\n");
        #endif
        
        return false;
    }
    else {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tChecksum passed\r\n");
        #endif
    }
    
    // Verify command length
    if (command[1] != *command_len) {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tCommand length FAILED\r\n");
        #endif
        
        return false;
    }
    else {
        #ifdef LINX_DEBUG
            DEBUG_UART_PutString("\tCommand length passed\r\n");
        #endif
    }
    
    return true;
}

uint8 LINX_CalculateChecksum(uint8 *command, uint8 command_len) {
    uint8 checksum = 0;
    uint8 i;
    
    for (i = 0; i < command_len - 1; ++i) {
        checksum += command[i];
    }
    
    return checksum;
}

void LINX_ProcessCommand(uint8 *command, uint8 command_len) {
    uint16 cmd = command[
    
    switch(command[
}

void LINX_SendResponse() {
    #ifdef LINX_DEBUG
        DEBUG_UART_PutString("Sending LINX response\r\n");
    #endif
}

/* [] END OF FILE */
