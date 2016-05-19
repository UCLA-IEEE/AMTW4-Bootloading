/*
 * IEEE @ UCLA: Advanced Microcontroller Topics Bootloaders
 * Copyright (C) 2016  Kevin Balke
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRIVER_SERIAL_H_
#define DRIVER_SERIAL_H_

// Prevent the C++ compiler from declaring these symbols.
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    Serial_module_0,
    Serial_module_1,
    Serial_module_2,
    Serial_module_3,
    Serial_module_4,
    Serial_module_5,
    Serial_module_6,
    Serial_module_7,
} Serial_module_e;

#define Serial_module_debug (Serial_module_0)

void Serial_init(Serial_module_e module, uint32_t baud);
void Serial_putc(Serial_module_e module, char c);
int Serial_getc(Serial_module_e module);
void Serial_puts(Serial_module_e module, const char * s);
void Serial_writebuf(Serial_module_e module, const uint8_t* buf, uint32_t len);
void Serial_flush(Serial_module_e module);
bool Serial_avail(Serial_module_e module);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_SERIAL_H_ */
