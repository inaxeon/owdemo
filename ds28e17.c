/*
 *   File:   ds28e17.c
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

#include "project.h"

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>

#include "onewire.h"
#include "ds28e17.h"
#include "ds2482.h"
#include "crc16_arc.h"

/* DS28E17 device command codes. */
#define DS28E17_WRITE_DATA_WITH_STOP        0x4B
#define DS28E17_WRITE_DATA_NO_STOP          0x5A
#define DS28E17_WRITE_DATA_ONLY             0x69
#define DS28E17_WRITE_DATA_ONLY_WITH_STOP   0x78
#define DS28E17_READ_DATA_WITH_STOP         0x87
#define DS28E17_WRITE_READ_DATA_WITH_STOP   0x2D
#define DS28E17_WRITE_CONFIGURATION         0xD2
#define DS28E17_READ_CONFIGURATION          0xE1
#define DS28E17_ENABLE_SLEEP_MODE           0x1E
#define DS28E17_READ_DEVICE_REVISION        0xC4

/* DS28E17 status bits */
#define DS28E17_STATUS_CRC                  0x01
#define DS28E17_STATUS_ADDRESS              0x02
#define DS28E17_STATUS_START                0x08

#define DS28E17_SPEED                       SPEED_400KHZ

#if DS28E17_SPEED == SPEED_100KHZ
#define DS28E17_BASE_WAIT                   90
#elif DS28E17_SPEED == SPEED_400KHZ
#define DS28E17_BASE_WAIT                   23
#elif DS28E17_SPEED == SPEED_900KHZ
#define DS28E17_BASE_WAIT                   10
#endif

#define DS28E17_BUSY_CHECKS                 100

static bool ds28e17_i2c_busy_wait(uint8_t count);
static bool ds28e17_check_error(const uint8_t *w1_buf);
static int ds28e17_set_i2c_speed(const uint8_t *id, uint8_t speed);

bool ds28e17_init(const uint8_t *id)
{
    return ds28e17_set_i2c_speed(id, DS28E17_SPEED);
}

bool ds28e17_i2c_read(const uint8_t *id, uint8_t slave_addr, uint8_t reg, uint8_t *buffer, uint8_t count)
{
    uint16_t crc;
    uint8_t w1_buf[3];

    w1_buf[0] = DS28E17_WRITE_READ_DATA_WITH_STOP;
    w1_buf[1] = slave_addr << 1;
    w1_buf[2] = 1;

    crc = crc16_arc(CRC16_ARC_INIT, w1_buf, 3);

    if (!ow_select(id))
        return false;

    if (!ow_write(w1_buf, 3))
        return false;

    crc = crc16_arc(crc, &reg, 1);
    
    if (!ow_write(&reg, 1))
        return false;

    w1_buf[0] = count;
    crc = crc16_arc(crc, w1_buf, 1);
    w1_buf[1] = ~(crc & 0xFF);
    w1_buf[2] = ~((crc >> 8) & 0xFF);

    if (!ow_write(w1_buf, 3))
        return false;

    /* Wait until busy flag clears (or timeout). */
    if (!ds28e17_i2c_busy_wait(1 + count + 2))
        return false;

    /* Read status from DS28E17. */
    if (!ow_read(w1_buf, 2))
        return false;

    /* Check error conditions. */
    if (!ds28e17_check_error(w1_buf))
        return false;

    /* Read received I2C data from DS28E17. */
    return ow_read(buffer, count);
}

bool ds28e17_i2c_write(const uint8_t *id, uint8_t slave_addr, uint8_t reg, const uint8_t *buffer, uint8_t count)
{
    uint16_t crc;
    uint8_t w1_buf[2];

    w1_buf[0] = DS28E17_WRITE_DATA_WITH_STOP;
    w1_buf[1] = slave_addr << 1;

    crc = crc16_arc(CRC16_ARC_INIT, w1_buf, 2);

    if (!ow_select(id))
        return false;

    if (!ow_write(w1_buf, 2))
        return false;

    w1_buf[0] = count + 1;
    w1_buf[1] = reg;
    crc = crc16_arc(crc, w1_buf, 2);

    if (!ow_write(w1_buf, 2))
        return false;

    crc = crc16_arc(crc, buffer, count);

    if (!ow_write(buffer, count))
        return false;

    w1_buf[0] = ~(crc & 0xFF);
    w1_buf[1] = ~((crc >> 8) & 0xFF);
    
    if (!ow_write(w1_buf, 2))
        return false;

    /* Wait until busy flag clears (or timeout). */
    if (!ds28e17_i2c_busy_wait(count + 2))
        return false;

    /* Read status from DS28E17. */
    if (!ow_read(w1_buf, 2))
        return false;

    /* Check error conditions. */
    if (!ds28e17_check_error(w1_buf))
        return false;

    return true;
}

/* Set I2C speed on DS28E17. */
static int ds28e17_set_i2c_speed(const uint8_t *id, uint8_t speed)
{
    uint8_t w1_buf[2];

    w1_buf[0] = DS28E17_WRITE_CONFIGURATION;
    w1_buf[1] = speed;
    
    if (!ow_select(id))
        return false;

    if (ow_write(w1_buf, 2))
        return false;

    return 0;
}

/* Wait a while until the busy flag clears. */
static bool ds28e17_i2c_busy_wait(uint8_t count)
{
    uint8_t i;
    uint8_t checks;
    bool bit;

    /* Check the busy flag first in any case.*/
    bit = true;
    if (!ow_bit_io(&bit))
        return false;
    if (!bit)
        return true;

    /*
     * Do a generously long sleep in the beginning,
     * as we have to wait at least this time for all
     * the I2C bytes at the given speed to be transferred.
     */
    for (i = 0; i < count; i++)
        _delay_us(DS28E17_BASE_WAIT);

    /* Now continusly check the busy flag sent by the DS28E17. */
    checks = DS28E17_BUSY_CHECKS;

    while (checks--)
    {
        /* Return success if the busy flag is cleared. */
        bit = true;
        if (!ow_bit_io(&bit))
            return false;
        if (!bit)
            return true;

        /* Wait one timeslot */
        _delay_us(DS28E17_BASE_WAIT);
    }

    /* Timeout */
    return false;
}

static bool ds28e17_check_error(const uint8_t *w1_buf)
{
    if (w1_buf[0] & DS28E17_STATUS_CRC)
        return false;
    if (w1_buf[0] & DS28E17_STATUS_ADDRESS)
        return false;
    if (w1_buf[0] & DS28E17_STATUS_START)
        return false;
    if (w1_buf[0] != 0 || w1_buf[1] != 0)
        return false;

    return true;
}
