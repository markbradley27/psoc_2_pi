/**
* \file LINX.h
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
#define LINX_H
    
#include "cytypes.h"
#include "stdbool.h"

// Uncomment to enable LINX debug mode
#define LINX_DEBUG
    
// Firmware-specific constants
#define FIRMWARE_VER_MAJOR (0x00)
#define FIRMWARE_VER_MINOR (0x00)
#define FIRMWARE_VER_SUBMINOR (0x00)
#define FIRMWARE_VER_BUILD (0x00)
#define LINX_COMMAND_BUFFER_SIZE 16
#define LINX_RESPONSE_BUFFER_SIZE 16
#define LINX_RESPONSE_DATA_SIZE 4
#define LINX_DEBUG_STR_SIZE 64

// LINX Common Statuses
#define L_OK (0x00)
#define L_FUNCTION_NOT_SUPPORTED (0x01)
#define L_REQUEST_RESEND (0x02)
#define L_UNKNOWN_ERROR (0x03)
#define L_DISCONECT (0x04)

// LINX Peripheral Statuses
#define LI2C_SADDR (0x80)
#define LI2C_EOF (0x81)
#define LI2C_WRITE_FAIL (0x82)
#define LI2C_READ_FAIL (0x83)
#define LI2C_CLOSE_FAIL (0x84)
#define LI2C_OPEN_FAIL (0x85)
#define LUART_OPEN_FAIL (0x80)
#define LUART_SET_BAUD_FAIL (0x81)
#define LUART_AVAILABLE_FAIL (0x82)
#define LUART_READ_FAIL (0x83)
#define LUART_WRITE_FAIL (0x84)
#define LUART_CLOSE_FAIL (0x85)
    
// LINX Device constants
#define LINX_MAX_BAUD_RATE (9600u)
#define LINX_DEVICE_FAMILY (0xFF)
#define LINX_DEVICE_ID (0x00)

void LINX_Initialize();
bool LINX_GetCommand(uint8 *command);
uint8 LINX_CalculateChecksum(uint8 *command, uint8 command_len);
void LINX_ProcessCommand(uint8 *command, uint8 *response);
void LINX_SendResponse(uint8 *response);
    
#endif
/* [] END OF FILE */