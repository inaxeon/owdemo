/*
 *   File:   mcp9808.c
 *   Author: Matt
 *
 *   Created on 12 July 2016, 10:32
 * 
 *   This is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *   This software is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "project.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mcp9808.h"
#include "ds28e17.h"
#include "util.h"

#define MCP9808_REG_CONFIG             0x01

#define MCP9808_REG_CONFIG_SHUTDOWN    0x0100
#define MCP9808_REG_CONFIG_CRITLOCKED  0x0080
#define MCP9808_REG_CONFIG_WINLOCKED   0x0040
#define MCP9808_REG_CONFIG_INTCLR      0x0020
#define MCP9808_REG_CONFIG_ALERTSTAT   0x0010
#define MCP9808_REG_CONFIG_ALERTCTRL   0x0008
#define MCP9808_REG_CONFIG_ALERTSEL    0x0004
#define MCP9808_REG_CONFIG_ALERTPOL    0x0002
#define MCP9808_REG_CONFIG_ALERTMODE   0x0001

#define MCP9808_REG_UPPER_TEMP         0x02
#define MCP9808_REG_LOWER_TEMP         0x03
#define MCP9808_REG_CRIT_TEMP          0x04
#define MCP9808_REG_AMBIENT_TEMP       0x05
#define MCP9808_REG_MANUF_ID           0x06
#define MCP9808_REG_DEVICE_ID          0x07

bool mcp9808_present(uint8_t *host)
{
    uint16_t manuf;
    uint16_t device;

    if (!ds28e17_i2c_read(host, MCP9808_I2CADDR_BASE, MCP9808_REG_MANUF_ID, (uint8_t *)&manuf, sizeof(uint16_t)))
        return false;

    manuf = SWAP16(manuf);

    if (!ds28e17_i2c_read(host, MCP9808_I2CADDR_BASE, MCP9808_REG_DEVICE_ID, (uint8_t *)&device, sizeof(uint16_t)))
        return false;

    device = SWAP16(device);

    if (manuf == 0x0054 && device == 0x0400)
        return true;

    return false;
}

bool mcp9808_read_decicelsius(uint8_t *host, int16_t *result)
{
    uint16_t ambient;
    int32_t decicelsius;

    if (!ds28e17_i2c_read(host, MCP9808_I2CADDR_BASE, MCP9808_REG_AMBIENT_TEMP, (uint8_t *)&ambient, sizeof(uint16_t)))
        return false;

    ambient = SWAP16(ambient);

    decicelsius = (ambient & 0x0FFF) * 10;
    decicelsius /= 16;
    
    if (ambient & 0x1000)
        decicelsius -= 2560;

    *result = (int16_t)decicelsius;

    return true;
}
