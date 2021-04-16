/*
 *   File:   mcp9808.h
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

#ifndef __MCP9808_H__
#define __MCP9808_H__

#include "project.h"

#include <stdint.h>
#include <stdbool.h>

#define MCP9808_I2CADDR_BASE           0x18

bool mcp9808_present(uint8_t *host);
bool mcp9808_read_decicelsius(uint8_t *host, int16_t *result);

#endif /* __MCP9808_H__ */
