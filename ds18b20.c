/*
 *   File:   ds18x20.c
 *   Author: Matt
 *
 *   Created on 26 July 2016, 11:13
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

#include "onewire.h"
#include "ds18b20.h"
#include "ds2482.h"
#include "ow_bitbang.h"
#include "crc8.h"

#define OW_SEARCH_FIRST             0xFF
#define OW_PRESENCE_ERR             0xFF
#define OW_DATA_ERR                 0xFE
#define OW_LAST_DEVICE              0x00

#define DS18B20_READ_ROM            0x33
#define DS18B20_SP_SIZE             9
#define DS18B20_READ                0xBE
#define DS18B20_CONVERT_T           0x44

#define DS18B20_INVALID_DECICELSIUS 0x7FFF

static bool ds18b20_read_scratchpad(uint8_t *id, uint8_t *sp, uint8_t n)
{
    uint8_t data = DS18B20_READ;

    if (!ow_select(id))
        return false;

    if (!ow_write(&data, 1))
        return false;

    if (!ow_read(sp, n))
        return false;

    if (crc8(sp, DS18B20_SP_SIZE))
        return false;

    return true;
}

/* Convert scratchpad data to physical value in unit decicelsius */
static int16_t ds18b20_raw_to_decicelsius(uint8_t *sp)
{
    uint16_t measure;
    uint8_t  negative;
    int16_t  decicelsius;
    uint16_t fract;

    measure = sp[0] | (sp[1] << 8);

    /* Check for negative */
    if (measure & 0x8000)
    {
        negative = 1;       /* Mark negative */
        measure ^= 0xffff;  /* convert to positive => (twos complement)++ */
        measure++;
    }
    else
    {
        negative = 0;
    }

    decicelsius = (measure >> 4);
    decicelsius *= 10;

    /* decicelsius += ((measure & 0x000F) * 640 + 512) / 1024;
     * 625/1000 = 640/1024
     */
    fract = (measure & 0x000F) * 640;
    if (!negative)
        fract += 512;
    fract /= 1024;
    decicelsius += fract;

    if (negative)
        decicelsius = -decicelsius;

    if (/* decicelsius == 850 || */ decicelsius < -550 || decicelsius > 1250)
        return DS18B20_INVALID_DECICELSIUS;
 
    return decicelsius;
}

// Returns fixed point output i.e. 10 = 1.0 degrees
bool ds18b20_read_decicelsius(uint8_t *id, int16_t *decicelsius)
{
    int16_t ret;
    uint8_t sp[DS18B20_SP_SIZE];

    if (!ds18b20_read_scratchpad(id, sp, DS18B20_SP_SIZE))
        return false;

    ret = ds18b20_raw_to_decicelsius(sp);

    if (ret == DS18B20_INVALID_DECICELSIUS)
        return false;

    *decicelsius = ret;
    return true;
}

bool ds18b20_start_meas(uint8_t *id)
{
    uint8_t data = DS18B20_CONVERT_T;
    
    if (!ow_select(id))
        return false;

    return ow_write(&data, 1);
}
