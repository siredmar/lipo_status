/**
 * \file touch.c
 * \brief This file contains the implementation of the touch screen module
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

/*--- Includes -------------------------------------------------------*/

#include "touch.h"

/*--- External Function Definitions ----------------------------------*/

/**
 * \brief Reads x and y position on touch interface.
 *
 * \param x out parameter of read x value
 * \param y out parameter of read y value
 * \returns <ul>
 *            <li>1: data of touch_read() is valid</li>
 *            <li>2: data of touch_read() is invalid</li>
 *          </ul>
 */
uint8_t touch_read(uint8_t *x, uint8_t *y)
{
   uint8_t data = valid;
   uint16_t Uy, Ux;
   ADCSRA |= (1 << ADEN);                  /* enable ADC */
   _delay_us(10);

   ADCSRA |= (1<<ADSC);                    /* eine 'dummy' ADC-Wandlung */
   while (ADCSRA & (1<<ADSC)){;}           /* auf Abschluss der Konvertierung warten */
   ADCW;

   DDRA |= ((1<<TOUCH_X1)|(1<<TOUCH_X2));  /* left (PA2) und right (PA6) Ausgang */
   PORTA |= (1<<TOUCH_X1);                 /* left (PA2) +3,3V */
   PORTA &= ~(1<<TOUCH_X2);                /* right (PA6) 0V */
   PORTA |= (1<<TOUCH_Y1)|(1<<TOUCH_Y2);
   _delay_ms(10);
   ADMUX = ((1<<REFS0) + 4);               /*(PA4 Top) auswaehlen */
   ADCSRA |= (1<<ADSC);

   while (ADCSRA & (1<<ADSC));
   Ux = ADCW;

   PORTA &= ~(1<<TOUCH_X1);                /* Left (PA2) 0V */
   DDRA &= ~((1<<TOUCH_X1)|(1<<TOUCH_X2)); /* Left (PA2) und Right (PA3) Eingang */

   if(Ux > 900){                           /* Touch meldet keine benutzung */
      data = invalid;                         /* merken fuer Rueckgabewert */
   }

   Ux = (Ux - 170) * 100;                  /* ADC Werte in Display- */
   Ux = (Ux / 660) * 128 / 100;            /* koordinaten umrechen */
   *x = 128 - Ux;

   DDRA |= ((1<<TOUCH_Y1)|(1<<TOUCH_Y2));  /* top (PA4) und bottom (PA0) Ausgang */
   PORTA |= (1<<TOUCH_Y1);                 /* top (PA4) +3,3V */
   PORTA &= ~(1<<TOUCH_Y2);                /* bottom (PA0) 0V */
   _delay_ms(10);
   PORTA |= (1<<TOUCH_X1)|(1<<TOUCH_X2);
   ADMUX = (((1<<REFS0) + 2) );            /* (PA6 right) auswaehlen */
   ADCSRA |= (1<<ADSC);

   while (ADCSRA & (1<<ADSC));
   Uy = ADCW;

   PORTA &= ~(1<<TOUCH_Y2);                /* top (PA4) 0V */
   DDRA &= ~((1<<TOUCH_Y2)|(1<<TOUCH_Y1)); /* Top (PA4) und Button (PA0) Eingang */

   if(Uy > 900){                           /* Touch: no usage */
      data = invalid;
   }

   Uy = Uy-32;
   *y = Uy*(64/16);


   ADCSRA &= ~(1 << ADEN);             /*disable ADC */

   return data;                        /*return valid or invalid */
}

/**
 * \brief Initializes ADC for touch interface read out.
 */
void touch_init(void)
{
  ADMUX = ((1<<REFS0)|(1<<REFS1));             /* AVCC with external capacitor,
                                                * Channel ADC0 */
  ADCSRA = ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); /* ADC ON, Prescaler :128 */
  ADCSRA &= ~(1 << ADEN);                      /* disable ADC */

  ADCSRA |= (1<<ADSC);                         /* 'dummy' read out */
    while (ADCSRA & (1<<ADSC));                /* wait until ready */
  ADCW;                                        /* fetch data */
  ADCSRA &= ~(1 << ADEN);                      /* disable ADC */

  DDRA &= ~((1<<PA0)|(1<<PA1)|(1<<PA2)|(1<<PA3)|(1<<PA4)|(1<<PA5)|(1<<PA6)|(1<<PA7));
}

