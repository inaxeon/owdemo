

/*
 *   File:   veml7700.c
 *   Author: Matt
 *
 *   Created on 22 February 2021, 06:26
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

#include <stdint.h>
#include <stdbool.h>

#include "veml7700.h"
#include "ds28e17.h"

#define VEML7700_I2C_ADDR       0x10

#define VEML7700_ALS_CONF_0     0x00
#define VEML7700_ALS            0x04

#define VEML7700_BASE_FACTOR    36
#define VEML7700_SCALE_FACTOR   1000

// Configuration register values
#define VEML7700_G_X1           (0 << 11)
#define VEML7700_G_X2           (1 << 11)
#define VEML7700_G_D8           (2 << 11)
#define VEML7700_G_D4           (3 << 11)

#define VEML7700_IT_800        (3 << 6)
#define VEML7700_IT_400        (2 << 6)
#define VEML7700_IT_200        (1 << 6)
#define VEML7700_IT_100        (0 << 6)
#define VEML7700_IT_50         (8 << 6)
#define VEML7700_IT_25         (12 << 6)

// Change these values to for different configurations
#define VEML7700_GAIN           VEML7700_G_X1
#define VEML7700_IT             VEML7700_IT_100

#if   VEML7700_GAIN == VEML7700_G_X2
#define VEML7700_GAIN_FACTOR    1
#elif VEML7700_GAIN == VEML7700_G_X1
#define VEML7700_GAIN_FACTOR    2
#elif VEML7700_GAIN == VEML7700_G_D4
#define VEML7700_GAIN_FACTOR    8
#elif VEML7700_GAIN == VEML7700_G_D8
#define VEML7700_GAIN_FACTOR    16
#endif

#if   VEML7700_IT == VEML7700_IT_800
#define VEML7700_IT_FACTOR      1
#elif VEML7700_IT == VEML7700_IT_400
#define VEML7700_IT_FACTOR      2
#elif VEML7700_IT == VEML7700_IT_200
#define VEML7700_IT_FACTOR      4
#elif VEML7700_IT == VEML7700_IT_100
#define VEML7700_IT_FACTOR      8
#elif VEML7700_IT == VEML7700_IT_50
#define VEML7700_IT_FACTOR      16
#elif VEML7700_IT == VEML7700_IT_25
#define VEML7700_IT_FACTOR      32
#endif

#define VEML7700_FACTOR    (VEML7700_BASE_FACTOR * VEML7700_GAIN_FACTOR * VEML7700_IT_FACTOR)

static uint32_t veml7700_scale_lux(uint16_t reg_value);

bool veml7700_init(const uint8_t *host_id)
{
    uint16_t confreg_value = (VEML7700_GAIN | VEML7700_IT);

    return ds28e17_i2c_write(host_id, VEML7700_I2C_ADDR, VEML7700_ALS_CONF_0, (uint8_t *)&confreg_value, sizeof(uint16_t));
}

// Returns fixed point output i.e. 10 = 1.0 lux
bool veml7700_read_lux(const uint8_t *host_id, uint32_t *lux)
{
    uint16_t reg_value;

    if (!ds28e17_i2c_read(host_id, VEML7700_I2C_ADDR, VEML7700_ALS, (uint8_t *)&reg_value, sizeof(uint16_t)))
        return false;

    *lux = veml7700_scale_lux(reg_value);

    return true;
}

static uint32_t veml7700_scale_lux(uint16_t reg_value)
{
    uint32_t final_value = ((uint32_t)reg_value * VEML7700_FACTOR);
    final_value /= VEML7700_SCALE_FACTOR;
    return final_value;
}