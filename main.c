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
#include "mcp9808.h"
#include "usart.h"
#include "util.h"

FILE uart_str = FDEV_SETUP_STREAM(print_char, NULL, _FDEV_SETUP_RW);

static void io_init(void);

#define DEV_UNKNOWN     0
#define DEV_DS18B20     1
#define DEV_VEML7700    2
#define DEV_MPC9808     3

int main(void)
{
    uint8_t i;
    uint8_t sensor_ids[MAX_SENSORS][OW_ROMCODE_SIZE];
    uint8_t dev_types[MAX_SENSORS];
    uint8_t num_sensors;
    uint8_t num_temp_sensors;
    uint8_t num_bridged_devs;
    uint8_t ow_device_types[2];
    uint8_t ow_device_counts[2];

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
    num_bridged_devs = ow_device_counts[1];
    num_sensors = num_temp_sensors + num_bridged_devs;

    if (num_bridged_devs)
    {
        for (i = 0; i < num_sensors; i++)
        {
            dev_types[i] = DEV_UNKNOWN;

            if (sensor_ids[i][0] == DS28E17_FAMILY_CODE)
            {
                ds28e17_init(sensor_ids[i]);

                if (mcp9808_present(sensor_ids[i]))
                {
                    dev_types[i] = DEV_MPC9808;
                }
                else
                {
                    dev_types[i] = DEV_VEML7700; //Can't easily probe VEML7700 so assume it's this if not MCP9808
                    veml7700_init(sensor_ids[i]);
                }
            }

            if (sensor_ids[i][0] == DS18B20_FAMILY_CODE)
            {
                dev_types[i] = DEV_DS18B20;
            }
        }
    }

    printf("Found %u native and %u bridged sensors of %u total\r\n\r\n", num_temp_sensors, num_bridged_devs, MAX_SENSORS);

    for (;;)
    {
        for (i = 0; i < num_sensors; i++)
        {
            // Don't broadcast 'start measure' command. DS28E17's don't know what to do with it.
            if (sensor_ids[i][0] == DS18B20_FAMILY_CODE)
            {
                if (!ds18b20_start_measure(sensor_ids[i]))
                    printf("Error starting measurement on temperature sensor %d\r\n", i);
            }
        }

        _delay_ms(1000);

        for (i = 0; i < num_sensors; i++)
        {
            if (dev_types[i] == DEV_DS18B20)
            {
                int16_t temperature; // single fixed point i.e. 10 = 1.0 degrees

                if (ds18b20_read_decicelsius(sensor_ids[i], (int16_t *)&temperature))
                {
                    char temperature_sign[2];
                    
                    temperature_sign[1] = 0;
                    temperature_sign[0] = (temperature < 0)  ? '-' : 0;
                        
                    printf("DS18B20 sensor     @ Index %d: Degrees C: %s%u.%u\r\n", i, temperature_sign, (abs(temperature) / 10), (abs(temperature) % 10));
                }
                else
                {
                    printf("Error reading from DS18B20 sensor %d\r\n", i);
                }
            }
            else if (dev_types[i] == DEV_MPC9808)
            {
                int16_t temperature; // single fixed point i.e. 10 = 1.0 degrees

                if (mcp9808_read_decicelsius(sensor_ids[i], (int16_t *)&temperature))
                {
                    char temperature_sign[2];
                    
                    temperature_sign[1] = 0;
                    temperature_sign[0] = (temperature < 0)  ? '-' : 0;
                        
                    printf("MCP9808 sensor     @ Index %d: Degrees C: %s%u.%u\r\n", i, temperature_sign, (abs(temperature) / 10), (abs(temperature) % 10));
                }
                else
                {
                    printf("Error reading from MPC9808 sensor %d\r\n", i);
                }
            }
            else if (dev_types[i] == DEV_VEML7700)
            {
                uint32_t lux; // single fixed point. i.e. 10 = 1.0 lux

                if (veml7700_read_decilux(sensor_ids[i], (uint32_t *)&lux))
                {
                    printf("VEML7700 sensor    @ Index %d:       Lux: %lu.%lu\r\n", i, (lux / 10), (lux % 10));
                }
                else
                {
                    printf("Error reading from VEML7700 sensor %d\r\n", i);
                }
            }
            else
            {
                printf("Unknown sensor     @ Index %d\r\n", i);
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
