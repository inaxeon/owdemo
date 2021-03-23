/*
 *   File:   crc16_arc.h
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

#ifndef _CRC16_ARC_H_
#define _CRC16_ARC_H_

#define CRC16_ARC_INIT 0x0000

uint16_t crc16_arc(uint16_t crc, const uint8_t *buf, int len);

#endif /* _CRC16_ARC_H_ */