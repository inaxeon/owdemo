/*
 *   File:   main.c
 *   Author: Matt
 *
 *   Created on 11 May 2018, 12:13
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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "iopins.h"
#include "onewire.h"
#include "ow_bitbang.h"
#include "ds2482.h"
#include "ds28e17.h"
#include "ds18b20.h"
#include "veml7700.h"
#include "usart.h"
#include "util.h"

FILE uart_str = FDEV_SETUP_STREAM(print_char, NULL, _FDEV_SETUP_RW);

static void io_init(void);

int main(void)
{
    uint8_t i;
    uint8_t sensor_ids[MAX_SENSORS][OW_ROMCODE_SIZE];
    uint8_t num_sensors;
    uint8_t num_temp_sensors;
    uint8_t num_light_sensors;
    uint8_t light_sensors_mask;
    uint8_t ow_device_types[2];
    uint8_t ow_device_counts[2];

    light_sensors_mask = 0;

    io_init();
    g_irq_enable();

    usart1_open(USART_CONT_RX, (((F_CPU / UART1_BAUD) / 16) - 1)); // Console

    stdout = &uart_str;

    printf("Starting up...\r\n");

    ow_device_types[0] = DS18B20_FAMILY_CODE;
    ow_device_types[1] = DS28E17_FAMILY_CODE;

    if (!onewire_search_devices(sensor_ids, ow_device_types, ow_device_counts, sizeof(ow_device_types)))
        printf("Hardware error searching for sensors\r\n");

    num_temp_sensors = ow_device_counts[0];
    num_light_sensors = ow_device_counts[1];
    num_sensors = num_temp_sensors + num_light_sensors;

    if (num_light_sensors)
    {
        for (i = 0; i < num_sensors; i++)
        {
            if (sensor_ids[i][0] == DS28E17_FAMILY_CODE)
            {
                light_sensors_mask |= (1 << i);
                ds28e17_init(sensor_ids[i]);
                veml7700_init(sensor_ids[i]);
            }
        }
    }

    printf("Found %u temperature and %u light sensors of %u total\r\n\r\n", num_temp_sensors, num_light_sensors, MAX_SENSORS);

    for (;;)
    {
        for (i = 0; i < num_sensors; i++)
        {
            // Don't broadcast 'start measure' command. DS28E17's don't know what to do with it.
            if ((light_sensors_mask & (1 << i)) == 0)
                ds18b20_start_meas(sensor_ids[i]);
        }

        _delay_ms(1000);

        for (i = 0; i < num_sensors; i++)
        {
            if (sensor_ids[i][0] == DS18B20_FAMILY_CODE)
            {
                int16_t temperature; // single fixed point i.e. 10 = 1.0 degrees

                if (ds18b20_read_decicelsius(sensor_ids[i], (int16_t *)&temperature))
                {
                    char temperature_sign[2];
                    
                    temperature_sign[1] = 0;

                    if (temperature < 0 )
                        temperature_sign[0] = '-';
                    else
                        temperature_sign[0] = 0;
                        
                    printf("Temperature sensor @ Index %d: Degrees C: %s%u.%u\r\n", i, temperature_sign, (temperature / 10), (temperature % 10));
                }
            }
            if (sensor_ids[i][0] == DS28E17_FAMILY_CODE)
            {
                uint32_t lux; // single fixed point. i.e. 10 = 1.0 lux

                if (veml7700_read_lux(sensor_ids[i], (uint32_t *)&lux))
                {
                    printf("Light sensor       @ Index %d:       Lux: %lu.%lu\r\n", i, (lux / 10), (lux % 10));
                }
            }
        }

        printf("\r\n");
    }
}


static void io_init(void)
{
#ifdef _LEONARDO_
    // Disable USB, because the bootloader has probably left it on
    USBCON &= ~_BV(USBE);
#endif
}
