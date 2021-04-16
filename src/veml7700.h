/*
 *   File:   veml7700.h
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

#ifndef __VEML7700_H__
#define __VEML7700_H__

bool veml7700_init(const uint8_t *host_id);
bool veml7700_read_decilux(const uint8_t *host_id, uint32_t *lux);

#endif /* __VEML7700_H__ */