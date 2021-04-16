/*
 *   File:   ds28e17.h
 *   Author: Matt
 *
 *   Created on 20 February 2021, 07:06
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

#ifndef __DS28E17_H__
#define __DS28E17_H__

#define DS28E17_FAMILY_CODE         0x19

#define SPEED_100KHZ                0x00
#define SPEED_400KHZ                0x01
#define SPEED_900KHZ                0x02

bool ds28e17_init(const uint8_t *id);
bool ds28e17_i2c_read(const uint8_t *id, uint8_t slave_addr, uint8_t reg, uint8_t *buffer, uint8_t count);
bool ds28e17_i2c_write(const uint8_t *id, uint8_t slave_addr, uint8_t reg, const uint8_t *buffer, uint8_t count);

#endif /* __DS28E17_H__ */