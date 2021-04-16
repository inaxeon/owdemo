/*
 *   File:   crc16_arc.c
 *   Author: Matt
 *
 *   Created on 20 February 2021, 10:16
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


#include <stdint.h>

#define CRC16_ARC_POLY 0xA001

uint16_t crc16_arc(uint16_t crc, const uint8_t *buf, int len)
{
    while (len--)
    {
        crc ^= *buf++;
        for (uint8_t i = 0; i < 8; i++)
            crc = crc & 1 ? (crc >> 1) ^ CRC16_ARC_POLY : crc >> 1;
    }
    return crc;
}