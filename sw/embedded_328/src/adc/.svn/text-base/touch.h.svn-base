/**
 * \file touch.h
 * \brief This file contains the interface of the touch screen module.
 *
 * Copyright (C) 2011  Armin Schlegel, Christian Eismann
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

#ifndef _TOUCH_H_
#define _TOUCH_H_

/*--- Includes -------------------------------------------------------*/

#include <avr/io.h>
#define F_CPU 7372800UL
#include <util/delay.h>

/*--- Macros ---------------------------------------------------------*/

#define invalid   0
#define valid   1
#define TOUCH_Y1 PA0   /* bottom */
#define TOUCH_X1 PA2   /* left   */
#define TOUCH_Y2 PA4   /* top    */
#define TOUCH_X2 PA6   /* right  */

/*--- External Function Prototypes -----------------------------------*/

extern uint8_t touch_read(uint8_t *x, uint8_t *y);
extern void touch_init(void);

#endif /* #ifndef _TOUCH_H_ */

